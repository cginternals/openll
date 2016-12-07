#include <cassert>
#include <iostream>
#include <map>
#include <random>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glbinding/Binding.h>
#include <glbinding/callbacks.h>
#include <glbinding/gl/gl.h>
#include <globjects/globjects.h>

#include <glm/gtc/matrix_transform.hpp>

#include <openll/GlyphRenderer.h>
#include <openll/FontLoader.h>
#include <openll/Typesetter.h>
#include <openll/stages/GlyphPreparationStage.h>

#include <openll/FontFace.h>
#include <openll/GlyphSequence.h>
#include <openll/Alignment.h>
#include <openll/LineAnchor.h>
#include <openll/layout/layoutbase.h>
#include <openll/layout/algorithm.h>

#include <cpplocate/cpplocate.h>
#include <cpplocate/ModuleInfo.h>

#include "PointDrawable.h"
#include "RectangleDrawable.h"

using namespace gl;

const auto lorem =
R"(Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.)";

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


std::string random_name(std::default_random_engine engine)
{
    std::uniform_int_distribution<char> charDistribution(32, 126);
    std::uniform_int_distribution<int> lengthDistribution(3, 15);
    const auto length = lengthDistribution(engine);
    std::vector<char> characters;
    for (int i = 0; i < length; ++i)
    {
        characters.push_back(charDistribution(engine));
    }
    return {characters.begin(), characters.end()};
}

std::vector<gloperate_text::Label> prepareLabels(gloperate_text::FontFace * font, glm::uvec2 viewport)
{
    std::vector<gloperate_text::Label> labels;

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-1.f, 1.f);

    for (int i = 0; i < 10; ++i)
    {
        auto string = random_name(generator);
        gloperate_text::GlyphSequence sequence;
        std::u32string unicode_string(string.begin(), string.end());
        sequence.setString(unicode_string);
        sequence.setWordWrap(true);
        sequence.setLineWidth(200.f);
        sequence.setAlignment(gloperate_text::Alignment::LeftAligned);
        sequence.setLineAnchor(gloperate_text::LineAnchor::Ascent);
        sequence.setFontSize(16.f);
        sequence.setFontFace(font);

        const glm::vec2 origin {distribution(generator), distribution(generator)};
        // compute  transform matrix
        glm::mat4 transform;
        transform = glm::translate(transform, glm::vec3(origin, 0.f));
        transform = glm::scale(transform, glm::vec3(1.f,
            static_cast<float>(viewport.x) / viewport.y, 1.f));
        transform = glm::scale(transform, glm::vec3(1/300.f));

        sequence.setAdditionalTransform(transform);
        labels.push_back({sequence, origin});
    }
    return labels;
}

gloperate_text::GlyphVertexCloud prepareCloud(const std::vector<gloperate_text::Label>& labels)
{
    std::vector<gloperate_text::GlyphSequence> sequences;
    for (const auto & label : labels)
    {
        sequences.push_back(gloperate_text::applyPlacement(label));
    }
    return gloperate_text::prepareGlyphs(sequences, true);
}

void preparePointDrawable(const std::vector<gloperate_text::Label>& labels, PointDrawable& pointDrawable)
{
    std::vector<glm::vec2> points;
    for (const auto & label : labels)
    {
        points.push_back(label.pointLocation);
    }
    pointDrawable.initialize(points);
}

void prepareRectangleDrawable(const std::vector<gloperate_text::Label>& labels, RectangleDrawable& rectangleDrawable)
{
    std::vector<glm::vec2> rectangles;
    for (const auto & label : labels)
    {
        rectangles.push_back(label.pointLocation);
        auto extent = gloperate_text::Typesetter::extent(label.sequence);
        rectangles.push_back(label.pointLocation + extent);
    }
    rectangleDrawable.initialize(rectangles);
}

int main()
{
    glfwInit();
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
    gloperate_text::GlyphVertexCloud cloud;
    std::vector<gloperate_text::Label> labels;
    PointDrawable pointDrawable {dataPath};
    RectangleDrawable rectangleDrawable {dataPath};
    glClearColor(1.f, 1.f, 1.f, 1.f);


    while (!glfwWindowShouldClose(window))
    {
        if (g_viewport_changed)
        {
            std::cout << "updated viewport (" << g_viewport.x << ", " << g_viewport.y << ")" << std::endl;
            glViewport(0, 0, g_viewport.x, g_viewport.y);
            labels = prepareLabels(font, g_viewport);
            gloperate_text::constantLayout(labels);
            cloud = prepareCloud(labels);
            preparePointDrawable(labels, pointDrawable);
            prepareRectangleDrawable(labels, rectangleDrawable);
        }

        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);

        gl::glDepthMask(gl::GL_FALSE);
        gl::glEnable(gl::GL_CULL_FACE);
        gl::glEnable(gl::GL_BLEND);
        gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

        renderer.render(cloud);
        pointDrawable.render();
        rectangleDrawable.render();

        gl::glDepthMask(gl::GL_TRUE);
        gl::glDisable(gl::GL_CULL_FACE);
        gl::glDisable(gl::GL_BLEND);

        g_viewport_changed = false;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
