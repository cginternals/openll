
#include <cmath>

#include <QColor>
#include <QVector2D>
#include <QVector4D>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>

#include "OpenGLFunctions.h"
#include "MathMacros.h"
#include "Plane3.h"
#include "AdaptiveGrid.h"


const char * AdaptiveGrid::s_vsSource =
   
    "#version 140\n"
    "uniform mat4 transform;\n"
    "uniform vec2 viewPlaneDistance;\n"
    "in vec4 a_vertex;\n"
    "flat out float v_type;\n" 
    "out vec3 v_vertex;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    float m = 1.0 - viewPlaneDistance[1];\n"
    "    float t = a_vertex.w;\n"
    "    vec4 vertex = transform * vec4(a_vertex.xyz, 1.0);\n"
    "    v_vertex = vertex.xyz;\n"
    "    // interpolate minor grid lines alpha based on viewPlaneDistance\n"
    "    v_type =  mix(1.0 - t, 1.0 - 2.0 * m * t, step(a_vertex.w, 0.7998));\n"
    "    gl_Position = vertex;\n"
    "}\n";

const char * AdaptiveGrid::s_fsSource = 

    "#version 140\n"
    "\n"
    "uniform vec2 viewPlaneDistance;\n"
    "uniform float znear;\n"
    "uniform float zfar; \n"
    "uniform vec3 color; \n"
    "flat in float v_type; \n"
    "in vec3 v_vertex; \n"
    "out vec4 fragColor; \n"
    "void main()\n"
    "{\n"
    "   float t = v_type;\n"
    "   float z = gl_FragCoord.z;\n"
    "\n"
    "   // complete function\n"
    "   // z = (2.0 * zfar * znear / (zfar + znear - (zfar - znear) * (2.0 * z - 1.0)));\n"
    "   // normalized to [0,1]\n"
    "   // z = (z - znear) / (zfar - znear);\n"
	"\n"
    "   // simplyfied with wolfram alpha\n"
    "   z = - znear * z / (zfar * z - zfar - znear * z);\n"
	"\n"
    "   float g = mix(t, 1.0, z * z);\n"
    "   float l = clamp(8.0 - length(v_vertex) / viewPlaneDistance[0], 0.0, 1.0);\n"
    "   fragColor = vec4(color, l * (1.0 - g * g));\n"
    "}\n";


AdaptiveGrid::AdaptiveGrid(
    OpenGLFunctions & gl
,   unsigned short segments
,   const QVector3D & location
,   const QVector3D & normal)
:   m_program(new QOpenGLShaderProgram)
,   m_buffer(QOpenGLBuffer::VertexBuffer)
,   m_location(location)
,   m_normal(normal)
,   m_size(0)
{
    m_transform = transform(m_location, m_normal); 

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, s_vsSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, s_fsSource);
    m_program->link();

    setColor(QColor::fromRgbF(0.8, 0.8, 0.8));

    m_vao.create();
    m_vao.bind();

    setupGridLineBuffer(segments);
    
    const int a_vertex = m_program->attributeLocation("a_vertex");

	gl.glVertexAttribPointer(a_vertex, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
    gl.glEnableVertexAttribArray(a_vertex);

    m_vao.release();
}

AdaptiveGrid::~AdaptiveGrid()
{
}

