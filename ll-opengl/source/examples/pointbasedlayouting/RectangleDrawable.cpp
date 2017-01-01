#include "RectangleDrawable.h"

#include <glm/vec2.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/Program.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Buffer.h>
#include <globjects/Shader.h>

using namespace gl;

RectangleDrawable::RectangleDrawable(const std::string & dataPath)
:   m_count(-1)
{
    auto vert = globjects::Shader::fromFile(GL_VERTEX_SHADER, dataPath + "/examples/rectangles.vert");
    auto frag = globjects::Shader::fromFile(GL_FRAGMENT_SHADER, dataPath + "/examples/rectangles.frag");

    m_program = new globjects::Program;
    m_program->attach(vert, frag);
    m_program->use();
    m_program->release();
}

RectangleDrawable::~RectangleDrawable()
{
}

void RectangleDrawable::initialize(const std::vector<glm::vec2> & rectangles)
{

    m_vao = new globjects::VertexArray;

    std::vector<glm::vec2> data;
    for (size_t i = 0; i < rectangles.size(); i += 2)
    {
        data.push_back(rectangles[i]);
        data.push_back({rectangles[i].x, rectangles[i+1].y});
        data.push_back({rectangles[i].x, rectangles[i+1].y});
        data.push_back(rectangles[i+1]);
        data.push_back(rectangles[i+1]);
        data.push_back({rectangles[i+1].x, rectangles[i].y});
        data.push_back({rectangles[i+1].x, rectangles[i].y});
        data.push_back(rectangles[i]);
    }
    m_count = data.size();

    auto buffer = new globjects::Buffer();
    buffer->setData(data, GL_STATIC_DRAW);

    auto binding = m_vao->binding(0);
    binding->setAttribute(0);
    binding->setBuffer(buffer, 0, sizeof(glm::vec2));
    binding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);
}

void RectangleDrawable::render()
{
    glEnable(GL_LINE_SMOOTH);
    m_program->use();
    m_vao->drawArrays(GL_LINES, 0, m_count);
    m_program->release();
}
