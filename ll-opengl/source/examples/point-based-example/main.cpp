#include <cassert>
#include <iostream>
#include <map>

#include <GLFW/glfw3.h>

#include <glbinding/Binding.h>
#include <glbinding/callbacks.h>
#include <glbinding/gl/gl.h>
#include <globjects/globjects.h>

#include <glm/gtc/matrix_transform.hpp>

#include <openll/GlyphRenderer.h>
#include <openll/FontLoader.h>
#include <openll/stages/GlyphPreparationStage.h>

#include <openll/FontFace.h>
#include <openll/GlyphSequence.h>
#include <openll/GlyphSequenceConfig.h>
#include <openll/Alignment.h>
#include <openll/LineAnchor.h>

#include <cpplocate/cpplocate.h>
#include <cpplocate/ModuleInfo.h>

using namespace gl;

glm::uvec2 g_viewport{640, 480};
bool g_viewport_changed = true;

void onResize(GLFWwindow*, int width, int height)
{
    g_viewport = {width, height};
    g_viewport_changed = true;
}

void onKeyPress(GLFWwindow* window, int key, int, int action, int mods)
{
    if (key == 'Q' && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}

void glInitialize()
{
    glbinding::Binding::initialize(false);
    glbinding::setCallbackMaskExcept(
        glbinding::CallbackMask::After | glbinding::CallbackMask::Parameters,
        {"glGetError"});
    glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
        const auto error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cout << error << " in " << call.function->name()
                      << " with parameters:" << std::endl;
            for (const auto& parameter : call.parameters)
            {
                std::cout << "    " << parameter->asString() << std::endl;
            }
        }
    });
    globjects::init();
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
}

gloperate_text::GlyphVertexCloud preparePoint(const glm::vec2 origin, gloperate_text::FontFace * font, glm::uvec2 viewport) {
	const std::string pointText = R"(.)";

	std::vector<gloperate_text::GlyphSequence> sequences;

	gloperate_text::GlyphSequence sequence;
	std::u32string unicode_string(pointText.begin(), pointText.end());
	sequence.setString(unicode_string);

	sequence.setWordWrap(true);
	sequence.setLineWidth(500.f);
	sequence.setAlignment(gloperate_text::Alignment::Centered);
	sequence.setLineAnchor(gloperate_text::LineAnchor::Baseline);
	sequence.setFontFace(font);
	sequence.setFontSize(64.f);

	const glm::vec4 margins{ 0.f, 0.f, 0.f, 0.f };
	const float ppiScale = 1.f;

	// compute  transform matrix
	glm::mat4 transform;
	// translate to lower left in NDC
	transform = glm::translate(transform, glm::vec3(-1.f, -1.f, 0.f));
	// scale glyphs to NDC size
	transform = glm::scale(transform, 2.f / glm::vec3(viewport.x, viewport.y, 1.f));
	// scale glyphs to pixel size with respect to the displays ppi
	transform = glm::scale(transform, glm::vec3(ppiScale));
	// translate to origin in point space - scale origin within
	// margined extend (i.e., viewport with margined areas removed)
	const auto marginedExtent = glm::vec2(viewport.x, viewport.y) / ppiScale
		- glm::vec2(margins[3] + margins[1], margins[2] + margins[0]);
	transform = glm::translate(transform
		, glm::vec3((0.5f * origin + 0.5f) * marginedExtent, 0.f) + glm::vec3(margins[3], margins[2], 0.f));

	sequence.setAdditionalTransform(transform);

	sequences.push_back(sequence);

	return gloperate_text::prepareGlyphs(sequences, true);
}

gloperate_text::GlyphVertexCloud prepareGlyphSequences(const glm::vec2 origin, std::string string, gloperate_text::GlyphSequenceConfig config, glm::uvec2 viewport)
{
	std::vector<gloperate_text::GlyphSequence> sequences;

	gloperate_text::GlyphSequence sequence;
	std::u32string unicode_string(string.begin(), string.end());
	sequence.setString(unicode_string);

	// set wordWrap, lineWidth, alignment, lineAnchor, fontFace, fontSize, fontColor
	sequence.setFromConfig(config);

	const glm::vec4 margins = config.margins();

	// compute  transform matrix
	glm::mat4 transform;
	// translate to lower left in NDC
	transform = glm::translate(transform, glm::vec3(-1.f, -1.f, 0.f));
	// scale glyphs to NDC size
	transform = glm::scale(transform, 2.f / glm::vec3(viewport.x, viewport.y, 1.f));
	// scale glyphs to pixel size with respect to the displays ppi
	transform = glm::scale(transform, glm::vec3(config.ppiScale()));
	// translate to origin in point space - scale origin within
	// margined extend (i.e., viewport with margined areas removed)
	const auto marginedExtent = glm::vec2(viewport.x, viewport.y) / config.ppiScale()
		- glm::vec2(margins[3] + margins[1], margins[2] + margins[0]);
	transform = glm::translate(transform
		, glm::vec3((0.5f * origin + 0.5f) * marginedExtent, 0.f) + glm::vec3(margins[3], margins[2], 0.f));

	sequence.setAdditionalTransform(transform);
	sequences.push_back(sequence);

	return gloperate_text::prepareGlyphs(sequences, true);
}

