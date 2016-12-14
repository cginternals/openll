
#include <openll/GlyphSequence.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <openll/FontFace.h>


namespace gloperate_text
{


GlyphSequence::GlyphSequence()
: m_wordWrap(false)
, m_lineWidth(0.f)
, m_alignment(Alignment::LeftAligned)
, m_anchor(LineAnchor::Baseline)
, m_fontColor(glm::vec4(0.f, 0.f, 0.f, 1.0))
, m_fontFace(nullptr)
, m_fontSize(12.f)
, m_transformValid(false)

{
}

GlyphSequence::~GlyphSequence()
{
}

size_t GlyphSequence::size() const
{
    return m_string.size();
}

size_t GlyphSequence::depictableSize() const
{
    auto count = size_t(0);
    for (const auto & c : m_string)
    {
        if (m_fontFace->depictable(c))
            ++count;
    }
    return count;
}

const std::u32string & GlyphSequence::string() const
{
    return m_string;
}

void GlyphSequence::setString(const std::u32string & string)
{
    if (m_string.compare(string) == 0)
        return;

    m_string = string;
}

const std::vector<char32_t> & GlyphSequence::chars(
    std::vector<char32_t> & allChars) const
{
    allChars.reserve(allChars.size() + size());

    for (const auto & c : m_string)
        allChars.push_back(c);

    return allChars;
}

const std::vector<char32_t> & GlyphSequence::depictableChars(
    std::vector<char32_t> & depictableChars) const
{
    depictableChars.reserve(depictableChars.size() + depictableSize());

    for (const auto & c : m_string)
    {
        if(m_fontFace->depictable(c))
            depictableChars.push_back(c);
    }
    return depictableChars;
}

bool GlyphSequence::wordWrap() const
{
    return m_wordWrap;
}

void GlyphSequence::setWordWrap(bool enable)
{
    m_wordWrap = enable;
}

float GlyphSequence::lineWidth() const
{
    assert(m_fontFace);
    // since typesetting is done in the font faces font size, the
    // given linewidth has to be scaled to the font faces font size
    return glm::max(m_lineWidth * m_fontFace->size() / m_fontSize, 0.f);
}

void GlyphSequence::setLineWidth(float lineWidth)
{
    m_lineWidth = lineWidth;
}


Alignment GlyphSequence::alignment() const
{
    return m_alignment;
}

void GlyphSequence::setAlignment(const Alignment alignment)
{
    m_alignment = alignment;
}

LineAnchor GlyphSequence::lineAnchor() const
{
    return m_anchor;
}

void GlyphSequence::setLineAnchor(const LineAnchor anchor)
{
    m_anchor = anchor;
}

const glm::vec4 & GlyphSequence::fontColor() const
{
    return m_fontColor;
}

void GlyphSequence::setFontColor(glm::vec4 fontColor)
{
    m_fontColor = fontColor;
}

FontFace * GlyphSequence::fontFace() const
{
    return m_fontFace;
}

void GlyphSequence::setFontFace(FontFace * fontFace)
{
    m_transformValid = false;
    m_fontFace = fontFace;
}

float GlyphSequence::fontSize() const
{
    return m_fontSize;
}

void GlyphSequence::setFontSize(float fontSize)
{
    m_transformValid = false;
    m_fontSize = fontSize;
}

void GlyphSequence::setFromConfig(const GlyphSequenceConfig config)
{
	setWordWrap(config.wordWrap());
	setLineWidth(config.lineWidth());
	setAlignment(config.alignment());
	setLineAnchor(config.anchor());
	setFontColor(config.fontColor());
	setFontFace(config.fontFace());
	setFontSize(config.fontSize());
}

const glm::mat4 & GlyphSequence::additionalTransform() const
{
    return m_additionalTransform;
}

void GlyphSequence::setAdditionalTransform(const glm::mat4 & additionalTransform)
{
    m_transformValid = false;
    m_additionalTransform = additionalTransform;
}

const glm::mat4 & GlyphSequence::transform() const
{
    if (!m_transformValid)
    {
        computeTransform();
        m_transformValid = true;
    }
    return m_transform;
}

void GlyphSequence::computeTransform() const
{
    assert(m_fontFace);

    m_transform = glm::mat4();

    m_transform = m_transform * m_additionalTransform;

    m_transform = glm::scale(m_transform, glm::vec3(m_fontSize / m_fontFace->size()));
}

} // namespace gloperate_text
