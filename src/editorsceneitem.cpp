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
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGeometryFactory>
#include <Qt3DRender/QCuboidMesh>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QPlaneMesh>
#include <Qt3DRender/QSphereMesh>
#include <Qt3DRender/QTorusMesh>
#include <Qt3DRender/QLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>

#include <cfloat>

#include "editorsceneitemcomponentsmodel.h"

EditorSceneItem::EditorSceneItem(EditorScene *scene, Qt3DCore::QEntity *entity,
                                 EditorSceneItem *parentItem,
                                 int index, QObject *parent)
    : QObject(parent)
    , m_entity(entity)
    , m_parentItem(parentItem)
    , m_componentsModel(new EditorSceneItemComponentsModel(this, this))
    , m_scene(scene)
    , m_selectionBox(Q_NULLPTR)
    , m_selectionTransform(Q_NULLPTR)
    , m_entityTransform(Q_NULLPTR)
    , m_entityMesh(Q_NULLPTR)
    , m_entityMeshType(EditorSceneItemMeshComponentsModel::Unknown)
    , m_entityMeshExtents(1.0f, 1.0f, 1.0f)
    , m_canRotate(true)
{
    if (m_parentItem != Q_NULLPTR)
        m_parentItem->addChild(this, index);

    // Selection box
    Qt3DCore::QComponentVector components = entity->components();
    Qt3DRender::QGeometryRenderer *entityMesh = Q_NULLPTR;
    bool isLight = false;
    Q_FOREACH (Qt3DCore::QComponent *component, components) {
        if (!m_entityTransform)
            m_entityTransform = qobject_cast<Qt3DCore::QTransform *>(component);
        if (!entityMesh)
            entityMesh = qobject_cast<Qt3DRender::QGeometryRenderer *>(component);
        if (qobject_cast<Qt3DRender::QLight *>(component)) {
            isLight = true;
            if (!qobject_cast<Qt3DRender::QDirectionalLight *>(component)
                    && !qobject_cast<Qt3DRender::QSpotLight *>(component)) {
                m_canRotate = false;
            }
        }
    }
    bool isCamera = qobject_cast<Qt3DRender::QCamera *>(entity);
    if (isCamera)
        m_entityMeshExtents = QVector3D(1.4f, 1.4f, 1.4f);

    // Selection transform is needed for child items, even if we don't have a box
    m_selectionTransform = new Qt3DCore::QTransform;

    // Don't show box itself unless the entity has mesh
    if (entityMesh || isLight || isCamera) {
        m_selectionBox = new Qt3DCore::QEntity(m_scene->rootEntity());

        m_selectionBox->setObjectName(QStringLiteral("__internal selection box"));

        m_selectionBox->addComponent(m_selectionTransform);

        m_selectionBox->addComponent(m_scene->selectionBoxMaterial());
        m_selectionBox->addComponent(m_scene->selectionBoxMesh());

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

    // Save entity item type
    if (isLight)
        m_itemType = EditorSceneItem::Light;
    else if (isCamera)
        m_itemType = EditorSceneItem::Camera;
    else if (entityMesh)
        m_itemType = EditorSceneItem::Mesh;
    else
        m_itemType = EditorSceneItem::Other;
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
    m_entityMeshCenter = QVector3D();
    switch (m_entityMeshType) {
    case EditorSceneItemMeshComponentsModel::Custom: {
        recalculateCustomMeshExtents(m_entityMesh);
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

void EditorSceneItem::recalculateCustomMeshExtents(Qt3DRender::QGeometryRenderer *mesh)
{
    // For custom meshes we need to calculate the extents from geometry
    Qt3DRender::QGeometry *meshGeometry = Q_NULLPTR;
    Qt3DRender::QGeometryFactoryPtr geometryFunctorPtr = mesh->geometryFactory();

    if (geometryFunctorPtr.data()) {
        // Execute the geometry functor to get the geometry, since its not normally available
        // on the application side.
        meshGeometry = geometryFunctorPtr.data()->operator()();
    }

    if (meshGeometry) {
        // Set default in case we can't determine the geometry: normalized mesh in range [-1,1]
        m_entityMeshExtents = QVector3D(2.0f, 2.0f, 2.0f);
        m_entityMeshCenter =  QVector3D();

        Qt3DRender::QAttribute *vPosAttribute = Q_NULLPTR;
        Q_FOREACH (Qt3DRender::QAttribute *attribute, meshGeometry->attributes()) {
            if (attribute->name() == Qt3DRender::QAttribute::defaultPositionAttributeName()) {
                vPosAttribute = attribute;
                break;
            }
        }
        if (vPosAttribute) {
            const float *bufferPtr =
                    reinterpret_cast<const float *>(vPosAttribute->buffer()->data().constData());
            uint stride = vPosAttribute->byteStride() / sizeof(float);
            uint offset = vPosAttribute->byteOffset() / sizeof(float);
            bufferPtr += offset;
            uint vertexCount = vPosAttribute->count();
            uint dataCount = vPosAttribute->buffer()->data().size() / sizeof(float);

            // Make sure we have valid data
            if (((vertexCount * stride) + offset) > dataCount)
                return;

            float minX = FLT_MAX;
            float minY = FLT_MAX;
            float minZ = FLT_MAX;
            float maxX = FLT_MIN;
            float maxY = FLT_MIN;
            float maxZ = FLT_MIN;

            if (stride)
                stride = stride - 3; // Three floats per vertex
            for (uint i = 0; i < vertexCount; i++) {
                float xVal = *bufferPtr++;
                minX = qMin(xVal, minX);
                maxX = qMax(xVal, maxX);
                float yVal = *bufferPtr++;
                minY = qMin(yVal, minY);
                maxY = qMax(yVal, maxY);
                float zVal = *bufferPtr++;
                minZ = qMin(zVal, minZ);
                maxZ = qMax(zVal, maxZ);
                bufferPtr += stride;
            }
            m_entityMeshExtents = QVector3D(maxX - minX, maxY - minY, maxZ - minZ);
            m_entityMeshCenter = QVector3D(minX + m_entityMeshExtents.x() / 2.0f,
                                           minY + m_entityMeshExtents.y() / 2.0f,
                                           minZ + m_entityMeshExtents.z() / 2.0f);
        }
    } else {
        m_entityMeshExtents = QVector3D();
        m_entityMeshCenter =  QVector3D();
    }
}

void EditorSceneItem::updateChildLightTransforms()
{
    Q_FOREACH (EditorSceneItem *child, m_children) {
        child->updateChildLightTransforms();
        if (child->itemType() == Light)
            m_scene->updateLightVisibleTransform(child->entity());
    }
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

QMatrix4x4 EditorSceneItem::composeSelectionBoxTransform()
{
    QMatrix4x4 totalTransform;

    Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(m_entity);
    if (camera) {
        totalTransform = m_scene->calculateVisibleSceneCameraMatrix(camera);
    } else if (m_itemType == Light) {
        totalTransform = m_scene->calculateVisibleLightMatrix(m_entity);
    } else {
        totalTransform = EditorUtils::totalAncestralTransform(m_entity);
        if (m_entityTransform)
            totalTransform *= m_entityTransform->matrix();
    }

    return totalTransform;
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
    QMatrix4x4 transformMatrix = composeSelectionBoxTransform();
    transformMatrix.translate(m_entityMeshCenter);
    m_selectionBoxCenter = transformMatrix * QVector3D();
    m_selectionBoxExtents = m_entityMeshExtents + QVector3D(0.002f, 0.002f, 0.002f);
    transformMatrix.scale(m_selectionBoxExtents);

    QVector3D ancestralScale = EditorUtils::totalAncestralScale(m_entity);
    m_selectionBoxExtents *= ancestralScale;
    if (m_entityTransform)
        m_selectionBoxExtents *= m_entityTransform->scale3D();

    m_selectionTransform->setMatrix(transformMatrix);

    // Check if we have lights as children and update their visible translations, as they are
    // not part of the normal scene.
    updateChildLightTransforms();

    emit selectionBoxTransformChanged(this);

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

        if (enabled) {
            // Update mesh extents if they are dirty, otherwise just update transform
            if (m_entityMeshExtents.isNull())
                recalculateMeshExtents();
            else
                updateSelectionBoxTransform();
        }

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

bool EditorSceneItem::setCustomProperty(QObject *component, const QString name,
                                        const QVariant &value)
{
    // Sets the 'name' property to value to a component or the entity itself if component is null.
    // If the property is defined using Q_PROPERTY, returns true
    // on success, else returns false. This complies with QObject::property().

    QObject *targetObj = component;
    if (!targetObj)
        targetObj = m_entity;

    QByteArray nameArray = name.toLatin1();
    const char *propertyName = nameArray.constData();

    m_scene->handlePropertyLocking(this, name, value.toBool());

    return targetObj->setProperty(propertyName, value);
}

QVariant EditorSceneItem::customProperty(QObject *component, const QString name) const
{
    // Returns the value of the 'name' property of the component.
    // If component is null, entity itself is checked for the property.
    // If the property doesn't exist, the returned variant
    // is invalid. This complies with QObject::property().

    QObject *targetObj = component;
    if (!targetObj)
        targetObj = m_entity;

    QByteArray nameArray = name.toLatin1();
    const char *propertyName = nameArray.constData();
    QVariant propertyVariant = targetObj->property(propertyName);
    if (propertyVariant.isValid())
        return propertyVariant;
    else
        return QVariant();
}
