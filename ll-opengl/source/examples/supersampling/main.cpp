
#include <iostream>
#include <algorithm>
#include <memory>

#include <cpplocate/cpplocate.h>
#include <cpplocate/ModuleInfo.h>

#include <glm/gtc/constants.hpp>
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

#include <globjects/Program.h>
#include <globjects/Shader.h>


#include "datapath.inl"


using namespace gl;


namespace
{
    auto g_size = glm::ivec2{ };
    bool g_config_changed = true;
    size_t g_algorithmID = 0;
    std::unique_ptr<gloperate_text::FontFace> g_font36;
    std::unique_ptr<gloperate_text::FontFace> g_font72;
    std::unique_ptr<gloperate_text::FontFace> g_font144;
    std::unique_ptr<gloperate_text::GlyphRenderer> g_renderer;
    std::unique_ptr<gloperate_text::GlyphVertexCloud> g_cloud;
}

void initialize()
{
    glClearColor(1.f, 1.f, 1.f, 1.f);

    const auto dataPath = common::retrieveDataPath("openll", "dataPath");

    gloperate_text::FontLoader loader;
    g_font = std::unique_ptr<gloperate_text::FontFace>(loader.load(dataPath + "/fonts/opensansr36/opensansr36.fnt"));
    g_font = std::unique_ptr<gloperate_text::FontFace>(loader.load(dataPath + "/fonts/opensansr72/opensansr72.fnt"));
    g_font = std::unique_ptr<gloperate_text::FontFace>(loader.load(dataPath + "/fonts/opensansr144/opensansr144.fnt"));
    g_renderer = std::unique_ptr<gloperate_text::GlyphRenderer>(new gloperate_text::GlyphRenderer);
    g_cloud = std::unique_ptr<gloperate_text::GlyphVertexCloud>(new gloperate_text::GlyphVertexCloud);
}

void deinitialize()
{
    globjects::detachAllObjects();
}

std::vector<gloperate_text::GlyphSequence> prepareSequences(gloperate_text::FontFace * font, glm::ivec2 viewport)
{
    std::vector<gloperate_text::GlyphSequence> labels;

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

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_config_changed)
    {
        glViewport(0, 0, g_size.x, g_size.y);
        auto sequences = prepareSequences(g_font.get(), g_size);
        g_cloud->updateWithSequences(sequences, true);
    }

    gl::glDepthMask(gl::GL_FALSE);
    gl::glEnable(gl::GL_CULL_FACE);
    gl::glEnable(gl::GL_BLEND);
    gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

    g_renderer->render(*g_cloud);

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
        glfwSetWindowShouldClose(window, 1);

    if (key == GLFW_KEY_F5 && action == GLFW_RELEASE)
        globjects::File::reloadAll();
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
    GLFWwindow * window = glfwCreateWindow(640, 480, "ll-opengl | supersampling", nullptr, nullptr);
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
