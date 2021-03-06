#include <cmath>
//#include <imgui.h>

#ifdef __EMSCRIPTEN__
    // GLEW
    #define GLEW_STATIC
    #include <GL/glew.h>
    #define IMGUI_API
    #include <imgui.h>
    #include <imgui_impl_glfw_gl3.h>
    #include <emscripten/emscripten.h>
    #include <emscripten/bind.h>
#else
    #include <GL/gl3w.h>    // Initialize with gl3wInit()
    #define IMGUI_IMPL_API
    #include <imgui/imgui.h>
    #include <imgui_impl_opengl3.h>
    #include <imgui_impl_glfw.h>
    
#endif // __EMSCRIPTEN__

#include <GLFW/glfw3.h>
#include <imgui_internal.h>
#include <iostream>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include "../GLUTesselator/include/GLU/tessellate.h"
#include <webAsmPlay/TrackBallInteractor.h>
#include <webAsmPlay/Camera.h>
#include <webAsmPlay/Debug.h>
#include <webAsmPlay/Renderable.h>
#include <webAsmPlay/Attributes.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imguitoolbar.h>
#include <tceGeom/vec2.h>
#include <webAsmPlay/Util.h>
#include <webAsmPlay/Debug.h>
#include <webAsmPlay/RenderableCollection.h>
#include <webAsmPlay/RenderablePolygon.h>
#include <webAsmPlay/FrameBuffer.h>
#include <webAsmPlay/Canvas.h>
#include <webAsmPlay/SkyBox.h>
#include <webAsmPlay/Textures.h>
#include <webAsmPlay/GridPlane.h>
#include <webAsmPlay/GeoClient.h>
#include <webAsmPlay/GeosTestCanvas.h>
#include <webAsmPlay/GeosUtil.h>

using namespace std;
using namespace geos::geom;
using namespace geos::simplify;
using namespace rsmz;
using namespace glm;
using namespace tce::geom;

#define ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

static ImVec4 clear_color = ImColor(114, 144, 154);
static int mouse_buttons_down = 0;

static bool mouse_buttons[GLFW_MOUSE_BUTTON_LAST + 1] = { false, };

GeosTestCanvas  * geosTestCanvas = NULL;
Canvas          * canvas         = NULL;
SkyBox          * skyBox         = NULL;

bool showViewMatrixPanel     = false;
bool showMVP_MatrixPanel     = false;
bool showSceneViewPanel      = false;
bool showPerformancePanel    = false;
bool showRenderSettingsPanel = false;
bool showLogPanel            = false;
bool showAttributePanel      = false;

bool isFirst = true;

FrameBuffer * frameBuffer = NULL;

void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void refresh(GLFWwindow* window)
{
#ifdef __EMSCRIPTEN__
    glfwPollEvents();

    glfwMarkWindowForRefresh(window);
#endif
}

int counter = 0;

void cback(char* data, int size, void* arg) {
    std::cout << "Callback " << data << " " << size << std::endl;
    counter++;
}

ImGuiTextBuffer * Buf = NULL;

struct AppLog
{
    //ImGuiTextBuffer     Buf;
    bool                ScrollToBottom;

    void    Clear()     { Buf->clear(); }

    void    AddLog(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        //Buf.appendv(fmt, args);
        //Buf.appendf("%s", "fdasdfasd");
        va_end(args);
        ScrollToBottom = true;
    }

    void    Draw(const char* title, bool* p_opened = NULL)
    {
        ImGui::SetNextWindowSize(ImVec2(500,400), ImGuiSetCond_FirstUseEver);
        ImGui::Begin(title, p_opened);
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::Separator();
        ImGui::BeginChild("scrolling");
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,1));
        if (copy) ImGui::LogToClipboard();
        ImGui::TextUnformatted(Buf->begin());
        if (ScrollToBottom)
            ImGui::SetScrollHere(1.0f);
        ScrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }
};

