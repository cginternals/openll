#include <openll/layout/algorithm.h>

#include <random>
#include <algorithm>

#include <openll/GlyphSequence.h>
#include <openll/FontFace.h>
#include <openll/layout/layoutbase.h>
#include <openll/layout/LabelArea.h>
#include <openll/Typesetter.h>


namespace gloperate_text
{


void constantLayout(std::vector<Label> & labels)
{
    for (auto & label : labels)
    {
        label.placement = {{0.f, 0.f}, Alignment::LeftAligned, LineAnchor::Bottom, true};
    }
}

void randomLayout(std::vector<Label> & labels)
{
    std::default_random_engine generator;
    std::bernoulli_distribution bool_distribution;
    for (auto & label : labels)
    {
        const auto extent = Typesetter::extent(label.sequence);
        glm::vec2 offset;
        offset.x = bool_distribution(generator) ? -extent.x : 0.f;
        offset.y = bool_distribution(generator) ? -extent.y : 0.f;
        label.placement = {offset, Alignment::LeftAligned, LineAnchor::Bottom, true};
    }
}

void greedyLayout(std::vector<Label> & labels)
{
    std::vector<LabelArea> labelAreas;
    for (auto & label : labels)
    {
        const auto extent = Typesetter::extent(label.sequence);
        const std::vector<glm::vec2> possibleOrigins {
            label.pointLocation, label.pointLocation - glm::vec2(extent.x, 0),
            label.pointLocation - glm::vec2(0, extent.y), label.pointLocation - extent
        };
        int bestCount = labelAreas.size() + 1;
        glm::vec2 bestOrigin;
        for (const auto& origin : possibleOrigins)
        {
            const LabelArea newLabelArea {origin, extent};
            const auto count = std::count_if(labelAreas.begin(), labelAreas.end(),
                [&](const LabelArea& other) { return newLabelArea.overlaps(other); });
            if (count < bestCount)
            {
                bestCount = count;
                bestOrigin = origin;
            }
        }
        label.placement = {bestOrigin - label.pointLocation, Alignment::LeftAligned, LineAnchor::Bottom, true};
        labelAreas.push_back({bestOrigin, extent});
    }
}

}
