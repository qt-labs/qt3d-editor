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
#include "transformcomponentproxyitem.h"
#include "editorsceneitemtransformcomponentsmodel.h"

#include <Qt3DCore/QTransform>

TransformComponentProxyItem::TransformComponentProxyItem(EditorSceneItemComponentsModel *sceneItemModel,
                                                         Qt3DCore::QTransform *component,
                                                         QObject *parent)
    : QObject(parent)
    , m_component(component)
    , m_model(new EditorSceneItemTransformComponentsModel(sceneItemModel, m_component, this))
{
}

TransformComponentProxyItem::~TransformComponentProxyItem()
{

}

Qt3DCore::QTransform *TransformComponentProxyItem::component() const
{
    return m_component;
}

EditorSceneItemTransformComponentsModel *TransformComponentProxyItem::model() const
{
    return m_model;
}

void TransformComponentProxyItem::beginResetComponent(Qt3DCore::QTransform *component)
{
    // Currently no transform replace is supported, as it is not needed
    // Might change if we want to support custom transform orderings
    m_component = component;
}

void TransformComponentProxyItem::endResetComponent()
{
    // Currently no transform replace is supported, as it is not needed
    // Might change if we want to support custom transform orderings
}
