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
#include "editorscene.h"
#include "editorsceneitem.h"
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QComponent>
#include <Qt3DCore/QCamera>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QCuboidMesh>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QPlaneMesh>
#include <Qt3DRender/QSphereMesh>
#include <Qt3DRender/QTorusMesh>
#include <Qt3DRender/QLight>

#include "editorsceneitemcomponentsmodel.h"

EditorSceneItem::EditorSceneItem(EditorScene *scene, Qt3DCore::QEntity *entity,
                                 EditorSceneItem *parentItem,
                                 int index, bool freeView, QObject *parent)
    : QObject(parent)
    , m_entity(entity)
    , m_parentItem(parentItem)
    , m_componentsModel(new EditorSceneItemComponentsModel(this, this))
    , m_freeView(freeView)
    , m_scene(scene)
    , m_selectionBox(Q_NULLPTR)
    , m_selectionTransform(Q_NULLPTR)
    , m_entityTransform(Q_NULLPTR)
    , m_entityMesh(Q_NULLPTR)
    , m_entityMeshType(EditorSceneItemMeshComponentsModel::Unknown)
    , m_entityMeshExtents(1.0f, 1.0f, 1.0f)
{
    if (m_parentItem != Q_NULLPTR)
        m_parentItem->addChild(this, index);

    // Selection box
    Qt3DCore::QComponentList components = entity->components();
    Qt3DRender::QGeometryRenderer *entityMesh = Q_NULLPTR;
    bool isLight = false;
    for (int i = 0; i < components.size(); i++) {
        if (!m_entityTransform)
            m_entityTransform = qobject_cast<Qt3DCore::QTransform *>(components.value(i));
        if (!entityMesh)
            entityMesh = qobject_cast<Qt3DRender::QGeometryRenderer *>(components.value(i));
        if (qobject_cast<Qt3DRender::QLight *>(components.value(i)))
            isLight = true;
    }
    bool isCamera = qobject_cast<Qt3DCore::QCamera *>(entity);
    if (isCamera)
        m_entityMeshExtents = QVector3D(1.5f, 1.5f, 2.0f);

    // Selection transform is need for child items, even if we don't have a box
    m_selectionTransform = new Qt3DCore::QTransform;

    // Don't show box itself unless the entity has mesh
    if (entityMesh || isLight || isCamera) {
        // Separate inner and outer boxes used to simulate two-sided material
        m_selectionBox = new Qt3DCore::QEntity(m_scene->rootEntity());
        Qt3DCore::QEntity *outerBox = new Qt3DCore::QEntity(m_selectionBox);
        // TODO: Inner box is not needed until wireframe material is implemented for box
        //Qt3DCore::QEntity *innerBox = new Qt3DCore::QEntity(m_selectionBox);

        m_selectionBox->setObjectName(QStringLiteral("__internal selection box"));
        outerBox->setObjectName(QStringLiteral("__internal selection box outside"));
        //innerBox->setObjectName(QStringLiteral("__internal selection box inside"));

        //Qt3DCore::QTransform *insideOutTransform = new Qt3DCore::QTransform();
        //insideOutTransform->setScale(-1.0f);
        //innerBox->addComponent(insideOutTransform);

        m_selectionBox->addComponent(m_selectionTransform);

        //innerBox->addComponent(m_scene->selectionBoxMaterial());
        outerBox->addComponent(m_scene->selectionBoxMaterial());
        //innerBox->addComponent(m_scene->selectionBoxMesh());
        outerBox->addComponent(m_scene->selectionBoxMesh());

        m_selectionBox->setEnabled(false);

        connect(m_selectionBox, &Qt3DCore::QEntity::enabledChanged,
                this, &EditorSceneItem::showSelectionBoxChanged);

        if (isLight || isCamera)
            updateSelectionBoxTransform();
        else
            handleMeshChange(entityMesh);
    } else {
        updateSelectionBoxTransform();
    }

    connect(this, &EditorSceneItem::freeViewChanged, this, &EditorSceneItem::setFreeViewFlag);
}

