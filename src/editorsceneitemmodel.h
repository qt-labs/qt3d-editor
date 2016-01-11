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

#include <QtCore/QAbstractItemModel>

namespace Qt3DCore {
    class QEntity;
}

class EditorScene;
class EditorSceneItem;
class EditorSceneItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum InsertableEntities {
        GenericEntity,
        CuboidEntity,
        CylinderEntity,
        PlaneEntity,
        SphereEntity,
        TorusEntity,
        CustomEntity,
        CameraEntity,
        LightEntity
    };
    Q_ENUM(InsertableEntities)

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
    Qt3DCore::QEntity *insertEntity(InsertableEntities type,
                                    const QModelIndex &parentIndex = QModelIndex());
    void insertExistingEntity(Qt3DCore::QEntity *entity, int row,
                              const QModelIndex &parentIndex = QModelIndex());
    Q_INVOKABLE QModelIndex lastInsertedIndex();
    Qt3DCore::QEntity *removeEntity(const QModelIndex &index = QModelIndex());
    const QString setEntityName(const QModelIndex &index, const QString &name);
    Q_INVOKABLE const QString entityName(const QModelIndex &index) const;
    Q_INVOKABLE QModelIndex sceneEntityIndex();
    Q_INVOKABLE QModelIndex getModelIndex(Qt3DCore::QEntity *entity);
    QModelIndex getModelIndexByName(const QString &entityName);

signals:
    void freeViewChanged(bool enabled);

public slots:
    void handleEntityNameChange();

private:
    QString generateValidName(const QString &desiredName, const Qt3DCore::QEntity *skipEntity);
    QString fixEntityName(const QString &desiredName);
    void collectEntityNames(QStringList &nameList, const Qt3DCore::QEntity *entity,
                            const Qt3DCore::QEntity *skipEntity);
    void connectEntity(Qt3DCore::QEntity *entity);
    void disconnectEntity(Qt3DCore::QEntity *entity);

    EditorScene *m_scene;
    QModelIndex m_lastInsertedIndex;
};

Q_DECLARE_METATYPE(EditorSceneItemModel*)

#endif // EDITORSCENEITEMMODEL_H
