#include "benchmark.h"

#include <algorithm>

#include <openll/layout/LabelArea.h>
#include <openll/Typesetter.h>

namespace
{

std::vector<gloperate_text::LabelArea> computeLabelAreas(const std::vector<gloperate_text::Label> & labels)
{
    std::vector<gloperate_text::LabelArea> areas;
    for (const auto& label : labels)
    {
        if (!label.placement.display) continue;
        auto origin = label.pointLocation + label.placement.offset;
        auto extent = gloperate_text::Typesetter::extent(label.sequence);
        areas.push_back({origin, extent});
    }
    return areas;
}

}

int labelOverlaps(const std::vector<gloperate_text::Label> & labels)
{
    auto areas = computeLabelAreas(labels);
    int counter = 0;
    for (size_t i = 0; i < areas.size(); ++i) {
        for (size_t j = i + 1; j < areas.size(); ++j) {
            if (areas[i].overlaps(areas[j]))
            {
                ++counter;
            }
        }
    }
    return counter;
}

float labelOverlapArea(const std::vector<gloperate_text::Label> & labels)
{
    auto areas = computeLabelAreas(labels);
    float area = 0;
    for (size_t i = 0; i < areas.size(); ++i) {
        for (size_t j = i + 1; j < areas.size(); ++j) {
            area += areas[i].overlapArea(areas[j]);
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
