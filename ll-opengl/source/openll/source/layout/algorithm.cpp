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

// an index for iterating over a 2-dimensional vector
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

// computes a graph in which all overlaps between all possible label positions are stored
// the graph is returned as an adjacency matrix for quick lookup of all overlapping labels of a given placed labels
std::vector<std::vector<std::vector<LabelCollision>>> createCollisionGraph(const std::vector<std::vector<LabelArea>>& labelAreas, const glm::vec2 & relativePadding = {0.f, 0.f})
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
        const auto & label1 = index1.element(labelAreas);
        for (Index2D index2; !index2.end(labelAreas); index2.next(labelAreas))
        {
            if (index1.outer == index2.outer)
                continue;
            const auto & label2 = index2.element(labelAreas);
            if (label1.paddedOverlaps(label2, relativePadding))
            {
                auto area = label1.paddedOverlapArea(label2, relativePadding);
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
            result.back().push_back({origin, extent, position});
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

// returns a random number in the range [0, size) which is different from the parameter except
template<typename T>
T randomIndexExcept(T except, T size, std::default_random_engine engine)
{
    std::uniform_int_distribution<T> positionDistribution(0, size - 2);
    auto randomNumber = positionDistribution(engine);
    if (randomNumber == except)
        return size - 1;
    return randomNumber;
}

float computePenalty(const LabelArea & labelArea, const std::vector<LabelCollision> & collisions,
    unsigned int priority, PenaltyFunction penaltyFunction, const std::vector<unsigned int> & chosenLabels)
{
    float overlapArea = 0.f;
    int overlapCount = 0;
    for (const auto & collision : collisions)
    {
        if (chosenLabels[collision.index] != collision.position)
            continue;
        overlapArea += collision.overlapArea;
        ++overlapCount;
    }
    overlapArea /= labelArea.area();
    return penaltyFunction(overlapCount, overlapArea, labelArea.position, priority);
}

LabelPlacement placementFor(const LabelArea & labelArea, const glm::vec2 & pointLocation)
{
    const auto visible = isVisible(labelArea.position);
    const auto position = labelArea.origin - pointLocation;
    return {position, Alignment::LeftAligned, LineAnchor::Bottom, visible};
}

}


float overlapArea(int, float overlapArea, RelativeLabelPosition, unsigned int)
{
    return overlapArea;
}
float overlapCount(int overlapCount, float, RelativeLabelPosition, unsigned int)
{
    return overlapCount;
}

float standard(int, float overlapArea, RelativeLabelPosition position, unsigned int priority)
{
    unsigned int positionPenalty = 0;
    switch (position)
    {
        case RelativeLabelPosition::UpperRight: positionPenalty = 0; break;
        case RelativeLabelPosition::UpperLeft:  positionPenalty = 1; break;
        case RelativeLabelPosition::LowerLeft:  positionPenalty = 2; break;
        case RelativeLabelPosition::LowerRight: positionPenalty = 3; break;
        case RelativeLabelPosition::Hidden:     return 0.02f * priority * priority;
        default: assert(false);
    }
    return 15.f * overlapArea + .03f * positionPenalty;
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
    const std::vector<RelativeLabelPosition> positions {
        RelativeLabelPosition::UpperRight, RelativeLabelPosition::UpperLeft,
        RelativeLabelPosition::LowerLeft, RelativeLabelPosition::LowerRight,
        RelativeLabelPosition::Hidden
    };
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, positions.size() - 1);
    for (auto & label : labels)
    {
        const auto extent = Typesetter::extent(label.sequence);
        const auto index = distribution(generator);
        const auto origin = labelOrigin(positions[index], label.pointLocation, extent);
        LabelArea area {origin, extent, positions[index]};
        label.placement = placementFor(area, label.pointLocation);
    }
}

void greedy(std::vector<Label> & labels, PenaltyFunction penaltyFunction, const glm::vec2 & relativePadding)
{
    std::vector<LabelArea> labelAreas;
    const std::vector<RelativeLabelPosition> positions {
        RelativeLabelPosition::UpperRight, RelativeLabelPosition::UpperLeft,
        RelativeLabelPosition::LowerLeft, RelativeLabelPosition::LowerRight,
        RelativeLabelPosition::Hidden
    };
    for (auto & label : labels)
    {
        const auto extent = Typesetter::extent(label.sequence);
        float bestPenalty = std::numeric_limits<float>::max();
        glm::vec2 bestOrigin;
        LabelArea bestLabelArea;
        // find best position for new label
        for (const auto& position : positions)
        {
            const auto origin = labelOrigin(position, label.pointLocation, extent);
            const LabelArea newLabelArea {origin, extent, position};
            float overlapArea = 0.f;
            int overlapCount = 0;
            for (const auto& other : labelAreas)
            {
                overlapArea += newLabelArea.paddedOverlapArea(other, relativePadding);
                overlapCount += newLabelArea.paddedOverlaps(other, relativePadding) ? 1 : 0;
            }
            overlapArea /= newLabelArea.area();
            auto penalty = penaltyFunction(overlapCount, overlapArea, position, label.priority);
            if (penalty < bestPenalty)
            {
                bestPenalty = penalty;
                bestLabelArea = newLabelArea;
            }
        }
        label.placement = placementFor(bestLabelArea, label.pointLocation);
        labelAreas.push_back(bestLabelArea);
    }
}

