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

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QSurface>
#include <QQuickWindow>

#include <QSGSimpleTextureNode>
#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DInput/QInputAspect>
#include <Qt3DCore/QCamera>

class ContextSaver
{
public:
    explicit ContextSaver(QOpenGLContext *context = QOpenGLContext::currentContext())
        : m_context(context)
        , m_surface(context ? context->surface() : Q_NULLPTR)
    {
    }

    ~ContextSaver()
    {
        if (m_context)
            m_context->makeCurrent(m_surface);
    }

    QOpenGLContext *context() const { return m_context; }
    QSurface *surface() const { return m_surface; }

private:
    QOpenGLContext *const m_context;
    QSurface *const m_surface;
};

class FrameBufferObjectRenderer : public QQuickFramebufferObject::Renderer
{
public:
    FrameBufferObjectRenderer(EditorViewportItem *item,
                              Qt3DCore::QAspectEngine *aspectEngine,
                              Qt3DRender::QRenderAspect *renderAspect,
                              Qt3DInput::QInputAspect *inputAspect)
        : m_item(item)
        , m_aspectEngine(aspectEngine)
        , m_renderAspect(renderAspect)
        , m_inputAspect(inputAspect)
    {
        ContextSaver saver;

        QVariantMap data;
        data.insert(QStringLiteral("surface"), QVariant::fromValue(saver.surface()));
        m_aspectEngine->setData(data);

        m_renderAspect->renderInitialize(saver.context());
        scheduleRootEntityChange();
    }

    void render() Q_DECL_OVERRIDE
    {
        if (m_item->scene() != Q_NULLPTR && m_aspectEngine->rootEntity()
                != m_item->scene()->rootEntity()) {
            scheduleRootEntityChange();
        }

        ContextSaver saver;
        Q_UNUSED(saver)

        m_renderAspect->renderSynchronous();

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
};

EditorViewportItem::EditorViewportItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
    , m_scene(Q_NULLPTR)
    , m_aspectEngine(new Qt3DCore::QAspectEngine(this))
    , m_renderAspect(new Qt3DRender::QRenderAspect(Qt3DRender::QRenderAspect::Synchronous))
    , m_inputAspect(new Qt3DInput::QInputAspect)
    , m_inputEnabled(false)
{
    setFlag(ItemHasContents, true);
    m_aspectEngine->registerAspect(m_renderAspect);
    m_aspectEngine->registerAspect(m_inputAspect);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

EditorViewportItem::~EditorViewportItem()
{
    // TODO: When Qt3D supports shutting down renderer, enable this again.
    // TODO: Until then, just don't do it, as it crashes the application.
    // TODO: It also means we cannot dynamically create and destroy viewports without memory leak.
    //m_renderAspect->renderShutdown();
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
        // TODO: maybe we also want non-free camera positions to be adjustable via input?
        // TODO: Note that QInputAspect::setCamera() takes QCamera parameter instead of generic
        // TODO: QEntity, so we will need to limit scene cameras to QCameras in that case.
        if (m_inputEnabled)
            m_inputAspect->setCamera(m_scene->m_freeViewCameraEntity);
        else
            m_inputAspect->setCamera(Q_NULLPTR);
        emit inputEnabledChanged(m_inputEnabled);
    }
}

void EditorViewportItem::setScene(EditorScene *scene)
{
    if (m_scene == scene)
        return;

    m_scene = scene;
    emit sceneChanged(scene);

    update();
}

void EditorViewportItem::applyRootEntityChange()
{
    if (m_scene != Q_NULLPTR && m_scene->rootEntity() != m_aspectEngine->rootEntity()) {
        m_aspectEngine->setRootEntity(m_scene->rootEntity());
        QVariantMap data;
        data.insert(QStringLiteral("surface"), QVariant::fromValue(static_cast<QSurface *>(window())));
        data.insert(QStringLiteral("eventSource"), QVariant::fromValue(this));
        m_aspectEngine->setData(data);
    }
}

QQuickFramebufferObject::Renderer *EditorViewportItem::createRenderer() const
{
    EditorViewportItem *self = const_cast<EditorViewportItem*>(this);
    return new FrameBufferObjectRenderer(self, m_aspectEngine, m_renderAspect, m_inputAspect);
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

void EditorViewportItem::wheelEvent(QWheelEvent *event)
{
    if (!m_inputEnabled)
        return;

    if (event->buttons() & Qt::RightButton) {
        // Change camera FOV
        float fov = m_scene->m_freeViewCameraEntity->lens()->fieldOfView();
        fov -= event->angleDelta().y() / 120.0f;
        fov = qBound(0.0f, fov, 180.0f);
        m_scene->m_freeViewCameraEntity->lens()->setFieldOfView(fov);
    } else {
        // Change camera position
        QVector3D position = m_scene->m_freeViewCameraEntity->position();
        position /= (1.0 + event->angleDelta().y() / 1200.0f);
        m_scene->m_freeViewCameraEntity->setPosition(position);
    }
    event->accept();
}
