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
#ifndef PROPERTYCHANGECOMMAND_H
#define PROPERTYCHANGECOMMAND_H

#include "editorsceneitemcomponentsmodel.h"

#include <QtWidgets/QUndoCommand>

class EditorSceneItemModel;

class PropertyChangeCommand : public QUndoCommand
{
public:
    PropertyChangeCommand(const QString &text, EditorSceneItemModel *sceneModel,
                          const QString &entityName,
                          EditorSceneItemComponentsModel::EditorSceneItemComponentTypes componentType,
                          const QString &propertyName,
                          const QVariant &newValue,
                          const QVariant &oldValue);
    PropertyChangeCommand(const QString &text, EditorSceneItemModel *sceneModel,
                          const QString &entityName,
                          EditorSceneItemComponentsModel::EditorSceneItemComponentTypes componentType,
                          const QString &propertyName,
                          const QVariant &newValue, const QVariant &oldValue,
                          const QString &propertyName2,
                          const QVariant &newValue2, const QVariant &oldValue2);

    virtual void undo();
    virtual void redo();

    virtual bool mergeWith(const QUndoCommand *other);
    virtual int id() const;

    bool isNonOp() const;

private:
    QObject *getTargetObject();
    void setProperty(QObject *obj, const QByteArray &propertyName, const QVariant &value);

    EditorSceneItemModel *m_sceneModel;
    QString m_entityName;
    EditorSceneItemComponentsModel::EditorSceneItemComponentTypes m_componentType;
    QByteArray m_propertyName;
    QVariant m_newValue;
    QVariant m_oldValue;
    QByteArray m_propertyName2;
    QVariant m_newValue2;
    QVariant m_oldValue2;
};

#endif // PROPERTYCHANGECOMMAND_H
