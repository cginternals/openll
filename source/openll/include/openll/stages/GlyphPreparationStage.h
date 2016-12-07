
#pragma once

#include <vector>

#include <glm/fwd.hpp>

#include <openll/GlyphVertexCloud.h>

#include <openll/openll_api.h>


namespace gloperate_text
{

class FontFace;
class GlyphSequence;

OPENLL_API GlyphVertexCloud prepareGlyphs(const std::vector<GlyphSequence>& sequences, bool optimized);


} // namespace gloperate_text
