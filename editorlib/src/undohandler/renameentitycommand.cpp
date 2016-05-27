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
#include "renameentitycommand.h"
#include "undohandler.h"

#include "editorscene.h"
#include "editorsceneitemmodel.h"

RenameEntityCommand::RenameEntityCommand(EditorSceneItemModel *sceneModel,
                                         const QString &oldName, const QString &newName) :
    m_sceneModel(sceneModel),
    m_oldName(oldName),
    m_newName(newName)
{
    setText(QObject::tr("Rename entity"));
}

void RenameEntityCommand::undo()
{
    if (isNonOp())
        return;
    QModelIndex index = m_sceneModel->getModelIndexByName(m_newName);
    m_sceneModel->setEntityName(index, m_oldName);
}

void RenameEntityCommand::redo()
{
    if (isNonOp())
        return;
    QModelIndex index = m_sceneModel->getModelIndexByName(m_oldName);
    QString actualNewName = m_sceneModel->setEntityName(index, m_newName);

    // The new name may have been adjusted due to name conflict
    if (!actualNewName.isEmpty())
        m_newName = actualNewName;
}

bool RenameEntityCommand::mergeWith(const QUndoCommand *other)
{
    // Can merge if both commands target same entity
    if (id() == other->id()) {
        const RenameEntityCommand *otherCommand = static_cast<const RenameEntityCommand *>(other);
        if (otherCommand->m_oldName == m_newName) {
            m_newName = otherCommand->m_newName;
            return true;
        }
    }
    return false;
}

int RenameEntityCommand::id() const
{
    return UndoHandler::RenameEntityCommandId;
}

bool RenameEntityCommand::isNonOp() const
{
    return m_oldName == m_newName;
}

