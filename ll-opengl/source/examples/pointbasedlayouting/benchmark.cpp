#include "benchmark.h"

#include <algorithm>
#include <map>

#include <openll/layout/LabelArea.h>
#include <openll/Typesetter.h>

namespace
{

std::vector<gloperate_text::LabelArea> computeLabelAreas(const std::vector<gloperate_text::Label> & labels)
{
    std::vector<gloperate_text::LabelArea> areas;
    for (const auto& label : labels)
    {
        auto origin = label.pointLocation + label.placement.offset;
        auto extent = gloperate_text::Typesetter::extent(label.sequence);
        const auto position = label.placement.display ?
            gloperate_text::relativeLabelPosition(label.placement.offset, extent) :
            gloperate_text::RelativeLabelPosition::Hidden;
        areas.push_back({origin, extent, position});
    }
    return areas;
}

}

int labelOverlaps(const std::vector<gloperate_text::Label> & labels, const glm::vec2 & relativePadding)
{
    auto areas = computeLabelAreas(labels);
    int counter = 0;
    for (size_t i = 0; i < areas.size(); ++i) {
        for (size_t j = i + 1; j < areas.size(); ++j) {
            if (areas[i].paddedOverlaps(areas[j], relativePadding))
            {
                ++counter;
            }
        }
    }
    return counter;
}

float labelOverlapArea(const std::vector<gloperate_text::Label> & labels, const glm::vec2 & relativePadding)
{
    auto areas = computeLabelAreas(labels);
    float area = 0;
    for (size_t i = 0; i < areas.size(); ++i) {
        for (size_t j = i + 1; j < areas.size(); ++j) {
            area += areas[i].paddedOverlapArea(areas[j], relativePadding);
        }
    }
    return area;
}

int labelsHidden(const std::vector<gloperate_text::Label> & labels)
{
    return std::count_if(labels.begin(), labels.end(), [](const gloperate_text::Label& label)
    {
        return !label.placement.display;
    });
}

std::map<gloperate_text::RelativeLabelPosition, unsigned int> labelPositionDesirability(const std::vector<gloperate_text::Label> & labels)
{
    std::map<gloperate_text::RelativeLabelPosition, unsigned int> result
    {
        {gloperate_text::RelativeLabelPosition::UpperRight, 0},
        {gloperate_text::RelativeLabelPosition::UpperLeft, 0},
        {gloperate_text::RelativeLabelPosition::LowerLeft, 0},
        {gloperate_text::RelativeLabelPosition::LowerRight, 0}
    };
    for (const auto & label : labels)
    {
        if (!label.placement.display) continue;
        const auto extent = gloperate_text::Typesetter::extent(label.sequence);
        const auto position = gloperate_text::relativeLabelPosition(label.placement.offset, extent);
        ++result[position];
    }
    return result;
}
