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
#include "editorsceneitemmodel.h"
#include "editorsceneitem.h"
#include "editorscene.h"
#include "editorutils.h"
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DCore/QNodeId>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCuboidMesh>
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QPlaneMesh>
#include <Qt3DRender/QSphereMesh>
#include <Qt3DRender/QTorusMesh>
#include <Qt3DRender/QLight>
#include <Qt3DRender/QPhongMaterial>

EditorSceneItemModel::EditorSceneItemModel(EditorScene *scene)
    : QAbstractItemModel(scene)
    , m_scene(scene)
{

}

EditorSceneItemModel::~EditorSceneItemModel()
{

}

QModelIndex EditorSceneItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    EditorSceneItem *parentItem = editorSceneItemFromIndex(parent);
    EditorSceneItem *childItem = nullptr;
    if (parentItem->childItems().count() > row)
        childItem = parentItem->childItems().at(row);
    if (childItem != nullptr)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();

}

QModelIndex EditorSceneItemModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    EditorSceneItem *childItem = editorSceneItemFromIndex(child);
    EditorSceneItem *parentItem = childItem->parentItem();

    if (parentItem == m_scene->rootItem())
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int EditorSceneItemModel::rowCount(const QModelIndex &parent) const
{
    EditorSceneItem *parentItem = editorSceneItemFromIndex(parent);
    return parentItem->childItems().count();
}

int EditorSceneItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

bool EditorSceneItemModel::hasChildren(const QModelIndex &parent) const
{
    EditorSceneItem *parentItem = editorSceneItemFromIndex(parent);
    return parentItem->childItems().count() > 0 ? true : false;
}

QVariant EditorSceneItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    EditorSceneItem *item = editorSceneItemFromIndex(index);

    switch (role) {
    case ItemRole:
        return QVariant::fromValue(item);
    case NameRole:
        return QVariant::fromValue(item->entity()->objectName());
    case IdRole:
        return QVariant::fromValue(item->entity()->id().id());
    default:
        return QVariant();
    }
}

QModelIndex EditorSceneItemModel::getModelIndex(Qt3DCore::QEntity *entity)
{
    return getModelIndexByName(entity->objectName());
}

QModelIndex EditorSceneItemModel::getModelIndexByName(const QString &entityName)
{
    QModelIndex selectedIndex = QModelIndex();
    QModelIndexList list = QAbstractItemModel::match(
                sceneEntityIndex(), NameRole, QVariant::fromValue(entityName), 1,
                Qt::MatchRecursive | Qt::MatchWrap | Qt::MatchExactly);
    if (list.count())
        selectedIndex = list.at(0);

    return selectedIndex;
}

void EditorSceneItemModel::handleEntityNameChange()
{
    Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>(sender());
    if (entity) {
        QModelIndex index = getModelIndex(entity);
        if (index.isValid()) {
            QVector<int> roles(1);
            roles[0] = NameRole;
            emit dataChanged(index, index, roles);
        }
    }
}

bool EditorSceneItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != NameRole)
        return false;

    EditorSceneItem *item = editorSceneItemFromIndex(index);
    item->entity()->setObjectName(value.toString());

    QVector<int> roles(1);
    roles[0] = NameRole;
    emit dataChanged(index, index, roles);
    return true;
}

Qt::ItemFlags EditorSceneItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

QHash<int, QByteArray> EditorSceneItemModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ItemRole] = "item";
    roles[NameRole] = "name";
    roles[IdRole] = "id";
    return roles;
}

void EditorSceneItemModel::resetModel()
{
    beginResetModel();

    // Reconnect all entities
    disconnectEntity(m_scene->sceneEntityItem()->entity());
    connectEntity(m_scene->sceneEntityItem()->entity());

    endResetModel();
}

EditorSceneItem *EditorSceneItemModel::editorSceneItemFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        EditorSceneItem *item = static_cast<EditorSceneItem *>(index.internalPointer());
        if (item)
            return item;
    }

    return m_scene->rootItem();
}

