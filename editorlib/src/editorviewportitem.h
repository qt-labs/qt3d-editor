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
#ifndef EDITORVIEWPORTITEM_H
#define EDITORVIEWPORTITEM_H

#include <QtQuick/QQuickFramebufferObject>

class EditorScene;
class EditorCameraController;

class QOffscreenSurface;

namespace Qt3DCore
{
class QAspectEngine;
class QEntity;
}

namespace Qt3DRender
{
class QRenderAspect;
}

namespace Qt3DInput
{
class QInputAspect;
}

namespace Qt3DLogic {
class QLogicAspect;
}

class EditorViewportItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(EditorScene* scene READ scene WRITE setScene NOTIFY sceneChanged)
    Q_PROPERTY(bool inputEnabled READ inputEnabled WRITE setInputEnabled NOTIFY inputEnabledChanged)
public:
    EditorViewportItem(QQuickItem *parent = 0);
    ~EditorViewportItem();

    EditorScene* scene() const;

    bool inputEnabled() const;
    void setInputEnabled(bool enable);
    void handleWindowChanged(QQuickWindow *win);

public slots:
    void setScene(EditorScene* scene);

private slots:
    void applyRootEntityChange();
    void handleInputCameraChange();

signals:
    void sceneChanged(EditorScene* scene);
    void inputEnabledChanged(bool enabled);

protected:
    Renderer *createRenderer() const Q_DECL_OVERRIDE;
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *nodeData) Q_DECL_OVERRIDE;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private:
    EditorScene* m_scene;

    Qt3DCore::QAspectEngine *m_aspectEngine;
    Qt3DRender::QRenderAspect *m_renderAspect;
    Qt3DInput::QInputAspect *m_inputAspect;
    Qt3DLogic::QLogicAspect *m_logicAspect;
    EditorCameraController *m_cameraController;

    bool m_inputEnabled;
    QOffscreenSurface *m_surface;
};

#endif // EDITORVIEWPORTITEM_H
