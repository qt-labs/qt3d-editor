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
#ifndef EDITORSCENEITEMMATERIALCOMPONENTSMODEL_H
#define EDITORSCENEITEMMATERIALCOMPONENTSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QAbstractListModel>

namespace Qt3DRender {
class QMaterial;
class QAbstractTexture;
}

class EditorSceneItemComponentsModel;

class EditorSceneItemMaterialComponentsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(MaterialComponentTypes)
public:
    enum MaterialComponentTypes {
        DiffuseMap = 1,
        DiffuseSpecularMap,
        Gooch,
        NormalDiffuseMap,
        NormalDiffuseMapAlpha,
        NormalDiffuseSpecularMap,
        PhongAlpha,
        Phong,
        PerVertexColor,
        Unknown = 1000
        // Keep PerVertexColor one last known type until it's used in materialCombobox
        // else it upsets the index
    };

    enum EditorSceneItemMaterialComponentsRoles {
        MaterialComponentType = Qt::UserRole + 1,
        MaterialComponent,
        MaterialDiffuseTextureUrl,
        MaterialSpecularTextureUrl,
        MaterialNormalTextureUrl
    };
    Q_ENUM(EditorSceneItemMaterialComponentsRoles)

    EditorSceneItemMaterialComponentsModel(Qt3DRender::QMaterial *MaterialComponent,
                                           EditorSceneItemComponentsModel *sceneItemModel,
                                           QObject *parent = 0);
    ~EditorSceneItemMaterialComponentsModel();

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void setMaterial(MaterialComponentTypes type);
    Q_INVOKABLE void removeMaterial(int index);

    void beginReplace();
    void endReplace(Qt3DRender::QMaterial *newMaterial);

Q_SIGNALS:
    void roleDataChanged(int roleIndex);

private:
    MaterialComponentTypes materialType(Qt3DRender::QMaterial *material) const;

    Qt3DRender::QAbstractTexture *getDiffuseTexture() const;
    Qt3DRender::QAbstractTexture *getSpecularTexture() const;
    Qt3DRender::QAbstractTexture *getNormalTexture() const;
    QUrl getTextureUrl(Qt3DRender::QAbstractTexture *texture) const;

    Qt3DRender::QMaterial *m_materialComponent;
    EditorSceneItemComponentsModel *m_sceneComponentsModel;
    MaterialComponentTypes m_type;
};

Q_DECLARE_METATYPE(EditorSceneItemMaterialComponentsModel*)
Q_DECLARE_METATYPE(EditorSceneItemMaterialComponentsModel::MaterialComponentTypes)

#endif // EDITORSCENEITEMMATERIALCOMPONENTSMODEL_H
