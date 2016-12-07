#pragma once

#include <vector>
#include <string>

#include <glm/fwd.hpp>

#include <globjects/base/Referenced.h>
#include <globjects/base/ref_ptr.h>

namespace globjects
{
    class VertexArray;
    class Program;
}

class RectangleDrawable
{
public:
    RectangleDrawable(const std::string& dataPath);
    ~RectangleDrawable();

    void initialize(const std::vector<glm::vec2>& rectangles);
    void render();

private:
    globjects::ref_ptr<globjects::VertexArray> m_vao;
    globjects::ref_ptr<globjects::Program> m_program;
    int m_count;
};
