#ifndef __WEB_ASM_PLAY_RENDERIABLE_POLYGON2D_H__
#define __WEB_ASM_PLAY_RENDERIABLE_POLYGON2D_H__

#include <vector>
#include <webAsmPlay/Renderable.h>

namespace geos
{
    namespace geom
    {
        class Polygon;
        class MultiPolygon;
    }
}

class RenderablePolygon : public Renderable
{
public:

    ~RenderablePolygon();

    static Renderable * create( const geos::geom::Polygon   * poly,
                                const glm::mat4             & trans             = glm::mat4(1.0),
                                const glm::vec4             & fillColor         = getDefaultFillColor(),
                                const glm::vec4             & outlineColor      = getDefaultOutlineColor(),
                                const bool                    renderOutline     = getDefaultRenderOutline(),
                                const bool                    renderFill        = getDefaultRenderFill());

    static Renderable * create( const geos::geom::MultiPolygon  * multyPoly,
                                const glm::mat4                 & trans         = glm::mat4(1.0),
                                const glm::vec4                 & fillColor     = getDefaultFillColor(),
                                const glm::vec4                 & outlineColor  = getDefaultOutlineColor(),
                                const bool                        renderOutline = getDefaultRenderOutline(),
                                const bool                        renderFill    = getDefaultRenderFill());

    static Renderable * create( const std::vector<const geos::geom::Geometry *> & polygons,
                                const glm::mat4                                 & trans         = glm::mat4(1.0),
                                const glm::vec4                                 & fillColor     = getDefaultFillColor(),
                                const glm::vec4                                 & outlineColor  = getDefaultOutlineColor(),
                                const bool                                        renderOutline = getDefaultRenderOutline(),
                                const bool                                        renderFill    = getDefaultRenderFill());

    void render(const glm::mat4 & MVP) const;

    static void ensureShaders();

private:

    struct TesselationResult
    {
        double  * vertsOut          = NULL;
        int     * triangleIndices   = NULL;
        int       numVerts          = 0;
        int       numTriangles      = 0;

        std::vector<GLuint> counterVertIndices;
        std::vector<GLuint> counterVertIndices2;
    };

    RenderablePolygon(  const GLuint                vao,
                        const GLuint                ebo,
                        const GLuint                ebo2,
                        const GLuint                vbo,
                        const int                   numTriangles,
                        const std::vector<GLuint> & counterVertIndices,
                        const size_t                numContourLines,
                        const bool                  isMulti,
                        const glm::vec4           & fillColor,
                        const glm::vec4           & outlineColor,
                        const bool                  renderOutline,
                        const bool                  renderFill);

    static Renderable * createFromTesselations( const std::vector<const TesselationResult>  & tesselations,
                                                const glm::vec4                             & fillColor,
                                                const glm::vec4                             & outlineColor,
                                                const bool                                    renderOutline,
                                                const bool                                    renderFill);

    static TesselationResult tessellatePolygon(const geos::geom::Polygon * poly, const glm::mat4 & trans);

    static void tesselateMultiPolygon(  const geos::geom::MultiPolygon       * multiPoly,
                                        const glm::mat4                      & trans,
                                        std::vector<const TesselationResult> & tesselationResults);

    static void ensureOutlineShader();

    const GLuint                vao;
    const GLuint                ebo;
    const GLuint                ebo2;
    const GLuint                vbo;
    const int                   numTriangles;
    const std::vector<GLuint>   counterVertIndices;
    const size_t                numContourLines;
}; 

#endif // __WEB_ASM_PLAY_RENDERIABLE_POLYGON2D_H__
