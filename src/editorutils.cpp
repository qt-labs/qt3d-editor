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
#include "editorutils.h"
#include "editorsceneitemmodel.h"
#include "editorsceneitem.h"
#include "qdummyobjectpicker.h"

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QTextureImage>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QSceneLoader>

#include <Qt3DCore/QTransform>

#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCuboidMesh>
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QPlaneMesh>
#include <Qt3DRender/QSphereMesh>
#include <Qt3DRender/QTorusMesh>

#include <Qt3DRender/QDiffuseMapMaterial>
#include <Qt3DRender/QDiffuseSpecularMapMaterial>
#include <Qt3DRender/QGoochMaterial>
#include <Qt3DRender/QNormalDiffuseMapMaterial>
#include <Qt3DRender/QNormalDiffuseMapAlphaMaterial>
#include <Qt3DRender/QNormalDiffuseSpecularMapMaterial>
#include <Qt3DRender/QPerVertexColorMaterial>
#include <Qt3DRender/QPhongAlphaMaterial>
#include <Qt3DRender/QPhongMaterial>

#include <Qt3DRender/QLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QSpotLight>

#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>

#include <QtCore/QtMath>

static const QString internalPrefix = QStringLiteral("__internal");

bool EditorUtils::isObjectInternal(QObject *obj)
{
    if (obj)
        return obj->objectName().startsWith(internalPrefix);
    else
        return false;
}

void EditorUtils::copyCameraProperties(Qt3DRender::QCamera *target, Qt3DRender::QCamera *source)
{
    target->setAspectRatio(source->aspectRatio());
    target->setBottom(source->bottom());
    target->setFarPlane(source->farPlane());
    target->setFieldOfView(source->fieldOfView());
    target->setLeft(source->left());
    target->setNearPlane(source->nearPlane());
    target->setPosition(source->position());
    target->setProjectionType(source->projectionType());
    target->setRight(source->right());
    target->setTop(source->top());
    target->setUpVector(source->upVector());
    target->setViewCenter(source->viewCenter());
}

