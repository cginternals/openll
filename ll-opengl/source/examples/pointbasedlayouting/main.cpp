
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <random>

#include <cpplocate/cpplocate.h>
#include <cpplocate/ModuleInfo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>

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
#include <openll/RawFile.h>

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
#include "GeoData.h"
#include "ScreenAlignedQuad.h"

#include "datapath.inl"

using namespace gl;

namespace
{
    auto g_size = glm::ivec2{ };
    bool g_config_changed = true;
    size_t g_algorithmID = 0;
    bool g_frames_visible = false;
    long int g_seed = 0;
    int g_numLabels = 64;

    bool g_3D = false;
    std::chrono::time_point<std::chrono::steady_clock> g_startTime;
    glm::mat4 g_mvp;

    bool g_geodata = false;
    bool g_geodataAvailable = false;
    glm::vec2 g_lowerLeftCoords  {-12.f, 35.f};
    glm::vec2 g_upperRightCoords {30.f, 72.f};

    std::unique_ptr<gloperate_text::FontFace> g_font;
    std::unique_ptr<PointDrawable> g_pointDrawable;
    std::unique_ptr<RectangleDrawable> g_rectangleDrawable;
    std::unique_ptr<gloperate_text::GlyphRenderer> g_renderer;
    std::unique_ptr<gloperate_text::GlyphVertexCloud> g_cloud;
    std::unique_ptr<ScreenAlignedQuad> g_quad;
    GeoData cities;

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
    std::uniform_real_distribution<float> x_distribution(-.8f, .8f);
    std::uniform_real_distribution<float> y_distribution(-.8f, .6f);
    std::uniform_real_distribution<float> z_distribution(-1.f, 1.f);
    std::uniform_int_distribution<unsigned int> priorityDistribution(1, 10);
    std::vector<Feature> citiesInArea;
    if (g_geodata)
    {
        citiesInArea = cities.featuresInArea(g_lowerLeftCoords, g_upperRightCoords);
    }

    for (int i = 0; i < g_numLabels; ++i)
    {
        auto string    = random_name(generator);
        auto priority  = priorityDistribution(generator);
        auto origin    = glm::vec3{x_distribution(generator), y_distribution(generator), 0.f};
        auto fontsize  = 10.f + priority;
        auto fontcolor = .5f - priority * .05f;

        if (g_3D)
        {
            glm::vec4 origin3D {glm::vec2(origin), z_distribution(generator), 1.f};
            auto projected = g_mvp * origin3D;
            projected /= projected.w;
            origin = glm::vec3(projected);
            fontsize  = 30.f - projected.z * 15.f;
            fontcolor = .8f * (std::exp(projected.z) - 1) / (glm::e<float>() - 1);
        }

        if (g_geodata)
        {
            string    = citiesInArea.at(i).name;
            priority  = citiesInArea.at(i).population / 50000;
            origin    = glm::vec3(citiesInArea.at(i).location, 0.f);
            fontsize  = 10.f + priority * .05f;
            fontcolor = .8f + priority * 0.02f;
        }

        const std::u32string unicode_string {string.begin(), string.end()};

        gloperate_text::GlyphSequence sequence;
        sequence.setString(unicode_string);
        sequence.setWordWrap(true);
        sequence.setLineWidth(400.f);
        sequence.setAlignment(gloperate_text::Alignment::LeftAligned);
        sequence.setLineAnchor(gloperate_text::LineAnchor::Ascent);
        sequence.setFontSize(fontsize);
        sequence.setFontFace(font);
        sequence.setFontColor(glm::vec4(glm::vec3(fontcolor), 1.f));
        sequence.setSuperSampling(gloperate_text::SuperSampling::Quincunx);

        // compute transform matrix
        glm::mat4 transform;
        transform = glm::translate(transform, origin);
        transform = glm::scale(transform, glm::vec3(1.f, static_cast<float>(viewport.x) / viewport.y, 1.f));
        transform = glm::scale(transform, glm::vec3(1 / 300.f));

        const auto placement = gloperate_text::LabelPlacement{ glm::vec2{ 0.f, 0.f }
            , gloperate_text::Alignment::LeftAligned, gloperate_text::LineAnchor::Baseline, true };

        sequence.setAdditionalTransform(transform);
        labels.push_back({sequence, glm::vec2(origin), priority, placement});
    }
    return labels;
}


