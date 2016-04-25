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
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DRender/QLight>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QSceneLoader>

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

void EditorSceneItemModel::handleSceneLoaderStatusChanged()
{
    Qt3DRender::QSceneLoader *sceneLoader = qobject_cast<Qt3DRender::QSceneLoader *>(sender());
    if (sceneLoader) {
        QVector<Qt3DCore::QEntity *> entities = sceneLoader->entities();
        if (!entities.isEmpty()) {
            Qt3DCore::QEntity *importedEntity = entities[0];
            if (sceneLoader->status() == Qt3DRender::QSceneLoader::Ready) {
                EditorSceneItem *item = m_scene->entityItem(importedEntity);
                if (item)
                    item->recalculateSubMeshesExtents();
                m_scene->createObjectPickerForEntity(importedEntity);
            } else if (sceneLoader->status() == Qt3DRender::QSceneLoader::Error) {
                m_scene->setError(tr("Failed to import an Entity"));
            }
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

    // Restore TreeView branch expansions, since resetting the model will collapse the branches
    QModelIndexList expandedIndexList;
    Q_FOREACH (const QString entityName, m_expandedItems)
        expandedIndexList.append(getModelIndexByName(entityName));
    emit expandItems(expandedIndexList);

    // Select scene root after reset
    emit selectIndex(sceneEntityIndex());
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

        // pos is worldPosition, adjust the actual position to account for ancestral transforms
        QMatrix4x4 ancestralTransform = EditorUtils::totalAncestralTransform(newEntity);
        QVector3D childPosition = ancestralTransform.inverted() * pos;

        Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();
        transform->setTranslation(childPosition);
        newEntity->addComponent(transform);

        if (type != EditorUtils::GroupEntity) {
            Qt3DRender::QGeometryRenderer *mesh = EditorUtils::createMeshForInsertableType(type);
            if (mesh) {
                newEntity->addComponent(mesh);
                newEntity->addComponent(new Qt3DExtras::QPhongMaterial());
            }
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
        case EditorUtils::GroupEntity: {
            newEntity->setObjectName(generateValidName(tr("New Group"), newEntity));
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

        m_expandedItems.removeOne(entity->objectName());
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

bool EditorSceneItemModel::canReparent(EditorSceneItem *newParentItem,
                                       EditorSceneItem *movedItem)
{
    // Dropping into same parent is invalid.
    // Dropping item into its descendant is invalid.
    return movedItem->parentItem() != newParentItem
            && movedItem != newParentItem
            && !EditorUtils::isDescendant(movedItem, newParentItem);
}

void EditorSceneItemModel::reparentEntity(const QModelIndex &newParentIndex,
                                          const QModelIndex &entityIndex)
{
    EditorSceneItem *newParentItem = editorSceneItemFromIndex(newParentIndex);
    EditorSceneItem *entityItem = editorSceneItemFromIndex(entityIndex);

    // Since Qt3D doesn't seem to like reparenting entities, we duplicate the moved entities
    // under a new parent and remove the old ones.

    Qt3DCore::QEntity *duplicate = duplicateEntity(entityItem->entity(), newParentItem->entity());

    m_scene->removeEntity(entityItem->entity());
    m_scene->addEntity(duplicate);

    resetModel();

    // Keep the moved item selected
    emit selectIndex(getModelIndexByName(duplicate->objectName()));
}

void EditorSceneItemModel::addExpandedItem(const QModelIndex &index)
{
    EditorSceneItem *item = editorSceneItemFromIndex(index);
    QString entityName = item->entity()->objectName();
    if (!m_expandedItems.contains(entityName))
        m_expandedItems.append(entityName);
}

void EditorSceneItemModel::removeExpandedItem(const QModelIndex &index)
{
    EditorSceneItem *item = editorSceneItemFromIndex(index);
    m_expandedItems.removeOne(item->entity()->objectName());
}

Qt3DCore::QEntity *EditorSceneItemModel::importEntity(const QUrl &fileUrl)
{
    Qt3DCore::QEntity *sceneLoaderEntity =
            new Qt3DCore::QEntity(m_scene->sceneEntityItem()->entity());
    Qt3DRender::QSceneLoader *sceneLoader = new Qt3DRender::QSceneLoader(sceneLoaderEntity);
    QObject::connect(sceneLoader, &Qt3DRender::QSceneLoader::statusChanged,
                     this, &EditorSceneItemModel::handleSceneLoaderStatusChanged);
    sceneLoader->setSource(fileUrl);
    sceneLoaderEntity->addComponent(sceneLoader);
    sceneLoaderEntity->setObjectName(generateValidName(tr("New Scene Loader"), sceneLoaderEntity));

    Qt3DCore::QTransform *loaderTransform = new Qt3DCore::QTransform();
    sceneLoaderEntity->addComponent(loaderTransform);

    QModelIndex parentIndex = sceneEntityIndex();
    int row = rowCount(parentIndex);
    insertExistingEntity(sceneLoaderEntity, row, parentIndex);

    return sceneLoaderEntity;
}

Qt3DCore::QEntity *EditorSceneItemModel::duplicateEntity(Qt3DCore::QEntity *entity,
                                                         Qt3DCore::QEntity *newParent,
                                                         const QVector3D &duplicateOffset)
{
    // Copies the entity, including making copies of all components and child entities
    // Both copies will retain their entity names.

    Qt3DCore::QEntity *newEntity = nullptr;
    Qt3DRender::QSceneLoader *sceneLoader = nullptr;

    // Check if it's a camera
    Qt3DRender::QCamera *oldCam = qobject_cast<Qt3DRender::QCamera *>(entity);
    if (oldCam) {
        Qt3DRender::QCamera *newCam = new Qt3DRender::QCamera(newParent);
        EditorUtils::copyCameraProperties(newCam, oldCam);
        newEntity = newCam;

        EditorUtils::copyLockProperties(entity, newEntity);

        // Unlock transform related properties in the duplicate
        EditorUtils::lockProperty(
                    QByteArrayLiteral("position") + EditorUtils::lockPropertySuffix8(),
                    newEntity, false);
        EditorUtils::lockProperty(
                    QByteArrayLiteral("upVector") + EditorUtils::lockPropertySuffix8(),
                    newEntity, false);
        EditorUtils::lockProperty(
                    QByteArrayLiteral("viewCenter") + EditorUtils::lockPropertySuffix8(),
                    newEntity, false);

        // Offset the position of the duplicate
        newCam->setPosition(oldCam->position() + duplicateOffset);
    } else {
        newEntity = new Qt3DCore::QEntity(newParent);
        // Duplicate non-internal components
        // Internals will get recreated when duplicate entity is added to scene
        Q_FOREACH (Qt3DCore::QComponent *component, entity->components()) {
            if (!EditorUtils::isObjectInternal(component)) {
                Qt3DCore::QComponent *newComponent = EditorUtils::duplicateComponent(component);
                if (newComponent) {
                    if (!sceneLoader) {
                        sceneLoader = qobject_cast<Qt3DRender::QSceneLoader *>(newComponent);
                        if (sceneLoader) {
                            connect(sceneLoader, &Qt3DRender::QSceneLoader::statusChanged,
                                    this, &EditorSceneItemModel::handleSceneLoaderStatusChanged);
                        }
                    }
                    newEntity->addComponent(newComponent);
                    Qt3DCore::QTransform *newTransform =
                            qobject_cast<Qt3DCore::QTransform *>(newComponent);
                    if (newTransform) {
                        newTransform->setTranslation(newTransform->translation()
                                                     + duplicateOffset * newTransform->scale3D());
                    }
                }
            }
        }
    }

    newEntity->setObjectName(entity->objectName());

    if (!sceneLoader) {
        // Duplicate child entities
        Q_FOREACH (QObject *child, entity->children()) {
            Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
            if (childEntity)
                duplicateEntity(childEntity, newEntity);
        }
    }

    return newEntity;
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