Qt3DCore::QComponent *EditorUtils::duplicateComponent(Qt3DCore::QComponent *component)
{
    // Check component type and create the same kind
    ComponentTypes type = componentType(component);
    Qt3DCore::QComponent *duplicate = nullptr;

    switch (type) {
    case LightDirectional: {
        Qt3DRender::QDirectionalLight *source =
                qobject_cast<Qt3DRender::QDirectionalLight *>(component);
        Qt3DRender::QDirectionalLight *newComponent = new Qt3DRender::QDirectionalLight();
        // Copy properties
        newComponent->setColor(source->color());
        newComponent->setWorldDirection(source->worldDirection());
        newComponent->setIntensity(source->intensity());
        duplicate = newComponent;
        break;
    }
    case LightPoint: {
        Qt3DRender::QPointLight *source = qobject_cast<Qt3DRender::QPointLight *>(component);
        Qt3DRender::QPointLight *newComponent = new Qt3DRender::QPointLight();
        newComponent->setColor(source->color());
        newComponent->setConstantAttenuation(source->constantAttenuation());
        newComponent->setIntensity(source->intensity());
        newComponent->setLinearAttenuation(source->linearAttenuation());
        newComponent->setQuadraticAttenuation(source->quadraticAttenuation());
        duplicate = newComponent;
        break;
    }
    case LightSpot: {
        Qt3DRender::QSpotLight *source = qobject_cast<Qt3DRender::QSpotLight *>(component);
        Qt3DRender::QSpotLight *newComponent = new Qt3DRender::QSpotLight();
        newComponent->setColor(source->color());
        newComponent->setConstantAttenuation(source->constantAttenuation());
        newComponent->setCutOffAngle(source->cutOffAngle());
        newComponent->setLocalDirection(source->localDirection());
        newComponent->setIntensity(source->intensity());
        newComponent->setLinearAttenuation(source->linearAttenuation());
        newComponent->setQuadraticAttenuation(source->quadraticAttenuation());
        duplicate = newComponent;
        break;
    }
    case LightBasic: {
        Qt3DRender::QLight *source = qobject_cast<Qt3DRender::QLight *>(component);
        Qt3DRender::QLight *newComponent = new Qt3DRender::QLight();
        newComponent->setColor(source->color());
        newComponent->setIntensity(source->intensity());
        duplicate = newComponent;
        break;
    }
    case MaterialDiffuseMap: {
        Qt3DRender::QDiffuseMapMaterial *source =
                qobject_cast<Qt3DRender::QDiffuseMapMaterial *>(component);
        Qt3DRender::QDiffuseMapMaterial *newComponent = new Qt3DRender::QDiffuseMapMaterial();
        newComponent->setAmbient(source->ambient());
        Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
        diffuseTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                           source->diffuse()->textureImages().at(0))->source());
        newComponent->diffuse()->addTextureImage(diffuseTextureImage);
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        newComponent->setTextureScale(source->textureScale());
        duplicate = newComponent;
        break;
    }
    case MaterialDiffuseSpecularMap: {
        Qt3DRender::QDiffuseSpecularMapMaterial *source =
                qobject_cast<Qt3DRender::QDiffuseSpecularMapMaterial *>(component);
        Qt3DRender::QDiffuseSpecularMapMaterial *newComponent =
                new Qt3DRender::QDiffuseSpecularMapMaterial();
        newComponent->setAmbient(source->ambient());
        Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
        diffuseTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                           source->diffuse()->textureImages().at(0))->source());
        newComponent->diffuse()->addTextureImage(diffuseTextureImage);
        newComponent->setShininess(source->shininess());
        Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
        specularTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                            source->specular()->textureImages().at(0))->source());
        newComponent->specular()->addTextureImage(specularTextureImage);
        newComponent->setTextureScale(source->textureScale());
        duplicate = newComponent;
        break;
    }
    case MaterialGooch: {
        Qt3DRender::QGoochMaterial *source = qobject_cast<Qt3DRender::QGoochMaterial *>(component);
        Qt3DRender::QGoochMaterial *newComponent = new Qt3DRender::QGoochMaterial();
        newComponent->setAlpha(source->alpha());
        newComponent->setBeta(source->beta());
        newComponent->setCool(source->cool());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        newComponent->setWarm(source->warm());
        duplicate = newComponent;
        break;
    }
    case MaterialNormalDiffuseMapAlpha: {
        Qt3DRender::QNormalDiffuseMapAlphaMaterial *source =
                qobject_cast<Qt3DRender::QNormalDiffuseMapAlphaMaterial *>(component);
        Qt3DRender::QNormalDiffuseMapAlphaMaterial *newComponent =
                new Qt3DRender::QNormalDiffuseMapAlphaMaterial();
        newComponent->setAmbient(source->ambient());
        Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
        diffuseTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                           source->diffuse()->textureImages().at(0))->source());
        newComponent->diffuse()->addTextureImage(diffuseTextureImage);
        Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
        normalTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                          source->normal()->textureImages().at(0))->source());
        newComponent->normal()->addTextureImage(normalTextureImage);
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        newComponent->setTextureScale(source->textureScale());
        duplicate = newComponent;
        break;
    }
    case MaterialNormalDiffuseMap: {
        Qt3DRender::QNormalDiffuseMapMaterial *source =
                qobject_cast<Qt3DRender::QNormalDiffuseMapMaterial *>(component);
        Qt3DRender::QNormalDiffuseMapMaterial *newComponent =
                new Qt3DRender::QNormalDiffuseMapMaterial();
        newComponent->setAmbient(source->ambient());
        Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
        diffuseTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                           source->diffuse()->textureImages().at(0))->source());
        newComponent->diffuse()->addTextureImage(diffuseTextureImage);
        Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
        normalTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                          source->normal()->textureImages().at(0))->source());
        newComponent->normal()->addTextureImage(normalTextureImage);
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        newComponent->setTextureScale(source->textureScale());
        duplicate = newComponent;
        break;
    }
    case MaterialNormalDiffuseSpecularMap: {
        Qt3DRender::QNormalDiffuseSpecularMapMaterial *source =
                qobject_cast<Qt3DRender::QNormalDiffuseSpecularMapMaterial *>(component);
        Qt3DRender::QNormalDiffuseSpecularMapMaterial *newComponent =
                new Qt3DRender::QNormalDiffuseSpecularMapMaterial();
        newComponent->setAmbient(source->ambient());
        Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
        diffuseTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                           source->diffuse()->textureImages().at(0))->source());
        newComponent->diffuse()->addTextureImage(diffuseTextureImage);
        Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
        normalTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                          source->normal()->textureImages().at(0))->source());
        newComponent->normal()->addTextureImage(normalTextureImage);
        newComponent->setShininess(source->shininess());
        Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
        specularTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                            source->specular()->textureImages().at(0))->source());
        newComponent->specular()->addTextureImage(specularTextureImage);
        newComponent->setTextureScale(source->textureScale());
        duplicate = newComponent;
        break;
    }
    case MaterialPerVertexColor: {
        // MaterialPerVertexColor has no properties
        Qt3DRender::QPerVertexColorMaterial *newComponent =
                new Qt3DRender::QPerVertexColorMaterial();
        duplicate = newComponent;
        break;
    }
    case MaterialPhongAlpha: {
        Qt3DRender::QPhongAlphaMaterial *source =
                qobject_cast<Qt3DRender::QPhongAlphaMaterial *>(component);
        Qt3DRender::QPhongAlphaMaterial *newComponent = new Qt3DRender::QPhongAlphaMaterial();
        newComponent->setAlpha(source->alpha());
        newComponent->setAmbient(source->ambient());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        duplicate = newComponent;
        break;
    }
    case MaterialPhong: {
        Qt3DRender::QPhongMaterial *source = qobject_cast<Qt3DRender::QPhongMaterial *>(component);
        Qt3DRender::QPhongMaterial *newComponent = new Qt3DRender::QPhongMaterial();
        newComponent->setAmbient(source->ambient());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        duplicate = newComponent;
        break;
    }
    case MeshCuboid: {
        Qt3DRender::QCuboidMesh *source = qobject_cast<Qt3DRender::QCuboidMesh *>(component);
        Qt3DRender::QCuboidMesh *newComponent = new Qt3DRender::QCuboidMesh();
        newComponent->setXExtent(source->xExtent());
        newComponent->setYExtent(source->yExtent());
        newComponent->setZExtent(source->zExtent());
        newComponent->setXYMeshResolution(source->xyMeshResolution());
        newComponent->setXZMeshResolution(source->xzMeshResolution());
        newComponent->setYZMeshResolution(source->yzMeshResolution());
        duplicate = newComponent;
        break;
    }
    case MeshCustom: {
        Qt3DRender::QMesh *source = qobject_cast<Qt3DRender::QMesh *>(component);
        Qt3DRender::QMesh *newComponent = new Qt3DRender::QMesh();
        newComponent->setSource(source->source());
        duplicate = newComponent;
        break;
    }
    case MeshCylinder: {
        Qt3DRender::QCylinderMesh *source = qobject_cast<Qt3DRender::QCylinderMesh *>(component);
        Qt3DRender::QCylinderMesh *newComponent = new Qt3DRender::QCylinderMesh();
        newComponent->setLength(source->length());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        duplicate = newComponent;
        break;
    }
    case MeshPlane: {
        Qt3DRender::QPlaneMesh *source = qobject_cast<Qt3DRender::QPlaneMesh *>(component);
        Qt3DRender::QPlaneMesh *newComponent = new Qt3DRender::QPlaneMesh();
        newComponent->setHeight(source->height());
        newComponent->setMeshResolution(source->meshResolution());
        newComponent->setWidth(source->width());
        duplicate = newComponent;
        break;
    }
    case MeshSphere: {
        Qt3DRender::QSphereMesh *source = qobject_cast<Qt3DRender::QSphereMesh *>(component);
        Qt3DRender::QSphereMesh *newComponent = new Qt3DRender::QSphereMesh();
        newComponent->setGenerateTangents(source->generateTangents());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        duplicate = newComponent;
        break;
    }
    case MeshTorus: {
        Qt3DRender::QTorusMesh *source = qobject_cast<Qt3DRender::QTorusMesh *>(component);
        Qt3DRender::QTorusMesh *newComponent = new Qt3DRender::QTorusMesh();
        newComponent->setMinorRadius(source->minorRadius());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        duplicate = newComponent;
        break;
    }
    case Transform: {
        Qt3DCore::QTransform *source = qobject_cast<Qt3DCore::QTransform *>(component);
        Qt3DCore::QTransform *newComponent = new Qt3DCore::QTransform();
        newComponent->setMatrix(source->matrix());
        duplicate = newComponent;
        break;
    }
    case ObjectPicker: {
        QDummyObjectPicker *source = qobject_cast<QDummyObjectPicker *>(component);
        QDummyObjectPicker *newComponent = new QDummyObjectPicker();
        newComponent->setHoverEnabled(source->hoverEnabled());
        duplicate = newComponent;
        break;
    }
    case SceneLoader: {
        Qt3DRender::QSceneLoader *source = qobject_cast<Qt3DRender::QSceneLoader *>(component);
        Qt3DRender::QSceneLoader *newComponent = new Qt3DRender::QSceneLoader();
        newComponent->setSource(source->source());
        duplicate = newComponent;
        break;
    }
    case Unknown:
        qWarning() << "Unsupported component:" << component;
        break;
    }

    // Copy property locks, except for transforms
    if (type != Transform)
        copyLockProperties(component, duplicate);

    return duplicate;
}

