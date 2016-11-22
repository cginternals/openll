
#include <openll/stages/GlyphPreparationStage.h>

#include <cassert>

#include <openll/FontFace.h>
#include <openll/GlyphSequence.h>
#include <openll/Typesetter.h>


namespace gloperate_text
{

OPENLL_API GlyphVertexCloud prepareGlyphs(FontFace * font, const std::vector<GlyphSequence>& sequences, bool optimized)
{
    // get total number of glyphs
    auto numGlyphs = size_t(0u);
    for (const auto & sequence : sequences)
        numGlyphs += sequence.size(*font);

    // prepare vertex cloud storage
    GlyphVertexCloud vertexCloud;
    vertexCloud.vertices().resize(numGlyphs);

    // typeset and transform all sequences
    assert(font);

    auto index = vertexCloud.vertices().begin();
    for (const auto & sequence : sequences)
    {
        auto extent = Typesetter::typeset(sequence, *font, index);
        index += sequence.size(*font);
    }

    if(optimized)
        vertexCloud.optimize(sequences, *font); // optimize and update drawable
    else
        vertexCloud.update(); // update drawable

    vertexCloud.setTexture(font->glyphTexture());

    return vertexCloud;
}


} // namespace gloperate_text
