#include <openll/layout/RelativeLabelPosition.h>

#include <cassert>

namespace gloperate_text
{

glm::vec2 labelOrigin(RelativeLabelPosition position, const glm::vec2 & origin, const glm::vec2 & extent)
{
    switch (position)
    {
    case RelativeLabelPosition::UpperRight: return origin;
    case RelativeLabelPosition::UpperLeft:  return origin - glm::vec2(extent.x, 0.f);
    case RelativeLabelPosition::LowerLeft:  return origin - extent;
    case RelativeLabelPosition::LowerRight: return origin - glm::vec2(0.f, extent.y);
    case RelativeLabelPosition::Hidden:     return origin;
    default: assert(false);
    }
}

bool OPENLL_API isVisible(RelativeLabelPosition position)
{
    return position != RelativeLabelPosition::Hidden;
}

RelativeLabelPosition relativeLabelPosition(const glm::vec2 & offset, const glm::vec2 & extent)
{
    const auto midpointOffset = offset + extent / 2.f;
    if (midpointOffset.x > 0 && midpointOffset.y > 0) return RelativeLabelPosition::UpperRight;
    if (midpointOffset.x < 0 && midpointOffset.y > 0) return RelativeLabelPosition::UpperLeft;
    if (midpointOffset.x < 0 && midpointOffset.y < 0) return RelativeLabelPosition::LowerLeft;
    if (midpointOffset.x > 0 && midpointOffset.y < 0) return RelativeLabelPosition::LowerRight;
    assert(false);
}

} // namespace gloperate_text
