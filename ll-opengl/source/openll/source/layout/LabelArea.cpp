#include <openll/layout/LabelArea.h>

#include <cmath>
#include <algorithm>

#include <glm/common.hpp>

namespace gloperate_text
{

bool LabelArea::overlaps(const LabelArea & other) const
{
    if (position == RelativeLabelPosition::Hidden || other.position == RelativeLabelPosition::Hidden)
        return false;
    const auto upperRight = origin + extent;
    const auto otherUpperRight = other.origin + other.extent;
    return
        origin.x     < otherUpperRight.x &&
        upperRight.x > other.origin.x    &&
        origin.y     < otherUpperRight.y &&
        upperRight.y > other.origin.y;
}

bool LabelArea::paddedOverlaps(const LabelArea & other, const glm::vec2 & relativePadding) const
{
    if (position == RelativeLabelPosition::Hidden || other.position == RelativeLabelPosition::Hidden)
        return false;
    const auto aLowerLeft = origin - extent * relativePadding;
    const auto bLowerLeft = other.origin - other.extent * relativePadding;
    const auto aUpperRight = origin + extent * (relativePadding + 1.f);
    const auto bUpperRight = other.origin + other.extent * (relativePadding + 1.f);
    return
        aLowerLeft.x  < bUpperRight.x  &&
        aUpperRight.x > bLowerLeft.x &&
        aLowerLeft.y  < bUpperRight.y  &&
        aUpperRight.y > bLowerLeft.y;
}

float LabelArea::overlapArea(const LabelArea & other) const
{
    if (position == RelativeLabelPosition::Hidden || other.position == RelativeLabelPosition::Hidden)
        return 0.f;
    const auto lower_left = glm::max(origin, other.origin);
    const auto upper_right = glm::min(origin + extent, other.origin + other.extent);
    return std::max(0.f, upper_right.x - lower_left.x) * std::max(0.f, upper_right.y - lower_left.y);
}

float LabelArea::paddedOverlapArea(const LabelArea & other, const glm::vec2 & relativePadding) const
{
    if (position == RelativeLabelPosition::Hidden || other.position == RelativeLabelPosition::Hidden)
        return 0.f;
    const auto lower_left  = glm::max(origin - extent * relativePadding,         other.origin - other.extent * relativePadding);
    const auto upper_right = glm::min(origin + extent * (relativePadding + 1.f), other.origin + other.extent * (relativePadding + 1.f));
    return std::max(0.f, upper_right.x - lower_left.x) * std::max(0.f, upper_right.y - lower_left.y);
}

float LabelArea::area() const
{
    return extent.x * extent.y;
}

} // namespace gloperate_text
