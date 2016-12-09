
#include <cassert>

#include <QVector2D>
#include <QMouseEvent>
#include <QKeyEvent>

#include "MathMacros.h"
#include "AbstractCoordinateProvider.h"
#include "Camera.h"
#include "NavigationMath.h"

#include "Navigation.h"


namespace
{
    static const QVector3D DEFAULT_EYE    = QVector3D(0.f, 1.2f, 2.4f);
    static const QVector3D DEFAULT_CENTER = QVector3D(0.f, 0.0f, 0.0f);
    static const QVector3D DEFAULT_UP     = QVector3D(0.f, 1.0f, 0.0f);

    static const float DEFAULT_SCALE_STEP = 0.1f;
    static const float DEFAULT_DISTANCE   = 2.0f;
    static const float DEFAULT_DIST_MIN   = 0.1f;
    static const float DEFAULT_DIST_MAX   = 4.0f;

    static const float ROTATION_HOR_DOF   = 0.8f * static_cast<float>(PI);
    static const float ROTATION_VER_DOF   = 0.8f * static_cast<float>(PI);

    static const float ROTATION_KEY_SCALE = 1.0f;

    //static const float NAV_CONSTRAINT_PAN_CIRCLE_R = 2.83;
    static const float CONSTRAINT_ROT_MAX_V_UP = 0.02f * static_cast<float>(PI);
    static const float CONSTRAINT_ROT_MAX_V_LO = 0.98f * static_cast<float>(PI);
}

Navigation::Navigation(Camera & camera)
: m_camera(camera)
, m_coordsProvider(nullptr)
, m_rotationHappened(false)
, m_mode(NoInteraction)
{
    reset();
}

Navigation::~Navigation()
{
}

void Navigation::setBoundaryHint(const AxisAlignedBoundingBox & aabb)
{
    m_aabb = aabb;
}

void Navigation::setCoordinateProvider(AbstractCoordinateProvider * provider)
{
    m_coordsProvider = provider;
}

void Navigation::reset(bool update)
{
    m_camera.setEye(DEFAULT_EYE);
    m_camera.setCenter(DEFAULT_CENTER);
    m_camera.setUp(DEFAULT_UP);

    m_mode = NoInteraction;

//    enforceWholeMapVisible();

    if (update)
        m_camera.update();
}


const QVector3D Navigation::mouseRayPlaneIntersection(
    bool & intersects
,   const QPoint & mouse
,   const QVector3D & p0) const
{
    // build a ray in object space from screen space mouse position and get
    // intersection with near and far planes.

    const QVector3D ln = m_coordsProvider->objAt(mouse, 0.0);
    const QVector3D lf = m_coordsProvider->objAt(mouse, 1.0);

    return NavigationMath::rayPlaneIntersection(intersects, ln, lf, p0);
}


const QVector3D Navigation::mouseRayPlaneIntersection(
    bool & intersects
,   const QPoint & mouse
,   const QVector3D & p0
,   const QMatrix4x4 & viewProjectionInverted) const
{
    // build a ray in object space from screen space mouse position and get
    // intersection with near and far planes.

    const QVector3D ln = m_coordsProvider->objAt(mouse, 0.0, viewProjectionInverted);
    const QVector3D lf = m_coordsProvider->objAt(mouse, 1.0, viewProjectionInverted);

    return NavigationMath::rayPlaneIntersection(intersects, ln, lf, p0);
}

const QVector3D Navigation::mouseRayPlaneIntersection(
    bool & intersects
,   const QPoint & mouse) const
{
    const float depth = m_coordsProvider->depthAt(mouse);

    // no scene object was picked - simulate picking on xz-plane
    if (depth >= 1.0 - std::numeric_limits<float>::epsilon())
        return mouseRayPlaneIntersection(intersects, mouse, QVector3D());

    return m_coordsProvider->objAt(mouse, depth);
}


void Navigation::panningBegin(const QPoint & mouse)
{
    assert(NoInteraction == m_mode);
    m_mode = PanInteraction;

    m_viewProjectionInverted = m_camera.viewProjectionInverted();
    
    bool intersects;
    m_i0 = mouseRayPlaneIntersection(intersects, mouse);
    m_i0Valid = intersects && NavigationMath::validDepth(m_coordsProvider->depthAt(mouse));

    m_eye = m_camera.eye();
    m_center = m_camera.center();
}

