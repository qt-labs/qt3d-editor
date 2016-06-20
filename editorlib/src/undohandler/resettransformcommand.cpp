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
#include "resettransformcommand.h"

#include "editorscene.h"
#include "editorsceneitem.h"
#include "editorsceneitemmodel.h"
#include "editorutils.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <QtQml/QQmlEngine>

ResetTransformCommand::ResetTransformCommand(EditorSceneItemModel *sceneModel,
                                             const QString &entityName) :
    m_sceneModel(sceneModel),
    m_entityName(entityName),
    m_transformMatrix(QMatrix4x4())
{
    setText(QObject::tr("Reset transform"));
}

ResetTransformCommand::~ResetTransformCommand()
{
}

void ResetTransformCommand::undo()
{
    QModelIndex index = m_sceneModel->getModelIndexByName(m_entityName);
    EditorSceneItem *item = m_sceneModel->editorSceneItemFromIndex(index);
    Qt3DCore::QTransform *transform = EditorUtils::entityTransform(item->entity());
    transform->setMatrix(m_transformMatrix);
    m_sceneModel->scene()->queueUpdateGroupSelectionBoxes();
}

void ResetTransformCommand::redo()
{
    QModelIndex index = m_sceneModel->getModelIndexByName(m_entityName);
    EditorSceneItem *item = m_sceneModel->editorSceneItemFromIndex(index);
    Qt3DCore::QTransform *transform = EditorUtils::entityTransform(item->entity());
    m_transformMatrix = transform->matrix();
    transform->setMatrix(QMatrix4x4());
    m_sceneModel->scene()->queueUpdateGroupSelectionBoxes();
}
