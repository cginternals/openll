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

namespace
{

struct LabelCollision
{
    size_t index;
    size_t position;
    float overlapArea;
};


struct Index2D
{
    template<typename T>
    void next(const std::vector<std::vector<T>>& container)
    {
        ++inner;
        if (inner == container[outer].size())
        {
            ++outer;
            inner = 0;
        }
    }

    template<typename T>
    bool end(const std::vector<std::vector<T>>& container) const
    {
        return outer == container.size();
    }

    template<typename T>
    T& element(std::vector<std::vector<T>>& container) const
    {
        return container[outer][inner];
    }

    template<typename T>
    const T& element(const std::vector<std::vector<T>>& container) const
    {
        return container[outer][inner];
    }

    size_t outer = 0;
    size_t inner = 0;
};

std::vector<std::vector<std::vector<LabelCollision>>> createCollisionGraph(const std::vector<std::vector<LabelArea>>& labelAreas)
{
    std::vector<std::vector<std::vector<LabelCollision>>> collisionGraph;
    collisionGraph.resize(labelAreas.size());
    for (size_t i = 0; i < labelAreas.size(); ++i)
    {
        collisionGraph[i].resize(labelAreas[i].size());
    }

    for (Index2D index1; !index1.end(labelAreas); index1.next(labelAreas))
    {
        auto & collisionElement = index1.element(collisionGraph);
        for (Index2D index2; !index2.end(labelAreas); index2.next(labelAreas))
        {
            if (index1.outer == index2.outer)
                continue;
            const auto & label1 = index1.element(labelAreas);
            const auto & label2 = index2.element(labelAreas);
            if (label1.overlaps(label2))
            {
                auto area = label1.overlapArea(label2);
                collisionElement.push_back({index2.outer, index2.inner, area});
                assert(index1.element(collisionGraph).size() > 0);
            }
        }
    }
    return collisionGraph;
}

// generate LabelArea objects for all possible label placements
std::vector<std::vector<LabelArea>> computeLabelAreas(const std::vector<Label> & labels, const std::vector<RelativeLabelPosition>& positions)
{
    std::vector<std::vector<LabelArea>> result;
    for (const auto & label : labels)
    {
        result.push_back({});
        const auto extent = Typesetter::extent(label.sequence);
        for (const auto& position : positions)
        {
            const auto origin = labelOrigin(position, label.pointLocation, extent);
            result.back().push_back({origin, extent});
        }
    }
    return result;
}

std::vector<unsigned int> randomStartLabelAreas(const std::vector<std::vector<LabelArea>> & labelAreas)
{
    std::vector<unsigned int> result;
    std::default_random_engine generator;
    for (const auto & singleLabelAreas : labelAreas)
    {
        std::uniform_int_distribution<int> distribution(0, singleLabelAreas.size() - 1);
        result.push_back(distribution(generator));
    }
    return result;
}

template<typename T>
T randomIndexExcept(T except, T size, std::default_random_engine engine)
{
    std::uniform_int_distribution<T> positionDistribution(0, size - 1);
    T randomNumber;
    do
    {
        randomNumber = positionDistribution(engine);
    }
    while (randomNumber == except);
    return randomNumber;
}

}

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

float overlapArea(int, float overlapArea, RelativeLabelPosition, bool, unsigned int)
{
    return overlapArea;
}
float overlapCount(int overlapCount, float, RelativeLabelPosition, bool, unsigned int)
{
    return overlapCount;
}

float standard(
    int, float overlapArea, RelativeLabelPosition position,
    bool displayed, unsigned int priority)
{
    unsigned int positionPenalty = 0;
    switch (position)
    {
        case RelativeLabelPosition::UpperRight: positionPenalty = 0; break;
        case RelativeLabelPosition::UpperLeft:  positionPenalty = 1; break;
        case RelativeLabelPosition::LowerLeft:  positionPenalty = 2; break;
        case RelativeLabelPosition::LowerRight: positionPenalty = 3; break;
        default: assert(false);
    }
    return 10.f * overlapArea + (displayed ? priority : 0.f) + .3f * positionPenalty;
}