Qt3DCore::QEntity *EditorSceneItemModel::insertEntity(EditorUtils::InsertableEntities type,
                                                      const QVector3D &pos,
                                                      const QModelIndex &parentIndex)
{
    int rows = rowCount(parentIndex);

    EditorSceneItem *parentItem = editorSceneItemFromIndex(parentIndex);
    beginInsertRows(parentIndex, rows, rows);

    Qt3DCore::QEntity *newEntity = nullptr;
    if (type == EditorUtils::CameraEntity) {
        Qt3DRender::QCamera *newCamera = new Qt3DRender::QCamera(parentItem->entity());
        newCamera->setObjectName(generateValidName(tr("New Camera"), newEntity));
        newCamera->setPosition(pos);
        newEntity = newCamera;
    } else {
        newEntity = new Qt3DCore::QEntity(parentItem->entity());
        Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();
        // This implementation assumes that entities inserted into a non-default position are
        // inserted as children of the scene root. If this is not always true, we need to account
        // for parent chain transforms when determining the initial translation.
        transform->setTranslation(pos);
        newEntity->addComponent(transform);

        Qt3DRender::QGeometryRenderer *mesh = EditorUtils::createMeshForInsertableType(type);
        if (mesh) {
            newEntity->addComponent(mesh);
            newEntity->addComponent(new Qt3DRender::QPhongMaterial());
        }

        switch (type) {
        case EditorUtils::CuboidEntity: {
            newEntity->setObjectName(generateValidName(tr("New Cube"), newEntity));
            break;
        }
        case EditorUtils::CylinderEntity: {
            newEntity->setObjectName(generateValidName(tr("New Cylinder"), newEntity));
            break;
        }
        case EditorUtils::PlaneEntity: {
            newEntity->setObjectName(generateValidName(tr("New Plane"), newEntity));
            break;
        }
        case EditorUtils::SphereEntity: {
            newEntity->setObjectName(generateValidName(tr("New Sphere"), newEntity));
            break;
        }
        case EditorUtils::TorusEntity: {
            newEntity->setObjectName(generateValidName(tr("New Torus"), newEntity));
            break;
        }
        case EditorUtils::CustomEntity: {
            newEntity->setObjectName(generateValidName(tr("New Custom"), newEntity));
            break;
        }
        case EditorUtils::LightEntity: {
            newEntity->setObjectName(generateValidName(tr("New Light"), newEntity));
            newEntity->addComponent(new Qt3DRender::QLight());
            break;
        }
        default:
            newEntity->setObjectName(generateValidName(tr("New Empty Entity"), newEntity));
            break;
        }
    }


    m_scene->addEntity(newEntity);

    endInsertRows();

    connectEntity(newEntity);

    // Store the index for the newly inserted item. It is the last child of parent.
    m_lastInsertedIndex = QModelIndex();
    int childCount = parentItem->childItems().count();
    if (childCount) {
        EditorSceneItem *childItem = parentItem->childItems().at(childCount - 1);
        if (childItem != nullptr)
            m_lastInsertedIndex = createIndex(childCount - 1, 0, childItem);
    }

    return newEntity;
}

void EditorSceneItemModel::insertExistingEntity(Qt3DCore::QEntity *entity, int row,
                                                const QModelIndex &parentIndex)
{
    EditorSceneItem *parentItem = editorSceneItemFromIndex(parentIndex);
    beginInsertRows(parentIndex, row, row);

    m_scene->addEntity(entity, row, parentItem->entity());

    endInsertRows();

    connectEntity(entity);

    m_lastInsertedIndex = createIndex(row, 0, parentItem->childItems().at(row));
}

QModelIndex EditorSceneItemModel::lastInsertedIndex()
{
    // Note that this index is guaranteed valid only if called immediately after
    // the entity is inserted.
    return m_lastInsertedIndex;
}

// RemoveEntity removes the entity from scene entity tree, which also deletes it.
void EditorSceneItemModel::removeEntity(const QModelIndex &index)
{
    QModelIndex parentIndex = parent(index);
    EditorSceneItem *item = editorSceneItemFromIndex(index);
    Qt3DCore::QEntity *entity = item->entity();

    if (m_scene->isRemovable(entity)) {
        beginRemoveRows(parentIndex, index.row(), index.row());

        m_scene->removeEntity(entity);

        endRemoveRows();
    }
}

const QString EditorSceneItemModel::setEntityName(const QModelIndex &index, const QString &name)
{
    // Make sure there are no duplicate entity names, as the entity names will be used
    // to generate the qml item IDs.

    if (name.isEmpty())
        return QString();

    EditorSceneItem *item = editorSceneItemFromIndex(index);
    if (item->entity()->objectName() == name)
        return QString();

    QString finalName = generateValidName(name, item->entity());

    setData(index, finalName, NameRole);

    return finalName;
}

const QString EditorSceneItemModel::entityName(const QModelIndex &index) const
{
    return data(index, NameRole).toString();
}

QModelIndex EditorSceneItemModel::sceneEntityIndex()
{
    // The scene entity is first child of the root
    return index(0, 0, QModelIndex());
}

QString EditorSceneItemModel::generateValidName(const QString &desiredName,
                                                const Qt3DCore::QEntity *skipEntity)
{
    QStringList nameList;
    collectEntityNames(nameList, m_scene->sceneEntityItem()->entity(), skipEntity);
    QString testName = desiredName;
    QString nameTemplate = QStringLiteral("%1_%2");
    int counter = 2;
    while (nameList.contains(fixEntityName(testName)))
        testName = nameTemplate.arg(desiredName).arg(counter++);

    return testName;
}

QString EditorSceneItemModel::fixEntityName(const QString &desiredName)
{
    // NOTE: This name fixing must match how parser mangles the names for ids during export
    return QString(desiredName.trimmed().toLatin1().toLower().replace(' ', '_'));
}

void EditorSceneItemModel::collectEntityNames(QStringList &nameList,
                                              const Qt3DCore::QEntity *entity,
                                              const Qt3DCore::QEntity *skipEntity)
{
    if (entity != skipEntity)
        nameList.append(fixEntityName(entity->objectName()));
    foreach (QObject *obj, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(obj);
        if (childEntity)
            collectEntityNames(nameList, childEntity, skipEntity);
    }
}

void EditorSceneItemModel::connectEntity(Qt3DCore::QEntity *entity)
{
    // Recursively connect this entity and all it's entity children
    foreach (QObject *obj, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(obj);
        if (childEntity)
            connectEntity(childEntity);
    }

    connect(entity, &QObject::objectNameChanged,
            this, &EditorSceneItemModel::handleEntityNameChange);
}

void EditorSceneItemModel::disconnectEntity(Qt3DCore::QEntity *entity)
{
    // Recursively connect this entity and all it's entity children
    foreach (QObject *obj, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(obj);
        if (childEntity)
            disconnectEntity(childEntity);
    }

    disconnect(entity, &QObject::objectNameChanged,
               this, &EditorSceneItemModel::handleEntityNameChange);
}