QString EditorUtils::nameDuplicate(Qt3DCore::QEntity *duplicate, Qt3DCore::QEntity *original,
                                   EditorSceneItemModel *sceneModel)
{
    if (original->objectName().isEmpty())
        return QString();

    QString newName = sceneModel->generateValidName(original->objectName() + QObject::tr("_Copy"),
                                                    original);
    duplicate->setObjectName(newName);

    // Rename possible children
    Q_FOREACH (QObject *child, duplicate->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            nameDuplicate(childEntity, childEntity, sceneModel);
    }

    return newName;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createWireframeBoxMesh(float extent)
{
    // Creates a box 'mesh' that is is made up of 12 GL_LINES between 8 vertices
    Qt3DRender::QGeometryRenderer *boxMesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *boxGeometry = new Qt3DRender::QGeometry(boxMesh);
    Qt3DRender::QBuffer *boxDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer,
                                                                 boxGeometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer,
                                                                   boxGeometry);
    QByteArray vertexBufferData;
    QByteArray indexBufferData;

    vertexBufferData.resize(8 * 3 * sizeof(float));
    indexBufferData.resize(12 * 2 * sizeof(ushort));

    float dimension = extent / 2.0f;

    float *vPtr = reinterpret_cast<float *>(vertexBufferData.data());
    vPtr[0]  = -dimension; vPtr[1]  = -dimension; vPtr[2]  = -dimension;
    vPtr[3]  = dimension;  vPtr[4]  = -dimension; vPtr[5]  = -dimension;
    vPtr[6]  = dimension;  vPtr[7]  = -dimension; vPtr[8]  = dimension;
    vPtr[9]  = -dimension; vPtr[10] = -dimension; vPtr[11] = dimension;
    vPtr[12] = -dimension; vPtr[13] = dimension;  vPtr[14] = -dimension;
    vPtr[15] = dimension;  vPtr[16] = dimension;  vPtr[17] = -dimension;
    vPtr[18] = dimension;  vPtr[19] = dimension;  vPtr[20] = dimension;
    vPtr[21] = -dimension; vPtr[22] = dimension;  vPtr[23] = dimension;

    ushort *iPtr = reinterpret_cast<ushort *>(indexBufferData.data());
    iPtr[0]  = 0; iPtr[1]  = 1;
    iPtr[2]  = 1; iPtr[3]  = 2;
    iPtr[4]  = 2; iPtr[5]  = 3;
    iPtr[6]  = 3; iPtr[7]  = 0;
    iPtr[8]  = 0; iPtr[9]  = 4;
    iPtr[10] = 1; iPtr[11] = 5;
    iPtr[12] = 2; iPtr[13] = 6;
    iPtr[14] = 3; iPtr[15] = 7;
    iPtr[16] = 4; iPtr[17] = 5;
    iPtr[18] = 5; iPtr[19] = 6;
    iPtr[20] = 6; iPtr[21] = 7;
    iPtr[22] = 7; iPtr[23] = 4;

    boxDataBuffer->setData(vertexBufferData);
    indexDataBuffer->setData(indexBufferData);

    addPositionAttributeToGeometry(boxGeometry, boxDataBuffer, 8);
    addIndexAttributeToGeometry(boxGeometry, indexDataBuffer, 24);

    boxMesh->setInstanceCount(1);
    boxMesh->setIndexOffset(0);
    boxMesh->setFirstInstance(0);
    boxMesh->setVertexCount(24);
    boxMesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    boxMesh->setGeometry(boxGeometry);

    return boxMesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createWireframePlaneMesh(int lineCount)
{
    Qt3DRender::QGeometryRenderer *planeMesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *planeGeometry = new Qt3DRender::QGeometry(planeMesh);
    Qt3DRender::QBuffer *planeDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer,
                                                                   planeGeometry);
    QByteArray vertexBufferData;
    QVector<QVector3D> vertices;

    // lineCount lines on x and z directions, each with two vector3Ds
    vertices.resize(lineCount * 2 * 2);
    vertexBufferData.resize(vertices.size() * 3 * sizeof(float));

    for (int i = 0; i < lineCount; i++) {
        int index = i * 2;
        vertices[index] = QVector3D(-1.0f + (float(i) * (2.0 / (lineCount - 1))), -1.0f, 0.0f);
        vertices[index + 1] = QVector3D(-1.0f + (float(i) * (2.0 / (lineCount - 1))), 1.0f, 0.0f);
        vertices[index + lineCount * 2] = QVector3D(-1.0f, -1.0f + (float(i) * (2.0 / (lineCount - 1))), 0.0f);
        vertices[index + lineCount * 2 + 1] = QVector3D(1.0f, -1.0f + (float(i) * (2.0 / (lineCount - 1))), 0.0f);
    }

    float *rawVertexArray = reinterpret_cast<float *>(vertexBufferData.data());
    int idx = 0;
    Q_FOREACH (const QVector3D &v, vertices) {
        rawVertexArray[idx++] = v.x();
        rawVertexArray[idx++] = v.y();
        rawVertexArray[idx++] = v.z();
    }

    planeDataBuffer->setData(vertexBufferData);

    addPositionAttributeToGeometry(planeGeometry, planeDataBuffer, lineCount * 4);

    planeMesh->setInstanceCount(1);
    planeMesh->setIndexOffset(0);
    planeMesh->setFirstInstance(0);
    planeMesh->setVertexCount(lineCount * 4);
    planeMesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    planeMesh->setGeometry(planeGeometry);

    return planeMesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createDefaultCustomMesh()
{
    Qt3DRender::QMesh *customMesh = new Qt3DRender::QMesh();
    customMesh->setSource(QUrl(QStringLiteral("qrc:/meshes/defaultmesh.obj")));
    return customMesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createRotateHandleMesh(float size)
{
    // TODO: proper mesh
    Qt3DRender::QSphereMesh *mesh = new Qt3DRender::QSphereMesh;
    mesh->setRadius(size / 2.0f);
    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createScaleHandleMesh(float size)
{
    // TODO: proper mesh
    Qt3DRender::QCuboidMesh *mesh = new Qt3DRender::QCuboidMesh;
    mesh->setXExtent(size);
    mesh->setYExtent(size);
    mesh->setZExtent(size);
    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createTranslateHandleMesh(float size)
{
    // TODO: proper mesh
    return createWireframeBoxMesh(size);
}

Qt3DRender::QGeometryRenderer *EditorUtils::createVisibleCameraMesh()
{
    // Creates a camera 'mesh' that is is made up of GL_LINES
    // TODO: Perhaps create a nice custom mesh for camera instead?
    Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(mesh);
    Qt3DRender::QBuffer *dataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer,
                                                              geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer,
                                                                   geometry);
    QByteArray vertexBufferData;
    QByteArray indexBufferData;

    vertexBufferData.resize(8 * 3 * sizeof(float));
    indexBufferData.resize(10 * 2 * sizeof(ushort));

    float *vPtr = reinterpret_cast<float *>(vertexBufferData.data());
    vPtr[0]  = -0.5f; vPtr[1]  = 0.5f;  vPtr[2]  = -0.5f;
    vPtr[3]  = -0.5f; vPtr[4]  = -0.5f; vPtr[5]  = -0.5f;
    vPtr[6]  = 0.5f;  vPtr[7]  = -0.5f; vPtr[8]  = -0.5f;
    vPtr[9]  = 0.5f;  vPtr[10] = 0.5f;  vPtr[11] = -0.5f;
    vPtr[12] = 0.0f;  vPtr[13] = 0.0f;  vPtr[14] = 0.5f;
    vPtr[15] = -0.1f; vPtr[16] = 0.5f;  vPtr[17] = -0.5f;
    vPtr[18] = 0.1f;  vPtr[19] = 0.5f;  vPtr[20] = -0.5f;
    vPtr[21] = 0.0f;  vPtr[22] = 0.7f;  vPtr[23] = -0.5f;

    ushort *iPtr = reinterpret_cast<ushort *>(indexBufferData.data());
    iPtr[0]  = 0; iPtr[1]  = 1;
    iPtr[2]  = 1; iPtr[3]  = 2;
    iPtr[4]  = 2; iPtr[5]  = 3;
    iPtr[6]  = 3; iPtr[7]  = 0;
    iPtr[8]  = 0; iPtr[9]  = 4;
    iPtr[10] = 1; iPtr[11] = 4;
    iPtr[12] = 2; iPtr[13] = 4;
    iPtr[14] = 3; iPtr[15] = 4;
    iPtr[16] = 5; iPtr[17] = 7;
    iPtr[18] = 6; iPtr[19] = 7;

    dataBuffer->setData(vertexBufferData);
    indexDataBuffer->setData(indexBufferData);

    addPositionAttributeToGeometry(geometry, dataBuffer, 8);
    addIndexAttributeToGeometry(geometry, indexDataBuffer, 20);

    mesh->setInstanceCount(1);
    mesh->setIndexOffset(0);
    mesh->setFirstInstance(0);
    mesh->setVertexCount(20);
    mesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    mesh->setGeometry(geometry);

    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createCameraViewVectorMesh()
{
    // Creates a camera target indicator 'mesh' that is is made up of GL_LINES
    Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(mesh);
    Qt3DRender::QBuffer *dataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer,
                                                              geometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer,
                                                                   geometry);
    QByteArray vertexBufferData;
    QByteArray indexBufferData;

    vertexBufferData.resize(2 * 3 * sizeof(float));
    indexBufferData.resize(1 * 2 * sizeof(ushort));

    float *vPtr = reinterpret_cast<float *>(vertexBufferData.data());
    vPtr[0]  = 0.0f;  vPtr[1]  = 0.0f;  vPtr[2]  = 0.0f;
    vPtr[3]  = 0.0f;  vPtr[4]  = 0.0f;  vPtr[5]  = -1.0f;

    ushort *iPtr = reinterpret_cast<ushort *>(indexBufferData.data());
    iPtr[0]  = 0; iPtr[1]  = 1;

    dataBuffer->setData(vertexBufferData);
    indexDataBuffer->setData(indexBufferData);

    addPositionAttributeToGeometry(geometry, dataBuffer, 2);
    addIndexAttributeToGeometry(geometry, indexDataBuffer, 2);

    mesh->setInstanceCount(1);
    mesh->setIndexOffset(0);
    mesh->setFirstInstance(0);
    mesh->setVertexCount(2);
    mesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    mesh->setGeometry(geometry);

    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createCameraViewCenterMesh(float size)
{
    // TODO: proper mesh
    Qt3DRender::QSphereMesh *mesh = new Qt3DRender::QSphereMesh;
    mesh->setRadius(size / 2.0f);
    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createLightMesh(EditorUtils::ComponentTypes type)
{
    Qt3DRender::QGeometryRenderer *mesh = nullptr;

    switch (type) {
    case LightDirectional: {
        Qt3DRender::QMesh *directionalMesh = new Qt3DRender::QMesh();
        directionalMesh->setSource(QUrl(QStringLiteral("qrc:/meshes/directionallight.obj")));
        mesh = directionalMesh;
        break;
    }
    case LightPoint: {
        Qt3DRender::QSphereMesh *pointMesh = new Qt3DRender::QSphereMesh();
        pointMesh->setRadius(0.2f);
        pointMesh->setRings(10);
        pointMesh->setSlices(10);
        mesh = pointMesh;
        break;
    }
    case LightSpot: {
        Qt3DRender::QMesh *spotMesh = new Qt3DRender::QMesh();
        spotMesh->setSource(QUrl(QStringLiteral("qrc:/meshes/spotlight.obj")));
        mesh = spotMesh;
        break;
    }
    default: { // LightBasic
        Qt3DRender::QSphereMesh *basicMesh = new Qt3DRender::QSphereMesh();
        basicMesh->setRadius(0.5f);
        basicMesh->setRings(10);
        basicMesh->setSlices(10);
        mesh = basicMesh;
        break;
    }
    }

    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createMeshForInsertableType(InsertableEntities type)
{
    Qt3DRender::QGeometryRenderer *mesh = nullptr;
    switch (type) {
    case CuboidEntity: {
        mesh = new Qt3DRender::QCuboidMesh();
        break;
    }
    case CylinderEntity: {
        mesh = new Qt3DRender::QCylinderMesh();
        break;
    }
    case PlaneEntity: {
        mesh = new Qt3DRender::QPlaneMesh();
        break;
    }
    case SphereEntity: {
        mesh = new Qt3DRender::QSphereMesh();
        break;
    }
    case TorusEntity: {
        mesh = new Qt3DRender::QTorusMesh();
        break;
    }
    case CustomEntity: {
        mesh = createDefaultCustomMesh();
        break;
    }
    case GroupEntity: {
        // Group entity mesh is only used for drag-insert placeholder
        mesh = new Qt3DRender::QCuboidMesh();
        break;
    }
    default:
        break;
    }

    return mesh;
}

void EditorUtils::addPositionAttributeToGeometry(Qt3DRender::QGeometry *geometry,
                                                 Qt3DRender::QBuffer *buffer, int count)
{
    Qt3DRender::QAttribute *posAttribute = new Qt3DRender::QAttribute();
    posAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    posAttribute->setBuffer(buffer);
    posAttribute->setDataType(Qt3DRender::QAttribute::Float);
    posAttribute->setDataSize(3);
    posAttribute->setByteOffset(0);
    posAttribute->setByteStride(0);
    posAttribute->setCount(count);
    posAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    geometry->addAttribute(posAttribute);
}

void EditorUtils::addIndexAttributeToGeometry(Qt3DRender::QGeometry *geometry,
                                              Qt3DRender::QBuffer *buffer, int count)
{
    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute();
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(buffer);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedShort);
    indexAttribute->setDataSize(1);
    indexAttribute->setByteOffset(0);
    indexAttribute->setByteStride(0);
    indexAttribute->setCount(count);

    geometry->addAttribute(indexAttribute);
}

void EditorUtils::updateCameraFrustumMesh(Qt3DRender::QGeometryRenderer *mesh,
                                          Qt3DRender::QCamera *camera)
{
    QMatrix4x4 projectionMatrix = camera->projectionMatrix().inverted();

    Qt3DRender::QGeometry *geometry = mesh->geometry();

    Qt3DRender::QBuffer *dataBuffer = nullptr;
    Q_FOREACH (Qt3DRender::QAttribute *attribute, geometry->attributes()) {
        if (attribute->name() == Qt3DRender::QAttribute::defaultPositionAttributeName()) {
            dataBuffer = attribute->buffer();
            break;
        }
    }

    if (dataBuffer) {
        QByteArray newData;
        newData.resize(dataBuffer->data().size());
        float *vPtr = reinterpret_cast<float *>(newData.data());

        vPtr[0]  = -1.0f; vPtr[1]  = -1.0f; vPtr[2]  = -1.0f;
        vPtr[3]  = 1.0f;  vPtr[4]  = -1.0f; vPtr[5]  = -1.0f;
        vPtr[6]  = 1.0f;  vPtr[7]  = 1.0f;  vPtr[8]  = -1.0f;
        vPtr[9]  = -1.0f; vPtr[10] = 1.0f;  vPtr[11] = -1.0f;
        vPtr[12] = -1.0f; vPtr[13] = -1.0f; vPtr[14] = 1.0f;
        vPtr[15] = 1.0f;  vPtr[16] = -1.0f; vPtr[17] = 1.0f;
        vPtr[18] = 1.0f;  vPtr[19] = 1.0f;  vPtr[20] = 1.0f;
        vPtr[21] = -1.0f; vPtr[22] = 1.0f;  vPtr[23] = 1.0f;

        for (int i = 0; i < 24; i += 3) {
            QVector3D vertex(vPtr[i], vPtr[i + 1], vPtr[i + 2]);
            vertex = projectionMatrix * vertex;
            vPtr[i] = vertex.x();
            vPtr[i + 1] = vertex.y();
            vPtr[i + 2] = vertex.z();
        }
        dataBuffer->setData(newData);
    }
}

Qt3DCore::QTransform *EditorUtils::entityTransform(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DCore::QTransform *transform = qobject_cast<Qt3DCore::QTransform *>(components.value(i));
        if (transform)
            return transform;
    }

    return nullptr;
}

Qt3DRender::QLight *EditorUtils::entityLight(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DRender::QLight *light = qobject_cast<Qt3DRender::QLight *>(components.value(i));
        if (light)
            return light;
    }

    return nullptr;
}

Qt3DRender::QObjectPicker *EditorUtils::entityPicker(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DRender::QObjectPicker *picker
                = qobject_cast<Qt3DRender::QObjectPicker *>(components.value(i));
        if (picker)
            return picker;
    }

    return nullptr;
}

Qt3DRender::QSceneLoader *EditorUtils::entitySceneLoader(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DRender::QSceneLoader *loader
                = qobject_cast<Qt3DRender::QSceneLoader *>(components.value(i));
        if (loader)
            return loader;
    }

    return nullptr;
}

Qt3DRender::QGeometryRenderer *EditorUtils::entityMesh(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DRender::QGeometryRenderer *mesh
                = qobject_cast<Qt3DRender::QGeometryRenderer *>(components.value(i));
        if (mesh)
            return mesh;
    }

    return nullptr;
}

