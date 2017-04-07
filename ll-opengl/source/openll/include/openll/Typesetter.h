
#pragma once

#include <glm/fwd.hpp>

#include <openll/GlyphVertexCloud.h>
#include <openll/SuperSampling.h>

#include <openll/openll_api.h>


namespace gloperate_text
{
enum class Alignment : unsigned char;

class GlyphSequence;
class FontFace;
class Glyph;


class OPENLL_API Typesetter
{
public:
    //Typesetter() = delete;
    //virtual ~Typesetter() = delete;

    static const char32_t & lineFeed();

    static glm::vec2 extent(const GlyphSequence & sequence);
    static std::pair<glm::vec2, glm::vec2> rectangle(
        const GlyphSequence & sequence, glm::vec3 origin);

    static glm::vec2 typeset(
        const GlyphSequence & sequence
    ,   const GlyphVertexCloud::Vertices::iterator & begin
    ,   bool dryrun = false);

private:

    static bool typeset_wordwrap(
        const GlyphSequence & sequence
    ,   const glm::vec2 & pen
    ,   const Glyph & glyph
    ,   const std::u32string::const_iterator & index
    ,   std::u32string::const_iterator & safe_forward);

    static std::u32string::const_iterator typeset_forward(
        const GlyphSequence & sequence
    ,   const std::u32string::const_iterator & begin
    ,   float & width);

    static void typeset_glyph(
        const FontFace & fontFace
    ,   const glm::vec2 & pen
    ,   const Glyph & glyph
    ,   const GlyphVertexCloud::Vertices::iterator & vertex);

    static void typeset_extent(
        const FontFace & fontFace
    ,   std::u32string::const_iterator index
    ,   const std::u32string::const_iterator & begin
    ,   glm::vec2 & pen
    ,   glm::vec2 & extent);

    static void typeset_align(
        const glm::vec2 & pen
    ,   const Alignment alignment
    ,   const GlyphVertexCloud::Vertices::iterator & begin
    ,   const GlyphVertexCloud::Vertices::iterator & end);

    static void anchor_transform(
        const GlyphSequence & sequence
    ,   const GlyphVertexCloud::Vertices::iterator & begin
    ,   const GlyphVertexCloud::Vertices::iterator & end);

    static void vertex_transform(
        const glm::mat4 & sequence
    ,   const glm::vec4 & fontColor
    ,   const SuperSampling & superSampling
    ,   const GlyphVertexCloud::Vertices::iterator & begin
    ,   const GlyphVertexCloud::Vertices::iterator & end);

    static glm::vec2 extent_transform(
        const GlyphSequence & sequence
    ,   const glm::vec2 & extent);
};


} // namespace gloperate_text
