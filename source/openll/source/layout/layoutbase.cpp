#include <openll/layout/layoutbase.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <openll/GlyphSequence.h>


namespace gloperate_text
{

GlyphSequence applyPlacement(const Label & label)
{
    auto sequence = label.sequence;
    sequence.setAlignment(label.placement.alignment);
    sequence.setLineAnchor(label.placement.lineAnchor);
    auto transform = glm::translate(glm::mat4(), glm::vec3(label.placement.location, 0.f));
    transform *= sequence.additionalTransform();
    sequence.setAdditionalTransform(transform);
    return sequence;
}

}
