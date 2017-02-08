
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <random>

#include <cpplocate/cpplocate.h>
#include <cpplocate/ModuleInfo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/gl/extension.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>

#include <GLFW/glfw3.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/base/File.h>

#include <openll/GlyphRenderer.h>
#include <openll/FontLoader.h>
#include <openll/Typesetter.h>

#include <openll/FontFace.h>
#include <openll/GlyphSequence.h>
#include <openll/GlyphVertexCloud.h>
#include <openll/Alignment.h>
#include <openll/LineAnchor.h>
#include <openll/SuperSampling.h>
#include <openll/layout/layoutbase.h>
#include <openll/layout/algorithm.h>

#include "PointDrawable.h"
#include "RectangleDrawable.h"
#include "benchmark.h"

#include "datapath.inl"

using namespace gl;

namespace
{
    auto g_size = glm::ivec2{ };
    bool g_config_changed = true;
    size_t g_algorithmID = 0;
    bool g_frames_visible = true;
    long int g_seed = 0;
    int g_numLabels = 64;
    std::unique_ptr<gloperate_text::FontFace> g_font;
    std::unique_ptr<PointDrawable> g_pointDrawable;
    std::unique_ptr<RectangleDrawable> g_rectangleDrawable;
    std::unique_ptr<gloperate_text::GlyphRenderer> g_renderer;
    std::unique_ptr<gloperate_text::GlyphVertexCloud> g_cloud;

    struct Algorithm
    {
        std::string name;
        std::function<void(std::vector<gloperate_text::Label>&)> function;
    };

    using namespace std::placeholders;

    std::vector<Algorithm> layoutAlgorithms
    {
        {"Constant",                                 gloperate_text::layout::constant},
        {"Random",                                   gloperate_text::layout::random},
        {"Greedy",                                   std::bind(gloperate_text::layout::greedy,                  _1, gloperate_text::layout::standard, glm::vec2(0.2f))},
        {"Discrete Gradient Descent",                std::bind(gloperate_text::layout::discreteGradientDescent, _1, gloperate_text::layout::standard, glm::vec2(0.2f))},
        {"Simulated Annealing",                      std::bind(gloperate_text::layout::simulatedAnnealing,      _1, gloperate_text::layout::standard, glm::vec2(0.f))},
        {"Simulated Annealing with padding",         std::bind(gloperate_text::layout::simulatedAnnealing,      _1, gloperate_text::layout::standard, glm::vec2(0.2f))},
        {"Simulated Annealing with selection",       std::bind(gloperate_text::layout::simulatedAnnealing,      _1, gloperate_text::layout::standard, glm::vec2(0.f))},
        {"Simulated Annealing (padding, selection)", std::bind(gloperate_text::layout::simulatedAnnealing,      _1, gloperate_text::layout::standard, glm::vec2(0.2f))},
    };
}




std::string random_name(std::default_random_engine engine)
{
    std::uniform_int_distribution<int> upperDistribution(65, 90);
    std::uniform_int_distribution<int> lowerDistribution(97, 122);
    std::uniform_int_distribution<int> lengthDistribution(3, 14);
    const auto length = lengthDistribution(engine);
    std::vector<char> characters;
    characters.push_back(static_cast<char>(upperDistribution(engine)));
    for (int i = 0; i < length; ++i)
    {
        characters.push_back(static_cast<char>(lowerDistribution(engine)));
    }
    return {characters.begin(), characters.end()};
}

