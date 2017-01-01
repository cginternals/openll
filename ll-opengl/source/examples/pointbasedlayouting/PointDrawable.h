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

class PointDrawable
{
public:
    PointDrawable(const std::string & dataPath);
    ~PointDrawable();

    void initialize(const std::vector<glm::vec2> & points);
    void render();

private:
    globjects::ref_ptr<globjects::VertexArray> m_vao;
    globjects::ref_ptr<globjects::Program> m_program;
    int m_count;
};
