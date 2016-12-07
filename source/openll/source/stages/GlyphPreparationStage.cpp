
#include <openll/stages/GlyphPreparationStage.h>

#include <cassert>

#include <openll/FontFace.h>
#include <openll/GlyphSequence.h>
#include <openll/Typesetter.h>


namespace gloperate_text
{

OPENLL_API GlyphVertexCloud prepareGlyphs(const std::vector<GlyphSequence>& sequences, bool optimized)
{
    if (sequences.empty())
    {
        return {};
    }

    // get total number of glyphs
    auto numGlyphs = size_t(0u);
    for (const auto & sequence : sequences)
        numGlyphs += sequence.depictableSize();

    // prepare vertex cloud storage
    GlyphVertexCloud vertexCloud;
    vertexCloud.vertices().resize(numGlyphs);

    auto index = vertexCloud.vertices().begin();
    for (const auto & sequence : sequences)
    {
        Typesetter::typeset(sequence, index);
        index += sequence.depictableSize();
    }

    FontFace * face = sequences[0].fontFace();

    if(optimized)
        vertexCloud.optimize(sequences); // optimize and update drawable
    else
        vertexCloud.update(); // update drawable

    vertexCloud.setTexture(face->glyphTexture());

    return vertexCloud;
}


} // namespace gloperate_text