EditorSceneItem::~EditorSceneItem()
{
    setParentItem(Q_NULLPTR);
    if (m_selectionBox) {
        m_selectionBox->setParent(static_cast<Qt3DCore::QNode *>(Q_NULLPTR));
        m_selectionBox->deleteLater();
    }
}

Qt3DCore::QEntity *EditorSceneItem::entity()
{
    return m_entity;
}

const QList<EditorSceneItem *> &EditorSceneItem::childItems()
{
    return m_children;
}

EditorSceneItem *EditorSceneItem::parentItem()
{
    return m_parentItem;
}

int EditorSceneItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->childItems().indexOf(const_cast<EditorSceneItem*>(this));

    return 0;
}

void EditorSceneItem::addChild(EditorSceneItem *child, int index)
{
    if (child == Q_NULLPTR)
        return;

    if (!m_children.contains(child)) {
        if (index < 0)
            m_children.append(child);
        else
            m_children.insert(index, child);
    }
}

void EditorSceneItem::removeChild(EditorSceneItem *child)
{
    if (child == Q_NULLPTR)
        return;

    if (m_children.contains(child))
        m_children.removeOne(child);
}

void EditorSceneItem::setParentItem(EditorSceneItem *parentItem)
{
    if (m_parentItem)
        m_parentItem->removeChild(this);

    if (parentItem) {
        parentItem->addChild(this);
        m_parentItem = parentItem;
    }
}

EditorSceneItemComponentsModel *EditorSceneItem::componentsModel() const
{
    return m_componentsModel;
}

void EditorSceneItem::setFreeViewFlag(bool enabled)
{
    m_freeView = enabled;
}

bool EditorSceneItem::freeViewFlag() const
{
    return m_freeView;
}

EditorScene *EditorSceneItem::scene() const
{
    return m_scene;
}

void EditorSceneItem::handleMeshChange(Qt3DRender::QGeometryRenderer *newMesh)
{
    if (newMesh != m_entityMesh) {
        if (m_entityMesh && isSelectionBoxShowing())
            connectEntityMesh(false);

        EditorSceneItemMeshComponentsModel::MeshComponentTypes newType =
                EditorSceneItemMeshComponentsModel::meshType(newMesh);
        m_entityMeshType = newType;
        m_entityMesh = newMesh;

        if (m_entityMesh && isSelectionBoxShowing())
            connectEntityMesh(true);

        recalculateMeshExtents();
    }
}

void EditorSceneItem::recalculateMeshExtents()
{
    switch (m_entityMeshType) {
    case EditorSceneItemMeshComponentsModel::Custom: {
        //Qt3DRender::QMesh *mesh = qobject_cast<Qt3DRender::QMesh *>(m_entityMesh);
        // TODO Calculate properly from geometry
        m_entityMeshExtents = QVector3D(2, 2, 2);
        break;
    }
    case EditorSceneItemMeshComponentsModel::Cuboid: {
        Qt3DRender::QCuboidMesh *mesh =
                qobject_cast<Qt3DRender::QCuboidMesh *>(m_entityMesh);
        m_entityMeshExtents = QVector3D(mesh->xExtent(), mesh->yExtent(), mesh->zExtent());
        break;
    }
    case EditorSceneItemMeshComponentsModel::Cylinder: {
        Qt3DRender::QCylinderMesh *mesh =
                qobject_cast<Qt3DRender::QCylinderMesh *>(m_entityMesh);
        float diameter = mesh->radius() * 2.0f;
        m_entityMeshExtents = QVector3D(diameter, mesh->length(), diameter);
        break;
    }
    case EditorSceneItemMeshComponentsModel::Plane: {
        Qt3DRender::QPlaneMesh *mesh =
                qobject_cast<Qt3DRender::QPlaneMesh *>(m_entityMesh);
        m_entityMeshExtents = QVector3D(mesh->width(), 0, mesh->height());
        break;
    }
    case EditorSceneItemMeshComponentsModel::Sphere: {
        Qt3DRender::QSphereMesh *mesh =
                qobject_cast<Qt3DRender::QSphereMesh *>(m_entityMesh);
        float diameter = mesh->radius() * 2.0f;
        m_entityMeshExtents = QVector3D(diameter, diameter, diameter);
        break;
    }
    case EditorSceneItemMeshComponentsModel::Torus: {
        Qt3DRender::QTorusMesh *mesh =
                qobject_cast<Qt3DRender::QTorusMesh *>(m_entityMesh);
        float minorDiameter = mesh->minorRadius() * 2.0f;
        float diameter = mesh->radius() * 2.0f + minorDiameter;
        m_entityMeshExtents = QVector3D(diameter, diameter, minorDiameter);
        break;
    }
    default:
        //Unsupported mesh type
        m_entityMeshExtents = QVector3D(1.0f, 1.0f, 1.0f);
        break;
    }
    updateSelectionBoxTransform();
}