std::vector<gloperate_text::Label> prepareLabels(gloperate_text::FontFace * font, glm::ivec2 viewport)
{
    std::vector<gloperate_text::Label> labels;

    std::default_random_engine generator;
    generator.seed(g_seed);
    std::uniform_real_distribution<float> y_distribution(-.8f, .6f);
    std::uniform_real_distribution<float> x_distribution(-.8f, .8f);
    std::uniform_int_distribution<unsigned int> priorityDistribution(1, 10);

    for (int i = 0; i < g_numLabels; ++i)
    {
        const auto string = random_name(generator);
        const std::u32string unicode_string {string.begin(), string.end()};
        const auto priority = priorityDistribution(generator);
        const auto origin = glm::vec2{x_distribution(generator), y_distribution(generator)};

        gloperate_text::GlyphSequence sequence;
        sequence.setString(unicode_string);
        sequence.setWordWrap(true);
        sequence.setLineWidth(400.f);
        sequence.setAlignment(gloperate_text::Alignment::LeftAligned);
        sequence.setLineAnchor(gloperate_text::LineAnchor::Ascent);
        sequence.setFontSize(10.f + priority);
        sequence.setFontFace(font);
        sequence.setFontColor(glm::vec4(glm::vec3(0.5f - priority * 0.05f), 1.f));
        sequence.setSuperSampling(gloperate_text::SuperSampling::Quincunx);

        // compute  transform matrix
        glm::mat4 transform;
        transform = glm::translate(transform, glm::vec3(origin, 0.f));
        transform = glm::scale(transform, glm::vec3(1.f,
            static_cast<float>(viewport.x) / viewport.y, 1.f));
        transform = glm::scale(transform, glm::vec3(1 / 300.f));

        const auto placement = gloperate_text::LabelPlacement{ glm::vec2{ 0.f, 0.f }
            , gloperate_text::Alignment::LeftAligned, gloperate_text::LineAnchor::Baseline, true };

        sequence.setAdditionalTransform(transform);
        labels.push_back({sequence, origin, priority, placement});
    }
    return labels;
}


gloperate_text::GlyphSequence prepareHeadline(gloperate_text::FontFace * font, glm::ivec2 viewport, const std::string & name)
{

    const std::u32string unicode_string {name.begin(), name.end()};
    const auto origin = glm::vec2{-0.9f, 0.75f};

    gloperate_text::GlyphSequence sequence;
    sequence.setString(unicode_string);
    sequence.setWordWrap(false);
    sequence.setLineWidth(800.f);
    sequence.setAlignment(gloperate_text::Alignment::LeftAligned);
    sequence.setLineAnchor(gloperate_text::LineAnchor::Descent);
    sequence.setFontSize(30.f);
    sequence.setFontFace(font);
    sequence.setFontColor(glm::vec4(0.4f, 0.4f, 1.0f, 1.f));

    // compute  transform matrix
    glm::mat4 transform;
    transform = glm::translate(transform, glm::vec3(origin, 0.f));
    transform = glm::scale(transform, glm::vec3(1.f,
        static_cast<float>(viewport.x) / viewport.y, 1.f));
    transform = glm::scale(transform, glm::vec3(1 / 300.f));

    sequence.setAdditionalTransform(transform);

    return sequence;
}

std::vector<gloperate_text::GlyphSequence> getSequences(const std::vector<gloperate_text::Label> & labels)
{
    std::vector<gloperate_text::GlyphSequence> sequences;
    for (const auto & label : labels)
    {
        if (label.placement.display)
        {
            sequences.push_back(gloperate_text::applyPlacement(label));
        }
    }
    return sequences;
}

