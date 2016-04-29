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
#include "editorsceneitemlightcomponentsmodel.h"
#include "editorsceneitemcomponentsmodel.h"
#include "editorsceneitem.h"
#include "editorscene.h"
#include "undohandler.h"
#include "lightcomponentproxyitem.h"
#include <Qt3DRender/QAbstractLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DCore/QEntity>
#include <QtCore/QStack>

EditorSceneItemLightComponentsModel::EditorSceneItemLightComponentsModel(
        Qt3DRender::QAbstractLight *lightComponent,
        EditorSceneItemComponentsModel *sceneItemModel, QObject *parent)
    : QAbstractListModel(parent)
    , m_lightComponent(lightComponent)
    , m_sceneComponentsModel(sceneItemModel)
    , m_type(Unknown)
{
    m_type = lightType(lightComponent);
}

EditorSceneItemLightComponentsModel::~EditorSceneItemLightComponentsModel()
{

}

int EditorSceneItemLightComponentsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return (m_lightComponent != nullptr);
}

QVariant EditorSceneItemLightComponentsModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)

    if (role == LightComponentType) {
        return m_type;
    } else if (role == LightComponent) {
        QVariant lightComponentData;
        if (m_type != Unknown)
            lightComponentData = QVariant::fromValue(m_lightComponent);
        return lightComponentData;
    } else {
        return QVariant();
    }

}

QHash<int, QByteArray> EditorSceneItemLightComponentsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LightComponentType] = "lightType";
    roles[LightComponent] = "lightComponentData";
    return roles;
}

void EditorSceneItemLightComponentsModel::setLight(LightComponentTypes type)
{
    if (type != m_type) {
        Qt3DRender::QAbstractLight *light = nullptr;
        switch (type) {
        case Directional:
            light = new Qt3DRender::QDirectionalLight();
            // Default the direction to such that doesn't align on any helper plane,
            // so that rotate will always work by default
            static_cast<Qt3DRender::QDirectionalLight *>(light)
                    ->setWorldDirection(QVector3D(1.0f, -1.0f, 1.0f));
            break;
        case Point:
            light = new Qt3DRender::QPointLight();
            break;
        case Spot:
            light = new Qt3DRender::QSpotLight();
            static_cast<Qt3DRender::QSpotLight *>(light)
                    ->setLocalDirection(QVector3D(1.0f, -1.0f, 1.0f));
            break;
        default:
            //Unsupported light type
            break;
        }

        if (light == nullptr)
            return;

        m_sceneComponentsModel->sceneItem()->scene()->undoHandler()->createReplaceComponentCommand(
                    m_sceneComponentsModel->sceneItem()->entity()->objectName(),
                    EditorSceneItemComponentsModel::Light,
                    light, m_lightComponent);
    }
}

void EditorSceneItemLightComponentsModel::removeLight(int index)
{
    Q_UNUSED(index)

    m_sceneComponentsModel->removeComponent(m_lightComponent);
    m_lightComponent = nullptr;
}

void EditorSceneItemLightComponentsModel::beginReplace()
{
    beginResetModel();
    m_type = Unknown;
    m_lightComponent = nullptr;
    endResetModel();
}

void EditorSceneItemLightComponentsModel::endReplace(Qt3DRender::QAbstractLight *newLight)
{
    beginResetModel();
    m_type = lightType(newLight);
    m_lightComponent = newLight;
    endResetModel();
}

EditorSceneItemLightComponentsModel::LightComponentTypes
EditorSceneItemLightComponentsModel::lightType(Qt3DRender::QAbstractLight *light) const
{
    if (qobject_cast<Qt3DRender::QDirectionalLight *>(light))
        return Directional;
    else if (qobject_cast<Qt3DRender::QSpotLight *>(light))
        return Spot;
    else if (qobject_cast<Qt3DRender::QPointLight *>(light))
        return Point;
    else
        return Unknown;
}
