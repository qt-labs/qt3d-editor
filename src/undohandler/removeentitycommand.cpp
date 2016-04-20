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
#include "removeentitycommand.h"

#include "editorscene.h"
#include "editorsceneitem.h"
#include "editorsceneitemmodel.h"
#include "editorutils.h"

#include <Qt3DCore/QEntity>
#include <QtQml/QQmlEngine>

const QString internalPrefix = QStringLiteral("__internal");

RemoveEntityCommand::RemoveEntityCommand(EditorSceneItemModel *sceneModel,
                                         const QString &entityName) :
    m_sceneModel(sceneModel),
    m_entityName(entityName),
    m_row(0),
    m_removedEntity(nullptr)
{
    setText(QObject::tr("Remove entity"));
}

RemoveEntityCommand::~RemoveEntityCommand()
{
    delete m_removedEntity;
}

void RemoveEntityCommand::undo()
{
    if (m_removedEntity) {
        // Insert the entity back
        QModelIndex parentIndex = m_sceneModel->getModelIndexByName(m_parentEntityName);
        m_sceneModel->insertExistingEntity(m_removedEntity, m_row, parentIndex);
        m_removedEntity = nullptr;
    }
}

void RemoveEntityCommand::redo()
{
    QModelIndex index = m_sceneModel->getModelIndexByName(m_entityName);
    QModelIndex parentIndex = m_sceneModel->parent(index);
    m_parentEntityName = m_sceneModel->entityName(parentIndex);
    m_row = index.row();
    EditorSceneItem *item = m_sceneModel->editorSceneItemFromIndex(index);
    m_removedEntity = m_sceneModel->duplicateEntity(item->entity(), nullptr);
    // Grab explicit ownership of the removed entity,
    // otherwise QML garbage collector may clean it up.
    QQmlEngine::setObjectOwnership(m_removedEntity, QQmlEngine::CppOwnership);
    m_sceneModel->removeEntity(index);
}
