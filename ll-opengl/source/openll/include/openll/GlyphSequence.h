
#pragma once

#include <string>
#include <vector>

#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <globjects/base/ref_ptr.h>

#include <openll/Alignment.h>
#include <openll/LineAnchor.h>
#include <openll/GlyphSequenceConfig.h>

#include <openll/openll_api.h>


namespace gloperate_text
{

class FontFace;
class Glyph;


class OPENLL_API GlyphSequence
{
public:
    GlyphSequence();
    virtual ~GlyphSequence();

    size_t size() const;
    size_t depictableSize() const;

    const std::u32string & string() const;
    void setString(const std::u32string & string);

    const std::vector<char32_t> & chars(
        std::vector<char32_t> & allChars) const;
    const std::vector<char32_t> & depictableChars(
        std::vector<char32_t> & allChars) const;

    bool wordWrap() const;
    void setWordWrap(bool enable);

    float lineWidth() const;
    void setLineWidth(
        float lineWidth);

    Alignment alignment() const;
    void setAlignment(Alignment alignment);

    LineAnchor lineAnchor() const;
    void setLineAnchor(const LineAnchor anchor);

    float fontSize() const;
    void setFontSize(float fontSize);

    FontFace * fontFace() const;
    void setFontFace(FontFace * fontFace);

    const glm::vec4 & fontColor() const;
    void setFontColor(glm::vec4 fontColor);

    const glm::mat4 & additionalTransform() const;
    void setAdditionalTransform(const glm::mat4 & additionalTransform);

    const glm::mat4 & transform() const;

	void setFromConfig(const GlyphSequenceConfig config);

protected:
    void computeTransform() const;

protected:
    std::u32string m_string;

    bool m_wordWrap;
    float m_lineWidth;

    Alignment m_alignment;
    LineAnchor m_anchor;

    glm::vec4 m_fontColor;
    FontFace * m_fontFace;
    float m_fontSize;

    glm::mat4 m_additionalTransform;
    mutable bool m_transformValid;
    mutable glm::mat4 m_transform;
};


} // namespace gloperate_text
