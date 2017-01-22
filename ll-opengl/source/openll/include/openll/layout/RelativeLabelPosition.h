
#pragma once

#include <glm/vec2.hpp>


namespace gloperate_text
{


enum class RelativeLabelPosition : unsigned char
{
    UpperRight, UpperLeft, LowerRight, LowerLeft
};

glm::vec2 labelOrigin(RelativeLabelPosition position, glm::vec2 origin, glm::vec2 extent);


} // namespace gloperate_text
