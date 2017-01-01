#include <openll/layout/LabelArea.h>

#include <cmath>
#include <algorithm>

#include <glm/common.hpp>

namespace gloperate_text
{

bool LabelArea::overlaps(const LabelArea & other) const
{
    return
        origin.x < other.origin.x + other.extent.x &&
        origin.x + extent.x > other.origin.x &&
        origin.y < other.origin.y + other.extent.y &&
        extent.y + origin.y > other.origin.y;
}

float LabelArea::overlapArea(const LabelArea & other) const
{
    const auto lower_left = glm::max(origin, other.origin);
    const auto upper_right = glm::min(origin + extent, other.origin + other.extent);
    return std::max(0.f, upper_right.x - lower_left.x) * std::max(0.f, upper_right.y - lower_left.y);
}

} // namespace gloperate_text
