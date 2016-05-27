/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D Editor of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "editorcameracontroller.h"
#include "editorutils.h"
#include "editorviewportitem.h"
#include <Qt3DRender/QCamera>
#include <Qt3DInput/QAxis>
#include <Qt3DInput/QAnalogAxisInput>
#include <Qt3DInput/QAction>
#include <Qt3DInput/QActionInput>
#include <Qt3DInput/QLogicalDevice>
#include <Qt3DInput/QMouseDevice>
#include <Qt3DInput/QMouseEvent>
#include <Qt3DInput/QMouseHandler>
#include <Qt3DLogic/QFrameAction>

EditorCameraController::EditorCameraController(EditorViewportItem *viewport,
                                               Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
    , m_camera(nullptr)
    , m_leftMouseButtonAction(new Qt3DInput::QAction())
    , m_rightMouseButtonAction(new Qt3DInput::QAction())
    , m_middleMouseButtonAction(new Qt3DInput::QAction())
    , m_rxAxis(new Qt3DInput::QAxis())
    , m_ryAxis(new Qt3DInput::QAxis())
    , m_leftMouseButtonInput(new Qt3DInput::QActionInput())
    , m_rightMouseButtonInput(new Qt3DInput::QActionInput())
    , m_middleMouseButtonInput(new Qt3DInput::QActionInput())
    , m_mouseRxInput(new Qt3DInput::QAnalogAxisInput())
    , m_mouseRyInput(new Qt3DInput::QAnalogAxisInput())
    , m_mouseDevice(new Qt3DInput::QMouseDevice())
    , m_logicalDevice(new Qt3DInput::QLogicalDevice())
    , m_frameAction(new Qt3DLogic::QFrameAction())
    , m_ignoreFirstLeftMousePress(false)
    , m_ignoreFirstRightMousePress(false)
    , m_ignoreFirstMiddleMousePress(false)
    , m_adjustCameraAtMouseRelease(false)
    , m_cameraUp(QVector3D(0.0f, 1.0f, 0.0f))
    , m_panSpeed(360.0f)
    , m_orbitSpeed(600.0f)
    , m_translateSpeed(7.0f)
    , m_wheelSpeed(-0.05f)
    , m_viewport(viewport)
{
    init();
}

EditorCameraController::~EditorCameraController()
{
}

void EditorCameraController::init()
{
    // Left Mouse Button Action
    m_leftMouseButtonInput->setButtons(QVector<int>() << Qt3DInput::QMouseEvent::LeftButton);
    m_leftMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_leftMouseButtonAction->addInput(m_leftMouseButtonInput);

    // Right Mouse Button Action
    m_rightMouseButtonInput->setButtons(QVector<int>() << Qt3DInput::QMouseEvent::RightButton);
    m_rightMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_rightMouseButtonAction->addInput(m_rightMouseButtonInput);

    // Middle Mouse Button Action
    m_middleMouseButtonInput->setButtons(QVector<int>() << Qt3DInput::QMouseEvent::MiddleButton);
    m_middleMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_middleMouseButtonAction->addInput(m_middleMouseButtonInput);

    // Mouse X
    m_mouseRxInput->setAxis(Qt3DInput::QMouseDevice::X);
    m_mouseRxInput->setSourceDevice(m_mouseDevice);
    m_rxAxis->addInput(m_mouseRxInput);

    // Mouse Y
    m_mouseRyInput->setAxis(Qt3DInput::QMouseDevice::Y);
    m_mouseRyInput->setSourceDevice(m_mouseDevice);
    m_ryAxis->addInput(m_mouseRyInput);

    m_logicalDevice->addAction(m_leftMouseButtonAction);
    m_logicalDevice->addAction(m_rightMouseButtonAction);
    m_logicalDevice->addAction(m_middleMouseButtonAction);
    m_logicalDevice->addAxis(m_rxAxis);
    m_logicalDevice->addAxis(m_ryAxis);

    QObject::connect(m_frameAction, &Qt3DLogic::QFrameAction::triggered,
                     this, &EditorCameraController::handleTriggered);

    addComponent(m_frameAction);
    addComponent(m_logicalDevice);
}

void EditorCameraController::adjustCamera(const QVector3D &translateVec)
{
    // Adjust viewcenter so that it is on a plane intersecting origin, if possible.
    // That way we get nice orbiting with middle button.
    // Otherwise just adjust it same as position.
    float t = 0.0f;
    QVector3D newPosition = m_camera->position() + translateVec;
    QVector3D intersection = EditorUtils::findIntersection(newPosition,
                                                           m_camera->viewVector().normalized(),
                                                           0, EditorUtils::cameraNormal(m_camera),
                                                           t);
    if (t > 0.0f)
        m_camera->setViewCenter(intersection);
    else
        m_camera->setViewCenter(m_camera->viewCenter() + translateVec);

    m_camera->setPosition(newPosition);
}

void EditorCameraController::handleTriggered(float dt)
{
    if (m_camera) {
        if (m_rightMouseButtonAction->isActive()) {
            // Ignore first press so you don't get the initial jolt,
            // as the mouse delta is usually maximum into some direction.
            if (m_ignoreFirstRightMousePress) {
                m_ignoreFirstRightMousePress = false;
            } else {
                // Move camera around with right mouse button
                QMatrix4x4 viewMatrix = m_camera->viewMatrix().inverted();
                float translateMultiplier = m_translateSpeed * dt * m_camera->viewVector().length();
                QVector3D translateVec(m_rxAxis->value() * translateMultiplier,
                                       m_ryAxis->value() * translateMultiplier, 0.0f);
                translateVec = viewMatrix.mapVector(translateVec);
                adjustCamera(translateVec);
            }
        } else if (m_leftMouseButtonAction->isActive()) {
            if (m_ignoreFirstLeftMousePress) {
                m_ignoreFirstLeftMousePress = false;
            } else {
                // Pan and tilt the camera with left mouse button
                m_camera->pan(m_rxAxis->value() * m_panSpeed * dt, m_cameraUp);
                m_camera->tilt(m_ryAxis->value() * m_panSpeed * dt);
                m_adjustCameraAtMouseRelease = true;
            }
        } else if (m_middleMouseButtonAction->isActive()) {
            if (m_ignoreFirstMiddleMousePress) {
                m_ignoreFirstMiddleMousePress = false;
            } else {
                // Orbit camera around viewCenter with middle mouse button
                m_camera->panAboutViewCenter(-m_rxAxis->value() * m_orbitSpeed * dt, m_cameraUp);
                m_camera->tiltAboutViewCenter(-m_ryAxis->value() * m_orbitSpeed * dt);
                m_adjustCameraAtMouseRelease = true;
            }
        }
    }
}

void EditorCameraController::handleWheel(QWheelEvent *event)
{
    if (m_camera) {
        // Find intersection of cursor and camera far plane.
        QVector3D planeNormal = EditorUtils::cameraNormal(m_camera);
        QVector3D planeOrigin = m_camera->position() + m_camera->viewVector();
        float cosAngle = QVector3D::dotProduct(planeOrigin.normalized(), planeNormal);
        float planeOffset = planeOrigin.length() * cosAngle;
        QVector3D ray = EditorUtils::unprojectRay(m_camera->viewMatrix(),
                                                  m_camera->projectionMatrix(),
                                                  m_viewport->width(), m_viewport->height(),
                                                  event->pos());
        float t = 0.0f;
        QVector3D intersection = EditorUtils::findIntersection(m_camera->position(), ray,
                                                               planeOffset, planeNormal, t);

        // We want to keep the same world position under cursor, so we use this formula to find
        // correct translation:
        // x = camera viewVector
        // y = vector from camera viewCenter to intersection
        // x_delta = Camera translation in x vector direction
        // y_delta = Camera translation in y vector direction
        // To solve y_delta, we can use similar triangles:
        // x.len()/y.len() = x_delta/y_delta
        // --> y_delta = (y.len() * x_delta) / x.len()

        QMatrix4x4 viewMatrix = m_camera->viewMatrix().inverted();
        QVector3D translateVec(0.0f, 0.0f, event->angleDelta().y() * m_wheelSpeed);
        translateVec = viewMatrix.mapVector(translateVec); // just x_delta vector for now

        float x_delta = translateVec.length();
        if (event->angleDelta().y() < 0)
            x_delta = -x_delta;

        // Add y_delta vector
        translateVec += ((intersection - m_camera->viewCenter()) * x_delta)
                / m_camera->viewVector().length();

        adjustCamera(translateVec);
    }
}

void EditorCameraController::handleMousePress(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        m_ignoreFirstLeftMousePress = true;
    if (event->buttons() & Qt::RightButton)
        m_ignoreFirstRightMousePress = true;
    if (event->buttons() & Qt::MiddleButton)
        m_ignoreFirstMiddleMousePress = true;
}

void EditorCameraController::handleMouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (m_adjustCameraAtMouseRelease) {
        // Make sure we have nice viewCenter at the end of pan/orbit
        adjustCamera(QVector3D());
        m_adjustCameraAtMouseRelease = false;
    }
}

Qt3DRender::QCamera *EditorCameraController::camera() const
{
    return m_camera;
}

void EditorCameraController::setCamera(Qt3DRender::QCamera *camera)
{
    if (m_camera != camera)
        m_camera = camera;
}
