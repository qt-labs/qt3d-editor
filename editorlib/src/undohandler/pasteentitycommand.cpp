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
#include "pasteentitycommand.h"

#include "editorsceneitem.h"
#include "editorsceneitemmodel.h"

#include <Qt3DCore/QEntity>
#include <QtQml/QQmlEngine>

PasteEntityCommand::PasteEntityCommand(EditorSceneItemModel *sceneModel,
                                       EditorScene::ClipboardOperation operation,
                                       const QString &entityName,
                                       const QVector3D &pos) :
    m_sceneModel(sceneModel),
    m_operation(operation),
    m_cutEntityName(entityName),
    m_pastePosition(pos),
    m_cutEntity(nullptr)
{
    setText(QObject::tr("Paste entity"));
}

PasteEntityCommand::~PasteEntityCommand()
{
    delete m_cutEntity;
}

void PasteEntityCommand::undo()
{
    if (!m_pastedEntityName.isEmpty()) {
        // Remove pasted entity
        QModelIndex index = m_sceneModel->getModelIndexByName(m_pastedEntityName);
        m_sceneModel->removeEntity(index);
        m_pastedEntityName.clear();
        // If was a cut operation, add the previously removed entity
        if (m_operation == EditorScene::ClipboardCut && m_cutEntity) {
            // Insert the entity back
            QModelIndex parentIndex = m_sceneModel->getModelIndexByName(m_cutParentEntityName);
            m_sceneModel->insertExistingEntity(m_cutEntity, m_cutFromRow, parentIndex);
            // Return selection to it, as it's invisible at start
            index = m_sceneModel->getModelIndex(m_cutEntity);
            EditorSceneItem *item = m_sceneModel->editorSceneItemFromIndex(index);
            item->scene()->setSelection(m_cutEntity);
            m_cutEntity = nullptr;
        }
    }
}

void PasteEntityCommand::redo()
{
    // Duplicate m_cutEntity to m_pastePosition
    QModelIndex index = m_sceneModel->getModelIndexByName(m_cutEntityName);
    if (!index.isValid())
        return;

    QModelIndex parentIndex = m_sceneModel->parent(index);
    EditorSceneItem *item = m_sceneModel->editorSceneItemFromIndex(index);
    Qt3DCore::QEntity *pastedEntity = m_sceneModel->duplicateEntity(item->entity(), nullptr,
                                                                    m_pastePosition, false);
    // Delete selected if cut operation
    if (m_operation == EditorScene::ClipboardCut) {
        m_cutEntity = m_sceneModel->duplicateEntity(item->entity(), nullptr);
        m_cutParentEntityName = m_sceneModel->entityName(parentIndex);
        m_cutFromRow = index.row();
        // Grab explicit ownership of the removed entity,
        // otherwise QML garbage collector may clean it up.
        QQmlEngine::setObjectOwnership(m_cutEntity, QQmlEngine::CppOwnership);
        // Delete original
        m_sceneModel->removeEntity(index);
    } else {
        EditorUtils::nameDuplicate(pastedEntity, item->entity(), m_sceneModel);
    }
    m_pastedEntityName = pastedEntity->objectName();
    m_sceneModel->insertExistingEntity(pastedEntity, -1, parentIndex);
    item->scene()->setSelection(pastedEntity);
}