// Returns the intersection point of a plane and a ray.
// Parameter t returns the distance in ray lengths. If t is negative, intersection
// is behind rayOrigin.
// If there is no intersection, i.e. plane and the ray are paraller, t is set to -1 and
// rayOrigin is returned.
QVector3D EditorUtils::findIntersection(const QVector3D &rayOrigin, const QVector3D &ray,
                                        float planeOffset, const QVector3D &planeNormal,
                                        float &t)
{
    float divisor = QVector3D::dotProduct(ray, planeNormal);
    if (qFuzzyCompare(1.0f, 1.0f + divisor)) {
        t = -1.0f;
        return rayOrigin;
    }

    t = -(QVector3D::dotProduct(rayOrigin, planeNormal) - planeOffset) / divisor;

    return rayOrigin + ray * t;

}

// Returns a direction vector from camera origin to viewport pixel in world coordinates
QVector3D EditorUtils::unprojectRay(const QMatrix4x4 &viewMatrix,
                                    const QMatrix4x4 &projectionMatrix,
                                    int viewPortWidth, int viewPortHeight,
                                    const QPoint &pos)
{
    float x = ((2.0f * pos.x()) / viewPortWidth) - 1.0f;
    float y = 1.0f - ((2.0f * pos.y()) / viewPortHeight);

    // Figure out the ray to the screen position
    QVector4D ray = projectionMatrix.inverted() * QVector4D(x, y, -1.0f, 1.0f);
    ray.setZ(-1.0f);
    ray.setW(0.0f);
    ray = viewMatrix.inverted() * ray;
    return ray.toVector3D().normalized();
}

