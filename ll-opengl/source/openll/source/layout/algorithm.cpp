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

namespace layout
{

void constant(std::vector<Label> & labels)
{
    for (auto & label : labels)
    {
        label.placement = {{0.f, 0.f}, Alignment::LeftAligned, LineAnchor::Bottom, true};
    }
}

void random(std::vector<Label> & labels)
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

void greedy(std::vector<Label> & labels)
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

void discreteGradientDescent(std::vector<Label> & labels)
{
    std::vector<std::vector<LabelArea>> labelAreas;
    std::vector<int> chosenLabels;
    auto chosenLabel = [&](int i) { return labelAreas[i][chosenLabels[i]]; };

    std::default_random_engine generator;

    // generate LabelArea objects for all possible label placements
    // generate random start layout
    for (auto & label : labels)
    {
        labelAreas.push_back({});
        const auto extent = Typesetter::extent(label.sequence);
        const std::vector<glm::vec2> possibleOrigins {
            label.pointLocation, label.pointLocation - glm::vec2(extent.x, 0),
            label.pointLocation - glm::vec2(0, extent.y), label.pointLocation - extent
        };
        for (const auto& origin : possibleOrigins)
        {
            labelAreas.back().push_back({origin, extent});
        }
        std::uniform_int_distribution<int> distribution(0, labelAreas.back().size() - 1);
        const auto value = distribution(generator);
        chosenLabels.push_back(value);
    }

    // upper limit to iterations
    for (int iteration = 0; iteration < 1000; ++iteration)
    {
        int bestImprovement = 0;
        int bestLabelIndex = -1;
        int bestLabelPositionIndex = -1;
        size_t labelIndex = 0;
        for (auto & singleLabelAreas : labelAreas)
        {
            std::vector<int> collisions;
            int bestIndex = 0;
            for (auto & labelArea : singleLabelAreas)
            {
                int count = 0;
                for (size_t i = 0; i < labels.size(); ++i)
                {
                    if (i == labelIndex) continue;
                    if (labelArea.overlaps(chosenLabel(i)))
                    {
                        ++count;
                    }
                }
                collisions.push_back(count);
                if (count < collisions[bestIndex])
                {
                    bestIndex = collisions.size() - 1;
                }
            }
            int improvement = collisions[chosenLabels[labelIndex]] - collisions[bestIndex];
            if (improvement > bestImprovement)
            {
                bestImprovement = improvement;
                bestLabelIndex = labelIndex;
                bestLabelPositionIndex = bestIndex;
            }
            ++labelIndex;
        }
        // local minimum found
        if (bestImprovement == 0) break;
        chosenLabels[bestLabelIndex] = bestLabelPositionIndex;
    }

    for (size_t i = 0; i < labels.size(); ++i)
    {
        labels[i].placement = {chosenLabel(i).origin - labels[i].pointLocation, Alignment::LeftAligned, LineAnchor::Bottom, true};
    }
}

} // namespace layout

} // namespace gloperate_text
