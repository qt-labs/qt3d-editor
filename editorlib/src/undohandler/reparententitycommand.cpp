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
#include "reparententitycommand.h"

#include "editorscene.h"
#include "editorsceneitemmodel.h"

#include <Qt3DCore/QEntity>

ReparentEntityCommand::ReparentEntityCommand(EditorSceneItemModel *sceneModel,
                                             const QString &newParentName,
                                             const QString &entityName) :
    m_sceneModel(sceneModel),
    m_newParentName(newParentName),
    m_entityName(entityName)
{
    setText(QObject::tr("Reparent entity"));
}

void ReparentEntityCommand::undo()
{
    QModelIndex newParentIndex = m_sceneModel->getModelIndexByName(m_originalParentName);
    QModelIndex entityIndex = m_sceneModel->getModelIndexByName(m_entityName);
    m_sceneModel->reparentEntity(newParentIndex, entityIndex);
}

void ReparentEntityCommand::redo()
{
    QModelIndex newParentIndex = m_sceneModel->getModelIndexByName(m_newParentName);
    QModelIndex entityIndex = m_sceneModel->getModelIndexByName(m_entityName);
    if (m_originalParentName.isEmpty())
        m_originalParentName = m_sceneModel->entityName(m_sceneModel->parent(entityIndex));
    m_sceneModel->reparentEntity(newParentIndex, entityIndex);
}