void annotatePoint(std::vector<gloperate_text::GlyphVertexCloud> & vertexClouds, const glm::vec2 origin, const glm::vec2 offset, std::string string, gloperate_text::GlyphSequenceConfig config, glm::uvec2 viewport)
{
	vertexClouds.push_back(prepareGlyphSequences(origin + offset, string, config, viewport));
	vertexClouds.push_back(preparePoint(origin, config.fontFace(), viewport));
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_AUTO_ICONIFY, 0);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    auto window = glfwCreateWindow(g_viewport.x, g_viewport.y, "Text-Demo", 0, nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glInitialize();

    glfwSetWindowSizeCallback(window, onResize);
    glfwSetKeyCallback(window, onKeyPress);

    cpplocate::ModuleInfo moduleInfo = cpplocate::findModule("openll");
    std::string dataPath = moduleInfo.value("dataPath");

    gloperate_text::FontLoader loader;
    auto font = loader.load(dataPath + "/fonts/opensansr36.fnt");
    gloperate_text::GlyphRenderer renderer;

	auto config = gloperate_text::GlyphSequenceConfig(font);
	
	auto configBigRed = gloperate_text::GlyphSequenceConfig(font);
	configBigRed.setFontColor(glm::vec4(1.f, 0.f, 0.f, 1.f));
	configBigRed.setFontSize(32.f);

	auto configVertical = gloperate_text::GlyphSequenceConfig(font);
	configVertical.setAlignment(gloperate_text::Alignment::Centered);
	configVertical.setAnchor(gloperate_text::LineAnchor::Center);
	configVertical.setWordWrap(true);
	configVertical.setLineWidth(0);

	std::vector<gloperate_text::GlyphVertexCloud> vertexClouds;
    gloperate_text::GlyphVertexCloud cloud, cloud2;
	gloperate_text::GlyphVertexCloud point, point2;
    glClearColor(1.f, 1.f, 1.f, 1.f);

    while (!glfwWindowShouldClose(window))
    {
        if (g_viewport_changed)
        {
            std::cout << "updated viewport (" << g_viewport.x << ", " << g_viewport.y << ")" << std::endl;
            glViewport(0, 0, g_viewport.x, g_viewport.y);

			vertexClouds.clear();

			annotatePoint(vertexClouds, glm::vec2(-0.2f, 0.f), glm::vec2(0.f, 0.1f), "annotated with default config", config, g_viewport);
			annotatePoint(vertexClouds, glm::vec2(0.2f, -0.2f), glm::vec2(0.f, -0.1f), "this one also with default config", config, g_viewport);
			annotatePoint(vertexClouds, glm::vec2(-0.2f, -0.6f), glm::vec2(0.1, 0.f), "is that point clear? hehe point.", config, g_viewport);
			
			annotatePoint(vertexClouds, glm::vec2(-0.8f, 0.8f), glm::vec2(0.1, -0.05f), "annotated with big red config", configBigRed, g_viewport);
			annotatePoint(vertexClouds, glm::vec2(-0.7f, 0.4f), glm::vec2(0.1, -0.05f), "still big and red!", configBigRed, g_viewport);
			
			annotatePoint(vertexClouds, glm::vec2(0.8f, 0.8f), glm::vec2(0.05f, 0.f), "one line vertical", configVertical, g_viewport);
        }

        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

        gl::glDepthMask(gl::GL_FALSE);
        gl::glEnable(gl::GL_CULL_FACE);
        gl::glEnable(gl::GL_BLEND);
        gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < vertexClouds.size(); i++) {
			renderer.render(vertexClouds[i]);
		}

        gl::glDepthMask(gl::GL_TRUE);
        gl::glDisable(gl::GL_CULL_FACE);
        gl::glDisable(gl::GL_BLEND);

        g_viewport_changed = false;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
