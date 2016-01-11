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
#include "editorsceneitemmeshcomponentsmodel.h"
#include "editorsceneitemcomponentsmodel.h"
#include "editorsceneitem.h"
#include "editorscene.h"
#include "undohandler.h"
#include "meshcomponentproxyitem.h"
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCuboidMesh>
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QPlaneMesh>
#include <Qt3DRender/QSphereMesh>
#include <Qt3DRender/QTorusMesh>
#include <Qt3DCore/QEntity>
#include <QtCore/QStack>

EditorSceneItemMeshComponentsModel::EditorSceneItemMeshComponentsModel(
        Qt3DRender::QGeometryRenderer *meshComponent,
        EditorSceneItemComponentsModel *sceneItemModel, QObject *parent)
    : QAbstractListModel(parent)
    , m_meshComponent(meshComponent)
    , m_sceneComponentsModel(sceneItemModel)
    , m_type(Unknown)
{
    if (qobject_cast<Qt3DRender::QMesh *>(meshComponent))
        m_type = Custom;
    else if (qobject_cast<Qt3DRender::QCuboidMesh *>(meshComponent))
        m_type = Cuboid;
    else if (qobject_cast<Qt3DRender::QCylinderMesh *>(meshComponent))
        m_type = Cylinder;
    else if (qobject_cast<Qt3DRender::QPlaneMesh *>(meshComponent))
        m_type = Plane;
    else if (qobject_cast<Qt3DRender::QSphereMesh *>(meshComponent))
        m_type = Sphere;
    else if (qobject_cast<Qt3DRender::QTorusMesh *>(meshComponent))
        m_type = Torus;
}

EditorSceneItemMeshComponentsModel::~EditorSceneItemMeshComponentsModel()
{

}

int EditorSceneItemMeshComponentsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return (m_meshComponent != Q_NULLPTR);
}

QVariant EditorSceneItemMeshComponentsModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)

    if (role == MeshComponentType) {
        return m_type;
    } else if (role == MeshComponent) {
        QVariant meshComponentData;
        if (m_type != Unknown)
            meshComponentData = QVariant::fromValue(m_meshComponent);
        return meshComponentData;
    } else {
        return QVariant();
    }

}

QHash<int, QByteArray> EditorSceneItemMeshComponentsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MeshComponentType] = "meshType";
    roles[MeshComponent] = "meshComponentData";
    return roles;
}

void EditorSceneItemMeshComponentsModel::setMesh(MeshComponentTypes type)
{
    if (type != m_type) {
        Qt3DRender::QGeometryRenderer *mesh = Q_NULLPTR;
        switch (type) {
        case Custom:
            mesh = new Qt3DRender::QMesh();
            break;
        case Cuboid:
            mesh = new Qt3DRender::QCuboidMesh();
            break;
        case Cylinder:
            mesh = new Qt3DRender::QCylinderMesh();
            break;
        case Plane:
            mesh = new Qt3DRender::QPlaneMesh();
            break;
        case Sphere:
            mesh = new Qt3DRender::QSphereMesh();
            break;
        case Torus:
            mesh = new Qt3DRender::QTorusMesh();
            break;
        default:
            //Unsupported mesh type
            break;
        }

        if (mesh == Q_NULLPTR)
            return;

        m_sceneComponentsModel->sceneItem()->scene()->undoHandler()->createReplaceComponentCommand(
                    m_sceneComponentsModel->sceneItem()->entity()->objectName(),
                    EditorSceneItemComponentsModel::Mesh,
                    mesh, m_meshComponent);
    }
}

void EditorSceneItemMeshComponentsModel::removeMesh(int index)
{
    Q_UNUSED(index)

    m_sceneComponentsModel->removeComponent(m_meshComponent);
    m_meshComponent = Q_NULLPTR;
}

void EditorSceneItemMeshComponentsModel::beginReplace()
{
    beginResetModel();
    m_type = Unknown;
    m_meshComponent = Q_NULLPTR;
    endResetModel();
}

void EditorSceneItemMeshComponentsModel::endReplace(Qt3DRender::QGeometryRenderer *newMesh)
{
    beginResetModel();
    m_type = meshType(newMesh);
    m_meshComponent = newMesh;
    endResetModel();
}

EditorSceneItemMeshComponentsModel::MeshComponentTypes
EditorSceneItemMeshComponentsModel::meshType(Qt3DRender::QGeometryRenderer *mesh)
{
    if (qobject_cast<Qt3DRender::QMesh *>(mesh))
        return Custom;
    else if (qobject_cast<Qt3DRender::QCuboidMesh *>(mesh))
        return Cuboid;
    else if (qobject_cast<Qt3DRender::QCylinderMesh *>(mesh))
        return Cylinder;
    else if (qobject_cast<Qt3DRender::QPlaneMesh *>(mesh))
        return Plane;
    else if (qobject_cast<Qt3DRender::QSphereMesh *>(mesh))
        return Sphere;
    else if (qobject_cast<Qt3DRender::QTorusMesh *>(mesh))
        return Torus;
    else
        return Unknown;
}
