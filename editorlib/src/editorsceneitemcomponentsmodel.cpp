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
#include "editorsceneitemcomponentsmodel.h"
#include "editorsceneitemtransformcomponentsmodel.h"
#include "qdummyobjectpicker.h"

#include "editorsceneitem.h"
#include "editorscene.h"
#include "editorutils.h"
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DCore/QComponent>

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QAbstractLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DRender/QObjectPicker>

EditorSceneItemComponentsModel::EditorSceneItemComponentsModel(EditorSceneItem *sceneItem,
                                                               QObject *parent)
    : QAbstractListModel(parent)
    , m_sceneItem(sceneItem)
    , m_transformItem(nullptr)
    , m_materialItem(nullptr)
    , m_meshItem(nullptr)
    , m_lightItem(nullptr)
{
}

EditorSceneItemComponentsModel::~EditorSceneItemComponentsModel()
{
    delete m_transformItem;
    delete m_materialItem;
    delete m_meshItem;
    delete m_lightItem;

    m_modelRowList.clear();
}

QModelIndex EditorSceneItemComponentsModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return createIndex(row, column);
}

int EditorSceneItemComponentsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_modelRowList.size();
}

// This method must be used to create the component proxies for entities where components are
// added programmatically instead of via appendNewComponent() method.
// Also initializes index maps.
void EditorSceneItemComponentsModel::initializeModel()
{
    Qt3DCore::QComponentVector componentList = m_sceneItem->entity()->components();
    QMap<EditorSceneItemComponentTypes, ComponentInfo> newInfoMap;
    for (int i = 0; i < componentList.size(); i++) {
        Qt3DCore::QComponent *component = componentList.at(i);
        EditorSceneItemComponentTypes type = typeOfComponent(component);

        switch (type) {
        case Light: {
            Qt3DRender::QAbstractLight *lightComponent
                    = qobject_cast<Qt3DRender::QAbstractLight *>(component);
            if (lightComponent && !m_lightItem)
                m_lightItem = new LightComponentProxyItem(this, lightComponent);
            break;
        }
        case Mesh: {
            Qt3DRender::QGeometryRenderer *meshComponent
                    = qobject_cast<Qt3DRender::QGeometryRenderer *>(component);
            if (meshComponent && !m_meshItem)
                m_meshItem = new MeshComponentProxyItem(this, meshComponent);
            break;
        }
        case Transform: {
            Qt3DCore::QTransform *transformComponent
                    = qobject_cast<Qt3DCore::QTransform *>(component);
            if (transformComponent && !m_transformItem)
                m_transformItem = new TransformComponentProxyItem(this, transformComponent);
            break;
        }
        case Material: {
            Qt3DRender::QMaterial *materialComponent
                    = qobject_cast<Qt3DRender::QMaterial *>(component);
            if (materialComponent && !m_materialItem)
                m_materialItem = new MaterialComponentProxyItem(this, materialComponent);
            break;
        }
        default:
            break;
        }

        // Map the type to component index. Only the first component of each type interests us.
        if (type >= 0 && type < SupportedComponentCount && !newInfoMap.contains(type))
            newInfoMap.insert(type, ComponentInfo(component, type));
    }

    // Camera entities have a special "component" for the entity itself
    if (qobject_cast<Qt3DRender::QCamera *>(m_sceneItem->entity()))
        newInfoMap.insert(CameraEntity, ComponentInfo(nullptr, CameraEntity));

    m_modelRowList.clear();
    // Resolve row indexes for components. We want to show components in predictable order.
    for (int i = 0; i < SupportedComponentCount; i++) {
        if (newInfoMap.contains(EditorSceneItemComponentTypes(i)))
            m_modelRowList.append(newInfoMap.value(EditorSceneItemComponentTypes(i)));
    }
}

