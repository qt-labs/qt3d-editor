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
#include "undohandler.h"

#include "editorscene.h"
#include "insertentitycommand.h"
#include "removeentitycommand.h"
#include "renameentitycommand.h"
#include "propertychangecommand.h"
#include "modelrolechangecommand.h"
#include "replacecomponentcommand.h"

#include <QtWidgets/QUndoStack>

UndoHandler::UndoHandler(EditorScene *scene, QObject *parent)
    : QObject(parent),
      m_undoStack(new QUndoStack(this)),
      m_scene(scene)
{
    connect(m_undoStack, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged()));
    connect(m_undoStack, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged()));
    connect(m_undoStack, SIGNAL(redoTextChanged(QString)), this, SIGNAL(redoTextChanged()));
    connect(m_undoStack, SIGNAL(undoTextChanged(QString)), this, SIGNAL(undoTextChanged()));

    // TODO: Do we want a limit and what limit we need?
    m_undoStack->setUndoLimit(1000);

    if (!parent)
        setParent(scene);
}

bool UndoHandler::canRedo() const
{
    int skipCount = nonOpCount(false);
    return ((m_undoStack->index() + skipCount) < m_undoStack->count());
}

bool UndoHandler::canUndo() const
{
    int skipCount = nonOpCount(true);
    return ((m_undoStack->index() - skipCount) > 0);
}

QString UndoHandler::redoText() const
{
    return m_undoStack->redoText();
}

QString UndoHandler::undoText() const
{
    return m_undoStack->undoText();
}

void UndoHandler::clear()
{
    m_undoStack->clear();
}

void UndoHandler::createInsertEntityCommand(int type, const QString &parentName,
                                            const QVector3D &pos)
{
    m_undoStack->push(new InsertEntityCommand(m_scene->sceneModel(),
                                              EditorUtils::InsertableEntities(type),
                                              parentName, pos));
}

void UndoHandler::createRemoveEntityCommand(const QString &entityName)
{
    m_undoStack->push(new RemoveEntityCommand(m_scene->sceneModel(), entityName));
}

void UndoHandler::createChangePropertyCommand(const QString &entityName,
                                              int componentType, const QString &propertyName,
                                              const QVariant &newValue, const QVariant &oldValue,
                                              bool pushToStack)
{
    PropertyChangeCommand *command = new PropertyChangeCommand(m_scene->sceneModel(), entityName,
                                                               EditorSceneItemComponentsModel::EditorSceneItemComponentTypes(componentType),
                                                               propertyName, newValue, oldValue);
    if (pushToStack) {
        m_undoStack->push(command);
    } else {
        // Temporary command, just call redo on it and delete it
        command->redo();
        delete command;
    }
}

void UndoHandler::createChangeModelRoleCommand(const QString &entityName,
                                               int componentType, int roleIndex,
                                               const QVariant &newValue, const QVariant &oldValue)
{
    m_undoStack->push(new ModelRoleChangeCommand(m_scene->sceneModel(), entityName,
                                                 EditorSceneItemComponentsModel::EditorSceneItemComponentTypes(componentType),
                                                 roleIndex, newValue, oldValue));
}

void UndoHandler::createRenameEntityCommand(const QString &oldName, const QString &newName)
{
    if (newName.isEmpty() || newName == oldName)
        return;
    m_undoStack->push(new RenameEntityCommand(m_scene->sceneModel(), oldName, newName));
}

void UndoHandler::createReplaceComponentCommand(const QString &entityName, int componentType,
                                                Qt3DCore::QComponent *newComponent,
                                                Qt3DCore::QComponent *oldComponent)
{
    if (newComponent == oldComponent)
        return;

    m_undoStack->push(new ReplaceComponentCommand(m_scene->sceneModel(), entityName,
                                                  EditorSceneItemComponentsModel::EditorSceneItemComponentTypes(componentType),
                                                  newComponent, oldComponent));
}

void UndoHandler::redo()
{
    int doCount = nonOpCount(false) + 1;
    while (doCount--)
        m_undoStack->redo();
}

void UndoHandler::undo()
{
    int doCount = nonOpCount(true) + 1;
    while (doCount--)
        m_undoStack->undo();
}

void UndoHandler::setClean()
{
    m_undoStack->setClean();
}

int UndoHandler::nonOpCount(bool checkUndos) const
{
    int count = 0;
    int index = m_undoStack->index();
    int increment = 1;
    if (checkUndos) {
        index--;
        increment = -1;
    }
    while (index >= 0 && index < m_undoStack->count()) {
        const QUndoCommand *nextCommand = m_undoStack->command(index);
        if (nextCommand->id() == PropertyChangeCommandId) {
            const PropertyChangeCommand *propertyCommand =
                    static_cast<const PropertyChangeCommand *>(nextCommand);
            if (propertyCommand->isNonOp())
                count++;
            else
                break;
        } else if (nextCommand->id() == RenameEntityCommandId) {
            const RenameEntityCommand *renameCommand =
                    static_cast<const RenameEntityCommand *>(nextCommand);
            if (renameCommand->isNonOp())
                count++;
            else
                break;
        } else {
            break;
        }
        index += increment;
    }
    return count;
}
