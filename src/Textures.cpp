
#include <fstream>
#include <vector>
//#include <SDL/SDL.h>
//#include <SDL/SDL_image.h>
//#include <SDL.h>
#include <SDL_image.h>
#include <webAsmPlay/Debug.h>
#include <webAsmPlay/Textures.h>

using namespace std;

namespace
{
    Textures * instance = NULL;

    int invertImage(int width, int height, void *image_pixels)
    {
        auto temp_row = std::unique_ptr<char>(new char[width]);
        if (temp_row.get() == nullptr) {
            SDL_SetError("Not enough memory for image inversion");
            return -1;
        }
        //if height is odd, don't need to swap middle row
        int height_div_2 = height / 2;
        for (int index = 0; index < height_div_2; index++)
        {
            //uses string.h
            memcpy((Uint8 *)temp_row.get(),
                (Uint8 *)(image_pixels)+
                width * index,
                width);

            memcpy(
                    (Uint8 *)(image_pixels)+
                    width * index,
                    (Uint8 *)(image_pixels)+
                    width * (height - index - 1),
                    width);
            memcpy(
                    (Uint8 *)(image_pixels)+
                    width * (height - index - 1),
                    temp_row.get(),
                    width);
        }
        return 0;
    }

    void invertImage(SDL_Surface * formattedSurf)
    {
        invertImage(formattedSurf->w*formattedSurf->format->BytesPerPixel, formattedSurf->h, (char *) formattedSurf->pixels);
    }
} 

Textures * Textures::getInstance()
{
    if(instance) { return instance ;}

    return instance = new Textures();
}

Textures::Textures()
{

}

Textures::~Textures()
{

}

GLuint Textures::load(const string & filename)
{
    SDL_Surface * img = IMG_Load(filename.c_str());

    GLuint texture;

    glGenTextures( 1, &texture );

    /* Typical Texture Generation Using Data From The Bitmap */
    glBindTexture( GL_TEXTURE_2D, texture );

    /* Generate The Texture */
    glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA, img->w,
                    img->h, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, img->pixels );

    /* Linear Filtering */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    SDL_FreeSurface(img);

    return texture;
}

GLuint Textures::loadCube(const vector<string> & files)
{
    if(files.size() != 6)
    {
        dmess("Error, cube texture incorrect number of files!")
    }

    SDL_Surface * xpos = IMG_Load(files[0].c_str());
    SDL_Surface * xneg = IMG_Load(files[1].c_str());
    SDL_Surface * ypos = IMG_Load(files[2].c_str());
    SDL_Surface * yneg = IMG_Load(files[3].c_str());
    SDL_Surface * zpos = IMG_Load(files[4].c_str());
    SDL_Surface * zneg = IMG_Load(files[5].c_str());

    if(!xpos) { dmess("Error cannot load: " << files[0]); return 0 ;}
    if(!xneg) { dmess("Error cannot load: " << files[1]); return 0 ;}
    if(!ypos) { dmess("Error cannot load: " << files[2]); return 0 ;}
    if(!yneg) { dmess("Error cannot load: " << files[3]); return 0 ;}
    if(!zpos) { dmess("Error cannot load: " << files[4]); return 0 ;}
    if(!zneg) { dmess("Error cannot load: " << files[5]); return 0 ;}

    /*
    invertImage(xpos);
    invertImage(xneg);
    invertImage(ypos);
    invertImage(yneg);
    invertImage(zpos);
    invertImage(zneg);
    */

    GLuint texCube;
    
    glGenTextures(1, &texCube);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texCube);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, xpos->w, xpos->h, 0, GL_RGB, GL_UNSIGNED_BYTE, xpos->pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, xneg->w, xneg->h, 0, GL_RGB, GL_UNSIGNED_BYTE, xneg->pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, ypos->w, ypos->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ypos->pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, yneg->w, yneg->h, 0, GL_RGB, GL_UNSIGNED_BYTE, yneg->pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, zpos->w, zpos->h, 0, GL_RGB, GL_UNSIGNED_BYTE, zpos->pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, zneg->w, zneg->h, 0, GL_RGB, GL_UNSIGNED_BYTE, zneg->pixels);
    
    SDL_FreeSurface(xpos);
    SDL_FreeSurface(xneg);
    SDL_FreeSurface(ypos);
    SDL_FreeSurface(yneg);
    SDL_FreeSurface(zpos);
    SDL_FreeSurface(zneg);

    return texCube;
}