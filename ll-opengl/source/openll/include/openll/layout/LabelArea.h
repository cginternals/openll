#pragma once

#include <glm/vec2.hpp>

#include <openll/openll_api.h>
#include <openll/layout/RelativeLabelPosition.h>

namespace gloperate_text
{

struct OPENLL_API LabelArea
{
public:
    bool overlaps(const LabelArea & other) const;
    bool paddedOverlaps(const LabelArea & other, const glm::vec2 & relativePadding) const;
    float overlapArea(const LabelArea & other) const;
    float paddedOverlapArea(const LabelArea & other, const glm::vec2 & relativePadding) const;
    float area() const;

public:
    glm::vec2 origin;
    glm::vec2 extent;
    RelativeLabelPosition position;
};

} // namespace gloperate_text