EditorSceneItemComponentsModel::EditorSceneItemComponentTypes
EditorSceneItemComponentsModel::typeOfComponent(QObject *component)
{
    if (component && component->objectName().startsWith("__internal"))
        return Internal;
    if (qobject_cast<Qt3DCore::QEntity *>(component)) {
        if (qobject_cast<Qt3DRender::QCamera *>(component))
            return CameraEntity;
    }
    if (qobject_cast<Qt3DRender::QAbstractLight *>(component))
        return Light;
    if (qobject_cast<Qt3DRender::QGeometryRenderer *>(component))
        return Mesh;
    if (qobject_cast<Qt3DCore::QTransform *>(component)) {
        if (qobject_cast<Qt3DRender::QCamera *>(m_sceneItem->entity()))
            return Internal;
        else
            return Transform;
    }
    if (qobject_cast<Qt3DRender::QMaterial *>(component))
        return Material;
    if (qobject_cast<QDummyObjectPicker *>(component))
        return ObjectPicker;
    if (qobject_cast<Qt3DRender::QCameraLens *>(component)) {
        if (qobject_cast<Qt3DRender::QCamera *>(m_sceneItem->entity()))
            return Internal;
    }
    return Unknown;
}

QVariant EditorSceneItemComponentsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_modelRowList.size()
            || (role != ComponentType && role != Component)) {
        return QVariant();
    }

    const ComponentInfo &compInfo = m_modelRowList.at(index.row());

    if (role == ComponentType)
        return QVariant::fromValue(compInfo.type);

    QVariant componentData = QVariant();

    // CameraEntity is not actually a component, so handle it first
    if (compInfo.type == CameraEntity) {
        Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(m_sceneItem->entity());
        if (camera)
            componentData = QVariant::fromValue(camera);
        return componentData;
    }

    switch (compInfo.type) {
    case Light:
        componentData = QVariant::fromValue(m_lightItem);
        break;
    case Mesh:
        componentData = QVariant::fromValue(m_meshItem);
        break;
    case Transform:
        componentData = QVariant::fromValue(m_transformItem);
        break;
    case Material:
        componentData = QVariant::fromValue(m_materialItem);
        break;
    default:
        componentData = QVariant::fromValue(compInfo.component);
        break;
    }

    return componentData;
}

Qt::ItemFlags EditorSceneItemComponentsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

QHash<int, QByteArray> EditorSceneItemComponentsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ComponentType] = "type";
    roles[Component] = "componentData";
    return roles;
}

void EditorSceneItemComponentsModel::appendNewComponent(EditorSceneItemComponentTypes type)
{
    Qt3DCore::QComponent *component = nullptr;

    QMap<EditorSceneItemComponentTypes, ComponentInfo> typeInfoMap;
    foreach (ComponentInfo info, m_modelRowList) {
        // Don't allow duplicates
        if (info.type == type)
            return;
        // Collect components in type to info map for later while we are iterating
        typeInfoMap.insert(info.type, info);
    }

    switch (type) {
    case Light:
        if (!m_lightItem) {
            Qt3DRender::QAbstractLight *lightComponent = new Qt3DRender::QPointLight();
            m_lightItem = new LightComponentProxyItem(this, lightComponent);
            component = lightComponent;
        }
        break;
    case Material: {
        if (!m_materialItem) {
            Qt3DRender::QMaterial *materialComponent = new Qt3DExtras::QPhongMaterial();
            m_materialItem = new MaterialComponentProxyItem(this, materialComponent);
            component = materialComponent;
        }
        break;
    }
    case Mesh: {
        if (!m_meshItem) {
            Qt3DRender::QGeometryRenderer *meshComponent = new Qt3DExtras::QCuboidMesh();
            m_meshItem = new MeshComponentProxyItem(this, meshComponent);
            component = meshComponent;
        }
        break;
    }
    case ObjectPicker:
        // Use a QDummyObjectPicker, as adding a real one replaces the one needed for editor.
        component = new QDummyObjectPicker();
        break;
    case Transform: {
        if (qobject_cast<Qt3DRender::QCamera *>(m_sceneItem->entity()))
            return;
        if (!m_transformItem) {
            Qt3DCore::QTransform *transformComponent = new Qt3DCore::QTransform();
            m_transformItem = new TransformComponentProxyItem(this, transformComponent);
            component = transformComponent;
        }
        break;
    }
    default:
        //Unsupported component type
        break;
    }

    if (component == nullptr)
        return;

    // Figure out new model index for the type. We want types to be in predictable order, so
    // adding a new component can modify multiple model indexes.
    // For simplicity's sake, we just recreate the list while we iterate through the map.
    QVector<ComponentInfo> newList;
    int modelRow = 0;
    int insertedRow = 0;
    for (int i = 0; i < SupportedComponentCount; i++) {
        ComponentInfo componentInfo;
        if (EditorSceneItemComponentTypes(i) == type) {
            insertedRow = modelRow;
            componentInfo = ComponentInfo(component, type);
        } else {
            componentInfo = typeInfoMap.value(EditorSceneItemComponentTypes(i));
        }
        if (componentInfo.type != Unknown) {
            newList.append(componentInfo);
            modelRow++;
        }
    }

    beginInsertRows(QModelIndex(), insertedRow, insertedRow);

    m_modelRowList = newList;
    m_sceneItem->entity()->addComponent(component);

    endInsertRows();
}

