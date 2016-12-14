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
#include <QtGui/QMatrix4x4>

namespace Qt3DCore {
class QEntity;
class QComponent;
class QTransform;
}
namespace Qt3DRender {
class QGeometryRenderer;
class QGeometry;
class QBuffer;
class QAbstractLight;
class QObjectPicker;
class QCamera;
class QCameraLens;
class QAttribute;
class QRenderPass;
class QSceneLoader;
}

class EditorSceneItemModel;
class EditorSceneItem;

class EditorUtils : public QObject
{
    Q_OBJECT
public:
    enum ComponentTypes {
        // Lights
        LightDirectional = 1,
        LightPoint,
        LightSpot,
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
        MaterialGeneric,
        // Meshes
        MeshCuboid,
        MeshCustom,
        MeshCylinder,
        MeshPlane,
        MeshSphere,
        MeshTorus,
        MeshGeneric,
        // Transforms
        Transform,
        // Other
        ObjectPicker,
        SceneLoader,
        Unknown = 1000
    };
    Q_ENUM(ComponentTypes)

    enum InsertableEntities {
        InvalidEntity,
        GroupEntity,
        CuboidEntity,
        CylinderEntity,
        PlaneEntity,
        SphereEntity,
        TorusEntity,
        CustomEntity,
        CameraEntity,
        LightEntity
    };
    Q_ENUM(InsertableEntities)


public:
    static bool isObjectInternal(QObject *obj);
    static void copyCameraProperties(Qt3DRender::QCamera *target, Qt3DCore::QEntity *source);
    static Qt3DCore::QComponent *duplicateComponent(Qt3DCore::QComponent *component);
    static QString nameDuplicate(Qt3DCore::QEntity *duplicate, Qt3DCore::QEntity *original,
                                 EditorSceneItemModel *sceneModel);

    static Qt3DRender::QGeometryRenderer *createWireframeBoxMesh(float extent = 1.0f);
    static Qt3DRender::QGeometryRenderer *createWireframePlaneMesh(int lineCount);
    static Qt3DRender::QGeometryRenderer *createSingleLineMesh();
    static Qt3DRender::QGeometryRenderer *createDefaultCustomMesh();
    static Qt3DRender::QGeometryRenderer *createVisibleCameraMesh();
    static Qt3DRender::QGeometryRenderer *createCameraViewVectorMesh();
    static Qt3DRender::QGeometryRenderer *createCameraViewCenterMesh(float size);
    static Qt3DRender::QGeometryRenderer *createLightMesh(ComponentTypes type);
    static Qt3DRender::QGeometryRenderer *createMeshForInsertableType(InsertableEntities type);
    static Qt3DRender::QGeometryRenderer *createArrowMesh();
    static Qt3DCore::QEntity *createArrowEntity(const QColor &color,
                                                Qt3DCore::QEntity *parent,
                                                const QMatrix4x4 &matrix,
                                                const QString &name);
    static void addPositionAttributeToGeometry(Qt3DRender::QGeometry *geometry,
                                               Qt3DRender::QBuffer *buffer,
                                               int count);
    static void addIndexAttributeToGeometry(Qt3DRender::QGeometry *geometry,
                                            Qt3DRender::QBuffer *buffer,
                                            int count);
    static void updateCameraFrustumMesh(Qt3DRender::QGeometryRenderer *mesh,
                                        Qt3DRender::QCamera *camera);

    static Qt3DCore::QTransform *entityTransform(Qt3DCore::QEntity *entity);
    static Qt3DRender::QAbstractLight *entityLight(Qt3DCore::QEntity *entity);
    static Qt3DRender::QObjectPicker *entityPicker(Qt3DCore::QEntity *entity);
    static Qt3DRender::QSceneLoader *entitySceneLoader(Qt3DCore::QEntity *entity);
    static Qt3DRender::QGeometryRenderer *entityMesh(Qt3DCore::QEntity *entity);
    static Qt3DRender::QCameraLens *entityCameraLens(Qt3DCore::QEntity *entity);
    static bool isGroupEntity(Qt3DCore::QEntity *entity);
    static QVector3D findIntersection(const QVector3D &rayOrigin, const QVector3D &ray,
                                      float planeOffset, const QVector3D &planeNormal,
                                      float &t);
    static QVector3D unprojectRay(const QMatrix4x4 &modelView,
                                  const QMatrix4x4 &projectionMatrix,
                                  int viewPortWidth, int viewPortHeight, const QPoint &pos);
    static QVector3D projectRay(const QMatrix4x4 &viewMatrix, const QMatrix4x4 &projectionMatrix,
                                int viewPortWidth, int viewPortHeight, const QVector3D &worldPos);
    static QVector3D absVector3D(const QVector3D &vector);
    static QVector3D maxVector3D(const QVector3D &vector, float minValue);

    static QVector3D rotateVector(const QVector3D &vector, const QVector3D &rotationAxis,
                                  qreal radians);
    static QVector3D projectVectorOnPlane(const QVector3D &vector, const QVector3D &planeNormal);
    static QMatrix4x4 totalAncestralTransform(Qt3DCore::QEntity *entity);
    static QVector3D totalAncestralScale(Qt3DCore::QEntity *entity);
    static QQuaternion totalAncestralRotation(Qt3DCore::QEntity *entity);
    static QList<Qt3DCore::QTransform *> ancestralTransforms(
            Qt3DCore::QEntity *entity, Qt3DCore::QEntity *topAncestor = nullptr);
    static QVector3D lightDirection(const Qt3DRender::QAbstractLight *light);

    static const QString lockPropertySuffix() { return QStringLiteral("_editorPropertyLock"); }
    static const QByteArray lockPropertySuffix8() { return QByteArrayLiteral("_editorPropertyLock"); }
    static const QString lockTransformPropertyName() {
        return QStringLiteral("allTransform_editorPropertyLock");
    }
    static QVector3D cameraNormal(Qt3DRender::QCamera *camera);
    static bool isDescendant(EditorSceneItem *ancestor, EditorSceneItem *descendantItem);
    static void copyLockProperties(const QObject *source, QObject *target);
    static void lockProperty(const QByteArray &lockPropertyName, QObject *obj, bool lock);
    static InsertableEntities insertableEntityType(Qt3DCore::QEntity *entity);
    static void setEnabledToSubtree(Qt3DCore::QEntity *entity, bool enable);
    static Qt3DCore::QEntity *findEntityByName(Qt3DCore::QEntity *entity, const QString &name);

private:
    // Private constructor to ensure no actual instance is created
    explicit EditorUtils() {}

    static ComponentTypes componentType(Qt3DCore::QComponent *component);
    static Qt3DRender::QAttribute *copyAttribute(
            Qt3DRender::QAttribute *oldAtt,
            QMap<Qt3DRender::QBuffer *, Qt3DRender::QBuffer *> &bufferMap);
    template <typename T> static void copyRenderParameters(T *source, T *target);
    template <typename T> static void copyFilterKeys(T *source, T *target);
    static void copyRenderStates(Qt3DRender::QRenderPass *source, Qt3DRender::QRenderPass *target);
};

#endif // EDITORUTILS_H