void EditorSceneItem::connectSelectionBoxTransformsRecursive(bool enabled)
{
    if (enabled) {
        if (m_entityTransform) {
            connect(m_entityTransform, &Qt3DCore::QTransform::matrixChanged,
                    this, &EditorSceneItem::updateSelectionBoxTransform);
            connect(m_entityTransform, &Qt3DCore::QTransform::rotationChanged,
                    this, &EditorSceneItem::updateSelectionBoxTransform);
            connect(m_entityTransform, &Qt3DCore::QTransform::translationChanged,
                    this, &EditorSceneItem::updateSelectionBoxTransform);
            connect(m_entityTransform, &Qt3DCore::QTransform::scale3DChanged,
                    this, &EditorSceneItem::updateSelectionBoxTransform);
        }
    } else {
        if (m_entityTransform) {
            disconnect(m_entityTransform, &Qt3DCore::QTransform::matrixChanged,
                       this, &EditorSceneItem::updateSelectionBoxTransform);
            disconnect(m_entityTransform, &Qt3DCore::QTransform::rotationChanged,
                       this, &EditorSceneItem::updateSelectionBoxTransform);
            disconnect(m_entityTransform, &Qt3DCore::QTransform::translationChanged,
                       this, &EditorSceneItem::updateSelectionBoxTransform);
            disconnect(m_entityTransform, &Qt3DCore::QTransform::scale3DChanged,
                       this, &EditorSceneItem::updateSelectionBoxTransform);
        }
    }
    if (m_parentItem)
        m_parentItem->connectSelectionBoxTransformsRecursive(enabled);
}

void EditorSceneItem::composeSelectionBoxTransformRecursive(QMatrix4x4 &transformMatrix)
{
    if (m_parentItem)
        m_parentItem->composeSelectionBoxTransformRecursive(transformMatrix);

    if (m_entityTransform) {
        if (qobject_cast<Qt3DCore::QCamera *>(m_entity))
            transformMatrix *= m_scene->calculateCameraConeMatrix(m_entityTransform);
        else
            transformMatrix *= m_entityTransform->matrix();
    }
}

