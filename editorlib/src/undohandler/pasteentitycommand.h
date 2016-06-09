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
#ifndef PASTEENTITYCOMMAND_H
#define PASTEENTITYCOMMAND_H

#include "editorutils.h"
#include "editorscene.h"

#include <QtWidgets/QUndoCommand>
#include <QtGui/QVector3D>

class EditorSceneItemModel;

namespace Qt3DCore {
class QEntity;
}

class PasteEntityCommand : public QUndoCommand
{
public:
    PasteEntityCommand(EditorSceneItemModel *sceneModel, EditorScene::ClipboardOperation operation,
                       const QString &entityName, const QString &parentName, const QVector3D &pos);
    virtual ~PasteEntityCommand();

    virtual void undo();
    virtual void redo();

private:
    EditorSceneItemModel *m_sceneModel;
    EditorScene::ClipboardOperation m_operation;
    QString m_pastedEntityName;
    Qt3DCore::QEntity *m_cutEntity;
    QString m_cutEntityName;
    QString m_parentName;
    QVector3D m_pastePosition;
    QString m_cutParentEntityName;
    int m_cutFromRow;
};

#endif // PASTEENTITYCOMMAND_H
