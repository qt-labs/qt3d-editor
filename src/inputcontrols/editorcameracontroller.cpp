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
#include <Qt3DRender/QCamera>
#include <Qt3DInput/QAxis>
#include <Qt3DInput/QAxisInput>
#include <Qt3DInput/QAction>
#include <Qt3DInput/QActionInput>
#include <Qt3DInput/QLogicalDevice>
#include <Qt3DInput/QKeyboardController>
#include <Qt3DInput/QMouseDevice>
#include <Qt3DInput/QMouseEvent>
#include <Qt3DInput/QMouseHandler>
#include <Qt3DLogic/QFrameAction>

EditorCameraController::EditorCameraController(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
    , m_camera(Q_NULLPTR)
    , m_leftMouseButtonAction(new Qt3DInput::QAction())
    , m_rightMouseButtonAction(new Qt3DInput::QAction())
    , m_rxAxis(new Qt3DInput::QAxis())
    , m_ryAxis(new Qt3DInput::QAxis())
    , m_leftMouseButtonInput(new Qt3DInput::QActionInput())
    , m_rightMouseButtonInput(new Qt3DInput::QActionInput())
    , m_mouseRxInput(new Qt3DInput::QAxisInput())
    , m_mouseRyInput(new Qt3DInput::QAxisInput())
    , m_mouseDevice(new Qt3DInput::QMouseDevice())
    , m_logicalDevice(new Qt3DInput::QLogicalDevice())
    , m_frameAction(new Qt3DLogic::QFrameAction())
    , m_ignorefirstLeftMousePress(false)
    , m_ignorefirstRightMousePress(false)
    , m_cameraUp(QVector3D(0.0f, 1.0f, 0.0f))
    , m_lookSpeed(270.0f)
    , m_translateSpeed(50.0f)
    , m_wheelSpeed(-0.05f)
{
    init();
}

EditorCameraController::~EditorCameraController()
{
}

void EditorCameraController::init()
{
    // Left Mouse Button Action
    m_leftMouseButtonInput->setButtons(QVariantList() << Qt3DInput::QMouseEvent::LeftButton);
    m_leftMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_leftMouseButtonAction->addInput(m_leftMouseButtonInput);

    // Right Mouse Button Action
    m_rightMouseButtonInput->setButtons(QVariantList() << Qt3DInput::QMouseEvent::RightButton);
    m_rightMouseButtonInput->setSourceDevice(m_mouseDevice);
    m_rightMouseButtonAction->addInput(m_rightMouseButtonInput);

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
    m_logicalDevice->addAxis(m_rxAxis);
    m_logicalDevice->addAxis(m_ryAxis);

    QObject::connect(m_frameAction, &Qt3DLogic::QFrameAction::triggered,
                     this, &EditorCameraController::handleTriggered);

    addComponent(m_frameAction);
    addComponent(m_logicalDevice);
}

void EditorCameraController::handleTriggered(float dt)
{
    if (m_camera != Q_NULLPTR) {
        if (m_rightMouseButtonAction->isActive()) {
            // Ignore first press so you don't get the initial jolt,
            // as the mouse delta is usually maximum into some direction.
            if (m_ignorefirstRightMousePress) {
                m_ignorefirstRightMousePress = false;
            } else {
                QMatrix4x4 viewMatrix = m_camera->viewMatrix().inverted();
                QVector3D translateVec(m_rxAxis->value() * m_translateSpeed * dt,
                                       m_ryAxis->value() * m_translateSpeed * dt, 0.0f);
                translateVec = viewMatrix.mapVector(translateVec);
                m_camera->setPosition(m_camera->position() + translateVec);
                m_camera->setViewCenter(m_camera->viewCenter() + translateVec);
            }
        } else if (m_leftMouseButtonAction->isActive()) {
            // Ignore first press so you don't get the initial jolt,
            // as the mouse delta is usually maximum into some direction.
            if (m_ignorefirstLeftMousePress) {
                m_ignorefirstLeftMousePress = false;
            } else {
                m_camera->pan(m_rxAxis->value() * m_lookSpeed * dt, m_cameraUp);
                m_camera->tilt(m_ryAxis->value() * m_lookSpeed * dt);
            }
        }
    }
}

void EditorCameraController::handleWheel(QWheelEvent *event)
{
    if (event->buttons() & Qt::RightButton) {
        // Change camera FOV
        float fov = m_camera->fieldOfView();
        fov -= event->angleDelta().y() / 120.0f;
        fov = qBound(0.0f, fov, 180.0f);
        m_camera->setFieldOfView(fov);
    } else {
        // Change camera position
        QMatrix4x4 viewMatrix = m_camera->viewMatrix().inverted();
        QVector3D translateVec(0.0f, 0.0f, event->angleDelta().y() * m_wheelSpeed);
        translateVec = viewMatrix.mapVector(translateVec);
        m_camera->setPosition(m_camera->position() + translateVec);
        m_camera->setViewCenter(m_camera->viewCenter() + translateVec);
    }
}

void EditorCameraController::handleMousePress(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        m_ignorefirstLeftMousePress = true;
    if (event->buttons() & Qt::RightButton)
        m_ignorefirstRightMousePress = true;
}

void EditorCameraController::handleMouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event)
}

Qt3DRender::QCamera *EditorCameraController::camera() const
{
    return m_camera;
}

void EditorCameraController::setCamera(Qt3DRender::QCamera *camera)
{
    if (m_camera != camera) {
        m_camera = camera;
        emit cameraChanged();
    }
}
