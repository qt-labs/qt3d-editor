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
#ifndef EDITORSCENEITEMLIGHTCOMPONENTSMODEL_H
#define EDITORSCENEITEMLIGHTCOMPONENTSMODEL_H

#include <QtCore/QObject>
#include <QtCore/QAbstractListModel>

namespace Qt3DRender {
class QAbstractLight;
}

class EditorSceneItemComponentsModel;

class EditorSceneItemLightComponentsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(LightComponentTypes)
public:
    enum LightComponentTypes {
        Directional = 1,
        Point,
        Spot,
        Unknown = 1000
    };

    enum EditorSceneItemLightComponentsRoles {
        LightComponentType = Qt::UserRole + 1,
        LightComponent
    };

    EditorSceneItemLightComponentsModel(Qt3DRender::QAbstractLight *lightComponent,
                                        EditorSceneItemComponentsModel *sceneItemModel,
                                        QObject *parent = 0);
    ~EditorSceneItemLightComponentsModel();

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void setLight(LightComponentTypes type);
    Q_INVOKABLE void removeLight(int index);

    void beginReplace();
    void endReplace(Qt3DRender::QAbstractLight *newLight);

private:
    LightComponentTypes lightType(Qt3DRender::QAbstractLight *light) const;

    Qt3DRender::QAbstractLight *m_lightComponent;
    EditorSceneItemComponentsModel *m_sceneComponentsModel;
    LightComponentTypes m_type;
};

Q_DECLARE_METATYPE(EditorSceneItemLightComponentsModel*)
Q_DECLARE_METATYPE(EditorSceneItemLightComponentsModel::LightComponentTypes)

#endif // EDITORSCENEITEMLIGHTCOMPONENTSMODEL_H
