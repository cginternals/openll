#pragma once

#include <QVector3D>
#include <QMatrix4x4>
#include <QScopedPointer>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class OpenGLFunctions;
class QColor;

class AdaptiveGrid
{
public:
    AdaptiveGrid(
        OpenGLFunctions & gl
    ,   unsigned short segments = 16
    ,   const QVector3D & location = QVector3D(0.0, 0.0, 0.0)
    ,   const QVector3D & normal = QVector3D(0.0, 1.0, 0.0));

    virtual ~AdaptiveGrid();

    void setNearFar(
        float zNear
    ,   float zFar);

    void setColor(const QColor & color);

    void update(
        const QVector3D & viewpoint
    ,   const QMatrix4x4 & modelViewProjection);

    void draw(OpenGLFunctions & gl);

private:
    void setupGridLineBuffer(unsigned short segments);

private:
    static const char * s_vsSource;
    static const char * s_fsSource;

private:
    QScopedPointer<QOpenGLShaderProgram> m_program;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_buffer;

    QVector3D m_location;
    QVector3D m_normal;

    QMatrix4x4 m_transform;

    unsigned short m_size;
};

