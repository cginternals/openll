
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
    auto g_frame = 0u;
    auto g_size = glm::ivec2{ };

    globjects::Program * g_computeProgram = nullptr;
}


void initialize()
{
    const auto dataPath = common::retrieveDataPath("openll", "dataPath");

    g_computeProgram = new globjects::Program();
    g_computeProgram->attach(globjects::Shader::fromFile(GL_COMPUTE_SHADER, dataPath + "shaders/glyph.geom"));
    g_computeProgram->setUniform("destTex", 0);
    g_computeProgram->ref();

    // ToDo
}

void deinitialize()
{
    // ToDo
    g_computeProgram->unref();

    globjects::detachAllObjects();
}

void draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // ToDo

    glViewport(0, 0, g_size.x, g_size.y);
    
    // ToDo
}


void error(int errnum, const char * errmsg)
{
    globjects::critical() << errnum << ": " << errmsg << std::endl;
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)
{
    g_size = glm::ivec2{ width, height };

    // ToDo
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
    GLFWwindow * window = glfwCreateWindow(640, 480, "ll-opengl | labeling-at-point", nullptr, nullptr);
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
