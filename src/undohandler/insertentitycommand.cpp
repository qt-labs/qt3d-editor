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
#include "insertentitycommand.h"

#include "editorscene.h"
#include "editorsceneitemmodel.h"

#include <Qt3DCore/QEntity>

InsertEntityCommand::InsertEntityCommand(EditorSceneItemModel *sceneModel,
                                         EditorUtils::InsertableEntities type,
                                         const QString &parentName, const QVector3D &pos) :
    m_sceneModel(sceneModel),
    m_type(type),
    m_parentName(parentName),
    m_insertPosition(pos)
{
    setText(QObject::tr("Add new entity"));
}

void InsertEntityCommand::undo()
{
    QModelIndex index = m_sceneModel->getModelIndexByName(m_insertedEntityName);
    m_sceneModel->removeEntity(index);
    m_insertedEntityName.clear();
}

void InsertEntityCommand::redo()
{
    QModelIndex parentIndex = m_sceneModel->getModelIndexByName(m_parentName);
    Qt3DCore::QEntity *entity = m_sceneModel->insertEntity(m_type, m_insertPosition, parentIndex);
    m_insertedEntityName = entity->objectName();
}
