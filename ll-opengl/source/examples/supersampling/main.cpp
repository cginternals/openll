
#include <iostream>
#include <algorithm>
#include <memory>

#include <cpplocate/cpplocate.h>
#include <cpplocate/ModuleInfo.h>

#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

#include <openll/FontFace.h>
#include <openll/GlyphSequence.h>
#include <openll/GlyphVertexCloud.h>
#include <openll/Alignment.h>
#include <openll/LineAnchor.h>
#include <openll/SuperSampling.h>

#include "datapath.inl"


using namespace gl;


namespace
{
    std::vector<int> g_fontSizes {36, 72, 144};
    std::map<int, gloperate_text::FontFace *> g_fonts;
    std::vector<gloperate_text::GlyphVertexCloud> g_clouds;
    auto g_size = glm::ivec2{ };
    bool g_config_changed = true;
    size_t g_patternID = 0;
    std::unique_ptr<gloperate_text::GlyphRenderer> g_renderer;

    struct SamplingPattern
    {
        std::string name;
        gloperate_text::SuperSampling pattern;
    };
    std::vector<SamplingPattern> g_patterns {
        {"None",     gloperate_text::SuperSampling::None},
        {"Grid1x3",  gloperate_text::SuperSampling::Grid1x3},
        {"Grid2x4",  gloperate_text::SuperSampling::Grid2x4},
        {"RGSS2x2",  gloperate_text::SuperSampling::RGSS2x2},
        {"Quincunx", gloperate_text::SuperSampling::Quincunx},
        {"Rooks8",   gloperate_text::SuperSampling::Rooks8},
        {"Grid3x3",  gloperate_text::SuperSampling::Grid3x3},
        {"Grid4x4",  gloperate_text::SuperSampling::Grid4x4}
    };
}

void initialize()
{
    glClearColor(1.f, 1.f, 1.f, 1.f);

    const auto dataPath = common::retrieveDataPath("openll", "dataPath");

    gloperate_text::FontLoader loader;
    for (auto size : g_fontSizes)
    {
        auto string = std::to_string(size);
        auto font = loader.load(dataPath + "/fonts/opensansr" + string + "/opensansr" + string + ".fnt");
        g_fonts[size] = font;
    }
    g_renderer = std::unique_ptr<gloperate_text::GlyphRenderer>(new gloperate_text::GlyphRenderer);
}

void deinitialize()
{
    for (auto pair : g_fonts)
    {
        delete pair.second;
    }
    globjects::detachAllObjects();
}

std::vector<gloperate_text::GlyphSequence> prepareSequences(
    glm::ivec2 viewport
,   gloperate_text::SuperSampling pattern
,   int size
,   float x_coord
    )
{
    std::vector<gloperate_text::GlyphSequence> sequences;

    auto y = -.8f;
    for (int renderSize = 5; renderSize < 30; ++renderSize)
    {
        const auto string = std::string{"Example (font "} + std::to_string(size) + ", size " + std::to_string(renderSize) + ")";
        const std::u32string unicode_string {string.begin(), string.end()};
        const auto origin = glm::vec2{x_coord, y};

        gloperate_text::GlyphSequence sequence;
        sequence.setString(unicode_string);
        sequence.setWordWrap(true);
        sequence.setLineWidth(400.f);
        sequence.setAlignment(gloperate_text::Alignment::LeftAligned);
        sequence.setLineAnchor(gloperate_text::LineAnchor::Ascent);
        sequence.setFontSize(renderSize);
        sequence.setFontFace(g_fonts[size]);
        sequence.setFontColor(glm::vec4(glm::vec3(0.f), 1.f));
        sequence.setSuperSampling(pattern);

        // compute  transform matrix
        glm::mat4 transform;
        transform = glm::translate(transform, glm::vec3(origin, 0.f));
        transform = glm::scale(transform, glm::vec3(1.f,
            static_cast<float>(viewport.x) / viewport.y, 1.f));
        transform = glm::scale(transform, glm::vec3(1 / 300.f));

        sequence.setAdditionalTransform(transform);
        sequences.push_back(sequence);
        y += 0.02f + renderSize * 0.005f;
    }
    return sequences;
}

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_config_changed)
    {
        glViewport(0, 0, g_size.x, g_size.y);

        std::cout << g_patterns[g_patternID].name <<std::endl;
        auto x = -.9f;
        g_clouds.clear();
        for (auto pair : g_fonts)
        {
            auto size = pair.first;
            auto sequences = prepareSequences(g_size, g_patterns[g_patternID].pattern, size, x);
            g_clouds.push_back({});
            g_clouds.back().updateWithSequences(sequences, true);
            x += 0.6f;
        }
    }

    gl::glDepthMask(gl::GL_FALSE);
    gl::glEnable(gl::GL_CULL_FACE);
    gl::glEnable(gl::GL_BLEND);
    gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

    for (auto cloud : g_clouds)
        g_renderer->render(cloud);

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

    if ('1' <= key && key <= '9' && action == GLFW_PRESS)
    {
        g_patternID = std::min(static_cast<size_t>(key - '1'), g_patterns.size() - 1);
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
