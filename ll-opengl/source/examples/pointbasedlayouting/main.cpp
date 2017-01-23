#include <cassert>
#include <iostream>
#include <map>
#include <random>

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
#include "benchmark.h"

using namespace gl;

glm::uvec2 g_viewport{640, 480};
bool g_config_changed = true;
size_t g_algorithmID = 0;

struct Algorithm
{
    std::string name;
    std::function<void(std::vector<gloperate_text::Label>&)> function;
};

using namespace std::placeholders;

std::vector<Algorithm> layoutAlgorithms
{
    {"constant",                          gloperate_text::layout::constant},
    {"random",                            gloperate_text::layout::random},
    {"greedy",                            std::bind(gloperate_text::layout::greedy, _1, gloperate_text::layout::overlapCount)},
    {"greedy with area",                  std::bind(gloperate_text::layout::greedy, _1, gloperate_text::layout::overlapArea)},
    {"discreteGradientDescent",           std::bind(gloperate_text::layout::discreteGradientDescent, _1, gloperate_text::layout::overlapCount)},
    {"discreteGradientDescent with area", std::bind(gloperate_text::layout::discreteGradientDescent, _1, gloperate_text::layout::overlapArea)},
    {"simulatedAnnealing with area",      std::bind(gloperate_text::layout::simulatedAnnealing, _1, gloperate_text::layout::overlapArea, false)},
    {"simulatedAnnealing",                std::bind(gloperate_text::layout::simulatedAnnealing, _1, gloperate_text::layout::standard, false)},
    {"simulatedAnnealing with selection", std::bind(gloperate_text::layout::simulatedAnnealing, _1, gloperate_text::layout::standard, true)},
};

void onResize(GLFWwindow*, int width, int height)
{
    g_viewport = {width, height};
    g_config_changed = true;
}

void onKeyPress(GLFWwindow * window, int key, int, int action, int mods)
{
    if (key == 'Q' && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL)
    {
        glfwSetWindowShouldClose(window, 1);
    }
    else if ('1' <= key && key <= '9' && action == GLFW_PRESS)
    {
        g_algorithmID = std::min(static_cast<size_t>(key - '1'), layoutAlgorithms.size() - 1);
        g_config_changed = true;
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
    std::uniform_int_distribution<int> charDistribution(32, 126);
    std::uniform_int_distribution<int> lengthDistribution(3, 15);
    const auto length = lengthDistribution(engine);
    std::vector<char> characters;
    for (int i = 0; i < length; ++i)
    {
        characters.push_back(static_cast<char>(charDistribution(engine)));
    }
    return {characters.begin(), characters.end()};
}

std::vector<gloperate_text::Label> prepareLabels(gloperate_text::FontFace * font, glm::uvec2 viewport)
{
    std::vector<gloperate_text::Label> labels;

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-1.f, 1.f);
    std::uniform_int_distribution<unsigned int> priorityDistribution(1, 10);

    for (int i = 0; i < 50; ++i)
    {
        const auto string = random_name(generator);
        const auto priority = priorityDistribution(generator);
        gloperate_text::GlyphSequence sequence;
        std::u32string unicode_string(string.begin(), string.end());
        sequence.setString(unicode_string);
        sequence.setWordWrap(true);
        sequence.setLineWidth(400.f);
        sequence.setAlignment(gloperate_text::Alignment::LeftAligned);
        sequence.setLineAnchor(gloperate_text::LineAnchor::Ascent);
        sequence.setFontSize(10.f + priority);
        sequence.setFontFace(font);
        sequence.setFontColor(glm::vec4(glm::vec3(0.5f - priority * 0.05f), 1.f));

        const auto origin = glm::vec2{distribution(generator), distribution(generator)};
        // compute  transform matrix
        glm::mat4 transform;
        transform = glm::translate(transform, glm::vec3(origin, 0.f));
        transform = glm::scale(transform, glm::vec3(1.f,
            static_cast<float>(viewport.x) / viewport.y, 1.f));
        transform = glm::scale(transform, glm::vec3(1 / 300.f));

        const auto placement = gloperate_text::LabelPlacement{ glm::vec2{ 0.f, 0.f }
            , gloperate_text::Alignment::LeftAligned, gloperate_text::LineAnchor::Baseline, true };

        sequence.setAdditionalTransform(transform);
        labels.push_back({sequence, origin, priority, placement });
    }
    return labels;
}

gloperate_text::GlyphVertexCloud prepareCloud(const std::vector<gloperate_text::Label> & labels)
{
    std::vector<gloperate_text::GlyphSequence> sequences;
    for (const auto & label : labels)
    {
        if (label.placement.display)
        {
            sequences.push_back(gloperate_text::applyPlacement(label));
        }
    }
    return gloperate_text::prepareGlyphs(sequences, true);
}

void preparePointDrawable(const std::vector<gloperate_text::Label> & labels, PointDrawable& pointDrawable)
{
    std::vector<glm::vec2> points;
    for (const auto & label : labels)
    {
        points.push_back(label.pointLocation);
    }
    pointDrawable.initialize(points);
}

void prepareRectangleDrawable(const std::vector<gloperate_text::Label> & labels, RectangleDrawable& rectangleDrawable)
{
    std::vector<glm::vec2> rectangles;
    for (const auto & label : labels)
    {
        auto sequence = gloperate_text::applyPlacement(label);
        auto extent = gloperate_text::Typesetter::rectangle(sequence, glm::vec3(label.pointLocation, 0.f));
        rectangles.push_back(extent.first);
        rectangles.push_back(extent.first + extent.second);
    }
    rectangleDrawable.initialize(rectangles);
}

void runAndBenchmark(std::vector<gloperate_text::Label> & labels, Algorithm algorithm)
{
    auto start = std::chrono::steady_clock::now();
    algorithm.function(labels);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Evaluation results for " << algorithm.name << ":" << std::endl
        << "Runtime:          " << diff.count() << "s" << std::endl
        << "Labels hidden:    " << labelsHidden(labels) << "/" << labels.size() << std::endl
        << "Overlaps:         " << labelOverlaps(labels) << std::endl
        << "Overlap area:     " << labelOverlapArea(labels) << std::endl
        << "Relative label positions:" << std::endl;
    auto positions = labelPositionDesirability(labels);
    std::cout
        << "  Upper Right:    " << positions[gloperate_text::RelativeLabelPosition::UpperRight] << std::endl
        << "  Upper Left:     " << positions[gloperate_text::RelativeLabelPosition::UpperLeft] << std::endl
        << "  Lower Right:    " << positions[gloperate_text::RelativeLabelPosition::LowerRight] << std::endl
        << "  Lower Left:     " << positions[gloperate_text::RelativeLabelPosition::LowerLeft] << std::endl;
    std::cout << std::endl;
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
    auto font = loader.load(dataPath + "/fonts/opensansr36/opensansr36.fnt");
    gloperate_text::GlyphRenderer renderer;
    gloperate_text::GlyphVertexCloud cloud;
    std::vector<gloperate_text::Label> labels;
    PointDrawable pointDrawable {dataPath};
    RectangleDrawable rectangleDrawable {dataPath};
    glClearColor(1.f, 1.f, 1.f, 1.f);


    while (!glfwWindowShouldClose(window))
    {
        if (g_config_changed)
        {
            glViewport(0, 0, g_viewport.x, g_viewport.y);
            labels = prepareLabels(font, g_viewport);
            runAndBenchmark(labels, layoutAlgorithms[g_algorithmID]);
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

        g_config_changed = false;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
