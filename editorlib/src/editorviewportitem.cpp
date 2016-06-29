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
#include "editorviewportitem.h"
#include "editorscene.h"
#include "editorcameracontroller.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOffscreenSurface>
#include <QQuickWindow>

#include <QSGSimpleTextureNode>
#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QInputSettings>
#include <Qt3DLogic/QLogicAspect>

#include <Qt3DRender/private/qrenderaspect_p.h>

class ContextSaver
{
public:
    explicit ContextSaver(QOpenGLContext *context = QOpenGLContext::currentContext())
        : m_context(context)
        , m_surface(context ? static_cast<QOffscreenSurface *>(context->surface()) : nullptr)
    {
    }

    ~ContextSaver()
    {
        if (m_context)
            m_context->makeCurrent(m_surface);
    }

    QOpenGLContext *context() const { return m_context; }
    QOffscreenSurface *surface() const { return m_surface; }

private:
    QOpenGLContext *const m_context;
    QOffscreenSurface *const m_surface;
};

class FrameBufferObjectRenderer : public QQuickFramebufferObject::Renderer
{
public:
    FrameBufferObjectRenderer(EditorViewportItem *item,
                              Qt3DCore::QAspectEngine *aspectEngine,
                              Qt3DRender::QRenderAspect *renderAspect,
                              Qt3DInput::QInputAspect *inputAspect,
                              Qt3DLogic::QLogicAspect *logicAspect,
                              QOffscreenSurface *surface)
        : m_item(item)
        , m_aspectEngine(aspectEngine)
        , m_renderAspect(renderAspect)
        , m_inputAspect(inputAspect)
        , m_logicAspect(logicAspect)
        , m_surface(surface)
    {
        ContextSaver saver;
        // HACK: Use dummy surface for renderer to work around crash when running in creator
        // TODO: Remove hack when QTBUG-54318 is fixed
        m_item->scene()->renderer()->setSurface(reinterpret_cast<QObject *>(m_surface));
        static_cast<Qt3DRender::QRenderAspectPrivate *>(
                    Qt3DRender::QRenderAspectPrivate::get(m_renderAspect))
                ->renderInitialize(saver.context());
        scheduleRootEntityChange();
    }

    void render() Q_DECL_OVERRIDE
    {
        if (m_item->scene() != nullptr && m_aspectEngine->rootEntity()
                != m_item->scene()->rootEntity()) {
            scheduleRootEntityChange();
        }

        ContextSaver saver;

        static_cast<Qt3DRender::QRenderAspectPrivate *>(
                    Qt3DRender::QRenderAspectPrivate::get(m_renderAspect))->renderSynchronous();

        // We may have called doneCurrent() so restore the context.
        saver.context()->makeCurrent(saver.surface());

        // Reset the state used by the Qt Quick scenegraph to avoid any
        // interference when rendering the rest of the UI.
        m_item->window()->resetOpenGLState();

        update();
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) Q_DECL_OVERRIDE
    {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);
        return new QOpenGLFramebufferObject(size, format);
    }

    void scheduleRootEntityChange()
    {
        QMetaObject::invokeMethod(m_item, "applyRootEntityChange", Qt::QueuedConnection);
    }

    EditorViewportItem *m_item;
    Qt3DCore::QAspectEngine *m_aspectEngine;
    Qt3DRender::QRenderAspect *m_renderAspect;
    Qt3DInput::QInputAspect *m_inputAspect;
    Qt3DLogic::QLogicAspect *m_logicAspect;
    QOffscreenSurface *m_surface;
};

EditorViewportItem::EditorViewportItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
    , m_scene(nullptr)
    , m_aspectEngine(new Qt3DCore::QAspectEngine(this))
    , m_renderAspect(new Qt3DRender::QRenderAspect(Qt3DRender::QRenderAspect::Synchronous))
    , m_inputAspect(new Qt3DInput::QInputAspect)
    , m_logicAspect(new Qt3DLogic::QLogicAspect)
    , m_cameraController(nullptr)
    , m_inputEnabled(true)
    , m_surface(nullptr)
{
    setFlag(ItemHasContents, true);
    m_aspectEngine->registerAspect(m_renderAspect);
    m_aspectEngine->registerAspect(m_inputAspect);
    m_aspectEngine->registerAspect(m_logicAspect);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    connect(this, &QQuickItem::windowChanged, this, &EditorViewportItem::handleWindowChanged);
}