QVector3D EditorUtils::absVector3D(const QVector3D &vector)
{
    return QVector3D(qAbs(vector.x()),
                     qAbs(vector.y()),
                     qAbs(vector.z()));
}

QVector3D EditorUtils::maxVector3D(const QVector3D &vector, float minValue)
{
    return QVector3D(qMax(minValue, vector.x()),
                     qMax(minValue, vector.y()),
                     qMax(minValue, vector.z()));
}

EditorUtils::ComponentTypes EditorUtils::componentType(Qt3DCore::QComponent *component)
{
    ComponentTypes componentType = Unknown;

    if (qobject_cast<Qt3DRender::QLight *>(component)) {
        if (qobject_cast<Qt3DRender::QDirectionalLight *>(component))
            componentType = LightDirectional;
        else if (qobject_cast<Qt3DRender::QPointLight *>(component))
            componentType = LightPoint;
        else if (qobject_cast<Qt3DRender::QSpotLight *>(component))
            componentType = LightSpot;
        else
            componentType = LightBasic;
    } else if (qobject_cast<Qt3DRender::QMaterial *>(component)) {
        if (qobject_cast<Qt3DRender::QDiffuseMapMaterial *>(component))
            componentType = MaterialDiffuseMap;
        else if (qobject_cast<Qt3DRender::QDiffuseSpecularMapMaterial *>(component))
            componentType = MaterialDiffuseSpecularMap;
        else if (qobject_cast<Qt3DRender::QGoochMaterial *>(component))
            componentType = MaterialGooch;
        // Inherits QNormalDiffuseMapMaterial, so must be tested first
        else if (qobject_cast<Qt3DRender::QNormalDiffuseMapAlphaMaterial *>(component))
            componentType = MaterialNormalDiffuseMapAlpha;
        else if (qobject_cast<Qt3DRender::QNormalDiffuseMapMaterial *>(component))
            componentType = MaterialNormalDiffuseMap;
        else if (qobject_cast<Qt3DRender::QNormalDiffuseSpecularMapMaterial *>(component))
            componentType = MaterialNormalDiffuseSpecularMap;
        else if (qobject_cast<Qt3DRender::QPerVertexColorMaterial *>(component))
            componentType = MaterialPerVertexColor;
        else if (qobject_cast<Qt3DRender::QPhongAlphaMaterial *>(component))
            componentType = MaterialPhongAlpha;
        else if (qobject_cast<Qt3DRender::QPhongMaterial *>(component))
            componentType = MaterialPhong;
    } else if (qobject_cast<Qt3DRender::QGeometryRenderer *>(component)) {
        if (qobject_cast<Qt3DRender::QMesh *>(component))
            componentType = MeshCustom;
        else if (qobject_cast<Qt3DRender::QCuboidMesh *>(component))
            componentType = MeshCuboid;
        else if (qobject_cast<Qt3DRender::QCylinderMesh *>(component))
            componentType = MeshCylinder;
        else if (qobject_cast<Qt3DRender::QPlaneMesh *>(component))
            componentType = MeshPlane;
        else if (qobject_cast<Qt3DRender::QSphereMesh *>(component))
            componentType = MeshSphere;
        else if (qobject_cast<Qt3DRender::QTorusMesh *>(component))
            componentType = MeshTorus;
    } else if (qobject_cast<Qt3DCore::QTransform *>(component)) {
        componentType = Transform;
    } else if (qobject_cast<QDummyObjectPicker *>(component)) {
        componentType = ObjectPicker;
    } else if (qobject_cast<Qt3DRender::QSceneLoader *>(component)) {
        componentType = SceneLoader;
    }

    return componentType;
}

