#include <cassert>
#include <iostream>
#include <map>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glbinding/Binding.h>
#include <glbinding/callbacks.h>
#include <glbinding/gl/gl.h>
#include <globjects/globjects.h>

#include <openll/GlyphRenderer.h>
#include <openll/FontLoader.h>
#include <openll/stages/GlyphPreparationStage.h>

#include <openll/FontFace.h>
#include <openll/GlyphSequence.h>
#include <openll/Alignment.h>
#include <openll/LineAnchor.h>

#include <cpplocate/cpplocate.h>
#include <cpplocate/ModuleInfo.h>

using namespace gl;

//const auto lorem =
//R"(Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.)";

glm::uvec2 g_viewport{640, 480};
bool g_viewport_changed = true;

void onResize(GLFWwindow* window, int width, int height)
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

	const auto fontSize = 64.f;
	const auto pixelPerInch = 72.f;

	const glm::vec4 margins{ 0.f, 0.f, 0.f, 0.f };
	sequence.setWordWrap(true);
	sequence.setLineWidth(500.f, fontSize, *font);
	sequence.setAlignment(gloperate_text::Alignment::Centered);
	sequence.setLineAnchor(gloperate_text::LineAnchor::Baseline);

	sequence.setTransform(origin, fontSize, *font, viewport, pixelPerInch, margins);
	sequences.push_back(sequence);

	return gloperate_text::prepareGlyphs(font, sequences, true);
}


gloperate_text::GlyphVertexCloud prepareGlyphSequences(const glm::vec2 origin, std::string string, gloperate_text::FontFace * font, const float fontSize, glm::uvec2 viewport)
{
	std::vector<gloperate_text::GlyphSequence> sequences;

	// font->setLinespace(1.25f);
	gloperate_text::GlyphSequence sequence;
	std::u32string unicode_string(string.begin(), string.end());
	sequence.setString(unicode_string);

	//const auto fontSize = 32.f;
	const auto pixelPerInch = 72.f;
	//const glm::vec2 origin{ 0.f, .5f };
	const glm::vec4 margins{ 0.f, 0.f, 0.f, 0.f };
	sequence.setWordWrap(true);
	sequence.setLineWidth(500.f, fontSize, *font);
	sequence.setAlignment(gloperate_text::Alignment::Centered);
	sequence.setLineAnchor(gloperate_text::LineAnchor::Baseline);

	sequence.setTransform(origin, fontSize, *font, viewport, pixelPerInch, margins);
	sequences.push_back(sequence);

	//sequences.data()[0].setTransform(origin.data(), fontSize.data(), *font.data()
	//    , { viewport.data()->width(), viewport.data()->height() });

	return gloperate_text::prepareGlyphs(font, sequences, true);
}

void annotatePoint(std::vector<gloperate_text::GlyphVertexCloud> & vertexClouds, const glm::vec2 origin, const glm::vec2 offset, std::string string, gloperate_text::FontFace * font, const float fontSize, glm::uvec2 viewport)
{
	vertexClouds.push_back(prepareGlyphSequences(origin+offset, string, font, fontSize, viewport));
	vertexClouds.push_back(preparePoint(origin, font, viewport));
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

			annotatePoint(vertexClouds, glm::vec2(-0.2f, 0), glm::vec2(0, 0.2f), "big Annotation for Point below", font, 32.f, g_viewport);
			annotatePoint(vertexClouds, glm::vec2(0.2f, -0.2), glm::vec2(0, -0.2), "small Annotation for Point above", font, 16.f, g_viewport);
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
