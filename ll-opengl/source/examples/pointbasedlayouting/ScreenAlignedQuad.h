#pragma once

#include <globjects/base/Referenced.h>
#include <globjects/base/ref_ptr.h>

#include <glm/vec2.hpp>

namespace globjects
{

class VertexArray;
class Program;
class Buffer;
class Texture;
class Shader;

}


class ScreenAlignedQuad
{
public:
    ScreenAlignedQuad(globjects::Texture * texture);

    void setTextureArea(glm::vec2 lowerLeft, glm::vec2 upperRight);
	void draw();

protected:
    void initialize();

protected:
    globjects::ref_ptr<globjects::VertexArray> m_vao;

    globjects::ref_ptr<globjects::Buffer> m_buffer;

    globjects::ref_ptr<globjects::Shader> m_vertexShader;
    globjects::ref_ptr<globjects::Shader> m_fragmentShader;

    globjects::ref_ptr<globjects::Program> m_program;
    globjects::ref_ptr<globjects::Texture> m_texture;

    int m_samplerIndex;

protected:
	static const char * s_defaultVertexShaderSource;
	static const char * s_defaultFagmentShaderSource;
};
