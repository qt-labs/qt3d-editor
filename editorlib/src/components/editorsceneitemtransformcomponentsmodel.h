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
#ifndef EDITORSCENEITEMTRANSFORMCOMPONENTSMODEL_H
#define EDITORSCENEITEMTRANSFORMCOMPONENTSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QAbstractListModel>

namespace Qt3DCore {
    class QTransform;
}

class EditorSceneItemComponentsModel;

class EditorSceneItemTransformComponentsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(TransformComponentTypes)
public:
    // TODO: We may need a ComposedMatrix too, which you can create in a similar fashion as the old
    // transform (i.e. add scale, rotation, translate one by one in any order and number).
    enum TransformComponentTypes {
        SRT = 1,
        Matrix,
//        ComposedMatrix,
        // For ComposedMatrix
//        Translate,
//        Rotate,
//        Scale,
        Unknown = 1000
    };

    enum EditorSceneItemTransformComponentsRoles {
        TransformComponentType = Qt::UserRole + 1,
        TransformComponent
    };

    EditorSceneItemTransformComponentsModel(EditorSceneItemComponentsModel *itemmodel,
                                            Qt3DCore::QTransform *transformComponent,
                                            QObject *parent = 0);
    ~EditorSceneItemTransformComponentsModel();

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void setTransform(TransformComponentTypes type);
    Q_INVOKABLE void appendNewTransform(TransformComponentTypes type);
    Q_INVOKABLE void removeTransform(int index);
    Q_INVOKABLE void moveTransform(int from, int to);

private:
    EditorSceneItemComponentsModel *m_sceneComponentsModel;
    Qt3DCore::QTransform *m_transformComponent;
    TransformComponentTypes m_type;
};

Q_DECLARE_METATYPE(EditorSceneItemTransformComponentsModel*)
Q_DECLARE_METATYPE(EditorSceneItemTransformComponentsModel::TransformComponentTypes)

#endif // EDITORSCENEITEMTRANSFORMCOMPONENTSMODEL_H
