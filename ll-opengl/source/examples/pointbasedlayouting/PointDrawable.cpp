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

void PointDrawable::initialize(const std::vector<glm::vec2> & points)
{
    m_count = points.size();

    m_vao = new globjects::VertexArray;

    auto buffer = new globjects::Buffer();
    buffer->setData(points, GL_STATIC_DRAW);

    auto binding = m_vao->binding(0);
    binding->setAttribute(0);
    binding->setBuffer(buffer, 0, sizeof(glm::vec2));
    binding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);
}

void PointDrawable::render()
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    m_program->use();
    m_vao->drawArrays(GL_POINTS, 0, m_count);
    m_program->release();
}
