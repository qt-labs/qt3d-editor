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
#include "meshcomponentproxyitem.h"
#include "editorsceneitemmeshcomponentsmodel.h"

#include <Qt3DRender/QGeometryRenderer>

MeshComponentProxyItem::MeshComponentProxyItem(EditorSceneItemComponentsModel *sceneItemModel,
                                               Qt3DRender::QGeometryRenderer *component,
                                               QObject *parent)
    : QObject(parent)
    , m_component(component)
    , m_model(new EditorSceneItemMeshComponentsModel(m_component, sceneItemModel, this))
{
}

MeshComponentProxyItem::~MeshComponentProxyItem()
{

}

Qt3DRender::QGeometryRenderer *MeshComponentProxyItem::component() const
{
    return m_component;
}

EditorSceneItemMeshComponentsModel *MeshComponentProxyItem::model() const
{
    return m_model;
}

void MeshComponentProxyItem::beginResetComponent(Qt3DRender::QGeometryRenderer *component)
{
    m_model->beginReplace();
    m_component = component;
}

void MeshComponentProxyItem::endResetComponent()
{
    m_model->endReplace(m_component);
}