void Navigation::panningEnd()
{
    assert(PanInteraction == m_mode);
    m_mode = NoInteraction;
}

void Navigation::panningProcess(const QPoint & mouse)
{
    assert(PanInteraction == m_mode);

    // The first click of the interaction yields a object space position m_i0.
    // this point is our constraint for panning, that means for every mouse 
    // position there has to be an appropriate positioning for the scene, so
    // that the point under the mouse remains m_i0.
    // With this point and the up normal we build a plane, that defines the 
    // panning space. For panning, a ray is created, pointing from the screen
    // pixel into the view frustum. This ray then is converted to object space
    // and used to intersect with the plane at p. 
    // The delta of m_i0 and p is the translation required for panning.

    // constrain mouse interaction to viewport (if disabled, could lead to mishaps)
    const QPoint clamped(
        clamp(0, m_camera.viewport().width(), mouse.x())
    ,   clamp(0, m_camera.viewport().height(), mouse.y()));

    bool intersects;
    m_i1 = mouseRayPlaneIntersection(intersects, clamped, m_i0, m_viewProjectionInverted);

    if (intersects)
        pan(m_i0 - m_i1);
}

void Navigation::pan(QVector3D t)
{
    //enforceTranslationConstraints(t);

    m_camera.setEye(t + m_eye);
    m_camera.setCenter(t + m_center);

    m_camera.update();
}


void Navigation::rotatingBegin(const QPoint & mouse)
{
    assert(NoInteraction == m_mode);
    m_mode = RotateInteraction;

    bool intersects;
    m_i0 = mouseRayPlaneIntersection(intersects, mouse);
    m_i0Valid = intersects && NavigationMath::validDepth(m_coordsProvider->depthAt(mouse));

    m_m0 = mouse;

    m_eye = m_camera.eye();
    m_center = m_camera.center();
}

void Navigation::rotatingEnd()
{
    assert(RotateInteraction == m_mode);
    m_mode = NoInteraction;
}

void Navigation::rotatingProcess(const QPoint & mouse)
{
    assert(RotateInteraction == m_mode);

    const QPointF delta = m_m0 - mouse;
    // setup the degree of freedom for horizontal rotation within a single action
    const float wDeltaX = deg(delta.x() / m_camera.viewport().width());
    // setup the degree of freedom for vertical rotation within a single action
    const float wDeltaY = deg(delta.y() / m_camera.viewport().height());

    rotate(wDeltaX, wDeltaY);
}

void Navigation::rotate(
    float hAngle
,   float vAngle)
{
    static const QVector3D up(0.0, 1.0, 0.0);

    m_rotationHappened = true;

    const QVector3D ray((m_camera.center() - m_eye).normalized());
    const QVector3D rotAxis(QVector3D::crossProduct(ray, up));

    hAngle *= ROTATION_HOR_DOF;
    vAngle *= ROTATION_VER_DOF;

    enforceRotationConstraints(hAngle, vAngle);

    QVector3D t = m_i0Valid ? m_i0 : m_center;

    QMatrix4x4 transform;
    transform.translate( t);
    transform.rotate(hAngle, up);
    transform.rotate(vAngle, rotAxis);
    transform.translate(-t);

    m_camera.setEye(transform * m_eye);
    m_camera.setCenter(transform * m_center);

    m_camera.update();
}


void Navigation::scaleAtMouse(
    const QPoint & mouse
,   float scale)
{
    const QVector3D& ln = m_camera.eye();
    const QVector3D& lf = m_camera.center();

    bool intersects;

    QVector3D i = mouseRayPlaneIntersection(intersects, mouse);

    if(!intersects && !NavigationMath::validDepth(m_coordsProvider->depthAt(mouse)))
        return;

    // scale the distance between the pointed position in the scene and the 
    // camera position - using ray constraints, the center is scaled appropriately.

    if (scale > 0.0)
        scale = 1.0 / (1.0 - scale) - 1.0; // ensure that scaling consistent in both direction

    // enforceScaleConstraints(scale, i);

    const QVector3D eye = ln + scale * (ln - i);
    m_camera.setEye(eye);

    // the center needs to be constrained to the ground plane, so calc the new
    // center based on the intersection with the scene and use this to obtain 
    // the new viewray-groundplane intersection as new center.
    const QVector3D center = lf + scale * (lf - i);

    m_camera.setCenter(NavigationMath::rayPlaneIntersection(intersects, eye, center));
    m_camera.update();
}

