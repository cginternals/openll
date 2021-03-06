#include "openll/GlyphSequenceConfig.h"

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
	// TODO
	// copied from GlyphSequence
	// not sure if we should return the raw line width (without scaling)

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

gloperate_text::LineAnchor GlyphSequenceConfig::anchor() const
{
	return m_anchor;
}

void GlyphSequenceConfig::setAnchor(const gloperate_text::LineAnchor anchor)
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


glm::vec4 GlyphSequenceConfig::margins() const
{
	return m_margins;
}

void GlyphSequenceConfig::setMargins(glm::vec4 margins)
{
	m_margins = margins;
}

float GlyphSequenceConfig::ppiScale() const
{
	return m_ppiScale;
}
void GlyphSequenceConfig::setPpiScale(float ppiScale)
{
	m_ppiScale = ppiScale;
}

} // namespace gloperate_text