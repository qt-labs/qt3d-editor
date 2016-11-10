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
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QSceneLoader>
#include <QtCore/QRegularExpression>

const QString validEntityNameMatcher = QStringLiteral("^[A-Za-z_][A-Za-z0-9_ ]*$");

EditorSceneItemModel::EditorSceneItemModel(EditorScene *scene)
    : QAbstractItemModel(scene)
    , m_scene(scene)
    , m_sceneLoaderEntity(nullptr)
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

EditorSceneItem *EditorSceneItemModel::getItemByName(const QString &entityName)
{
    QModelIndex index = getModelIndexByName(entityName);
    return editorSceneItemFromIndex(index);
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

void EditorSceneItemModel::handleImportEntityLoaderStatusChanged()
{
    Qt3DRender::QSceneLoader *sceneLoader = qobject_cast<Qt3DRender::QSceneLoader *>(sender());
    if (m_sceneLoaderEntity && sceneLoader) {
        if (sceneLoader->status() == Qt3DRender::QSceneLoader::Ready
                || sceneLoader->status() == Qt3DRender::QSceneLoader::Error) {
            if (sceneLoader->status() == Qt3DRender::QSceneLoader::Ready) {
                QList<Qt3DCore::QEntity *> entityChildren;
                Q_FOREACH (QObject *child, m_sceneLoaderEntity->children()) {
                    Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
                    if (childEntity && pruneUnimportableEntitites(childEntity, true))
                        entityChildren.append(childEntity);
                }
                Qt3DCore::QEntity *parentEntity = m_scene->sceneEntityItem()->entity();
                if (entityChildren.size() > 1) {
                    // Create a new group entity if imported top level has more than one entity
                    parentEntity = new Qt3DCore::QEntity(parentEntity);
                    parentEntity->addComponent(new Qt3DCore::QTransform());
                    parentEntity->setObjectName(m_sceneLoaderEntity->objectName());
                    m_scene->addEntity(parentEntity);
                } else if (entityChildren.size() == 1) {
                    entityChildren[0]->setObjectName(m_sceneLoaderEntity->objectName());
                }
                Q_FOREACH (Qt3DCore::QEntity *childEntity, entityChildren) {
                    Qt3DCore::QEntity *duplicate = duplicateEntity(childEntity, parentEntity);

                    // Workaround for crash when deleting the originally loaded entities.
                    // TODO: Once the crash issue is fixed in Qt3D, the first pruning above can
                    // TODO: be changed to delete and this one removed.
                    pruneUnimportableEntitites(duplicate, false);

                    ensureTransforms(duplicate);
                    m_scene->addEntity(duplicate, -1, parentEntity);
                    fixEntityNames(duplicate);
                }
                resetModel();
                m_scene->setSelection(m_sceneLoaderEntity);
            } else if (sceneLoader->status() == Qt3DRender::QSceneLoader::Error) {
                m_scene->setError(tr("Failed to import an Entity"));
            }

            // Delete the loaded entities before deleting m_sceneLoaderEntity, as otherwise
            // it crashes Qt3D
            for (auto node : m_sceneLoaderEntity->childNodes())
                delete qobject_cast<Qt3DCore::QEntity *>(node);
            // Can't delete yet, as sceneloader still does things after this method exits
            m_sceneLoaderEntity->deleteLater();
            m_sceneLoaderEntity = nullptr;
            emit importEntityInProgressChanged(false);
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
                m_scene->setError(tr("Failed to load a scene with scene loader"));
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
    m_scene->queueEnsureSelection();
    m_scene->queueUpdateGroupSelectionBoxes();

    QAbstractItemModel::beginResetModel();

    // Reconnect all entities
    disconnectEntity(m_scene->sceneEntityItem()->entity());
    connectEntity(m_scene->sceneEntityItem()->entity());

    QAbstractItemModel::endResetModel();

    // Restore TreeView branch expansions, since resetting the model will collapse the branches
    QModelIndexList expandedIndexList;
    Q_FOREACH (const QString entityName, m_expandedItems)
        expandedIndexList.append(getModelIndexByName(entityName));
    emit expandItems(expandedIndexList);

    emit resetComplete();
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

    Qt3DCore::QEntity *newEntity = createEntity(type, pos, parentIndex);

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

    m_scene->queueUpdateGroupSelectionBoxes();

    return newEntity;
}

Qt3DCore::QEntity *EditorSceneItemModel::createEntity(EditorUtils::InsertableEntities type,
                                                      const QVector3D &pos,
                                                      const QModelIndex &parentIndex)
{
    EditorSceneItem *parentItem = editorSceneItemFromIndex(parentIndex);

    Qt3DCore::QEntity *newEntity = nullptr;
    if (type == EditorUtils::CameraEntity) {
        Qt3DRender::QCamera *newCamera = new Qt3DRender::QCamera(parentItem->entity());
        //: This string is entity name, no non-ascii characters allowed
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
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Cube"), newEntity));
            break;
        }
        case EditorUtils::CylinderEntity: {
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Cylinder"), newEntity));
            break;
        }
        case EditorUtils::PlaneEntity: {
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Plane"), newEntity));
            break;
        }
        case EditorUtils::SphereEntity: {
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Sphere"), newEntity));
            break;
        }
        case EditorUtils::TorusEntity: {
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Torus"), newEntity));
            break;
        }
        case EditorUtils::CustomEntity: {
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Custom"), newEntity));
            break;
        }
        case EditorUtils::LightEntity: {
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Light"), newEntity));
            newEntity->addComponent(new Qt3DRender::QPointLight());
            break;
        }
        case EditorUtils::GroupEntity: {
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Group"), newEntity));
            break;
        }
        default:
            //: This string is entity name, no non-ascii characters allowed
            newEntity->setObjectName(generateValidName(tr("New Empty Entity"), newEntity));
            break;
        }
    }

    return newEntity;
}

void EditorSceneItemModel::insertExistingEntity(Qt3DCore::QEntity *entity, int row,
                                                const QModelIndex &parentIndex)
{
    EditorSceneItem *parentItem = editorSceneItemFromIndex(parentIndex);
    if (row < 0)
        row = rowCount(parentIndex);

    m_scene->addEntity(entity, row, parentItem->entity());

    resetModel();

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

        // Remove entity and child entities from expanded entities list
        removeExpandedItems(entity);

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
    QString oldName = item->entity()->objectName();
    if (oldName == name)
        return QString();

    QString finalName = generateValidName(name, item->entity());

    if (m_expandedItems.removeOne(oldName))
        m_expandedItems.append(finalName);

    setData(index, finalName, NameRole);

    m_scene->renameEntityInMultiSelectionList(oldName, finalName);

    if (oldName == m_scene->clipboardContent())
        m_scene->setClipboardContent(name);

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

QStringList EditorSceneItemModel::parentList(const QStringList &originalList)
{
    const int count = originalList.length();
    QVector<EditorSceneItem *> itemList(count);
    QVector<bool> skipList(count);

    for (int i = 0; i < count; ++i) {
        itemList[i] = editorSceneItemFromIndex(getModelIndexByName(originalList.at(i)));
        skipList[i] = false;
    }

    QStringList prunedList;
    prunedList.reserve(count);
    for (int i = 0; i < count; ++i) {
        bool pruned = false;
        for (int j = count - 1; j >= 0; --j) {
            if (i == j || skipList.at(j))
                continue;
            if (EditorUtils::isDescendant(itemList.at(j), itemList.at(i))) {
                pruned = true;
                // We don't need to consider discovered descendants as ancestors in future.
                skipList[i] = true;
                break;
            }
        }
        if (!pruned)
            prunedList.append(originalList.at(i));
    }

    return prunedList;
}

bool EditorSceneItemModel::canReparent(EditorSceneItem *newParentItem,
                                       EditorSceneItem *movedItem, bool allowSameParent)
{
    // Dropping into camera is invalid.
    // Reparenting camera is invalid.
    // Dropping into same parent is invalid.
    // Dropping item into its descendant is invalid.
    // If allowSameParent is true, "reparenting" under the same parent is allowed. This is useful
    // for entity tree copy & paste functionality.
    bool reparentOk = newParentItem->itemType() != EditorSceneItem::Camera
            && movedItem->itemType() != EditorSceneItem::Camera
            && movedItem != newParentItem
            && !EditorUtils::isDescendant(movedItem, newParentItem);
    if (!allowSameParent)
        reparentOk = reparentOk && movedItem->parentItem() != newParentItem;
    return reparentOk;
}

void EditorSceneItemModel::reparentEntity(const QModelIndex &newParentIndex,
                                          const QModelIndex &entityIndex)
{
    EditorSceneItem *newParentItem = editorSceneItemFromIndex(newParentIndex);
    EditorSceneItem *entityItem = editorSceneItemFromIndex(entityIndex);

    // Find the complete world transform of the entityItem
    QMatrix4x4 totalOldMatrix = EditorUtils::totalAncestralTransform(entityItem->entity());
    QMatrix4x4 entityOldMatrix;
    Qt3DCore::QTransform *entityTransform = EditorUtils::entityTransform(entityItem->entity());
    if (entityTransform)
        entityOldMatrix = entityTransform->matrix();
    QMatrix4x4 worldMatrix = totalOldMatrix * entityOldMatrix;

    // Since Qt3D doesn't seem to like reparenting entities, we duplicate the moved entities
    // under a new parent and remove the old ones.

    Qt3DCore::QEntity *duplicate = duplicateEntity(entityItem->entity(), newParentItem->entity());

    m_scene->removeEntity(entityItem->entity());
    m_scene->addEntity(duplicate);

    // Make sure the world transform doesn't change when reparenting
    QMatrix4x4 totalNewMatrix = EditorUtils::totalAncestralTransform(duplicate);
    QMatrix4x4 newMatrix = totalNewMatrix.inverted() * worldMatrix;
    entityTransform = EditorUtils::entityTransform(duplicate);
    if (entityTransform)
        entityTransform->setMatrix(newMatrix);

    resetModel();
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

void EditorSceneItemModel::removeExpandedItems(Qt3DCore::QEntity *entity)
{
    m_expandedItems.removeOne(entity->objectName());
    Q_FOREACH (QObject *obj, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(obj);
        if (childEntity)
            removeExpandedItems(childEntity);
    }
}

QString EditorSceneItemModel::importEntity(const QUrl &fileUrl)
{
    m_sceneLoaderEntity = new Qt3DCore::QEntity(m_scene->rootEntity());
    QString fileName = fileUrl.toString();
    int index = fileName.lastIndexOf(QLatin1Char('/'));
    if (index > 0 && index < fileName.size())
        fileName = fileName.mid(index + 1);
    index = fileName.indexOf(QLatin1Char('.'));
    if (index > 0)
        fileName = fileName.left(index);

    QRegularExpression re(validEntityNameMatcher);
    QRegularExpressionMatch match = re.match(fileName);

    if (!match.hasMatch()) {
        //: This string is entity name, no non-ascii characters allowed
        fileName = tr("New imported entity");
    }

    m_sceneLoaderEntity->setObjectName(generateValidName(fileName, m_sceneLoaderEntity));
    Qt3DRender::QSceneLoader *sceneLoader = new Qt3DRender::QSceneLoader(m_sceneLoaderEntity);
    QObject::connect(sceneLoader, &Qt3DRender::QSceneLoader::statusChanged,
                     this, &EditorSceneItemModel::handleImportEntityLoaderStatusChanged);
    sceneLoader->setSource(fileUrl);
    m_sceneLoaderEntity->addComponent(sceneLoader);
    m_sceneLoaderEntity->setEnabled(false);

    emit importEntityInProgressChanged(true);

    return m_sceneLoaderEntity->objectName();
}

QString EditorSceneItemModel::insertSceneLoaderEntity(const QUrl &fileUrl)
{
    // TODO: No way from UI to currently insert a scene loader entity.
    // TODO: Do we want to support actual scene loader entitites in addition to importing
    // TODO: entities as entity trees?
    // TODO: If so, should it be similar to custom mesh object where you can change the
    // TODO: source scene at will, or similar to importing entity where you select the scene
    // TODO: at the time of the import and cannot change it?
    Qt3DCore::QEntity *sceneLoaderEntity =
            new Qt3DCore::QEntity(m_scene->sceneEntityItem()->entity());
    Qt3DRender::QSceneLoader *sceneLoader = new Qt3DRender::QSceneLoader(sceneLoaderEntity);
    QObject::connect(sceneLoader, &Qt3DRender::QSceneLoader::statusChanged,
                     this, &EditorSceneItemModel::handleSceneLoaderStatusChanged);
    sceneLoader->setSource(fileUrl);
    sceneLoaderEntity->addComponent(sceneLoader);
    //: This string is entity name, no non-ascii characters allowed
    sceneLoaderEntity->setObjectName(generateValidName(tr("New Scene Loader"), sceneLoaderEntity));

    Qt3DCore::QTransform *loaderTransform = new Qt3DCore::QTransform();
    sceneLoaderEntity->addComponent(loaderTransform);

    QModelIndex parentIndex = sceneEntityIndex();
    int row = rowCount(parentIndex);
    insertExistingEntity(sceneLoaderEntity, row, parentIndex);

    return sceneLoaderEntity->objectName();
}

Qt3DCore::QEntity *EditorSceneItemModel::duplicateEntity(Qt3DCore::QEntity *entity,
                                                         Qt3DCore::QEntity *newParent,
                                                         const QVector3D &duplicateOffset,
                                                         bool offset)
{
    // Copies the entity, including making copies of all components and child entities
    // Both copies will retain their entity names.

    // If offset flag is false, the entity will be placed exactly at duplicateOffset position,
    // otherwise duplicateOffset will be used as the offset from the position of the one being
    // duplicated.

    Qt3DCore::QEntity *newEntity = nullptr;
    Qt3DRender::QSceneLoader *sceneLoader = nullptr;

    // Check if it's a camera
    Qt3DRender::QCamera *oldCam = qobject_cast<Qt3DRender::QCamera *>(entity);
    bool isCamera = oldCam || EditorUtils::entityCameraLens(entity);
    if (isCamera) {
        Qt3DRender::QCamera *newCam = new Qt3DRender::QCamera(newParent);
        EditorUtils::copyCameraProperties(newCam, entity);
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
        if (offset)
            newCam->setPosition(newCam->position() + duplicateOffset);
        else
            newCam->setPosition(duplicateOffset);
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
                        if (offset) {
                            newTransform->setTranslation(newTransform->translation()
                                                         + duplicateOffset);
                        } else {
                            QVector3D newPos;
                            if (!newParent) {
                                newPos = EditorUtils::totalAncestralTransform(entity).inverted()
                                        * duplicateOffset;
                            } else {
                                Qt3DCore::QTransform *newParentTransform =
                                        EditorUtils::entityTransform(newParent);
                                if (newParentTransform) {
                                    newPos = (EditorUtils::totalAncestralTransform(newParent)
                                              * newParentTransform->matrix()).inverted()
                                            * duplicateOffset;
                                } else {
                                    newPos = EditorUtils::totalAncestralTransform(newParent)
                                            * duplicateOffset;
                                }
                            }
                            newTransform->setTranslation(newPos);
                        }
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

void EditorSceneItemModel::abortImportEntity()
{
    if (m_sceneLoaderEntity) {
        delete m_sceneLoaderEntity;
        m_sceneLoaderEntity = nullptr;
        emit importEntityInProgressChanged(false);
    }
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

void EditorSceneItemModel::fixEntityNames(Qt3DCore::QEntity *entity)
{
    QString desiredName = entity->objectName();

    // Check that desired name matches our naming criteria
    QRegularExpression re(validEntityNameMatcher);
    QRegularExpressionMatch match = re.match(desiredName);

    if (!match.hasMatch())
        desiredName = tr("Unnamed Entity");

    QString newName = generateValidName(desiredName, entity);
    entity->setObjectName(newName);

    // Rename possible children
    Q_FOREACH (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            fixEntityNames(childEntity);
    }
}

void EditorSceneItemModel::ensureTransforms(Qt3DCore::QEntity *entity)
{
    // Every entity except scene entity should have a transform
    if (m_scene->sceneEntityItem()->entity() != entity && !EditorUtils::entityTransform(entity))
        entity->addComponent(new Qt3DCore::QTransform);

    Q_FOREACH (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            ensureTransforms(childEntity);
    }
}

bool EditorSceneItemModel::pruneUnimportableEntitites(Qt3DCore::QEntity *entity, bool justCheck)
{
    // Delete all branches that do not end in meshes or lights
    // Delete all non-group, non-mesh, non-light entities

    bool isGroup = EditorUtils::isGroupEntity(entity);
    bool isMesh = EditorUtils::entityMesh(entity) != nullptr;
    bool isLight = EditorUtils::entityLight(entity) != nullptr;

    if (!isGroup && !isMesh && !isLight) {
        if (!justCheck)
            delete entity;
        return false;
    }

    // At this point, we know our ancestors are only meshes, lights, or groups.
    bool hasValidChild = false;
    Q_FOREACH (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            hasValidChild = pruneUnimportableEntitites(childEntity, justCheck) || hasValidChild;
    }

    // Prune empty groups
    if (isGroup && !hasValidChild && !justCheck)
        delete entity;

    return isMesh || isLight || hasValidChild;
}