AppLog logPanel;

void dmessCallback(const string & file, const size_t line, const string & message)
{
    cout << file << " " << line << " " << message;
    /*
#ifdef __EMSCRIPTEN__
    if(Buf) { Buf->append("%s %i %s", file.c_str(), (int)line, message.c_str()) ;}
#else
    if(Buf) { Buf->appendf("%s %i %s", file.c_str(), (int)line, message.c_str()) ;}
#endif
     */
}

//vector<Renderable *> pickedRenderiables;

//extern void (*debugLoggerFunc)(const std::string & file, const std::string & line, const std::string & message);

GeoClient * client = NULL;

void mainLoop(GLFWwindow * window)
{
    if(!Buf) {  Buf = new ImGuiTextBuffer() ;}
    // Game loop
    
#ifdef __EMSCRIPTEN__
    // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
    glfwPollEvents();

    ImGui_ImplGlfwGL3_NewFrame();
#endif


    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) {}
                if (ImGui::MenuItem("Open", "Ctrl+O")) {

                }
                if(ImGui::MenuItem("Test Web Worker"))
                {
                    #ifdef __EMSCRIPTEN__
                        worker_handle worker = emscripten_create_worker("worker.js");
                        emscripten_call_worker(worker, "one", 0, 0, cback, (void*)42);
                    #else
                        dmess("Implement me!");
                    #endif
                }

                if(ImGui::MenuItem("Load Geometry"))
                {
                    //GeoClient::getInstance()->loadGeometry(canvas);

                    if(!client) { client = new GeoClient(window) ;}

                    client->loadGeometry(canvas);
                }

                if(ImGui::MenuItem("Load All Geometry"))
                {
                    //GeoClient::getInstance()->loadGeometry(canvas);

                    if(!client) { client = new GeoClient(window) ;}

                    client->loadAllGeometry(canvas);
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("View"))
            {
                if(ImGui::MenuItem("View Matrix"))     { showViewMatrixPanel     = !showViewMatrixPanel     ;}
                if(ImGui::MenuItem("MVP Matrix"))      { showMVP_MatrixPanel     = !showMVP_MatrixPanel     ;}
                if(ImGui::MenuItem("Geos Tests"))      { showSceneViewPanel      = !showSceneViewPanel      ;}
                if(ImGui::MenuItem("Performance"))     { showPerformancePanel    = !showPerformancePanel    ;}
                if(ImGui::MenuItem("Render Settings")) { showRenderSettingsPanel = !showRenderSettingsPanel ;}
                if(ImGui::MenuItem("Log"))             { showLogPanel            = !showLogPanel            ;}
                if(ImGui::MenuItem("Attributes"))      { showAttributePanel      = !showAttributePanel      ;}

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    {
        static uint32_t myImageTextureId2 = 0;
        if(!myImageTextureId2) { myImageTextureId2 = Textures::load("if_Info_131908.png") ;}
        //dmess("myImageTextureId2 " << myImageTextureId2);
             static ImGui::Toolbar toolbar("myFirstToolbar##foo");
             if (toolbar.getNumButtons()==0)  {
                 char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
                 for (int i=0;i<9;i++) {
                     strcpy(tmp,"toolbutton ");
                     sprintf(&tmp[strlen(tmp)],"%d",i+1);
                     //uv0 = ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                     //uv1 = ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);
                     uv0 = ImVec2(0,0);
                     uv1 = ImVec2(1,1);
 
                     toolbar.addButton(ImGui::Toolbutton(tmp,(void*)myImageTextureId2,uv0,uv1));
                 }
                 //toolbar.addSeparator(16);
                 //toolbar.addButton(ImGui::Toolbutton("toolbutton 11",(void*)myImageTextureId2,uv0,uv1,ImVec2(32,32),true,false,ImVec4(0.8,0.8,1.0,1)));  // Note that separator "eats" one toolbutton index as if it was a real button
                 //toolbar.addButton(ImGui::Toolbutton("toolbutton 12",(void*)myImageTextureId2,uv0,uv1,ImVec2(48,24),true,false,ImVec4(1.0,0.8,0.8,1)));  // Note that separator "eats" one toolbutton index as if it was a real button
 
                 toolbar.setProperties(false,false,true,ImVec2(0.5f,0.f));

                 //toolbar.setProperties(true,true,false,ImVec2(0.0f,0.0f),ImVec2(0.25f,0.9f),ImVec4(0.85,0.85,1,1));
 
                 //toolbar.setScaling(2.0f,1.1f);
             }
             const int pressed = toolbar.render();
             if (pressed>=0) fprintf(stderr,"Toolbar1: pressed:%d\n",pressed);
         }

    // Rendering
    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    
    static float time = 0.f;
    
    time += ImGui::GetIO().DeltaTime;

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    if(showPerformancePanel)
    {
        ImGui::Begin("Performance", &showPerformancePanel);

            static float f = 0.0f;
            static float frameTimes[100] = {0.f};
            memcpy(&frameTimes[0], &frameTimes[1], sizeof(frameTimes) - sizeof(frameTimes[0]));
            frameTimes[ARRAYSIZE(frameTimes) - 1] = ImGui::GetIO().Framerate;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::PlotLines("Frame History", frameTimes, ARRAYSIZE(frameTimes), 0, "", 0.0f, 100.0f, ImVec2(0, 50));

        ImGui::End();
    }

    if(showViewMatrixPanel)
    {
        ImGui::Begin("View Matrix", &showViewMatrixPanel);

            ImGui::Text(mat4ToStr(canvas->getViewRef()).c_str());

        ImGui::End();
    }
    
    if(isFirst) { ImGui::SetNextWindowPos(ImVec2(0,0)) ;}

    if(showMVP_MatrixPanel)
    {
        ImGui::Begin("MVP Matrix", &showMVP_MatrixPanel);

            ImGui::Text(mat4ToStr(canvas->getMVP_Ref()).c_str());

        ImGui::End();
    }

    isFirst = false;

    if(showRenderSettingsPanel)
    {
        ImGui::Begin("Render Settings", &showRenderSettingsPanel);

            static bool fillPolygons            = true;
            static bool renderPolygonOutlines   = true;
            static bool renderSkyBox            = true;

            static bool _fillPolygons           = true;
            static bool _renderPolygonOutlines  = true;
            static bool _renderSkyBox           = true;

            ImGui::Checkbox("Fill Polygons",    &_fillPolygons);
            ImGui::Checkbox("Polygon Outlines", &_renderPolygonOutlines);
            ImGui::Checkbox("SkyBox",           &_renderSkyBox);

            if(fillPolygons != _fillPolygons)
            {
                fillPolygons = _fillPolygons;

                for(Renderable * r : canvas->getRenderiablesRef()) { r->setRenderFill(fillPolygons) ;}

                for(Renderable * r : geosTestCanvas->getRenderiablesRef()) { r->setRenderFill(fillPolygons) ;}

                Renderable::setDefaultRenderFill(fillPolygons);
            }

            if(renderPolygonOutlines != _renderPolygonOutlines)
            {
                renderPolygonOutlines = _renderPolygonOutlines;

                for(Renderable * r : canvas->getRenderiablesRef()) { r->setRenderOutline(renderPolygonOutlines) ;}

                for(Renderable * r : geosTestCanvas->getRenderiablesRef()) { r->setRenderOutline(renderPolygonOutlines) ;}

                Renderable::setDefaultRenderOutline(renderPolygonOutlines);
            }

            if(renderSkyBox != _renderSkyBox)
            {
                renderSkyBox = _renderSkyBox;

                if(renderSkyBox) { canvas->setSkyBox(skyBox) ;}
                else             { canvas->setSkyBox(NULL)   ;}
            }

            static vec4 fillColor(Renderable::getDefaultFillColor());
            static vec4 outlineColor(Renderable::getDefaultOutlineColor());
            
            ImGui::ColorEdit4("Fill", (float*)&fillColor, true);
            ImGui::ColorEdit4("Outline", (float*)&outlineColor, true);

            /*
            static vec4 fillColor(Renderable::getDefaultFillColor());

            ImGui::ColorPicker4("##picker",
                                (float*)&fillColor,
                                //ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
                                0);
                                */

        ImGui::End();
    }
    
    if(showLogPanel) { logPanel.Draw("Log", &showLogPanel) ;}

    canvas->render();
    
    string attrsStr;

    if(client)
    {
        vector<pair<Renderable *, Attributes *> > picked = client->pickRenderables(canvas->getCursorPosWC());

        for(pair<Renderable *, Attributes *> & r : picked)
        {
            get<0>(r)->render(canvas->getMVP_Ref());

            attrsStr += r.second->toString();
        }
    }

    if(showAttributePanel)
    {
        ImGui::Begin("Attributes", &showAttributePanel);

            ImGui::Text(attrsStr.c_str());

        ImGui::End();
    }

    geosTestCanvas->setEnabled(showSceneViewPanel);

    if(showSceneViewPanel)
    {
        ImGui::Begin("Geos Tests", &showSceneViewPanel);

            const ImVec2 pos = ImGui::GetCursorScreenPos();

            const ImVec2 sceneWindowSize = ImGui::GetWindowSize();

            geosTestCanvas->setArea(__(pos), __(sceneWindowSize));

            geosTestCanvas->setWantMouseCapture(GImGui->IO.WantCaptureMouse);

            ImGui::GetWindowDrawList()->AddImage(   (void *)geosTestCanvas->render(),
                                                    pos,
                                                    ImVec2(pos.x + sceneWindowSize.x, pos.y + sceneWindowSize.y),
                                                    ImVec2(0, 1),
                                                    ImVec2(1, 0));
            
            static float buffer1 = 0.1;
            static float buffer2 = 0.02;
            static float buffer3 = 0.22;

            ImGui::SliderFloat("buffer1", &buffer1, 0.0f, 0.3f, "buffer1 = %.3f");
            ImGui::SliderFloat("buffer2", &buffer2, 0.0f, 0.3f, "buffer2 = %.3f");
            ImGui::SliderFloat("buffer3", &buffer3, 0.0f, 0.3f, "buffer3 = %.3f");

            geosTestCanvas->setGeomParameters(buffer1, buffer2, buffer3);

        ImGui::End();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ImGui::Render();

#ifndef __EMSCRIPTEN__

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#endif
    glfwMakeContextCurrent(window);

    glfwSwapBuffers(window);
}

void mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
    {
        return;
    }

    geosTestCanvas->onMouseButton(window, button, action, mods);

    if(!GImGui->IO.WantCaptureMouse)
    {
        canvas->onMouseButton(window, button, action, mods);
    }

    if (mouse_buttons[button] != action) {
      mouse_buttons[button] = action;
      mouse_buttons_down += action == GLFW_PRESS ? 1 : -1;
    }

#ifndef __EMSCRIPTEN__
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
#else
    ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
#endif
    
    refresh(window);
}

