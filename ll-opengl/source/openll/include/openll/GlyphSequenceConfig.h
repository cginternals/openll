#pragma once

#include <vector>
#include <functional>

#include <openll/Alignment.h>
#include <openll/FontFace.h>
#include <openll/LineAnchor.h>

#include <openll/openll_api.h> //TODO: Are all includes needed?

namespace gloperate_text
{

class OPENLL_API GlyphSequenceConfig
{
public:
	explicit GlyphSequenceConfig(gloperate_text::FontFace * font);
	~GlyphSequenceConfig();

	//TODO register callbacks from every GlyphSequence to notify the sequence when config has changed
	//how to get the function's origin glyhsequence? Say seq1 and seq2 both register a callback?
	//or should it be only for GlyphSequence, i.e. we don't register callbacks but objects?
	//TODO unregister callbacks?
	void addOnConfigChanged(std::function<void()>);

	bool wordWrap() const;
	void setWordWrap(bool enable);

	float lineWidth() const;
	void setLineWidth(float lineWidth);

	gloperate_text::Alignment alignment() const;
	void setAlignment(gloperate_text::Alignment alignment);

	gloperate_text::LineAnchor lineAnchor() const;
	void setLineAnchor(const gloperate_text::LineAnchor anchor);

	float fontSize() const;
	void setFontSize(float fontSize);

	gloperate_text::FontFace * fontFace() const;
	void setFontFace(gloperate_text::FontFace * fontFace);

	const glm::vec4 & fontColor() const;
	void setFontColor(glm::vec4 fontColor);


protected:

	//maybe define a special callback type?
	//using callback_t = std::function<void()>;
	std::vector<std::function<void()>> m_registeredOnConfigChanged;

	bool m_wordWrap;
	float m_lineWidth;

	gloperate_text::Alignment m_alignment;
	gloperate_text::LineAnchor m_anchor;

	glm::vec4 m_fontColor;
	gloperate_text::FontFace * m_fontFace;
	float m_fontSize;

	glm::vec4 m_margins;
	float m_ppiScale;

	//TODO: is it useful to set them here? Does not make sense for batch processing...
	//glm::mat4 m_additionalTransform;
	//std::u32string m_string;
};

}// namespace gloperate_text