void AdaptiveGrid::setupGridLineBuffer(unsigned short segments)
{
    std::vector<QVector4D> points;

    float type;
    int  n = 1;

    const float g(static_cast<float>(segments));

    type = .2f; // sub gridlines, every 0.125, except every 0.5
    for (float f = -g + .125f; f < g; f += .125f)
        if (n++ % 4)
        {
            points.push_back(QVector4D( g, 0.f, f, type));
            points.push_back(QVector4D(-g, 0.f, f, type));
            points.push_back(QVector4D( f, 0.f, g, type));
            points.push_back(QVector4D( f, 0.f,-g, type));
        }
    type = .4f; // grid lines every 1.0 units, offseted by 0.5
    for (float f = -g + .5f; f < g; f += 1.f)
    {
        points.push_back(QVector4D( g, 0.f, f, type));
        points.push_back(QVector4D(-g, 0.f, f, type));
        points.push_back(QVector4D( f, 0.f, g, type));
        points.push_back(QVector4D( f, 0.f,-g, type));
    }
    type = .8f; // grid lines every 1.0 units
    for (float f = -g + 1.f; f < g; f += 1.f) 
    {
        points.push_back(QVector4D( g, 0.f, f, type));
        points.push_back(QVector4D(-g, 0.f, f, type));
        points.push_back(QVector4D( f, 0.f, g, type));
        points.push_back(QVector4D( f, 0.f,-g, type));
    }
    // use hesse normal form and transform each grid line onto the specified plane.
    QMatrix4x4 T; // ToDo
    for (QVector4D & point : points)
        point = QVector4D(QVector3D(T * QVector4D(point.toVector3D(), 1.f)), point.w());

	m_size = points.size(); // segments * 64 - 4;
    
    m_buffer.create();
    m_buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_buffer.bind();
	m_buffer.allocate(points.data(), sizeof(float)* 4 * points.size());
}

void AdaptiveGrid::setNearFar(
    float zNear
,   float zFar)
{
	m_program->bind();
    m_program->setUniformValue("znear", zNear);
    m_program->setUniformValue("zfar", zFar);
	m_program->release();
}

void AdaptiveGrid::setColor(const QColor & color)
{
	m_program->bind();
    m_program->setUniformValue("color", QVector3D(color.redF(), color.greenF(), color.blueF()));
	m_program->release();
}

void AdaptiveGrid::update(
    const QVector3D & viewpoint
,   const QMatrix4x4 & modelViewProjection)
{
    // Project the camera's eye position onto the grid plane.
    bool intersects; //  should always intersect.
    const QVector3D i = intersection(intersects, m_location, m_normal
        , viewpoint, viewpoint - m_normal);

    // This transforms the horizontal plane vectors u and v  onto the actual 
    // grid plane. Than express the intersection i as a linear combination of 
    // u and v by solving the linear equation system.
    // Finally round the s and t values to get the nearest major grid point.

    const float l = (viewpoint - i).length();

    const float distancelog = log(l * .4998f) / log(8.f);
    const float distance = pow(8.f, ceil(distancelog));

    QMatrix4x4 T; // ToDo
    const QVector3D uv(QVector3D(T * QVector4D(distance, 0.f, 0.f, 1.f)) - m_location);
    const float u[3] = { uv.x(), uv.y(), uv.z() };
    const QVector3D vv(QVector3D(T * QVector4D(0.f, 0.f, distance, 1.f)) - m_location);
    const float v[3] = { vv.x(), vv.y(), vv.z() };

    const size_t j = u[0] != 0.0 ? 0 : u[1] != 0.0 ? 1 : 2;
    const size_t k = v[0] != 0.0 && j != 0 ? 0 : v[1] != 0.0  && j != 1 ? 1 : 2;

    const QVector3D av(i - m_location);
    const float a[3] = { av.x(), av.y(), av.z() };

    const float t = v[k] ? a[k] / v[k] : 0.f;
    const float s = u[j] ? (a[j] - t * v[j]) / u[j] : 0.f;

    const QVector3D irounded = round(s) * uv + round(t) * vv;

    QMatrix4x4 offset;
    offset.translate(irounded);
    offset.scale(distance);

	m_program->bind();
    m_program->setUniformValue("viewPlaneDistance", QVector2D(l, mod(distancelog, 1.f)));
	m_program->setUniformValue("transform", modelViewProjection * offset);
	m_program->release();
} 

void AdaptiveGrid::draw(
    OpenGLFunctions & gl)
{
    gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.glEnable(GL_BLEND);

    gl.glEnable(GL_DEPTH_TEST);

    m_program->bind();

    m_vao.bind();
    gl.glDrawArrays(GL_LINES, 0, m_size);
    m_vao.release();

    m_program->release();

    gl.glDisable(GL_BLEND);
}