void preparePointDrawable(const std::vector<gloperate_text::Label> & labels, PointDrawable& pointDrawable)
{
    std::vector<Point> points;
    for (const auto & label : labels)
    {
        points.push_back({
            label.pointLocation,
            label.placement.display ? glm::vec3(.7f, .0f, .0f) : glm::vec3(.5f, .5f, .5f),
            (label.placement.display ? 4.f : 2.f) * g_size.x / 640
        });
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
    auto positions = labelPositionDesirability(labels);
    std::cout << "Evaluation results for " << algorithm.name << ":" << std::endl
        << "Runtime:                 " << diff.count() << "s" << std::endl
        << "Labels hidden:           " << labelsHidden(labels) << "/" << labels.size() << std::endl
        << "Overlaps:                " << labelOverlaps(labels) << std::endl
        << "Overlap area:            " << labelOverlapArea(labels) << std::endl
        << "Padding Violations:      " << labelOverlaps(labels, {0.2f, 0.2f}) << std::endl
        << "Padding Overlap:         " << labelOverlapArea(labels, {0.2f, 0.2f}) << std::endl
        << "Relative label positions:" << std::endl
        << "  Upper Right:           " << positions[gloperate_text::RelativeLabelPosition::UpperRight] << std::endl
        << "  Upper Left:            " << positions[gloperate_text::RelativeLabelPosition::UpperLeft]  << std::endl
        << "  Lower Left:            " << positions[gloperate_text::RelativeLabelPosition::LowerLeft]  << std::endl
        << "  Lower Right:           " << positions[gloperate_text::RelativeLabelPosition::LowerRight] << std::endl;
    std::cout << std::endl;
}

void initialize()
{
    glClearColor(1.f, 1.f, 1.f, 1.f);

    const auto dataPath = common::retrieveDataPath("openll", "dataPath");

    gloperate_text::FontLoader loader;
    g_font = std::unique_ptr<gloperate_text::FontFace>(loader.load(dataPath + "/fonts/opensansr36/opensansr36.fnt"));
    g_pointDrawable = std::unique_ptr<PointDrawable>(new PointDrawable(dataPath));
    g_rectangleDrawable = std::unique_ptr<RectangleDrawable>(new RectangleDrawable(dataPath));
    g_renderer = std::unique_ptr<gloperate_text::GlyphRenderer>(new gloperate_text::GlyphRenderer);
    g_cloud = std::unique_ptr<gloperate_text::GlyphVertexCloud>(new gloperate_text::GlyphVertexCloud);
}

void deinitialize()
{

    globjects::detachAllObjects();
}

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_config_changed)
    {
        glViewport(0, 0, g_size.x, g_size.y);
        auto labels = prepareLabels(g_font.get(), g_size);
        runAndBenchmark(labels, layoutAlgorithms[g_algorithmID]);
        auto sequences = getSequences(labels);
        sequences.push_back(prepareHeadline(g_font.get(), g_size, layoutAlgorithms[g_algorithmID].name));
        g_cloud->updateWithSequences(sequences, true);
        preparePointDrawable(labels, *g_pointDrawable);
        prepareRectangleDrawable(labels, *g_rectangleDrawable);
    }

    gl::glDepthMask(gl::GL_FALSE);
    gl::glEnable(gl::GL_CULL_FACE);
    gl::glEnable(gl::GL_BLEND);
    gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

    g_renderer->render(*g_cloud);
    g_pointDrawable->render();
    if (g_frames_visible)
        g_rectangleDrawable->render();

    gl::glDepthMask(gl::GL_TRUE);
    gl::glDisable(gl::GL_CULL_FACE);
    gl::glDisable(gl::GL_BLEND);

    g_config_changed = false;
}


void error(int errnum, const char * errmsg)
{
    globjects::critical() << errnum << ": " << errmsg << std::endl;
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)
{
    g_size = glm::ivec2{ width, height };
    g_config_changed = true;
}

void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
            glfwSetWindowShouldClose(window, 1);
    }
    if (key == GLFW_KEY_F5 && action == GLFW_RELEASE)
    {
        globjects::File::reloadAll();
    }
    else if (key == 'F' && action == GLFW_PRESS)
    {
        g_frames_visible = !g_frames_visible;
    }
    else if (key == 'R' && action == GLFW_PRESS)
    {
        g_seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        g_config_changed = true;
    }
    else if (key == '-' && action == GLFW_PRESS)
    {
        g_numLabels = std::max(g_numLabels - 8, 8);
        g_config_changed = true;
    }
    else if ((key == '+' || key == '=') && action == GLFW_PRESS)
    {
        g_numLabels = std::min(g_numLabels + 8, 1024);
        g_config_changed = true;
    }
    else if ('1' <= key && key <= '9' && action == GLFW_PRESS)
    {
        g_algorithmID = std::min(static_cast<size_t>(key - '1'), layoutAlgorithms.size() - 1);
        g_config_changed = true;
    }
}


int main()
{
#ifdef SYSTEM_DARWIN
    globjects::critical() << "macOS does currently not support compute shader (OpenGL 4.3. required).";
    return 0;
#endif

    // Initialize GLFW
    if (!glfwInit())
        return 1;

    glfwSetErrorCallback(error);
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a context and, if valid, make it current
    GLFWwindow * window = glfwCreateWindow(640, 480, "ll-opengl | minimal-label", nullptr, nullptr);
    if (!window)
    {
        globjects::critical() << "Context creation failed. Terminate execution.";

        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    // Initialize globjects (internally initializes glbinding, and registers the current context)
    globjects::init();

    std::cout << std::endl
        << "OpenGL Version:  " << glbinding::ContextInfo::version() << std::endl
        << "OpenGL Vendor:   " << glbinding::ContextInfo::vendor() << std::endl
        << "OpenGL Renderer: " << glbinding::ContextInfo::renderer() << std::endl << std::endl;

    globjects::DebugMessage::enable();

    // globjects::info() << "Press F5 to reload compute shader." << std::endl << std::endl;


    glfwGetFramebufferSize(window, &g_size[0], &g_size[1]);
    initialize();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        draw();
        glfwSwapBuffers(window);
    }
    deinitialize();

    // Properly shutdown GLFW
    glfwTerminate();

    return 0;
}
