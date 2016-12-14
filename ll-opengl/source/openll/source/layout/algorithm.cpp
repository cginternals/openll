#include <openll/layout/algorithm.h>

#include <random>
#include <algorithm>

#include <openll/GlyphSequence.h>
#include <openll/FontFace.h>
#include <openll/layout/layoutbase.h>
#include <openll/Typesetter.h>

namespace
{

struct LabelArea
{
    glm::vec2 origin;
    glm::vec2 extent;

    bool overlaps(const LabelArea& other) const
    {
        return
            origin.x < other.origin.x + other.extent.x &&
            origin.x + extent.x > other.origin.x &&
            origin.y < other.origin.y + other.extent.y &&
            extent.y + origin.y > other.origin.y;
    }
};
}

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

void greedyLayout(std::vector<Label> & labels)
{
    std::vector<LabelArea> labelAreas;
    for (auto & label : labels)
    {
        auto extent = Typesetter::extent(label.sequence);
        std::vector<glm::vec2> possibleOrigins {
            label.pointLocation, label.pointLocation - glm::vec2(extent.x, 0),
            label.pointLocation - glm::vec2(0, extent.y), label.pointLocation - extent
        };
        int bestCount = labelAreas.size() + 1;
        glm::vec2 bestOrigin;
        for (const auto& origin : possibleOrigins)
        {
            LabelArea newLabelArea {origin, extent};
            auto count = std::count_if(labelAreas.begin(), labelAreas.end(),
                [&](const LabelArea& other) { return newLabelArea.overlaps(other); });
            if (count < bestCount)
            {
                bestCount = count;
                bestOrigin = origin;
            }
        }
        label.placement = {bestOrigin - label.pointLocation, Alignment::LeftAligned, LineAnchor::Descent};
        labelAreas.push_back({bestOrigin, extent});
    }
}

}