// Rotates vector around rotationAxis. The rotationAxis must be normalized.
QVector3D EditorUtils::rotateVector(const QVector3D &vector,
                                    const QVector3D &rotationAxis,
                                    qreal radians)
{
    const qreal cosAngle = qCos(radians);

    // Use Rodrigues' rotation formula to find rotated vector
    return (vector * cosAngle
            + (QVector3D::crossProduct(rotationAxis, vector) * qSin(radians))
            + rotationAxis * QVector3D::dotProduct(rotationAxis, vector) * (1.0 - cosAngle));
}

QVector3D EditorUtils::projectVectorOnPlane(const QVector3D &vector, const QVector3D &planeNormal)
{
    float distance = vector.distanceToPlane(QVector3D(), planeNormal);
    return vector - distance * planeNormal;
}

QMatrix4x4 EditorUtils::totalAncestralTransform(Qt3DCore::QEntity *entity)
{
    QMatrix4x4 totalTransform;
    QList<Qt3DCore::QTransform *> transforms = ancestralTransforms(entity);

    for (int i = transforms.size() - 1; i >= 0; i--)
        totalTransform *= transforms.at(i)->matrix();

    return totalTransform;
}

QVector3D EditorUtils::totalAncestralScale(Qt3DCore::QEntity *entity)
{
    QVector3D totalScale(1.0f, 1.0f, 1.0f);
    QList<Qt3DCore::QTransform *> transforms = ancestralTransforms(entity);

    for (int i = transforms.size() - 1; i >= 0; i--)
        totalScale *= transforms.at(i)->scale3D();

    return totalScale;
}