void Navigation::resetScaleAtMouse(const QPoint & mouse)
{
    const QVector3D& ln = m_camera.eye();
    const QVector3D& lf = m_camera.center();

    // set the distance between pointed position in the scene and camera to 
    // default distance
    bool intersects;
    QVector3D i = mouseRayPlaneIntersection(intersects, mouse);
    if (!intersects && !NavigationMath::validDepth(m_coordsProvider->depthAt(mouse)))
        return;

    float scale = (DEFAULT_DISTANCE / (ln - i).length());

    //enforceScaleConstraints(scale, i);

    m_camera.setEye(i - scale * (i - ln));
    m_camera.setCenter(i - scale * (i - lf));

    m_camera.update();
}

void Navigation::scaleAtCenter(float scale)
{
    const QVector3D& ln = m_camera.eye();
    const QVector3D& lf = m_camera.center();

    bool intersects;
    QVector3D i = NavigationMath::rayPlaneIntersection(intersects, ln, lf);
    if (!intersects)
        return;

    //enforceScaleConstraints(scale, i);

    m_camera.setEye(ln + scale * (ln - i));
    m_camera.setCenter(lf + scale * (lf - i));

    m_camera.update();
}


void Navigation::enforceTranslationConstraints(QVector3D & p) const
{
    QMatrix4x4 m;
    m.translate(p);

    const QVector2D center(NavigationMath::xz(m * m_center));
    if (NavigationMath::insideSquare(center))
        return;

    const QVector2D i = NavigationMath::raySquareIntersection(center);
    p = QVector3D(i.x(), 0., i.y()) - m_center;
}

void Navigation::enforceRotationConstraints(
    float & /*hAngle*/
,   float & vAngle) const
{
    // hAngle is not constrained, vAngle is.

    // retrieve the angle between camera-center to up and test how much closer
    // to up/down it can be rotated and clamp if required.

    static const QVector3D up(0.0, 1.0, 0.0);
    const float va = deg(acos(
        QVector3D::dotProduct((m_eye - m_center).normalized(), up)));

    if (vAngle <= 0.0)
        vAngle = ma(vAngle, deg(CONSTRAINT_ROT_MAX_V_UP) - va);
    else
        vAngle = mi(vAngle, deg(CONSTRAINT_ROT_MAX_V_LO) - va);
}
 
void Navigation::enforceScaleConstraints(
    float & scale
,   QVector3D & i) const
{
    // first constraint: i must be within the ground quad...
    QVector2D i2 = NavigationMath::xz(i);

    if (!NavigationMath::insideSquare(i2))
    {
        i2 = NavigationMath::raySquareIntersection(i2);
        i = QVector3D(i2.x(), 0., i2.y());
    }

    // second constraint: scale factor must be within min and max... 
    const QVector3D eye = m_eye + scale * (m_eye - i);

    const float ds = (eye - i).length();

    if ((scale > 0. && ds >= DEFAULT_DIST_MAX)
    ||  (scale < 0. && ds <= DEFAULT_DIST_MIN)
    ||  (eye.y() <= m_center.y()))	// last fixes abnormal scales (e.g., resulting from mouse flywheels)
        scale = 0.0;
}

//void Navigation::enforceWholeMapVisible(const float offset)
//{
//    const float h(_swmBounds.urb.y());
//
//    const QVector3D bbox[8] =
//    {
//        // front
//        QVector3D(-1.f, 0.f, +1.f)
//        , QVector3D(+1.f, 0.f, +1.f)
//        , QVector3D(+1.f, h, +1.f)
//        , QVector3D(-1.f, h, +1.f)
//        // back
//        , QVector3D(-1.f, 0.f, -1.f)
//        , QVector3D(+1.f, 0.f, -1.f)
//        , QVector3D(+1.f, h, -1.f)
//        , QVector3D(-1.f, h, -1.f)
//    };
//
//    float nearest = FLT_MAX;
//
//    QVector3D farthestCamera = _center;
//    float farthestDistanceSquared = 0.0;
//
//    // temporaries for modelview matrix update
//    QMatrix4x4 modelView, projection, modelViewProjection;
//    float zNear, zFar;
//
//    // retrieve the closest point to the ray in view direction
//    for (int i = 0; i < 8; ++i)
//    {
//        const QVector3D &p(bbox[i]);
//
//        // check if the point is already visible, if not adjust camera
//        updateMatrices(farthestCamera, _center, _up, h,
//            fieldOfView(), _aspect, zNear, zFar,
//            modelView, projection, modelViewProjection);
//
//        if (NavigationUtils::pointIsVisible(modelViewProjection, p))
//            continue;
//
//        // so request new camera pos
//        const QVector3D newCamera = NavigationUtils::cameraWithPointInView(
//            _camera, _center, _up, fieldOfView(), _aspect, p);
//
//        const float ls = (_center - newCamera).lengthSquared();
//
//        // retrieve distance from current center
//        if (ls > farthestDistanceSquared)
//        {
//            farthestDistanceSquared = ls;
//            farthestCamera = newCamera;
//        }
//    }
//
//    // adjust distance, so that znear is always max distance to new camera
//    float d = sqrt(farthestDistanceSquared) + offset;
//
//    d = qMin<float>(d, NAV_DEFAULT_DIST_MAX);
//
//    _camera = _center + (_camera - _center).normalized() * d;
//
//    setDirty();
//}


