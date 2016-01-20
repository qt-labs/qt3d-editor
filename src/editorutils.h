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
#ifndef EDITORUTILS_H
#define EDITORUTILS_H

#include <QtCore/QObject>

namespace Qt3DCore {
class QEntity;
class QComponent;
class QTransform;
}
namespace Qt3DRender {
class QGeometryRenderer;
}

class EditorSceneItemModel;

class EditorUtils
{
    enum ComponentTypes {
        // Lights
        LightDirectional = 1,
        LightPoint,
        LightSpot,
        LightBasic,
        // Materials
        MaterialDiffuseMap,
        MaterialDiffuseSpecularMap,
        MaterialGooch,
        MaterialNormalDiffuseMap,
        MaterialNormalDiffuseMapAlpha,
        MaterialNormalDiffuseSpecularMap,
        MaterialPerVertexColor,
        MaterialPhongAlpha,
        MaterialPhong,
        // Meshes
        MeshCuboid,
        MeshCustom,
        MeshCylinder,
        MeshPlane,
        MeshSphere,
        MeshTorus,
        // Transforms
        Transform,
        // Other
        CameraLens,
        FrameGraph,
        KeyboardInput,
        Layer,
        Logic,
        MouseInput,
        ObjectPicker,
        Unknown = 1000
    };

public:
    static Qt3DCore::QComponent *duplicateComponent(Qt3DCore::QComponent *component,
                                                    Qt3DCore::QEntity *parent,
                                                    EditorSceneItemModel *sceneModel);
    static void nameDuplicate(QObject *duplicate, QObject *original, Qt3DCore::QEntity *parent,
                              EditorSceneItemModel *sceneModel);

    static Qt3DRender::QGeometryRenderer *createWireframeBoxMesh();
    static Qt3DRender::QGeometryRenderer *createWireframePlaneMesh(int lineCount);
    static Qt3DRender::QGeometryRenderer *createDefaultCustomMesh();
    static Qt3DRender::QGeometryRenderer *createRotateHandleMesh(float size);
    static Qt3DRender::QGeometryRenderer *createScaleHandleMesh(float size);

    static Qt3DCore::QTransform *entityTransform(Qt3DCore::QEntity *entity);
    static QVector3D findIntersection(const QVector3D &rayOrigin, const QVector3D &ray,
                                      float planeOffset, const QVector3D &planeNormal,
                                      float &t);
    static QVector3D unprojectRay(const QMatrix4x4 &modelView,
                                  const QMatrix4x4 &projectionMatrix,
                                  int viewPortWidth, int viewPortHeight, const QPoint &pos);
    static QVector3D absVector3D(const QVector3D &vector);
    static QVector3D maxVector3D(const QVector3D &vector, float minValue);

    static QVector3D rotateVector(const QVector3D &vector, const QVector3D &rotationAxis,
                                  qreal radians);
private:
    static ComponentTypes componentType(Qt3DCore::QComponent *component);
    QString m_copyString;
};

#endif // EDITORUTILS_H