QQuaternion EditorUtils::totalAncestralRotation(Qt3DCore::QEntity *entity)
{
    QQuaternion totalRotation;
    QList<Qt3DCore::QTransform *> transforms = ancestralTransforms(entity);

    for (int i = transforms.size() - 1; i >= 0; i--)
        totalRotation *= transforms.at(i)->rotation();

    return totalRotation;
}

QList<Qt3DCore::QTransform *> EditorUtils::ancestralTransforms(Qt3DCore::QEntity *entity,
                                                               Qt3DCore::QEntity *topAncestor)
{
    Qt3DCore::QEntity *parent = entity->parentEntity();
    QList<Qt3DCore::QTransform *> transforms;
    while (parent && parent != topAncestor) {
        Qt3DCore::QTransform *transform = entityTransform(parent);
        if (transform)
            transforms.append(transform);
        parent = parent->parentEntity();
    }
    return transforms;
}

QVector3D EditorUtils::lightDirection(const Qt3DRender::QLight *light)
{
    QVector3D direction;
    const Qt3DRender::QDirectionalLight *dirLight =
            qobject_cast<const Qt3DRender::QDirectionalLight *>(light);
    const Qt3DRender::QSpotLight *spotLight =
            qobject_cast<const Qt3DRender::QSpotLight *>(light);
    if (dirLight)
        direction = dirLight->worldDirection();
    else if (spotLight)
        direction = spotLight->localDirection();
    return direction;
}

