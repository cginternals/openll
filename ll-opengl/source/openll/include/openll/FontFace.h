
#pragma once

#include <map>
#include <vector>
#include <unordered_map>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <globjects/base/ref_ptr.h>
#include <globjects/base/Referenced.h>
#include <globjects/Texture.h>

#include <openll/openll_api.h>

#include <openll/Glyph.h>


namespace gloperate_text
{


/**
*  @brief
*   Font related data for glyph based text rendering.
*
*   The glyph-based font face is described by, e.g., font-size,
*   line spacing, a glyph catalogue, as well as kerning information.
*   The glyph catalogue is based on a set of glyphs referring to a
*   texture atlas (see Glyph). All measures are provided in float
*   eventhough most glyph-textures and associated font data is
*   encoded via integer values. FontFace explicitly relies on floating
*   values to reduce the need of casting as well as to simplify the
*   use for dpi aware text rendering. Most measures can be interpreted
*   as points (by means of the unit pt), again, easing the use for
*   arbitrary dpi.
*
*   The font face interface is designed to access most basic font
*   settings ascent, descent, and linegap (leading). Additional
*   font settings are provided via interface but are derived from
*   or mapped to the above mentioned three settings, e.g., font size
*   is the sum of descent and ascent. This is to provide as much
*   convenience measures for type setting/font rendering as possible.
*
*   Note: This class does not provide dpi awareness. This has to be
*   handled outside of this class, e.g., during layouting and rendering.
*/
class OPENLL_API FontFace : public globjects::Referenced
{
public:
    /**
    *  @brief
    *   Constructor
    *
    *   Constructs an unconfigured, empty font face specification. The
    *   appropriate setters should be used for configuring the font
    *   face. Alternatively, the FontImporter provides the import of a
    *   font from a font configuration file (e.g., provided by
    *   http://kvazars.com/littera/).
    */
    FontFace();

    /**
     * @brief
     *  Destructor
     *
     *  Releases all glyphs. The glyph texture remains if it is shared.
     */
    virtual ~FontFace();

    /**
    * @brief
    *   The size of the font in pt.
    *
    *   The font size is the measure from the tops of the tallest
    *   glyphs (ascenders) to the bottom of the lowest descenders in
    *   pt. It is derived via the sum of ascent and descent.
    *
    * @return
    *   The font size in pt (ascent + descent).
    */
    float size() const;

    /**
    * @brief
    *   The font's base in pt.
    *
    *   The base is the distance from the baseline to the top of
    *   the line in pt.
    *
    * @return
    *   The distance from the baseline to the top of the line in pt.
    */
    float base() const;

    /**
    * @brief
    *   Set the font's base in pt.
    *
    *   The base is the distance from the baseline to the top of
    *   the line in pt.
    *
    * @oaram[in] base
    *   The distance from the baseline to the top of the line in pt.
    */
    void setBase(float base);

    /**
    * @brief
    *   The font's ascent in pt.
    *
    *   The ascent is the distance from the baseline to the tops of
    *   the tallest glyphs (ascenders) in pt.
    *
    * @return
    *   The distance from the baseline to the topmost ascenders in pt.
    */
    float ascent() const;

    /**
    * @brief
    *   Set the font's ascent in pt.
    *
    *   The ascent is the distance from the baseline to the tops of
    *   the tallest glyphs (ascenders) in pt.
    *
    * @oaram[in] ascent
    *   The distance from the baseline to the topmost ascenders in pt.
    */
    void setAscent(float ascent);

    /**
    * @brief
    *   The font's descent in pt.
    *
    *   The descent is the distance from the baseline to the lowest
    *   descenders in pt.
    *
    * @return
    *   The distance from the baseline to the lowest descenders in pt.
    *   Note: this value is usually negative (if the fonts lowest
    *   descenders are below the baseline).
    */
    float descent() const;

    /**
    * @brief
    *   Set the font's descent in pt.
    *
    *   The descent is the distance from the baseline to the lowest
    *   descenders in pt.
    *
    * @param[in] descent
    *   The distance from the baseline to the lowest descenders in pt.
    *   Note: this value is usually negative (if the fonts lowest
    *   descenders are below the baseline).
    */
    void setDescent(float descent);

    /**
    * @brief
    *   The font's leading/linegap in pt.
    *
    *   The leading is the distance from the lowest descenders to the
    *   topmost ascenders of a subsequent text line in pt.
    *
    * @return
    *   The gap between two subsequent lines of text in pt.
    */
    float linegap() const;

    /**
    * @brief
    *   Set the font's leading/linegap in pt.
    *
    *   The leading is the distance from the lowest descenders to the
    *   topmost ascenders of a subsequent text line in pt.
    *
    * @param[in] linegap
    *   The gap between two subsequent lines of text in pt.
    */
    void setLinegap(float linegap);

    /**
    * @brief
    *   The relative baseline-to-baseline distance w.r.t. the font's size.
    *
    *   The linespace relative linespace is derived as follows:
    *       linespace = size / lineHeight;
    *   Note that descent is usually a negative value.
    *
    * @return
    *   The relative baseline-to-basline distance w.r.t. the font's size.
    */
    float linespace() const;

    /**
    * @brief
    *   Set the relative baseline-to-basline distance w.r.t. the font's size.
    *
    *   The linespace is mapped to linegap as follows:
    *       linegap = size * (linespace - 1)
    *
    * @param[in] linespace
    *   The relative baseline-to-basline distance w.r.t. the font's size.
    *   Note: values < 1.0 will result in a negative linegap.
    */
    void setLinespace(float linespace);

