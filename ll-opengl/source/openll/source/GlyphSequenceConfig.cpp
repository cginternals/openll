#include "openll/GlyphSequenceConfig.h"

//#include <glm/vec2.hpp>

//#include <glm/vec3.hpp>
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gloperate_text
{

//default values taken from GlyphSequence.cpp
GlyphSequenceConfig::GlyphSequenceConfig(gloperate_text::FontFace * font)
	: m_wordWrap(false)
	, m_lineWidth(0.f)
	, m_alignment(gloperate_text::Alignment::LeftAligned)
	, m_anchor(gloperate_text::LineAnchor::Baseline)
	, m_fontColor(glm::vec4(0.f, 0.f, 0.f, 1.0))
	, m_fontSize(12.f)
	, m_margins({ 0.f,0.f, 0.f, 0.f })
	, m_ppiScale(1.f)
{
	m_fontFace = font;
}

GlyphSequenceConfig::~GlyphSequenceConfig()
{
}

void GlyphSequenceConfig::addOnConfigChanged(std::function<void()> callback)
{
	m_registeredOnConfigChanged.push_back(callback);
}

//TODO unregister callbacks?

bool GlyphSequenceConfig::wordWrap() const
{
	return m_wordWrap;
}

void GlyphSequenceConfig::setWordWrap(bool enable)
{
	m_wordWrap = enable;
}

float GlyphSequenceConfig::lineWidth() const
{
	// since typesetting is done in the font faces font size, the
	// given linewidth has to be scaled to the font faces font size
	return glm::max(m_lineWidth * m_fontFace->size() / m_fontSize, 0.f);
}

void GlyphSequenceConfig::setLineWidth(float lineWidth)
{
	m_lineWidth = lineWidth;
}


gloperate_text::Alignment GlyphSequenceConfig::alignment() const
{
	return m_alignment;
}

void GlyphSequenceConfig::setAlignment(const gloperate_text::Alignment alignment)
{
	m_alignment = alignment;
}

gloperate_text::LineAnchor GlyphSequenceConfig::lineAnchor() const
{
	return m_anchor;
}

void GlyphSequenceConfig::setLineAnchor(const gloperate_text::LineAnchor anchor)
{
	m_anchor = anchor;
}

const glm::vec4 & GlyphSequenceConfig::fontColor() const
{
	return m_fontColor;
}

void GlyphSequenceConfig::setFontColor(glm::vec4 fontColor)
{
	m_fontColor = fontColor;
}

gloperate_text::FontFace * GlyphSequenceConfig::fontFace() const
{
	return m_fontFace;
}

void GlyphSequenceConfig::setFontFace(gloperate_text::FontFace * fontFace)
{
	m_fontFace = fontFace;
}

float GlyphSequenceConfig::fontSize() const
{
	return m_fontSize;
}

void GlyphSequenceConfig::setFontSize(float fontSize)
{
	m_fontSize = fontSize;
}

} // namespace gloperate_text