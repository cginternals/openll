
#include <iostream>
#include <algorithm>

#include <glm/vec2.hpp>

#include <glbinding/gl/gl.h>
#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/base/File.h>

#include <globjects/Uniform.h>
#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Texture.h>

#include <openll/Alignment.h>
#include <openll/FontFace.h>
#include <openll/FontLoader.h>
#include <openll/GlyphRenderer.h>
#include <openll/GlyphSequence.h>
#include <openll/GlyphSequenceConfig.h>
#include <openll/LineAnchor.h>
#include <openll/stages/GlyphPreparationStage.h>

#include "datapath.inl"
#include "PointDrawable.h"


using namespace gl;


namespace
{
	auto g_frame = 0u; //unused
	auto g_size = glm::ivec2{};
	auto g_size_changed = true;

	std::vector<Point> g_points;
	PointDrawable * g_pointDrawable = nullptr;

	float g_time = 0.f;

	std::vector<gloperate_text::GlyphVertexCloud> g_vertexClouds;
	gloperate_text::FontFace * g_font;

	//different configs we will use in this demo
	std::vector<gloperate_text::GlyphSequenceConfig> g_configs;
}

gloperate_text::GlyphVertexCloud prepareGlyphSequences(const glm::vec2 origin, std::string string, gloperate_text::GlyphSequenceConfig config)
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
	transform = glm::scale(transform, 2.f / glm::vec3(g_size.x, g_size.y, 1.f));
	// scale glyphs to pixel size with respect to the displays ppi
	transform = glm::scale(transform, glm::vec3(config.ppiScale()));
	// translate to origin in point space - scale origin within
	// margined extend (i.e., viewport with margined areas removed)
	const auto marginedExtent = glm::vec2(g_size.x, g_size.y) / config.ppiScale()
		- glm::vec2(margins[3] + margins[1], margins[2] + margins[0]);
	transform = glm::translate(transform
		, glm::vec3((0.5f * origin + 0.5f) * marginedExtent, 0.f) + glm::vec3(margins[3], margins[2], 0.f));

	sequence.setAdditionalTransform(transform);
	sequences.push_back(sequence);

	return gloperate_text::prepareGlyphs(sequences, true);
}

gloperate_text::GlyphVertexCloud preparePoint(const glm::vec2 origin, gloperate_text::FontFace * font) {
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
	transform = glm::scale(transform, 2.f / glm::vec3(g_size.x, g_size.y, 1.f));
	// scale glyphs to pixel size with respect to the displays ppi
	transform = glm::scale(transform, glm::vec3(ppiScale));
	// translate to origin in point space - scale origin within
	// margined extend (i.e., viewport with margined areas removed)
	const auto marginedExtent = glm::vec2(g_size.x, g_size.y) / ppiScale
		- glm::vec2(margins[3] + margins[1], margins[2] + margins[0]);
	transform = glm::translate(transform
		, glm::vec3((0.5f * origin + 0.5f) * marginedExtent, 0.f) + glm::vec3(margins[3], margins[2], 0.f));

	sequence.setAdditionalTransform(transform);

	sequences.push_back(sequence);

	return gloperate_text::prepareGlyphs(sequences, true);
}


void annotatePoint(const glm::vec2 origin, const glm::vec2 offset, std::string string, gloperate_text::GlyphSequenceConfig config)
{
	g_vertexClouds.push_back(prepareGlyphSequences(origin + offset, string, config));
}