EditorViewportItem::~EditorViewportItem()
{
    if (m_renderAspect) {
        static_cast<Qt3DRender::QRenderAspectPrivate *>
            (Qt3DRender::QRenderAspectPrivate::get(m_renderAspect))->renderShutdown();
    }

    if (m_surface)
        m_surface->deleteLater();
}

EditorScene *EditorViewportItem::scene() const
{
    return m_scene;
}

bool EditorViewportItem::inputEnabled() const
{
    return m_inputEnabled;
}

void EditorViewportItem::setInputEnabled(bool enable)
{
    if (enable != m_inputEnabled) {
        m_inputEnabled = enable;
        handleInputCameraChange();
        emit inputEnabledChanged(m_inputEnabled);
    }
}

void EditorViewportItem::handleWindowChanged(QQuickWindow *win)
{
    if (!m_surface && win) {
        m_surface = new QOffscreenSurface;
        QSurfaceFormat format = win->format();
        format.setSamples(4);
        m_surface->setFormat(format);
        m_surface->create();
    }
}

void EditorViewportItem::setScene(EditorScene *scene)
{
    if (m_scene == scene)
        return;

    m_scene = scene;

    connect(scene, &EditorScene::activeSceneCameraIndexChanged,
            this, &EditorViewportItem::handleInputCameraChange);
    connect(scene, &EditorScene::freeViewChanged,
            this, &EditorViewportItem::handleInputCameraChange);

    emit sceneChanged(scene);

    update();
}

void EditorViewportItem::applyRootEntityChange()
{
    if (m_scene != nullptr && m_scene->rootEntity() != m_aspectEngine->rootEntity()) {
        m_aspectEngine->setRootEntity(Qt3DCore::QEntityPtr(m_scene->rootEntity()));

        m_cameraController = new EditorCameraController(this, m_scene->rootEntity());
        if (m_inputEnabled)
            m_cameraController->setCamera(m_scene->inputCamera());
    }
}

void EditorViewportItem::handleInputCameraChange()
{
    if (m_cameraController) {
        if (m_inputEnabled)
            m_cameraController->setCamera(m_scene->inputCamera());
        else
            m_cameraController->setCamera(nullptr);
    }
}

QQuickFramebufferObject::Renderer *EditorViewportItem::createRenderer() const
{
    EditorViewportItem *self = const_cast<EditorViewportItem*>(this);
    return new FrameBufferObjectRenderer(self, m_aspectEngine, m_renderAspect,
                                         m_inputAspect, m_logicAspect, m_surface);
}

QSGNode *EditorViewportItem::updatePaintNode(QSGNode *node,
                                             QQuickItem::UpdatePaintNodeData *nodeData)
{
    node = QQuickFramebufferObject::updatePaintNode(node, nodeData);
    QSGSimpleTextureNode *textureNode = static_cast<QSGSimpleTextureNode *>(node);
    if (textureNode)
        textureNode->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    return node;
}

void EditorViewportItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickFramebufferObject::geometryChanged(newGeometry, oldGeometry);

    // Find surface selector in framegraph and set the area
    Qt3DRender::QRenderSettings *renderSettings
            = m_scene->rootEntity()->findChild<Qt3DRender::QRenderSettings *>();
    if (renderSettings) {
        Qt3DCore::QNode *frameGraphRoot = renderSettings->activeFrameGraph();
        if (frameGraphRoot) {
            Qt3DRender::QRenderSurfaceSelector *surfaceSelector
                    = frameGraphRoot->findChild<Qt3DRender::QRenderSurfaceSelector *>();
            if (surfaceSelector)
                surfaceSelector->setExternalRenderTargetSize(newGeometry.size().toSize());
        }
    }
}

void EditorViewportItem::mousePressEvent(QMouseEvent *event)
{
    if (m_inputEnabled && m_cameraController) {
        m_cameraController->handleMousePress(event);
        event->accept();
    } else {
        event->ignore();
    }
}

void EditorViewportItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_inputEnabled)
        event->accept();
    else
        event->ignore();
}

void EditorViewportItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_inputEnabled && m_cameraController) {
        m_cameraController->handleMouseRelease(event);
        event->accept();
    } else {
        event->ignore();
    }
}

void EditorViewportItem::wheelEvent(QWheelEvent *event)
{
    if (m_inputEnabled && m_cameraController)
        m_cameraController->handleWheel(event);
    else
        event->ignore();
}