void greedy(std::vector<Label> & labels, PenaltyFunction penaltyFunction)
{
    std::vector<LabelArea> labelAreas;
    const std::vector<RelativeLabelPosition> positions {
        RelativeLabelPosition::UpperRight, RelativeLabelPosition::UpperLeft,
        RelativeLabelPosition::LowerLeft, RelativeLabelPosition::LowerRight
    };
    for (auto & label : labels)
    {
        const auto extent = Typesetter::extent(label.sequence);
        float bestPenalty = std::numeric_limits<float>::max();
        glm::vec2 bestOrigin;
        for (const auto& position : positions)
        {
            const auto origin = labelOrigin(position, label.pointLocation, extent);
            const LabelArea newLabelArea {origin, extent};
            float overlapArea = 0.f;
            int overlapCount = 0;
            for (const auto& other : labelAreas)
            {
                overlapArea += newLabelArea.overlapArea(other);
                overlapCount += newLabelArea.overlaps(other) ? 1 : 0;
            }
            overlapArea /= newLabelArea.area();
            auto penalty = penaltyFunction(overlapCount, overlapArea, position, true, 1);
            if (penalty < bestPenalty)
            {
                bestPenalty = penalty;
                bestOrigin = origin;
            }
        }
        label.placement = {bestOrigin - label.pointLocation, Alignment::LeftAligned, LineAnchor::Bottom, true};
        labelAreas.push_back({bestOrigin, extent});
    }
}

void discreteGradientDescent(std::vector<Label> & labels, PenaltyFunction penaltyFunction)
{
    const std::vector<RelativeLabelPosition> positions {
        RelativeLabelPosition::UpperRight, RelativeLabelPosition::UpperLeft,
        RelativeLabelPosition::LowerLeft, RelativeLabelPosition::LowerRight
    };

    const std::vector<std::vector<LabelArea>> labelAreas = computeLabelAreas(labels, positions);
    std::vector<unsigned int> chosenLabels = randomStartLabelAreas(labelAreas);
    const auto collisionGraph = createCollisionGraph(labelAreas);
    const auto chosenLabel = [&](unsigned int i) { return labelAreas[i][chosenLabels[i]]; };

    // upper limit to iterations
    for (int iteration = 0; iteration < 1000; ++iteration)
    {
        float bestImprovement = 0.f;
        int bestLabelIndex = -1;
        int bestLabelPositionIndex = -1;
        size_t labelIndex = 0;
        for (auto & singleLabelAreas : labelAreas)
        {
            std::vector<float> penalties;
            int bestIndex = 0;
            for (size_t index = 0; index < singleLabelAreas.size(); ++index)
            {
                const auto & labelArea = singleLabelAreas[index];
                float overlapArea = 0.f;
                int overlapCount = 0;
                for (const auto & collison : collisionGraph[labelIndex][index])
                {
                    if (chosenLabels[collison.index] != collison.position)
                        continue;
                    ++overlapCount;
                    overlapArea += labelArea.overlapArea(chosenLabel(collison.index));
                }
                overlapArea /= labelArea.area();
                const auto penalty = penaltyFunction(overlapCount, overlapArea, positions[index], true, 1);
                penalties.push_back(penalty);
                if (penalty < penalties[bestIndex])
                {
                    bestIndex = index;
                }
            }
            float improvement = penalties[chosenLabels[labelIndex]] - penalties[bestIndex];
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

void simulatedAnnealing(std::vector<Label> & labels, PenaltyFunction penaltyFunction)
{
    // based on https://www.eecs.harvard.edu/shieber/Biblio/Papers/tog-final.pdf

    const std::vector<RelativeLabelPosition> positions {
        RelativeLabelPosition::UpperRight, RelativeLabelPosition::UpperLeft,
        RelativeLabelPosition::LowerLeft, RelativeLabelPosition::LowerRight
    };

    const std::vector<std::vector<LabelArea>> labelAreas = computeLabelAreas(labels, positions);
    std::vector<unsigned int> chosenLabels = randomStartLabelAreas(labelAreas);
    const auto collisionGraph = createCollisionGraph(labelAreas);
    const auto chosenLabel = [&](unsigned int i) { return labelAreas[i][chosenLabels[i]]; };

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned int> labelDistribution(0, labels.size() - 1);

    float temperature = 0.91023922662f;
    unsigned int temperatureChanges = 0;
    unsigned int changesAtTemperature = 0;
    unsigned int stepsAtTemperature = 0;

    while (true)
    {
        const auto labelIndex = labelDistribution(generator);
        const auto oldPosition = chosenLabels[labelIndex];
        const auto newPosition = randomIndexExcept<unsigned int>(oldPosition, labelAreas[labelIndex].size(), generator);

        const auto computePenalty = [&](const LabelArea & labelArea, size_t position)
        {
            float overlapArea = 0.f;
            int overlapCount = 0;
            for (const auto & collison : collisionGraph[labelIndex][position])
            {
                if (chosenLabels[collison.index] != collison.position)
                    continue;
                overlapArea += labelArea.overlapArea(chosenLabel(collison.index));
                ++overlapCount;
            }
            overlapArea /= labelArea.area();
            return penaltyFunction(overlapCount, overlapArea, positions[position], true, 1);
        };

        const auto oldPenalty = computePenalty(chosenLabel(labelIndex), oldPosition);
        const auto newPenalty = computePenalty(labelAreas[labelIndex][newPosition], newPosition);
        const auto improvement = oldPenalty - newPenalty;

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
