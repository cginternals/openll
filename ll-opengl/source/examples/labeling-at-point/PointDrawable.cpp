#include "PointDrawable.h"

#include <glm/vec2.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Program.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>
#include <globjects/Shader.h>

using namespace gl;


namespace
{

template <typename Class, typename Type>
std::ptrdiff_t offset(Type Class::*member)
{
    return reinterpret_cast<std::ptrdiff_t>(&(((Class*)0)->*member));
}

}

PointDrawable::PointDrawable(const std::string & dataPath)
:   m_count(-1)
{
    auto vert = globjects::Shader::fromFile(GL_VERTEX_SHADER, dataPath + "/examples/points.vert");
    auto frag = globjects::Shader::fromFile(GL_FRAGMENT_SHADER, dataPath + "/examples/points.frag");

    m_program = new globjects::Program;
    m_program->attach(vert, frag);
    m_program->use();
    m_program->release();
}

PointDrawable::~PointDrawable()
{
}

void PointDrawable::initialize(const std::vector<Point> & points)
{
    m_count = points.size();

    m_vao = new globjects::VertexArray;

    auto buffer = new globjects::Buffer();
    buffer->setData(points, GL_STATIC_DRAW);

    auto binding = m_vao->binding(0);
    binding->setAttribute(0);
    binding->setBuffer(buffer, 0, sizeof(Point));
    binding->setFormat(2, GL_FLOAT, GL_FALSE, offset(&Point::coords));

    binding = m_vao->binding(1);
    binding->setAttribute(1);
    binding->setBuffer(buffer, 0, sizeof(Point));
    binding->setFormat(3, GL_FLOAT, GL_FALSE, offset(&Point::color));

    binding = m_vao->binding(2);
    binding->setAttribute(2);
    binding->setBuffer(buffer, 0, sizeof(Point));
    binding->setFormat(1, GL_FLOAT, GL_FALSE, offset(&Point::size));

    m_vao->enable(0);
    m_vao->enable(1);
    m_vao->enable(2);
}

void PointDrawable::render()
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    m_program->use();
    m_vao->drawArrays(GL_POINTS, 0, m_count);
    m_program->release();
}