gloperate_text::GlyphSequence prepareHeadline(gloperate_text::FontFace * font, glm::ivec2 viewport, const std::string & name)
{

    const std::u32string unicode_string {name.begin(), name.end()};
    const auto origin = glm::vec2{-0.9f, 0.9f};

    gloperate_text::GlyphSequence sequence;
    sequence.setString(unicode_string);
    sequence.setWordWrap(false);
    sequence.setLineWidth(800.f);
    sequence.setAlignment(gloperate_text::Alignment::LeftAligned);
    sequence.setLineAnchor(gloperate_text::LineAnchor::Ascent);
    sequence.setFontSize(30.f);
    sequence.setFontFace(font);
    sequence.setFontColor(glm::vec4(0.4f, 0.4f, 1.0f, 1.f));

    // compute  transform matrix
    glm::mat4 transform;
    transform = glm::translate(transform, glm::vec3(origin, 0.f));
    transform = glm::scale(transform, glm::vec3(1.f, static_cast<float>(viewport.x) / viewport.y, 1.f));
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

    // parameters specify which columns contain data to be loaded
    auto csvValid = cities.loadCSV(dataPath + "/geodata/cities.csv", 1, 2, 3, 4);
    auto raw = gloperate_text::RawFile(dataPath + "/geodata/world.8192.4096.rgba.ub.raw");
    g_geodataAvailable = csvValid && raw.isValid();
    if (g_geodataAvailable)
    {
        auto texture = globjects::Texture::createDefault(GL_TEXTURE_2D);
        texture->ref();
        texture->image2D(0, GL_RGBA, glm::ivec2(8192, 4096), 0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<const GLvoid *>(raw.data()));
        g_quad = std::unique_ptr<ScreenAlignedQuad>(new ScreenAlignedQuad(texture));
    }

    g_startTime = std::chrono::steady_clock::now();

    std::cout
        << "Press 1-9 to choose different layout algorithms" << std::endl
        << "Press Q to use city data"
        << (g_geodataAvailable ? "" : " (currently not available, must be downloaded beforehand)") << std::endl
        << "Press W to use random 2D data (default)" << std::endl
        << "Press E to use random rotating 3D data" << std::endl
        << "Press + and - to increase/decrease number of features" << std::endl
        << "Press F to toggle rendering of frames around labels" << std::endl
        << "Press R to use a different seed for random data" << std::endl;
}

void deinitialize()
{

    globjects::detachAllObjects();
}

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_3D)
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> duration = now - g_startTime;
        auto timePassed = duration.count();

        const auto model = glm::rotate(std::fmod(timePassed / 5, glm::pi<float>() * 2), glm::vec3(0.f, 1.f, 0.f));
        const auto view = glm::lookAt(
            glm::vec3(0.f, 0.f, 1.5f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
        const auto projection = glm::perspective(2.0f, 1.0f, 0.1f, 3.f);

        g_mvp = projection * view * model;
    }

    if (g_config_changed || g_3D)
    {
        glViewport(0, 0, g_size.x, g_size.y);
        if (g_geodata)
        {
            auto width = (g_upperRightCoords - g_lowerLeftCoords).x;
            auto height = width * g_size.y / g_size.x;
            g_upperRightCoords.y = g_lowerLeftCoords.y + height;
            auto texCoords = cities.textureCoordsForArea(g_lowerLeftCoords, g_upperRightCoords);
            g_quad->setTextureArea(texCoords.first, texCoords.second);
        }
        auto labels = prepareLabels(g_font.get(), g_size);
        runAndBenchmark(labels, layoutAlgorithms[g_algorithmID]);
        auto sequences = getSequences(labels);
        sequences.push_back(prepareHeadline(g_font.get(), g_size, layoutAlgorithms[g_algorithmID].name));
        g_cloud->updateWithSequences(sequences, true);
        preparePointDrawable(labels, *g_pointDrawable);
        prepareRectangleDrawable(labels, *g_rectangleDrawable);
    }

    glDepthMask(GL_FALSE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (g_geodata)
    {
        g_quad->render();
    }
    g_pointDrawable->render();
    if (g_frames_visible)
    {
        g_rectangleDrawable->render();
    }
    g_renderer->render(*g_cloud);

    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

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
    else if (key == 'Q' && action == GLFW_PRESS && g_geodataAvailable)
    {
        g_geodata = true;
        g_3D = false;
        g_config_changed = true;
    }
    else if (key == 'W' && action == GLFW_PRESS)
    {
        g_3D = false;
        g_geodata = false;
        g_config_changed = true;
    }
    else if (key == 'E' && action == GLFW_PRESS)
    {
        g_3D = true;
        g_geodata = false;
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
    GLFWwindow * window = glfwCreateWindow(640, 480, "ll-opengl | pointbasedlayouting", nullptr, nullptr);
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
