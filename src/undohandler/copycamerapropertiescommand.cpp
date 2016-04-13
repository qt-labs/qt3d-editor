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
#include "copycamerapropertiescommand.h"
#include "undohandler.h"

#include "editorscene.h"
#include "editorsceneitemmodel.h"
#include "editorsceneitem.h"

#include <Qt3DRender/QCamera>
#include <QtQml/QQmlEngine>

CopyCameraPropertiesCommand::CopyCameraPropertiesCommand(const QString &text,
                                                         EditorSceneItemModel *sceneModel,
                                                         const QString &sourceCamera,
                                                         const QString &targetCamera) :
    m_sceneModel(sceneModel),
    m_sourceCamera(sourceCamera),
    m_targetCamera(targetCamera),
    m_originalSource(nullptr),
    m_originalTarget(nullptr)
{
    if (text.isEmpty())
        setText(QObject::tr("Copy camera properties"));
    else
        setText(text);
}

CopyCameraPropertiesCommand::~CopyCameraPropertiesCommand()
{
    delete m_originalSource;
    delete m_originalTarget;
}

void CopyCameraPropertiesCommand::undo()
{
    Qt3DRender::QCamera *targetEntity = cameraEntity(m_targetCamera);
    EditorUtils::copyCameraProperties(targetEntity, m_originalTarget);
}

void CopyCameraPropertiesCommand::redo()
{
    Qt3DRender::QCamera *sourceEntity = cameraEntity(m_sourceCamera);
    Qt3DRender::QCamera *targetEntity = cameraEntity(m_targetCamera);

    if (!m_originalSource) {
        m_originalSource = new Qt3DRender::QCamera;
        m_originalTarget = new Qt3DRender::QCamera;

        // Grab explicit ownership of the property holder cameras,
        // otherwise QML garbage collector may clean them up.
        QQmlEngine::setObjectOwnership(m_originalSource, QQmlEngine::CppOwnership);
        QQmlEngine::setObjectOwnership(m_originalTarget, QQmlEngine::CppOwnership);

        // Store original values, as either of the cameras may be freeview camera, which
        // may move around between undo/redo.
        EditorUtils::copyCameraProperties(m_originalSource, sourceEntity);
        EditorUtils::copyCameraProperties(m_originalTarget, targetEntity);
    }

    EditorUtils::copyCameraProperties(targetEntity, m_originalSource);
}

Qt3DRender::QCamera *CopyCameraPropertiesCommand::cameraEntity(const QString &cameraName)
{
    Qt3DRender::QCamera *cameraEntity = nullptr;

    if (!cameraName.isEmpty()) {
        QModelIndex index = m_sceneModel->getModelIndexByName(cameraName);
        EditorSceneItem *item = m_sceneModel->editorSceneItemFromIndex(index);
        cameraEntity = qobject_cast<Qt3DRender::QCamera *>(item->entity());
    }

    // Empty source or target camera name indicates freeview camera of the scene
    if (!cameraEntity)
        cameraEntity = m_sceneModel->scene()->freeViewCamera();

    return cameraEntity;
}
