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
#ifndef UNDOHANDLER_H
#define UNDOHANDLER_H

#include "editorsceneitemmodel.h"

#include <QtCore/QObject>
#include <Qt3DCore/QComponent>

class EditorScene;
class QUndoStack;

/**
 * UndoHandler is used to expose the QUndoStack to the QML UI.
 */
class UndoHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)

    // Text for the undo and redo actions
    Q_PROPERTY(QString redoText READ redoText NOTIFY redoTextChanged)
    Q_PROPERTY(QString undoText READ undoText NOTIFY undoTextChanged)

public:
    enum MergeableCommandIds {
        PropertyChangeCommandId,
        RenameEntityCommandId
    };

    explicit UndoHandler(EditorScene *scene, QObject *parent = 0);

    // The accessor methods for our properties
    bool canRedo() const;
    bool canUndo() const;
    QString redoText() const;
    QString undoText() const;
    void clear();
    Q_INVOKABLE bool isClean() const;

    Q_INVOKABLE void beginMacro(const QString &macroName = QString());
    Q_INVOKABLE void endMacro();

    // TODO: Add undo support for add/remove components, if that is generally supported.

    Q_INVOKABLE void createInsertEntityCommand(int type, const QString &parentName,
                                               const QVector3D &pos);
    Q_INVOKABLE void createRemoveEntityCommand(const QString &entityName);
    Q_INVOKABLE void createChangePropertyCommand(const QString &entityName,
                                                 int componentType,
                                                 const QString &propertyName,
                                                 const QVariant &newValue,
                                                 const QVariant &oldValue,
                                                 bool pushToStack,
                                                 const QString &text = QString());
    Q_INVOKABLE void createChangePropertyCommand(const QString &entityName,
                                                 int componentType,
                                                 const QString &propertyName,
                                                 const QVariant &newValue,
                                                 const QVariant &oldValue,
                                                 const QString &propertyName2,
                                                 const QVariant &newValue2,
                                                 const QVariant &oldValue2,
                                                 bool pushToStack,
                                                 const QString &text = QString());
    Q_INVOKABLE void createChangeModelRoleCommand(const QString &entityName,
                                                  int componentType,
                                                  int roleIndex,
                                                  const QVariant &newValue,
                                                  const QVariant &oldValue,
                                                  const QString &text = QString());
    Q_INVOKABLE void createRenameEntityCommand(const QString &oldName,
                                               const QString &newName);
    void createReplaceComponentCommand(const QString &entityName,
                                       int componentType,
                                       Qt3DCore::QComponent *newComponent,
                                       Qt3DCore::QComponent *oldComponent);
    Q_INVOKABLE void createDuplicateEntityCommand(const QString &entityName);
    Q_INVOKABLE void createPasteEntityCommand(const QVector3D &pos, const QString &parentName);
    Q_INVOKABLE void createCopyCameraPropertiesCommand(const QString &targetCamera,
                                                       const QString &sourceCamera = QString(),
                                                       const QString &text = QString());
    Q_INVOKABLE void createChangeGenericPropertyCommand(QObject *obj,
                                                        const QString &propertyName,
                                                        const QVariant &newValue,
                                                        const QVariant &oldValue,
                                                        const QString &text = QString());
    Q_INVOKABLE void createReparentEntityCommand(const QString &newParentName,
                                                 const QString &entityName);
    Q_INVOKABLE void createImportEntityCommand(const QUrl &url);
    Q_INVOKABLE void createResetEntityCommand(const QString &entityName);
    Q_INVOKABLE void createResetTransformCommand(const QString &entityName);

signals:
    void canRedoChanged();
    void canUndoChanged();
    void redoTextChanged();
    void undoTextChanged();

public slots:
    void redo();
    void undo();
    void setClean();

private:
    int nonOpCount(bool checkUndos) const;

    QUndoStack *m_undoStack;
    EditorScene *m_scene;
};

#endif // UNDOHANDLER_H