    /**
    * @brief
    *   The baseline-to-basline distance in pt.
    *
    *   The lineheight is derived as follows:
    *       lineheight = size + linegap
    *   or alternatively:
    *       lineheight = size * linespace
    *
    * @return
    *   The line height (baseline-to-basline distance) in pt.
    */
    float lineHeight() const;

    /**
    * @brief
    *   Set the baseline-to-baseline distance in pt.
    *
    * @param[in] lineHeight
    *   The line height (baseline-to-baseline distance) in pt.
    *   Note: negative values will result in negative linegap.
    */
    void setLineHeight(float lineHeight);

    /**
    * @brief
    *   The size/extent of the glyph texture in px.
    *
    *   This can only be set via setGlyphTexture.
    *
    * @return
    *   The size/extent of the glyph texture in px.
    */
    const glm::uvec2 & glyphTextureExtent() const;

    /**
    * @brief
    *   Sets the glyph texture atlas extent.
    *
    * @param[in] extent
    *   The texture extent in px
    */
    void setGlyphTextureExtent(const glm::uvec2 & extent);

    /**
    * @brief
    *   The padding applied to every glyph in px.
    *
    *   This can only be set via setGlyphTexture.
    *
    * @return
    *   The CSS style (top, right, bottom, and left) padding applied
    *   to every glyph within the texture in px.
    */
    const glm::vec4 & glyphTexturePadding() const;

    /**
    * @brief
    *   Sets/updates the padding used for the glyph texture atlas.
    *
    * @param[in] padding
    *   The CSS style (top, right, bottom, and left) padding applied
    *   to every glyph within the texture in px.
    */
    void setGlyphTexturePadding(const glm::vec4 & padding);

    /**
    * @brief
    *   The font face's associated glyph atlas.
    *
    *   All glyph data is associated to this texture atlas.
    *
    * @return
    *   The texture object containing the texture atlas.
    */
    globjects::Texture * glyphTexture() const;

    /**
    * @brief
    *   Sets/updates the glyph texture atlas used for all comprised glyphs.
    *
    * @param[in] texture
    *   The new texture atlas for all glyphs
    */
    void setGlyphTexture(globjects::Texture * texture);

    /**
    * @brief
    *   Check if a glyph of a specific index is available.
    *
    * @return
    *   True if a glyph for the provided index was added.
    */
    bool hasGlyph(GlyphIndex index) const;

    /**
    * @brief
    *   Direct access to an indexed glyph.
    *
    *   If the glyph does not exists, a glyph with the given index
    *   will be added to the font face's glyphs.
    *
    * @param[in] index
    *   Index of the glyph to access.
    *
    * @return
    *   Reference to the glyph with the matching index.
    */
    Glyph & glyph(GlyphIndex index);

    /**
    * @brief
    *   Read-only access to an indexed glyph.
    *
    *   If the glyph does not exists, a reference to an empty glyph
    *   is returned (and an assertion is thrown).
    *
    * @param[in] index
    *   Index of the glyph to access.
    *
    * @return
    *   Reference to the glyph with the matching index. Blank glyph non found.
    */
    const Glyph & glyph(GlyphIndex index) const;

    /**
    * @brief
    *   Add a glyph to the font face's set of glyphs.
    *
    *   If the glyph already exists (assertion), the existing glyph remains.
    *
    * @param[in] glyph
    *   The glyph to add to the set of glyphs.
    */
    void addGlyph(const Glyph & glyph);

    /**
    * @brief
    *   Generates a vector of all comprised glyph indices.
    *
    * @return
    *   A vector of all glyph indices available to this font face.
    */
    std::vector<GlyphIndex> glyphs() const;

    /**
    * @brief
    *   Check if a glyph is depictable/renderable
    *
    *   If the glyph's subtexture vertical or horizontal extent is
    *   zero the glyph does not need to be depicted/rendered. E.g.,
    *   spaces, line feeds, other control sequences as well as
    *   unknown glyphs do not need to be processed for rendering.
    *
    * @param[in] index
    *   Index of the glyph to access.
    *
    * @return
    *   Returns true if the glyph needs to be depicted/rendered.
    */
    bool depictable(GlyphIndex index) const;

    /**
    * @brief
    *   Kerning for a glyph and a subsequent glyph in pt.
    *
    *   If the glyph or the subsequent glyph are unknown to this font
    *   face (assertion), 0.f will be returned. For more details on
    *   kerning, refer to the Glyph class.
    *
    * @param[in] index
    *   The current glyph index (e.g., of the current pen-position).
    * @param[in] subsequentIndex
    *   The glyph index of the subsequent/next glyph.
    *
    * @return
    *   The kerning (usually negative) between the two glyphs in pt.
    *   If either on of the glyphs is unknown to this font face or
    *   no specific kerning for the glyph pair is available a zero
    *   kerning is returned.
    */
    float kerning(GlyphIndex index, GlyphIndex subsequentIndex) const;

    /**
    * @brief
    *   Set the kerning for a glyph w.r.t. to a subsequent glyph in pt.
    *
    *   If the glyph is known to this font face, the values are
    *   forwarded to the glyphs kerning setter (see Glyph for more information).
    *
    * @param[in] index
    *   The target glyph index.
    * @param[in] subsequentIndex
    *   The glyph index of the respective subsequent/next glyph.
    * @param[in] kerning
    *   Kerning of the two glyphs in pt.
    */
    void setKerning(GlyphIndex index, GlyphIndex subsequentIndex, float kerning);


protected:

    float m_base;
    float m_ascent;
    float m_descent;
    float m_linegap;

    glm::uvec2 m_glyphTextureExtent;
    glm::vec4  m_glyphTexturePadding;

    globjects::ref_ptr<globjects::Texture> m_glyphTexture;

    std::unordered_map<GlyphIndex, Glyph> m_glyphs;
};


} // namespace gloperate_text