Vec2i lastShiftKeyDownMousePos;

void cursorPosCallback(GLFWwindow * window, double xpos, double ypos)
{
    //dmess("x " << xpos << " y " << ypos);

    geosTestCanvas->onMousePosition(window, Vec2d(xpos, ypos));

    canvas->onMousePosition(window, Vec2d(xpos, ypos));

    refresh(window);
}

void scrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
    //dmess("ScrollCallback " << xoffset << " " << yoffset);

    geosTestCanvas->onMouseScroll(window, Vec2d(xoffset, yoffset));

    if(!GImGui->IO.WantCaptureMouse)
    {
        canvas->onMouseScroll(window, Vec2d(xoffset, yoffset));
    }

#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfwGL3_ScrollCallback(window, xoffset, yoffset);
#else
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
#endif
    refresh(window);
}

void keyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
    geosTestCanvas->onKey(window, key, scancode, action, mods);

    if(!GImGui->IO.WantCaptureKeyboard) { canvas->onKey(window, key, scancode, action, mods) ;}
 
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
#else
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
#endif

    refresh(window);
}

void charCallback(GLFWwindow * window, unsigned int c)
{
    geosTestCanvas->onChar(window, c);

    canvas->onChar(window, c);

#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfwGL3_CharCallback(window, c);
#else
    ImGui_ImplGlfw_CharCallback(window, c);
#endif
    refresh(window);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Need to use this to get true size because of retina displays.
    glfwGetWindowSize(window, &width, &height);

    canvas->setArea(Vec2i(0,0), Vec2i(width, height));

    refresh(window);
}

