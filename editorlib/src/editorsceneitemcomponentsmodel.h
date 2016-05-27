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
#ifndef EDITORSCENEITEMCOMPONENTSMODEL_H
#define EDITORSCENEITEMCOMPONENTSMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QMap>
#include <Qt3DCore/QComponent>
#include "transformcomponentproxyitem.h"
#include "materialcomponentproxyitem.h"
#include "meshcomponentproxyitem.h"
#include "lightcomponentproxyitem.h"

class EditorSceneItem;

class EditorSceneItemComponentsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(EditorSceneItemComponentTypes)

public:
    // The enum order dictates the order the components appear in the model (and UI)
    enum EditorSceneItemComponentTypes {
        GeneralEntity = 0,
        CameraEntity,
        Light,
        Mesh,
        Transform,
        Material,
        ObjectPicker,
        SupportedComponentCount,
        Internal = 900,
        Unknown = 1000
    };
    Q_ENUM(EditorSceneItemComponentTypes)

    enum EditorSceneItemComponentRoles {
        ComponentType = Qt::UserRole + 1,
        Component
    };

    EditorSceneItemComponentsModel(EditorSceneItem *sceneItem, QObject *parent = 0);
    ~EditorSceneItemComponentsModel();

    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void appendNewComponent(EditorSceneItemComponentTypes type);
    Q_INVOKABLE void removeComponent(int index);
    void removeComponent(Qt3DCore::QComponent *component);
    void replaceComponent(Qt3DCore::QComponent *oldComponent, Qt3DCore::QComponent *newComponent);

    void initializeModel();
    EditorSceneItemComponentTypes typeOfComponent(QObject *component);

    MaterialComponentProxyItem *materialProxy() const { return m_materialItem; }
    EditorSceneItem *sceneItem() const { return m_sceneItem; }

private:
    EditorSceneItem *m_sceneItem;
    TransformComponentProxyItem *m_transformItem;
    MaterialComponentProxyItem *m_materialItem;
    MeshComponentProxyItem *m_meshItem;
    LightComponentProxyItem *m_lightItem;

    struct ComponentInfo {
        ComponentInfo()
            : component(nullptr), type(Unknown) {}
        ComponentInfo(Qt3DCore::QComponent *comp, EditorSceneItemComponentTypes compType)
            : component(comp), type(compType) {}

        Qt3DCore::QComponent *component;
        EditorSceneItemComponentTypes type;
    };

    // Vector index equals model row
    QVector<ComponentInfo> m_modelRowList;
};

Q_DECLARE_METATYPE(EditorSceneItemComponentsModel*)
Q_DECLARE_METATYPE(EditorSceneItemComponentsModel::EditorSceneItemComponentTypes)

#endif // EDITORSCENEITEMCOMPONENTSMODEL_H
