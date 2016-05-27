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
#include "importentitycommand.h"

#include "editorsceneitem.h"
#include "editorsceneitemmodel.h"

#include <Qt3DCore/QEntity>
#include <QtQml/QQmlEngine>

ImportEntityCommand::ImportEntityCommand(EditorSceneItemModel *sceneModel, const QUrl &url) :
    m_sceneModel(sceneModel),
    m_url(url),
    m_importedEntity(nullptr)
{
    setText(QObject::tr("Import entity"));
}

ImportEntityCommand::~ImportEntityCommand()
{
    delete m_importedEntity;
}

void ImportEntityCommand::undo()
{
    if (m_sceneModel->importEntityInProgress()) {
        m_sceneModel->abortImportEntity();
    } else {
        QModelIndex index = m_sceneModel->getModelIndexByName(m_importedEntityName);
        EditorSceneItem *item = m_sceneModel->editorSceneItemFromIndex(index);
        m_importedEntity = m_sceneModel->duplicateEntity(item->entity(), nullptr);
        // Grab explicit ownership of the duplicated entity,
        // otherwise QML garbage collector may clean it up.
        QQmlEngine::setObjectOwnership(m_importedEntity, QQmlEngine::CppOwnership);

        m_sceneModel->removeEntity(index);
    }
}

void ImportEntityCommand::redo()
{
    if (m_importedEntity) {
        m_sceneModel->insertExistingEntity(m_importedEntity, -1, m_sceneModel->sceneEntityIndex());
        m_importedEntity = nullptr;
    } else {
        m_importedEntityName = m_sceneModel->importEntity(m_url);
    }
}