void EditorSceneItemComponentsModel::removeComponent(int index)
{
    if (index < 0 || index >= m_modelRowList.size()) // Sanity check
        return;

    const ComponentInfo componentInfo = m_modelRowList.at(index);

    beginRemoveRows(QModelIndex(), index, index);

    m_sceneItem->entity()->removeComponent(componentInfo.component);

    m_modelRowList.removeAt(index);

    switch (componentInfo.type) {
    case Light:
        m_lightItem->deleteLater();
        m_lightItem = 0;
        break;
    case Mesh:
        m_meshItem->deleteLater();
        m_meshItem = 0;
        break;
    case Transform:
        m_transformItem->deleteLater();
        m_transformItem = 0;
        break;
    case Material:
        m_materialItem->deleteLater();
        m_materialItem = 0;
        break;
    default:
        break;
    }
    endRemoveRows();

    if (componentInfo.component->entities().size() == 0)
        componentInfo.component->deleteLater();
}

void EditorSceneItemComponentsModel::removeComponent(Qt3DCore::QComponent *component)
{
    // Then find model index
    for (int i = 0; i < m_modelRowList.size(); i++) {
        const ComponentInfo &info = m_modelRowList.at(i);
        if (info.component == component) {
            removeComponent(i);
            break;
        }
    }
}

// Replaces old component with another of the same type.
// The caller assumes the ownership of the old component.
void EditorSceneItemComponentsModel::replaceComponent(Qt3DCore::QComponent *oldComponent,
                                                      Qt3DCore::QComponent *newComponent)
{
    Q_ASSERT(oldComponent);
    Q_ASSERT(newComponent);

    EditorSceneItemComponentTypes type = typeOfComponent(newComponent);
    Q_ASSERT(type == typeOfComponent(oldComponent));

    switch (type) {
    case Light: {
        m_lightItem->beginResetComponent(static_cast<Qt3DRender::QPointLight *>(newComponent));
        break;
    }
    case Mesh: {
        m_meshItem->beginResetComponent(static_cast<Qt3DRender::QGeometryRenderer *>(newComponent));
        break;
    }
    case Transform: {
        m_transformItem->beginResetComponent(static_cast<Qt3DCore::QTransform *>(newComponent));
        break;
    }
    case Material: {
        m_materialItem->beginResetComponent(static_cast<Qt3DRender::QMaterial *>(newComponent));
        break;
    }
    default:
        qWarning() << "Unknown component type in" << __FUNCTION__;
        break;
    }

    m_sceneItem->entity()->removeComponent(oldComponent);
    m_sceneItem->entity()->addComponent(newComponent);

    int modelRow = 0;
    for (; modelRow < m_modelRowList.size(); modelRow++) {
        ComponentInfo &info = m_modelRowList[modelRow];
        if (info.type == type) {
            info.component = newComponent;
            break;
        }
    }

    QModelIndex modelIndex = this->index(modelRow);
    emit dataChanged(modelIndex, modelIndex);

    switch (type) {
    case Light: {
        m_lightItem->endResetComponent();
        m_sceneItem->setCanRotate(qobject_cast<Qt3DRender::QDirectionalLight *>(newComponent)
                                  || qobject_cast<Qt3DRender::QSpotLight *>(newComponent));
        m_sceneItem->scene()->handleLightTypeChanged(m_sceneItem);
        break;
    }
    case Mesh: {
        m_meshItem->endResetComponent();
        m_sceneItem->handleMeshChange(static_cast<Qt3DRender::QGeometryRenderer *>(newComponent));
        break;
    }
    case Transform: {
        m_transformItem->endResetComponent();
        break;
    }
    case Material: {
        m_materialItem->endResetComponent();
        break;
    }
    default:
        qWarning() << "Unknown component type in" << __FUNCTION__;
        break;
    }
}
