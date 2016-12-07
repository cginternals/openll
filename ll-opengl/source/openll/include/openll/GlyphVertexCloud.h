#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <globjects/base/ref_ptr.h>
#include <globjects/Texture.h>

#include <openll/Drawable.h>

#include <openll/openll_api.h>


namespace gloperate_text
{

class FontFace;
class GlyphSequence;


class OPENLL_API GlyphVertexCloud
{
public:
    struct Vertex
    {
        glm::vec3 origin;
        glm::vec3 vtan;
        glm::vec3 vbitan;
        // vec2 lowerLeft and vec2 upperRight in glyph texture (uv)
        glm::vec4 uvRect;
        glm::vec4 fontColor;
    };

    using Vertices = std::vector<Vertex>;

public:
    GlyphVertexCloud();
    virtual ~GlyphVertexCloud();

    const globjects::Texture * texture() const;
    void setTexture(globjects::Texture * texture);

    gloperate_text::Drawable * drawable();
    const gloperate_text::Drawable * drawable() const;

    Vertices & vertices();
    const Vertices & vertices() const;

    void update();
    // allows for volatile optimizations
    void update(const Vertices & vertices);

    void optimize(const std::vector<GlyphSequence> & sequences);

protected:
    static gloperate_text::Drawable * createDrawable();

protected:
    Vertices m_vertices;

    globjects::ref_ptr<gloperate_text::Drawable> m_drawable;
    globjects::ref_ptr<globjects::Texture> m_texture;
};


} // namespace gloperate_text