void EditorUtils::copyLockProperties(const QObject *source, QObject *target)
{
    QList<QByteArray> customProps = source->dynamicPropertyNames();
    Q_FOREACH (const QByteArray &propName, customProps) {
        if (propName.endsWith(lockPropertySuffix8())) {
            target->setProperty(propName.constData(),
                                source->property(propName.constData()));
        }
    }
}

void EditorUtils::lockProperty(const QByteArray &lockPropertyName, QObject *obj, bool lock)
{
    QVariant propVal = obj->property(lockPropertyName);
    if (propVal.isValid() && propVal.toBool() != lock)
        obj->setProperty(lockPropertyName, QVariant::fromValue(lock));
}

QVector3D EditorUtils::cameraNormal(Qt3DRender::QCamera *camera)
{
    QVector3D planeNormal;
    if (camera) {
        planeNormal = camera->position() - camera->viewCenter();
        planeNormal.normalize();
    }
    return planeNormal;
}

bool EditorUtils::isDescendant(EditorSceneItem *ancestor, EditorSceneItem *descendantItem)
{
    bool descendant = ancestor == descendantItem;
    if (!descendant) {
        Q_FOREACH (EditorSceneItem *item, ancestor->childItems()) {
            if (isDescendant(item, descendantItem)) {
                descendant = true;
                break;
            }
        }
    }
    return descendant;
}

