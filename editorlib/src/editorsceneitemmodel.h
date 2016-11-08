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
#ifndef EDITORSCENEITEMMODEL_H
#define EDITORSCENEITEMMODEL_H

#include "editorutils.h"

#include <QtCore/QAbstractItemModel>

namespace Qt3DCore {
    class QEntity;
}

class EditorScene;
class EditorSceneItem;
class EditorSceneItemModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool importEntityInProgress READ importEntityInProgress NOTIFY importEntityInProgressChanged)

public:
    enum EditorSceneItemRoles {
        ItemRole = Qt::UserRole + 1,
        NameRole,
        IdRole
    };

    EditorSceneItemModel(EditorScene *scene = 0);
    ~EditorSceneItemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;
    void resetModel();

    Q_INVOKABLE EditorSceneItem *editorSceneItemFromIndex(const QModelIndex &index) const;
    Qt3DCore::QEntity *insertEntity(EditorUtils::InsertableEntities type,
                                    const QVector3D &pos,
                                    const QModelIndex &parentIndex = QModelIndex());
    void insertExistingEntity(Qt3DCore::QEntity *entity, int row,
                              const QModelIndex &parentIndex = QModelIndex());
    Qt3DCore::QEntity *createEntity(EditorUtils::InsertableEntities type,
                                    const QVector3D &pos,
                                    const QModelIndex &parentIndex);
    Q_INVOKABLE QModelIndex lastInsertedIndex();
    void removeEntity(const QModelIndex &index = QModelIndex());
    const QString setEntityName(const QModelIndex &index, const QString &name);
    Q_INVOKABLE const QString entityName(const QModelIndex &index) const;
    Q_INVOKABLE QModelIndex sceneEntityIndex();
    Q_INVOKABLE QModelIndex getModelIndex(Qt3DCore::QEntity *entity);
    Q_INVOKABLE QModelIndex getModelIndexByName(const QString &entityName);
    EditorSceneItem *getItemByName(const QString &entityName);
    QString generateValidName(const QString &desiredName, const Qt3DCore::QEntity *skipEntity);
    EditorScene *scene() { return m_scene; }
    Q_INVOKABLE QStringList parentList(const QStringList &originalList);
    Q_INVOKABLE bool canReparent(EditorSceneItem *newParentItem, EditorSceneItem *movedItem,
                                 bool allowSameParent = false);
    void reparentEntity(const QModelIndex &newParentIndex, const QModelIndex &entityIndex);
    Q_INVOKABLE void addExpandedItem(const QModelIndex &index);
    Q_INVOKABLE void removeExpandedItem(const QModelIndex &index);
    void removeExpandedItems(Qt3DCore::QEntity *entity);
    void clearExpandedItems() { m_expandedItems.clear(); }
    QString importEntity(const QUrl &fileUrl);
    QString insertSceneLoaderEntity(const QUrl &fileUrl);
    Qt3DCore::QEntity *duplicateEntity(Qt3DCore::QEntity *entity,
                                       Qt3DCore::QEntity *newParent = nullptr,
                                       const QVector3D &duplicateOffset = QVector3D(),
                                       bool offset = true);
    Q_INVOKABLE bool importEntityInProgress() const { return m_sceneLoaderEntity != nullptr; }
    void abortImportEntity();

signals:
    void freeViewChanged(bool enabled);
    void expandItems(const QModelIndexList &items);
    void importEntityInProgressChanged(bool importInProgress);
    void resetComplete();

private slots:
    void handleEntityNameChange();
    void handleImportEntityLoaderStatusChanged();
    void handleSceneLoaderStatusChanged();

private:
    QString fixEntityName(const QString &desiredName);
    void collectEntityNames(QStringList &nameList, const Qt3DCore::QEntity *entity,
                            const Qt3DCore::QEntity *skipEntity);
    void connectEntity(Qt3DCore::QEntity *entity);
    void disconnectEntity(Qt3DCore::QEntity *entity);
    void fixEntityNames(Qt3DCore::QEntity *entity);
    void ensureTransforms(Qt3DCore::QEntity *entity);
    bool pruneUnimportableEntitites(Qt3DCore::QEntity *entity, bool justCheck);

    EditorScene *m_scene;
    QModelIndex m_lastInsertedIndex;
    QStringList m_expandedItems;
    Qt3DCore::QEntity *m_sceneLoaderEntity;
};

Q_DECLARE_METATYPE(EditorSceneItemModel*)

#endif // EDITORSCENEITEMMODEL_H
