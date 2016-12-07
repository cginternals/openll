
#pragma once

#include <QtGlobal>
#include <QObject>

#include <QMatrix4x4>
#include <QMap>

#include "AxisAlignedBoundingBox.h"

class QVector2D;
class QVector3D;
class QSize;

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

class AbstractCoordinateProvider;
class Camera;


class Navigation: public QObject
{
	Q_OBJECT

	enum InteractionMode
	{
		NoInteraction
	,	PanInteraction
	,	RotateInteraction
	};

public:
	Navigation(Camera & camera);
	virtual ~Navigation();

    void setBoundaryHint(const AxisAlignedBoundingBox & aabb);
    void setCoordinateProvider(AbstractCoordinateProvider * provider);

	virtual void reset(bool update = true);

	// event handling

	virtual void keyPressEvent        (QKeyEvent   * event);
	virtual void keyReleaseEvent      (QKeyEvent   * event);

	virtual void mouseMoveEvent       (QMouseEvent * event);
	virtual void mousePressEvent      (QMouseEvent * event);
	virtual void mouseReleaseEvent    (QMouseEvent * event);
	virtual void mouseDoubleClickEvent(QMouseEvent * event);

	virtual void wheelEvent           (QWheelEvent * event);

protected:
	void panningBegin(const QPoint & mouse);
	void panningProcess(const QPoint & mouse);
	void panningEnd();

	void rotatingBegin(const QPoint & mouse);
	void rotatingProcess(const QPoint & mouse);
	void rotatingEnd();

    void pan(QVector3D t);
    void rotate(float hAngle, float vAngle);

	void scaleAtCenter(float scale);
	void scaleAtMouse(const QPoint & mouse,	float scale);
	void resetScaleAtMouse(const QPoint & mouse);

//	void enforceWholeMapVisible(const float offset = 0.08);

	// constraints

	void enforceTranslationConstraints(QVector3D & p) const;
	void enforceRotationConstraints(
		float & hAngle
	,	float & vAngle) const;
	void enforceScaleConstraints(
		float & scale
	,	QVector3D & i) const;

	// math

	const QVector3D mouseRayPlaneIntersection(
        bool & intersects
    ,   const QPoint & mouse) const;
    const QVector3D mouseRayPlaneIntersection(
        bool & intersects
    ,   const QPoint & mouse
    ,   const QVector3D & p0) const;
    const QVector3D mouseRayPlaneIntersection(
        bool & intersects
    ,   const QPoint & mouse
	,	const QVector3D & p0
	,	const QMatrix4x4 & viewProjectionInverted) const;

protected:
    Camera & m_camera;
    AxisAlignedBoundingBox m_aabb;

    AbstractCoordinateProvider * m_coordsProvider;

    bool m_rotationHappened;
	InteractionMode m_mode;

    QVector3D m_eye;
    QVector3D m_center;
    QMatrix4x4 m_viewProjectionInverted;

    QVector3D  m_i0;
    QVector3D  m_i1;
    bool  m_i0Valid; // stores if initial interaction pick yielded valid depth
    QPointF    m_m0;
};
