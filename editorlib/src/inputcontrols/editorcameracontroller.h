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

#ifndef EDITORCAMERACONTROLLER_H
#define EDITORCAMERACONTROLLER_H

#include <Qt3DCore/QEntity>
#include <QtGui/QVector3D>
#include <QtGui/QWheelEvent>

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
class QCamera;
}

namespace Qt3DLogic {
class QFrameAction;
}

namespace Qt3DInput {
class QKeyboardController;
class QMouseDevice;
class QLogicalDevice;
class QAction;
class QActionInput;
class QAxis;
class QAnalogAxisInput;
}

class EditorViewportItem;

class EditorCameraController : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    explicit EditorCameraController(EditorViewportItem *viewport,
                                    Qt3DCore::QNode *parent = nullptr);
    ~EditorCameraController();

    Qt3DRender::QCamera *camera() const;
    void setCamera(Qt3DRender::QCamera *camera);

    void handleWheel(QWheelEvent *event);
    void handleMousePress(QMouseEvent *event);
    void handleMouseRelease(QMouseEvent *event);

private Q_SLOTS:
    void handleTriggered(float dt);

private:
    void init();
    void adjustCamera(const QVector3D &translateVec);

    Qt3DRender::QCamera *m_camera;
    Qt3DInput::QAction *m_leftMouseButtonAction;
    Qt3DInput::QAction *m_rightMouseButtonAction;
    Qt3DInput::QAction *m_middleMouseButtonAction;
    Qt3DInput::QAxis *m_rxAxis;
    Qt3DInput::QAxis *m_ryAxis;
    Qt3DInput::QActionInput *m_leftMouseButtonInput;
    Qt3DInput::QActionInput *m_rightMouseButtonInput;
    Qt3DInput::QActionInput *m_middleMouseButtonInput;
    Qt3DInput::QAnalogAxisInput *m_mouseRxInput;
    Qt3DInput::QAnalogAxisInput *m_mouseRyInput;
    Qt3DInput::QMouseDevice *m_mouseDevice;
    Qt3DInput::QLogicalDevice *m_logicalDevice;
    Qt3DLogic::QFrameAction *m_frameAction;
    bool m_ignoreFirstLeftMousePress;
    bool m_ignoreFirstRightMousePress;
    bool m_ignoreFirstMiddleMousePress;
    bool m_adjustCameraAtMouseRelease;
    QVector3D m_cameraUp;
    float m_panSpeed;
    float m_orbitSpeed;
    float m_translateSpeed;
    float m_wheelSpeed;
    EditorViewportItem *m_viewport;
};

QT_END_NAMESPACE

#endif
