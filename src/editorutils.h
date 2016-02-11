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
class QCamera;
}
namespace Qt3DRender {
class QGeometryRenderer;
class QGeometry;
class QBuffer;
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
    static bool isObjectInternal(QObject *obj);
    static void copyCameraProperties(Qt3DCore::QCamera *target, Qt3DCore::QCamera *source);
    static Qt3DCore::QEntity *duplicateEntity(Qt3DCore::QEntity *entity,
                                              Qt3DCore::QEntity *newParent = Q_NULLPTR);
    static Qt3DCore::QComponent *duplicateComponent(Qt3DCore::QComponent *component,
                                                    Qt3DCore::QEntity *parent);
    static void nameDuplicate(Qt3DCore::QEntity *duplicate, Qt3DCore::QEntity *original,
                              EditorSceneItemModel *sceneModel);

    static Qt3DRender::QGeometryRenderer *createWireframeBoxMesh(float extent = 1.0f);
    static Qt3DRender::QGeometryRenderer *createWireframePlaneMesh(int lineCount);
    static Qt3DRender::QGeometryRenderer *createDefaultCustomMesh();
    static Qt3DRender::QGeometryRenderer *createRotateHandleMesh(float size);
    static Qt3DRender::QGeometryRenderer *createScaleHandleMesh(float size);
    static Qt3DRender::QGeometryRenderer *createVisibleCameraMesh();
    static Qt3DRender::QGeometryRenderer *createCameraViewVectorMesh();
    static Qt3DRender::QGeometryRenderer *createCameraViewCenterMesh(float size);
    static void addPositionAttributeToGeometry(Qt3DRender::QGeometry *geometry,
                                               Qt3DRender::QBuffer *buffer,
                                               int count);
    static void addIndexAttributeToGeometry(Qt3DRender::QGeometry *geometry,
                                            Qt3DRender::QBuffer *buffer,
                                            int count);
    static void updateCameraFrustumMesh(Qt3DRender::QGeometryRenderer *mesh,
                                         Qt3DCore::QCamera *camera);

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
    static QVector3D projectVectorOnPlane(const QVector3D &vector, const QVector3D &planeNormal);
private:
    static ComponentTypes componentType(Qt3DCore::QComponent *component);
    QString m_copyString;
};

#endif // EDITORUTILS_H