void windowFocusCallback(GLFWwindow* window, int focused)
{
    if(focused)
    {
        refresh(window);
    }
}

void cursorEnterCallback(GLFWwindow * window, int /* entered */)
{
    refresh(window);
}

// Is called whenever a key is pressed/released via GLFW
void keyCallback1(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) { glfwSetWindowShouldClose(window, GL_TRUE) ;}
}

void initOpenGL(GLFWwindow* window)
{
    debugLoggerFunc = &dmessCallback;

    // Define the viewport dimensions
    static int width, height;
    //glfwGetFramebufferSize(window, &width, &height); 
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Renderable::ensureShader();
    //GridPlane  ::ensureShader();
    RenderablePolygon::ensureShaders();

    canvas = new Canvas(false);

    dmess("width " << width << " height " << height);

    //canvas->setArea(Vec2i(0,0), Vec2i(width / 2, height / 2));
    canvas->setArea(Vec2i(0,0), Vec2i(width, height));

    geosTestCanvas = new GeosTestCanvas();

    skyBox = new SkyBox();

    canvas->setSkyBox(skyBox);

    //GridPlane * gridPlane = new GridPlane();

    //canvas->addRenderiable(gridPlane);
}

void initGeometry()
{
    #ifdef WORKING

    Geometry * pp = scopedGeosGeometry(GeosUtil::makeBox(-0.1,-0.1,0.1,0.1));

    Geometry * p = scopedGeosGeometry(GeosUtil::makeBox(-0.5,-0.5,0.5,0.5));

    Geometry * ppp = scopedGeosGeometry(GeosUtil::makeBox(-0.05,-0.6,0.05,0.6));

    Geometry * pppp = scopedGeosGeometry(GeosUtil::makeBox(-0.6,-0.05,0.6,0.05));

    p = scopedGeosGeometry(p->buffer(0.1));
    //*

    p = scopedGeosGeometry(p->difference(pp));

    p = scopedGeosGeometry(p->difference(ppp));

    p = scopedGeosGeometry(p->difference(pppp));
    //*/

    const mat4 trans = scale(mat4(1.0), vec3(0.6, 0.6, 0.6));

    Renderable * r = Renderable::create(p, trans);
    //Renderable * r = Renderable::create(dynamic_cast<Polygon *>(p)->getExteriorRing());

    r->setFillColor(vec4(0.3,0.3,0,1));
        
    r->setOutlineColor(vec4(1,0,0,1));

    //geosTestCanvas->addRenderiable(r);
    canvas->addRenderiable(r);

    /*
    p = scopedGeosGeometry(GeosUtil::makeBox(-0.5,-0.5,-4,0.4));

    //r = Renderable::create(p, trans);
    r = Renderable::create(p);

    r->setFillColor(vec4(0.3,0.0,0.3,1));
        
    r->setOutlineColor(vec4(1,0,0,1));

    canvas->addRenderiable(r);
    //*/

    #endif
}
