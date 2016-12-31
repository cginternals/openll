#pragma once

#include <glm/vec2.hpp>

#include <openll/openll_api.h>

namespace gloperate_text
{

struct OPENLL_API LabelArea
{
public:
    bool overlaps(const LabelArea& other) const;
    float overlapArea(const LabelArea& other) const;

public:
    glm::vec2 origin;
    glm::vec2 extent;
};

} // namespace gloperate_text