void initialize()
{
	const auto dataPath = common::retrieveDataPath("ll-opengl", "dataPath");

	gloperate_text::FontLoader loader;
	g_font = loader.load(dataPath + "/fonts/opensansr36/opensansr36.fnt");
	
	g_pointDrawable = new PointDrawable{ dataPath };

	//initialize all used configs. There is no need to initialize them and never change them. It's just this example.
	auto defaultConfig = gloperate_text::GlyphSequenceConfig(g_font);
	g_configs.push_back(defaultConfig);

	auto configBigRed = gloperate_text::GlyphSequenceConfig(g_font);
	configBigRed.setFontColor(glm::vec4(1.f, 0.f, 0.f, 1.f));
	configBigRed.setFontSize(32.f);
	g_configs.push_back(configBigRed);

	auto configVertical = gloperate_text::GlyphSequenceConfig(g_font);
	configVertical.setAlignment(gloperate_text::Alignment::Centered);
	configVertical.setAnchor(gloperate_text::LineAnchor::Center);
	configVertical.setWordWrap(true);
	configVertical.setLineWidth(0);
	g_configs.push_back(configVertical);


	//this is so bad hard-coded
	std::vector<glm::vec2> points{glm::vec2(-0.2, 0.f), glm::vec2(0.2, -0.2), glm::vec2(-0.2, -0.6)
		, glm::vec2(-0.8, 0.8), glm::vec2(-0.7, 0.4), glm::vec2(0.8, 0.8) };

	for (auto point : points) {
		g_points.push_back({
			point,
			glm::vec3(0.f, 0.f, 1.f), //color: blue
			10.f * g_size.x / 1000 //size
		});
	}
	g_pointDrawable->initialize(g_points);

	glClearColor(1.f, 1.f, 1.f, 1.f);
}

void deinitialize()
{
	globjects::detachAllObjects();
}

void draw(gloperate_text::GlyphRenderer &renderer)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (g_size_changed)
	{
		glViewport(0, 0, g_size.x, g_size.y);

		g_vertexClouds.clear();

		annotatePoint(glm::vec2(-0.2f, 0.f), glm::vec2(0.f, 0.1f), "annotated with default config", g_configs.at(0));
		annotatePoint(glm::vec2(0.2f, -0.2f), glm::vec2(0.f, -0.1f), "this one also with default config", g_configs.at(0));
		annotatePoint(glm::vec2(-0.2f, -0.6f), glm::vec2(0.1, 0.f), "is that point clear? hehe point.", g_configs.at(0));

		annotatePoint(glm::vec2(-0.8f, 0.8f), glm::vec2(0.1, -0.05f), "annotated with big red config", g_configs.at(1));
		annotatePoint(glm::vec2(-0.7f, 0.4f), glm::vec2(0.1, -0.05f), "still big and red!", g_configs.at(1));

		annotatePoint(glm::vec2(0.8f, 0.8f), glm::vec2(0.05f, 0.f), "one line vertical", g_configs.at(2));
	}
	
	gl::glDepthMask(gl::GL_FALSE);
	gl::glEnable(gl::GL_CULL_FACE);
	gl::glEnable(gl::GL_BLEND);
	gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);

	glm::vec3 myRotationAxis(0, 1, 0);
	auto m = glm::mat4();
	auto rotatedM = glm::rotate(m, 0.2f, myRotationAxis);
	g_time+=0.004;

	g_pointDrawable->render();

	for (size_t i = 0; i < g_vertexClouds.size(); i++) {
		glm::vec3 myRotationAxis(0, 1, 0);
		auto m = glm::mat4();
		auto rotatedM = glm::rotate(m, g_time, myRotationAxis);
		renderer.renderInWorld(g_vertexClouds[i],rotatedM);
	}

	gl::glDepthMask(gl::GL_TRUE);
	gl::glDisable(gl::GL_CULL_FACE);
	gl::glDisable(gl::GL_BLEND);

	g_size_changed = false;
}


void error(int errnum, const char * errmsg)
{
	globjects::critical() << errnum << ": " << errmsg << std::endl;
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)
{
	g_size = glm::ivec2{ width, height };

	g_size_changed = true;
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a context and, if valid, make it current
	GLFWwindow * window = glfwCreateWindow(1000, 1000, "ll-opengl | labeling-at-point", nullptr, nullptr);

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

	gloperate_text::GlyphRenderer renderer;

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		draw(renderer);
		glfwSwapBuffers(window);
	}
	deinitialize();

	// Properly shutdown GLFW
	glfwTerminate();

	return 0;
}
