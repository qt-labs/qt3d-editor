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
#ifndef MESHCOMPONENTPROXYITEM_H
#define MESHCOMPONENTPROXYITEM_H

#include <QObject>

namespace Qt3DRender {
class QGeometryRenderer;
}

class EditorSceneItemMeshComponentsModel;
class EditorSceneItemComponentsModel;

class MeshComponentProxyItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Qt3DRender::QGeometryRenderer* component READ component CONSTANT)
    Q_PROPERTY(EditorSceneItemMeshComponentsModel* model READ model CONSTANT)
public:
    explicit MeshComponentProxyItem(EditorSceneItemComponentsModel *sceneItemModel,
                                    Qt3DRender::QGeometryRenderer *component,
                                    QObject *parent = 0);
    ~MeshComponentProxyItem();

    Qt3DRender::QGeometryRenderer* component() const;

    EditorSceneItemMeshComponentsModel* model() const;

    void beginResetComponent(Qt3DRender::QGeometryRenderer *component);
    void endResetComponent();

private:
    Qt3DRender::QGeometryRenderer *m_component;
    EditorSceneItemMeshComponentsModel *m_model;
};

Q_DECLARE_METATYPE(MeshComponentProxyItem*)

#endif // MESHCOMPONENTPROXYITEM_H
