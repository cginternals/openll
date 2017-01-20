#include <openll/layout/algorithm.h>

#include <random>
#include <algorithm>
#include <limits>

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

float overlapArea(const LabelArea & target, const LabelArea & other)
{
    return target.overlapArea(other);
}
float overlapCount(const LabelArea & target, const LabelArea & other)
{
    return target.overlaps(other) ? 1.f : 0.f;
}

void greedy(std::vector<Label> & labels, ScoringFunction scoringFunction)
{
    std::vector<LabelArea> labelAreas;
    for (auto & label : labels)
    {
        const auto extent = Typesetter::extent(label.sequence);
        const std::vector<glm::vec2> possibleOrigins {
            label.pointLocation, label.pointLocation - glm::vec2(extent.x, 0),
            label.pointLocation - glm::vec2(0, extent.y), label.pointLocation - extent
        };
        float bestScore = std::numeric_limits<float>::max();
        glm::vec2 bestOrigin;
        for (const auto& origin : possibleOrigins)
        {
            const LabelArea newLabelArea {origin, extent};
            float score = 0.f;
            for (const auto& other : labelAreas)
            {
                score += scoringFunction(newLabelArea, other);
            }
            if (score < bestScore)
            {
                bestScore = score;
                bestOrigin = origin;
            }
        }
        label.placement = {bestOrigin - label.pointLocation, Alignment::LeftAligned, LineAnchor::Bottom, true};
        labelAreas.push_back({bestOrigin, extent});
    }
}

void discreteGradientDescent(std::vector<Label> & labels, ScoringFunction scoringFunction)
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
        float bestImprovement = 0.f;
        int bestLabelIndex = -1;
        int bestLabelPositionIndex = -1;
        size_t labelIndex = 0;
        for (auto & singleLabelAreas : labelAreas)
        {
            std::vector<float> scores;
            int bestIndex = 0;
            for (auto & labelArea : singleLabelAreas)
            {
                float score = 0.f;
                for (size_t i = 0; i < labels.size(); ++i)
                {
                    if (i == labelIndex) continue;
                    score += scoringFunction(labelArea, chosenLabel(i));
                }
                scores.push_back(score);
                if (score < scores[bestIndex])
                {
                    bestIndex = scores.size() - 1;
                }
            }
            float improvement = scores[chosenLabels[labelIndex]] - scores[bestIndex];
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


void simulatedAnnealing(std::vector<Label> & labels, ScoringFunction scoringFunction)
{
    // based on https://www.eecs.harvard.edu/shieber/Biblio/Papers/tog-final.pdf

    std::vector<std::vector<LabelArea>> labelAreas;
    std::vector<unsigned int> chosenLabels;
    auto chosenLabel = [&](unsigned int i) { return labelAreas[i][chosenLabels[i]]; };

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

    std::uniform_int_distribution<unsigned int> labelDistribution(0, labels.size() - 1);

    float temperature = 0.91023922662f;
    unsigned int temperatureChanges = 0;
    unsigned int changesAtTemperature = 0;
    unsigned int stepsAtTemperature = 0;

    while (true)
    {
        const auto labelIndex = labelDistribution(generator);
        std::uniform_int_distribution<unsigned int> positionDistribution(0, labelAreas[labelIndex].size() - 1);
        size_t newPosition = 0;
        do
        {
            newPosition = positionDistribution(generator);
        }
        while (newPosition == chosenLabels[labelIndex]);

        float improvement = 0.f;
        auto newLabelArea = labelAreas[labelIndex][newPosition];
        auto oldLabelArea = chosenLabel(labelIndex);
        for (size_t i = 0; i < labels.size(); ++i)
        {
            if (i == labelIndex) continue;
            improvement += scoringFunction(oldLabelArea, chosenLabel(i)) - scoringFunction(newLabelArea, chosenLabel(i));
        }

        float chance = std::exp(improvement / temperature);
        std::bernoulli_distribution doAnyway(chance);
        if (improvement > 0 || doAnyway(generator))
        {
            chosenLabels[labelIndex] = newPosition;
            ++changesAtTemperature;
        }

        ++stepsAtTemperature;
        if (changesAtTemperature > 5 * labels.size() || stepsAtTemperature > 20 * labels.size())
        {
            // converged
            if (changesAtTemperature == 0) break;
            if (temperatureChanges == 100) break;

            temperature *= 0.9f;
            changesAtTemperature = 0;
            stepsAtTemperature = 0;
            ++temperatureChanges;
        }
    }

    for (size_t i = 0; i < labels.size(); ++i)
    {
        labels[i].placement = {chosenLabel(i).origin - labels[i].pointLocation, Alignment::LeftAligned, LineAnchor::Bottom, true};
    }
}

} // namespace layout

} // namespace gloperate_text