void EditorSceneItem::connectEntityMesh(bool enabled)
{
    if (enabled) {
        switch (m_entityMeshType) {
        case EditorSceneItemMeshComponentsModel::Custom: {
            Qt3DRender::QMesh *mesh = qobject_cast<Qt3DRender::QMesh *>(m_entityMesh);
            connect(mesh, &Qt3DRender::QMesh::sourceChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Cuboid: {
            Qt3DRender::QCuboidMesh *mesh =
                    qobject_cast<Qt3DRender::QCuboidMesh *>(m_entityMesh);
            connect(mesh, &Qt3DRender::QCuboidMesh::xExtentChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            connect(mesh, &Qt3DRender::QCuboidMesh::yExtentChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            connect(mesh, &Qt3DRender::QCuboidMesh::zExtentChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Cylinder: {
            Qt3DRender::QCylinderMesh *mesh =
                    qobject_cast<Qt3DRender::QCylinderMesh *>(m_entityMesh);
            connect(mesh, &Qt3DRender::QCylinderMesh::radiusChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            connect(mesh, &Qt3DRender::QCylinderMesh::lengthChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Plane: {
            Qt3DRender::QPlaneMesh *mesh =
                    qobject_cast<Qt3DRender::QPlaneMesh *>(m_entityMesh);
            connect(mesh, &Qt3DRender::QPlaneMesh::widthChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            connect(mesh, &Qt3DRender::QPlaneMesh::heightChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Sphere: {
            Qt3DRender::QSphereMesh *mesh =
                    qobject_cast<Qt3DRender::QSphereMesh *>(m_entityMesh);
            connect(mesh, &Qt3DRender::QSphereMesh::radiusChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Torus: {
            Qt3DRender::QTorusMesh *mesh =
                    qobject_cast<Qt3DRender::QTorusMesh *>(m_entityMesh);
            connect(mesh, &Qt3DRender::QTorusMesh::radiusChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            connect(mesh, &Qt3DRender::QTorusMesh::minorRadiusChanged,
                    this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        default:
            //Unsupported mesh type
            break;
        }
    } else {
        switch (m_entityMeshType) {
        case EditorSceneItemMeshComponentsModel::Custom: {
            Qt3DRender::QMesh *mesh = qobject_cast<Qt3DRender::QMesh *>(m_entityMesh);
            disconnect(mesh, &Qt3DRender::QMesh::sourceChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Cuboid: {
            Qt3DRender::QCuboidMesh *mesh =
                    qobject_cast<Qt3DRender::QCuboidMesh *>(m_entityMesh);
            disconnect(mesh, &Qt3DRender::QCuboidMesh::xExtentChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            disconnect(mesh, &Qt3DRender::QCuboidMesh::yExtentChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            disconnect(mesh, &Qt3DRender::QCuboidMesh::zExtentChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Cylinder: {
            Qt3DRender::QCylinderMesh *mesh =
                    qobject_cast<Qt3DRender::QCylinderMesh *>(m_entityMesh);
            disconnect(mesh, &Qt3DRender::QCylinderMesh::radiusChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            disconnect(mesh, &Qt3DRender::QCylinderMesh::lengthChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Plane: {
            Qt3DRender::QPlaneMesh *mesh =
                    qobject_cast<Qt3DRender::QPlaneMesh *>(m_entityMesh);
            disconnect(mesh, &Qt3DRender::QPlaneMesh::widthChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            disconnect(mesh, &Qt3DRender::QPlaneMesh::heightChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Sphere: {
            Qt3DRender::QSphereMesh *mesh =
                    qobject_cast<Qt3DRender::QSphereMesh *>(m_entityMesh);
            disconnect(mesh, &Qt3DRender::QSphereMesh::radiusChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        case EditorSceneItemMeshComponentsModel::Torus: {
            Qt3DRender::QTorusMesh *mesh =
                    qobject_cast<Qt3DRender::QTorusMesh *>(m_entityMesh);
            disconnect(mesh, &Qt3DRender::QTorusMesh::radiusChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            disconnect(mesh, &Qt3DRender::QTorusMesh::minorRadiusChanged,
                       this, &EditorSceneItem::recalculateMeshExtents);
            break;
        }
        default:
            //Unsupported mesh type
            break;
        }
    }
}

void EditorSceneItem::updateSelectionBoxTransform()
{
    // Transform mesh extents, first scale, then translate
    QMatrix4x4 transformMatrix;
    composeSelectionBoxTransformRecursive(transformMatrix);
    transformMatrix.scale(m_entityMeshExtents + QVector3D(0.05f, 0.05f, 0.05f));
    m_selectionTransform->setMatrix(transformMatrix);

    // TODO: How to handle group selection?
    // TODO: Currently there is no box for entity, unless it has a mesh.
    // TODO: Calculating and keeping group box updated seems potentially complicated, if the box
    // TODO: is supposed to encompass all the child items
    // TODO: Additional problem with groups is that group parent entity may have a mesh itself
}

void EditorSceneItem::setShowSelectionBox(bool enabled)
{
    if (m_selectionBox) {
        connectSelectionBoxTransformsRecursive(enabled);
        connectEntityMesh(enabled);

        if (enabled)
            updateSelectionBoxTransform();

        m_selectionBox->setEnabled(enabled);
    }
}

bool EditorSceneItem::isSelectionBoxShowing() const
{
    if (m_selectionBox)
        return m_selectionBox->isEnabled();
    else
        return false;
}