void Navigation::mouseMoveEvent(QMouseEvent * event)
{
    event->ignore();

    switch (m_mode)
    {
    case PanInteraction:
        panningProcess(event->pos());
        event->accept();
        break;

    case RotateInteraction:
        rotatingProcess(event->pos());
        event->accept();
        break;

    default:
        break;
    }
}

void Navigation::mousePressEvent(QMouseEvent * event)
{
    event->ignore();

    switch (event->button())
    {
    case Qt::LeftButton:
        if (m_mode == RotateInteraction)
            break;
        panningBegin(event->pos());
        event->accept();
        break;

    case Qt::RightButton:
        if (m_mode == PanInteraction)
            break;
        rotatingBegin(event->pos());
        event->accept();
        break;

    default:
        break;
    }
}

void Navigation::mouseReleaseEvent(QMouseEvent * event)
{
    event->ignore();

    switch (m_mode)
    {
    case PanInteraction:
        if (event->button() == Qt::LeftButton)
        {
            panningEnd();
            event->accept();
        }
        break;

    case RotateInteraction:
        if (event->button() == Qt::RightButton)
        {
            rotatingEnd();
            if (m_rotationHappened)
            {
                event->accept();
                m_rotationHappened = false;
            }
        }
        break;

    case NoInteraction:
    default:
        //if (event->button() == Qt::MiddleButton)
        //{
        //    enforceWholeMapVisible();
        //    event->accept();
        //}
        break;
    }
}

void Navigation::mouseDoubleClickEvent(QMouseEvent * event)
{
    event->ignore();
}

void Navigation::wheelEvent(QWheelEvent * event)
{
    event->ignore();

    if (m_mode == NoInteraction)
    {
		static const float wheelNormalizationFactor = 1.f / 120.f;
        scaleAtMouse(event->pos(),
            - event->delta() * wheelNormalizationFactor * DEFAULT_SCALE_STEP);
        event->accept();
    }
}

void Navigation::keyPressEvent(QKeyEvent * event)
{
    event->ignore();

    switch (event->key())
    {
    case Qt::Key_Space:
        // reset while panning/rotating leads to inconsistent state
        if (m_mode == NoInteraction)
        {
            reset();
            event->accept();
        }
        break;

    case Qt::Key_Plus:
        scaleAtCenter(-DEFAULT_SCALE_STEP);
        event->accept();
        break;
    case Qt::Key_Minus:
        scaleAtCenter(+DEFAULT_SCALE_STEP);
        event->accept();
        break;
    default:
        break;
    }

    if (m_mode == NoInteraction)
    {
        m_eye = m_camera.eye();
        m_center = m_camera.center();

        switch (event->key())
        {
        case Qt::Key_Left:
            rotate(-ROTATION_KEY_SCALE, 0.0);
            event->accept();
            break;
        case Qt::Key_Right:
            rotate(+ROTATION_KEY_SCALE, 0.0);
            event->accept();
            break;
        case Qt::Key_Up:
            rotate(0.0, +ROTATION_KEY_SCALE);
            event->accept();
            break;
        case Qt::Key_Down:
            rotate(0.0, -ROTATION_KEY_SCALE);
            event->accept();
            break;
        default:
            break;
        }
    }
}

void Navigation::keyReleaseEvent(QKeyEvent * event)
{
    event->ignore();
}
