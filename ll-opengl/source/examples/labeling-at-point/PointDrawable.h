#pragma once

#include <vector>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <globjects/base/Referenced.h>
#include <globjects/base/ref_ptr.h>

namespace globjects
{
    class VertexArray;
    class Program;
}

struct Point
{
    glm::vec2 coords;
    glm::vec3 color;
    float size;
};

class PointDrawable
{
public:
    PointDrawable(const std::string & dataPath);
    ~PointDrawable();

    void initialize(const std::vector<Point> & points);
    void render();

private:
    globjects::ref_ptr<globjects::VertexArray> m_vao;
    globjects::ref_ptr<globjects::Program> m_program;
    int m_count;
};