void discreteGradientDescent(std::vector<Label> & labels, PenaltyFunction penaltyFunction, const glm::vec2 & relativePadding)
{
    const std::vector<RelativeLabelPosition> positions {
        RelativeLabelPosition::UpperRight, RelativeLabelPosition::UpperLeft,
        RelativeLabelPosition::LowerLeft, RelativeLabelPosition::LowerRight,
        RelativeLabelPosition::Hidden
    };

    const std::vector<std::vector<LabelArea>> labelAreas = computeLabelAreas(labels, positions);
    std::vector<unsigned int> chosenLabels = randomStartLabelAreas(labelAreas);
    const auto collisionGraph = createCollisionGraph(labelAreas, relativePadding);
    const auto chosenLabel = [&](unsigned int i) { return labelAreas[i][chosenLabels[i]]; };

    auto localComputePenalty = [&](size_t labelIndex, size_t position) {
        return computePenalty(labelAreas[labelIndex][position], collisionGraph[labelIndex][position],
            labels[labelIndex].priority, penaltyFunction, chosenLabels);
    };

    // upper limit to iterations
    for (int iteration = 0; iteration < 1000; ++iteration)
    {
        float bestImprovement = 0.f;
        int bestLabelIndex = -1;
        int bestLabelPositionIndex = -1;
        size_t labelIndex = 0;

        // find single label change, that yields the largest improvement
        for (auto & singleLabelAreas : labelAreas)
        {
            // find change for a specific label, that yields the largest improvement
            std::vector<float> penalties;
            int bestIndex = 0;
            for (size_t index = 0; index < singleLabelAreas.size(); ++index)
            {
                const auto penalty = localComputePenalty(labelIndex, index);
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

        // execute best change
        chosenLabels[bestLabelIndex] = bestLabelPositionIndex;
    }

    for (size_t i = 0; i < labels.size(); ++i)
    {
        labels[i].placement = placementFor(chosenLabel(i), labels[i].pointLocation);
    }
}

void simulatedAnnealing(std::vector<Label> & labels, PenaltyFunction penaltyFunction, const glm::vec2 & relativePadding)
{
    // based on https://www.eecs.harvard.edu/shieber/Biblio/Papers/tog-final.pdf

    const std::vector<RelativeLabelPosition> positions {
        RelativeLabelPosition::UpperRight, RelativeLabelPosition::UpperLeft,
        RelativeLabelPosition::LowerLeft, RelativeLabelPosition::LowerRight,
        RelativeLabelPosition::Hidden
    };

    const std::vector<std::vector<LabelArea>> labelAreas = computeLabelAreas(labels, positions);
    std::vector<unsigned int> chosenLabels = randomStartLabelAreas(labelAreas);
    const auto collisionGraph = createCollisionGraph(labelAreas, relativePadding);
    const auto chosenLabel = [&](unsigned int i) { return labelAreas[i][chosenLabels[i]]; };

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned int> labelDistribution(0, labels.size() - 1);

    // annealing schedule parameters (taken from original paper)
    const auto startingTemperature = 0.91023922662f;
    const auto maxTemperatureChanges = 50;
    const auto temperatureDecreaseFactor = 0.9f;
    const auto maxChangesAtTemperature = 5 * labels.size();
    const auto maxStepsAtTemperature = 20 * labels.size();

    auto temperature = startingTemperature;
    unsigned int temperatureChanges = 0;
    unsigned int changesAtTemperature = 0;
    unsigned int stepsAtTemperature = 0;

    auto localComputePenalty = [&](size_t labelIndex, size_t position) {
        return computePenalty(labelAreas[labelIndex][position], collisionGraph[labelIndex][position],
            labels[labelIndex].priority, penaltyFunction, chosenLabels);
    };

    while (true)
    {
        // generate a random change
        const auto labelIndex = labelDistribution(generator);
        const auto oldPosition = chosenLabels[labelIndex];
        const auto newPosition = randomIndexExcept<unsigned int>(oldPosition, labelAreas[labelIndex].size(), generator);

        const auto oldPenalty = localComputePenalty(labelIndex, oldPosition);
        const auto newPenalty = localComputePenalty(labelIndex, newPosition);
        const auto improvement = oldPenalty - newPenalty;

        // change is accepted, either
        // 1. ... if it is an improvement
        // 2. ... according to a probability computed from the temperature and how much worse it is
        const float chance = std::exp(improvement / temperature);
        std::bernoulli_distribution doAnyway(chance);
        if (improvement > 0 || doAnyway(generator))
        {
            chosenLabels[labelIndex] = newPosition;
            ++changesAtTemperature;
        }

        // advance annealing schedule
        ++stepsAtTemperature;
        if (changesAtTemperature > maxChangesAtTemperature || stepsAtTemperature > maxStepsAtTemperature)
        {
            // converged
            if (changesAtTemperature == 0) break;
            if (temperatureChanges == maxTemperatureChanges) break;

            temperature *= temperatureDecreaseFactor;
            changesAtTemperature = 0;
            stepsAtTemperature = 0;
            ++temperatureChanges;
        }
    }

    for (size_t i = 0; i < labels.size(); ++i)
    {
        labels[i].placement = placementFor(chosenLabel(i), labels[i].pointLocation);
    }
}

} // namespace layout

} // namespace gloperate_text
