#include <openll/layout/constantalgorithm.h>

#include <openll/GlyphSequence.h>
#include <openll/FontFace.h>
#include <openll/layout/layoutbase.h>


#include <openll/openll_api.h>

namespace gloperate_text
{


void constantLayout(std::vector<Label> & labels)
{
    for (auto & label : labels)
    {
        label.placement = {{0.f, 0.f}, gloperate_text::Alignment::LeftAligned, gloperate_text::LineAnchor::Baseline};
    }
}

}
