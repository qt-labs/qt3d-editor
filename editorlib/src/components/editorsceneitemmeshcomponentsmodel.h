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
#ifndef EDITORSCENEITEMMESHCOMPONENTSMODEL_H
#define EDITORSCENEITEMMESHCOMPONENTSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QAbstractListModel>

namespace Qt3DRender {
class QGeometryRenderer;
}

class EditorSceneItemComponentsModel;

class EditorSceneItemMeshComponentsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(MeshComponentTypes)
public:
    enum MeshComponentTypes {
        Cuboid = 1,
        Custom,
        Cylinder,
        Plane,
        Sphere,
        Torus,
        SubMeshes, // E.g. SceneLoader entity
        Unknown = 1000
    };

    enum EditorSceneItemMeshComponentsRoles {
        MeshComponentType = Qt::UserRole + 1,
        MeshComponent
    };

    EditorSceneItemMeshComponentsModel(Qt3DRender::QGeometryRenderer *meshComponent,
                                       EditorSceneItemComponentsModel *sceneItemModel,
                                       QObject *parent = 0);
    ~EditorSceneItemMeshComponentsModel();

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void setMesh(MeshComponentTypes type);
    Q_INVOKABLE void removeMesh(int index);

    void beginReplace();
    void endReplace(Qt3DRender::QGeometryRenderer *newMesh);

    static MeshComponentTypes meshType(Qt3DRender::QGeometryRenderer *mesh);

private:
    Qt3DRender::QGeometryRenderer *m_meshComponent;
    EditorSceneItemComponentsModel *m_sceneComponentsModel;
    MeshComponentTypes m_type;
};

Q_DECLARE_METATYPE(EditorSceneItemMeshComponentsModel*)
Q_DECLARE_METATYPE(EditorSceneItemMeshComponentsModel::MeshComponentTypes)

#endif // EDITORSCENEITEMMESHCOMPONENTSMODEL_H
