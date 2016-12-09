#include <openll/layout/algorithm.h>

#include <random>

#include <openll/GlyphSequence.h>
#include <openll/FontFace.h>
#include <openll/layout/layoutbase.h>

namespace gloperate_text
{


void constantLayout(std::vector<Label> & labels)
{
    for (auto & label : labels)
    {
        label.placement = {{0.f, 0.f}, Alignment::LeftAligned, LineAnchor::Descent};
    }
}

void randomLayout(std::vector<Label> & labels)
{

    std::default_random_engine generator;
    std::bernoulli_distribution bool_distribution;
    for (auto & label : labels)
    {
        auto alignment = bool_distribution(generator) ? Alignment::LeftAligned : Alignment::RightAligned;
        auto lineAnchor = bool_distribution(generator) ? LineAnchor::Ascent : LineAnchor::Descent;
        label.placement = {{0.f, 0.f}, alignment, lineAnchor};
    }
}

}
