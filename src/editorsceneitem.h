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
#ifndef EDITORSCENEITEM_H
#define EDITORSCENEITEM_H

#include <QtCore/QObject>
#include <QtGui/QVector3D>

#include "editorsceneitemmeshcomponentsmodel.h" // For mesh type determination

namespace Qt3DCore {
    class QEntity;
    class QTransform;
}

namespace Qt3DRender {
    class QGeometryRenderer;
}

class EditorScene;
class EditorSceneItemComponentsModel;

class EditorSceneItem : public QObject
{
    Q_OBJECT
    Q_ENUMS(ItemType)

    Q_PROPERTY(EditorSceneItemComponentsModel* componentsModel READ componentsModel CONSTANT)
    Q_PROPERTY(bool showSelectionBox READ isSelectionBoxShowing WRITE setShowSelectionBox NOTIFY showSelectionBoxChanged)
public:
    enum ItemType {
        Camera = 1,
        Light,
        Mesh,
        Other
    };

    EditorSceneItem(EditorScene *scene, Qt3DCore::QEntity *entity,
                    EditorSceneItem *parentItem = Q_NULLPTR,
                    int index = -1, bool freeView = false, QObject *parent = Q_NULLPTR);
    ~EditorSceneItem();

    Q_INVOKABLE Qt3DCore::QEntity *entity();

    const QList<EditorSceneItem *> &childItems();
    EditorSceneItem *parentItem();

    int childNumber() const;

    void addChild(EditorSceneItem *child, int index = -1);
    void removeChild(EditorSceneItem *child);

    void setParentItem(EditorSceneItem *parentItem);

    EditorSceneItemComponentsModel* componentsModel() const;

    // TODO: Freeview flag doesn't belong to this class, should use it from scene.
    void setFreeViewFlag(bool enabled);
    bool freeViewFlag() const;

    void setShowSelectionBox(bool enabled);
    bool isSelectionBoxShowing() const;

    EditorScene *scene() const;

    Q_INVOKABLE ItemType itemType() { return m_itemType; }

    Q_INVOKABLE bool setCustomProperty(const QString name, const QVariant &value);
    Q_INVOKABLE QVariant customProperty(const QString name) const;
    Qt3DCore::QTransform *selectionTransform() const { return m_selectionTransform; }
    Qt3DCore::QTransform *entityTransform() const { return m_entityTransform; }
    QVector3D selectionBoxExtents() const { return m_selectionBoxExtents; }
    QVector3D entityMeshExtents() const { return m_entityMeshExtents; }
    QVector3D selectionBoxCenter() const { return m_selectionBoxCenter; }

public slots:
    void updateSelectionBoxTransform();
    void handleMeshChange(Qt3DRender::QGeometryRenderer *newMesh);
    void recalculateMeshExtents();

signals:
    void freeViewChanged(bool enabled);
    void showSelectionBoxChanged(bool enabled);
    void selectionBoxTransformChanged(EditorSceneItem *item);

private:
    void connectSelectionBoxTransformsRecursive(bool enabled);
    void composeSelectionBoxTransformRecursive(QMatrix4x4 &transformMatrix);
    void connectEntityMesh(bool enabled);
    void recalculateCustomMeshExtents(Qt3DRender::QGeometryRenderer *mesh);

    Qt3DCore::QEntity *m_entity; // Not owned

    EditorSceneItem *m_parentItem; // Not owned
    QList<EditorSceneItem *> m_children;

    EditorSceneItemComponentsModel *m_componentsModel;

    bool m_freeView;

    EditorScene *m_scene; // Not owned

    Qt3DCore::QEntity *m_selectionBox;  // Created, but not owned
    Qt3DCore::QTransform *m_selectionTransform;  // Created, but not owned
    Qt3DCore::QTransform *m_entityTransform; // Not owned
    Qt3DRender::QGeometryRenderer *m_entityMesh; // Not owned
    EditorSceneItemMeshComponentsModel::MeshComponentTypes m_entityMeshType;
    ItemType m_itemType;

    QVector3D m_entityMeshExtents;
    QVector3D m_entityMeshCenter;
    QVector3D m_selectionBoxExtents;
    QVector3D m_selectionBoxCenter;
};

Q_DECLARE_METATYPE(EditorSceneItem*)

#endif // EDITORSCENEITEM_H
