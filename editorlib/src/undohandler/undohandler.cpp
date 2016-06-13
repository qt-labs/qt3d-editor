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
#include "duplicateentitycommand.h"
#include "pasteentitycommand.h"
#include "copycamerapropertiescommand.h"
#include "genericpropertychangecommand.h"
#include "reparententitycommand.h"
#include "importentitycommand.h"
#include "resetentitycommand.h"
#include "resettransformcommand.h"
#include "editorsceneitem.h"

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

bool UndoHandler::isClean() const
{
    return m_undoStack->isClean();
}

void UndoHandler::clear()
{
    m_undoStack->clear();
}

void UndoHandler::beginMacro(const QString &macroName)
{
    m_undoStack->beginMacro(macroName);
}

void UndoHandler::endMacro()
{
    m_undoStack->endMacro();
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
                                              bool pushToStack, const QString &text)
{
    PropertyChangeCommand *command = new PropertyChangeCommand(
                text, m_scene->sceneModel(), entityName,
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

void UndoHandler::createChangePropertyCommand(const QString &entityName, int componentType,
                                              const QString &propertyName,
                                              const QVariant &newValue, const QVariant &oldValue,
                                              const QString &propertyName2,
                                              const QVariant &newValue2, const QVariant &oldValue2,
                                              bool pushToStack, const QString &text)
{
    PropertyChangeCommand *command = new PropertyChangeCommand(
                text, m_scene->sceneModel(), entityName,
                EditorSceneItemComponentsModel::EditorSceneItemComponentTypes(componentType),
                propertyName, newValue, oldValue, propertyName2, newValue2, oldValue2);
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
                                               const QVariant &newValue, const QVariant &oldValue,
                                               const QString &text)
{
    m_undoStack->push(new ModelRoleChangeCommand(text, m_scene->sceneModel(), entityName,
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

void UndoHandler::createDuplicateEntityCommand(const QString &entityName)
{
    if (entityName.isEmpty())
        return;

    m_undoStack->push(new DuplicateEntityCommand(m_scene->sceneModel(), entityName));
}

void UndoHandler::createPasteEntityCommand(const QVector3D &pos, const QString &parentName)
{
    if (m_scene->clipboardContent().isEmpty()
            || m_scene->clipboardOperation() == EditorScene::ClipboardNone) {
        return;
    }

    m_undoStack->push(new PasteEntityCommand(m_scene->sceneModel(),
                                             m_scene->clipboardOperation(),
                                             m_scene->clipboardContent(),
                                             parentName, pos));
}

void UndoHandler::createCopyCameraPropertiesCommand(const QString &targetCamera,
                                                    const QString &sourceCamera,
                                                    const QString &text)
{
    if (sourceCamera == targetCamera)
        return;

    m_undoStack->push(new CopyCameraPropertiesCommand(text, m_scene->sceneModel(), sourceCamera,
                                                      targetCamera));
}

// Note: The obj needs to be guaranteed to not be deleted by subsequent commands
void UndoHandler::createChangeGenericPropertyCommand(QObject *obj,
                                                     const QString &propertyName,
                                                     const QVariant &newValue,
                                                     const QVariant &oldValue,
                                                     const QString &text)
{
    if (!obj || propertyName.isEmpty() || newValue == oldValue)
        return;

    m_undoStack->push(new GenericPropertyChangeCommand(text, obj, propertyName, newValue, oldValue));
}

void UndoHandler::createReparentEntityCommand(const QString &newParentName,
                                              const QString &entityName)
{
    if (newParentName.isEmpty() || entityName.isEmpty())
        return;

    // Don't create commands for illegal reparentings
    EditorSceneItem *newParentItem = m_scene->sceneModel()->getItemByName(newParentName);
    EditorSceneItem *entityItem = m_scene->sceneModel()->getItemByName(entityName);

    if (m_scene->sceneModel()->canReparent(newParentItem, entityItem)) {
        m_undoStack->push(new ReparentEntityCommand(m_scene->sceneModel(), newParentName,
                                                    entityName));
    }
}

void UndoHandler::createImportEntityCommand(const QUrl &url)
{
    if (url.isEmpty() || m_scene->sceneModel()->importEntityInProgress())
        return;

    m_undoStack->push(new ImportEntityCommand(m_scene->sceneModel(), url));
}

void UndoHandler::createResetEntityCommand(const QString &entityName)
{
    m_undoStack->push(new ResetEntityCommand(m_scene->sceneModel(), entityName));
}

void UndoHandler::createResetTransformCommand(const QString &entityName)
{
    m_undoStack->push(new ResetTransformCommand(m_scene->sceneModel(), entityName));
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
