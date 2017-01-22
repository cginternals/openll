#include <openll/layout/RelativeLabelPosition.h>

#include <cassert>

namespace gloperate_text
{

glm::vec2 labelOrigin(RelativeLabelPosition position, glm::vec2 origin, glm::vec2 extent)
{
    switch (position)
    {
    case RelativeLabelPosition::UpperRight: return origin;
    case RelativeLabelPosition::UpperLeft: return origin - glm::vec2(extent.x, 0.f);
    case RelativeLabelPosition::LowerLeft: return origin - extent;
    case RelativeLabelPosition::LowerRight: return origin - glm::vec2(0.f, extent.y);
    default: assert(false);
    }
}

} // namespace gloperate_text
