
#pragma once

#include <glm/vec2.hpp>

#include <openll/openll_api.h>

namespace gloperate_text
{


enum class RelativeLabelPosition : unsigned char
{
    UpperRight, UpperLeft, LowerRight, LowerLeft
};

glm::vec2 OPENLL_API labelOrigin(RelativeLabelPosition position, const glm::vec2 & origin, const glm::vec2 & extent);
RelativeLabelPosition OPENLL_API relativeLabelPosition(const glm::vec2 & offset, const glm::vec2 & extent);


} // namespace gloperate_text
