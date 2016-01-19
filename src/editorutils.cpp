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
#include "qdummyobjectpicker.h"

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QAbstractTextureProvider>
#include <Qt3DRender/QTextureImage>

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

#include <Qt3DCore/QCameraLens>
#include <Qt3DRender/QFrameGraph>
#include <Qt3DRender/QLayer>
#include <Qt3DInput/QKeyboardInput>
#include <Qt3DInput/QMouseInput>
#include <Qt3DLogic/QLogicComponent>

Qt3DCore::QComponent *EditorUtils::duplicateComponent(Qt3DCore::QComponent *component,
                                                      Qt3DCore::QEntity *parent,
                                                      EditorSceneItemModel *sceneModel)
{
    // Check component type and create the same kind
    ComponentTypes type = componentType(component);

    switch (type) {
    case LightDirectional: {
        Qt3DRender::QDirectionalLight *source =
                qobject_cast<Qt3DRender::QDirectionalLight *>(component);
        Qt3DRender::QDirectionalLight *newComponent = new Qt3DRender::QDirectionalLight();
        nameDuplicate(newComponent, source, parent, sceneModel);
        // Copy properties
        newComponent->setColor(source->color());
        newComponent->setDirection(source->direction());
        newComponent->setIntensity(source->intensity());
        return newComponent;
    }
    case LightPoint: {
        Qt3DRender::QPointLight *source = qobject_cast<Qt3DRender::QPointLight *>(component);
        Qt3DRender::QPointLight *newComponent = new Qt3DRender::QPointLight();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setAttenuation(source->attenuation());
        newComponent->setColor(source->color());
        newComponent->setConstantAttenuation(source->constantAttenuation());
        newComponent->setIntensity(source->intensity());
        newComponent->setLinearAttenuation(source->linearAttenuation());
        newComponent->setQuadraticAttenuation(source->quadraticAttenuation());
        return newComponent;
    }
    case LightSpot: {
        Qt3DRender::QSpotLight *source = qobject_cast<Qt3DRender::QSpotLight *>(component);
        Qt3DRender::QSpotLight *newComponent = new Qt3DRender::QSpotLight();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setAttenuation(source->attenuation());
        newComponent->setColor(source->color());
        newComponent->setConstantAttenuation(source->constantAttenuation());
        newComponent->setCutOffAngle(source->cutOffAngle());
        newComponent->setDirection(source->direction());
        newComponent->setIntensity(source->intensity());
        newComponent->setLinearAttenuation(source->linearAttenuation());
        newComponent->setQuadraticAttenuation(source->quadraticAttenuation());
        return newComponent;
    }
    case LightBasic: {
        Qt3DRender::QLight *source = qobject_cast<Qt3DRender::QLight *>(component);
        Qt3DRender::QLight *newComponent = new Qt3DRender::QLight();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setColor(source->color());
        newComponent->setIntensity(source->intensity());
        return newComponent;
    }
    case MaterialDiffuseMap: {
        Qt3DRender::QDiffuseMapMaterial *source =
                qobject_cast<Qt3DRender::QDiffuseMapMaterial *>(component);
        Qt3DRender::QDiffuseMapMaterial *newComponent = new Qt3DRender::QDiffuseMapMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setAmbient(source->ambient());
        Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
        diffuseTextureImage->setSource(qobject_cast<Qt3DRender::QTextureImage *>(
                                           source->diffuse()->textureImages().at(0))->source());
        newComponent->diffuse()->addTextureImage(diffuseTextureImage);
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        newComponent->setTextureScale(source->textureScale());
        return newComponent;
    }
    case MaterialDiffuseSpecularMap: {
        Qt3DRender::QDiffuseSpecularMapMaterial *source =
                qobject_cast<Qt3DRender::QDiffuseSpecularMapMaterial *>(component);
        Qt3DRender::QDiffuseSpecularMapMaterial *newComponent =
                new Qt3DRender::QDiffuseSpecularMapMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
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
        return newComponent;
    }
    case MaterialGooch: {
        Qt3DRender::QGoochMaterial *source = qobject_cast<Qt3DRender::QGoochMaterial *>(component);
        Qt3DRender::QGoochMaterial *newComponent = new Qt3DRender::QGoochMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setAlpha(source->alpha());
        newComponent->setBeta(source->beta());
        newComponent->setCool(source->cool());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        newComponent->setWarm(source->warm());
        return newComponent;
    }
    case MaterialNormalDiffuseMapAlpha: {
        Qt3DRender::QNormalDiffuseMapAlphaMaterial *source =
                qobject_cast<Qt3DRender::QNormalDiffuseMapAlphaMaterial *>(component);
        Qt3DRender::QNormalDiffuseMapAlphaMaterial *newComponent =
                new Qt3DRender::QNormalDiffuseMapAlphaMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
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
        return newComponent;
    }
    case MaterialNormalDiffuseMap: {
        Qt3DRender::QNormalDiffuseMapMaterial *source =
                qobject_cast<Qt3DRender::QNormalDiffuseMapMaterial *>(component);
        Qt3DRender::QNormalDiffuseMapMaterial *newComponent =
                new Qt3DRender::QNormalDiffuseMapMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
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
        return newComponent;
    }
    case MaterialNormalDiffuseSpecularMap: {
        Qt3DRender::QNormalDiffuseSpecularMapMaterial *source =
                qobject_cast<Qt3DRender::QNormalDiffuseSpecularMapMaterial *>(component);
        Qt3DRender::QNormalDiffuseSpecularMapMaterial *newComponent =
                new Qt3DRender::QNormalDiffuseSpecularMapMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
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
        return newComponent;
    }
    case MaterialPerVertexColor: {
        Qt3DRender::QPerVertexColorMaterial *source =
                qobject_cast<Qt3DRender::QPerVertexColorMaterial *>(component);
        Qt3DRender::QPerVertexColorMaterial *newComponent =
                new Qt3DRender::QPerVertexColorMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
        return newComponent;
    }
    case MaterialPhongAlpha: {
        Qt3DRender::QPhongAlphaMaterial *source =
                qobject_cast<Qt3DRender::QPhongAlphaMaterial *>(component);
        Qt3DRender::QPhongAlphaMaterial *newComponent = new Qt3DRender::QPhongAlphaMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setAlpha(source->alpha());
        newComponent->setAmbient(source->ambient());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        return newComponent;
    }
    case MaterialPhong: {
        Qt3DRender::QPhongMaterial *source = qobject_cast<Qt3DRender::QPhongMaterial *>(component);
        Qt3DRender::QPhongMaterial *newComponent = new Qt3DRender::QPhongMaterial();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setAmbient(source->ambient());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        return newComponent;
    }
    case MeshCuboid: {
        Qt3DRender::QCuboidMesh *source = qobject_cast<Qt3DRender::QCuboidMesh *>(component);
        Qt3DRender::QCuboidMesh *newComponent = new Qt3DRender::QCuboidMesh();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setXExtent(source->xExtent());
        newComponent->setYExtent(source->yExtent());
        newComponent->setZExtent(source->zExtent());
        newComponent->setXYMeshResolution(source->xyMeshResolution());
        newComponent->setXZMeshResolution(source->xzMeshResolution());
        newComponent->setYZMeshResolution(source->yzMeshResolution());
        return newComponent;
    }
    case MeshCustom: {
        Qt3DRender::QMesh *source = qobject_cast<Qt3DRender::QMesh *>(component);
        Qt3DRender::QMesh *newComponent = new Qt3DRender::QMesh();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setSource(source->source());
        return newComponent;
    }
    case MeshCylinder: {
        Qt3DRender::QCylinderMesh *source = qobject_cast<Qt3DRender::QCylinderMesh *>(component);
        Qt3DRender::QCylinderMesh *newComponent = new Qt3DRender::QCylinderMesh();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setLength(source->length());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        return newComponent;
    }
    case MeshPlane: {
        Qt3DRender::QPlaneMesh *source = qobject_cast<Qt3DRender::QPlaneMesh *>(component);
        Qt3DRender::QPlaneMesh *newComponent = new Qt3DRender::QPlaneMesh();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setHeight(source->height());
        newComponent->setMeshResolution(source->meshResolution());
        newComponent->setWidth(source->width());
        return newComponent;
    }
    case MeshSphere: {
        Qt3DRender::QSphereMesh *source = qobject_cast<Qt3DRender::QSphereMesh *>(component);
        Qt3DRender::QSphereMesh *newComponent = new Qt3DRender::QSphereMesh();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setGenerateTangents(source->generateTangents());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        return newComponent;
    }
    case MeshTorus: {
        Qt3DRender::QTorusMesh *source = qobject_cast<Qt3DRender::QTorusMesh *>(component);
        Qt3DRender::QTorusMesh *newComponent = new Qt3DRender::QTorusMesh();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setMinorRadius(source->minorRadius());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        return newComponent;
    }
    case Transform: {
        Qt3DCore::QTransform *source = qobject_cast<Qt3DCore::QTransform *>(component);
        Qt3DCore::QTransform *newComponent = new Qt3DCore::QTransform();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setMatrix(source->matrix());
        return newComponent;
    }
        /*
    case CameraLens: {
        Qt3DCore::QCameraLens *source = qobject_cast<Qt3DCore::QCameraLens *>(component);
        Qt3DCore::QCameraLens *newComponent = new Qt3DCore::QCameraLens();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setAspectRatio(source->aspectRatio());
        newComponent->setBottom(source->bottom());
        newComponent->setFarPlane(source->farPlane());
        newComponent->setFieldOfView(source->fieldOfView());
        newComponent->setLeft(source->left());
        newComponent->setNearPlane(source->nearPlane());
        newComponent->setProjectionType(source->projectionType());
        newComponent->setRight(source->right());
        newComponent->setTop(source->top());
        return newComponent;
    }
    case FrameGraph: {
        Qt3DRender::QFrameGraph *source = qobject_cast<Qt3DRender::QFrameGraph *>(component);
        Qt3DRender::QFrameGraph *newComponent = new Qt3DRender::QFrameGraph();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setActiveFrameGraph(source->activeFrameGraph());
        return newComponent;
    }
    case KeyboardInput: {
        //Qt3DInput::QKeyboardInput *source = qobject_cast<Qt3DInput::QKeyboardInput *>(component);
        Qt3DInput::QKeyboardInput *newComponent = new Qt3DInput::QKeyboardInput();
        nameDuplicate(newComponent, source, parent, sceneModel);
        // No properties to copy?
        return newComponent;
    }
    case Layer: {
        Qt3DRender::QLayer *source = qobject_cast<Qt3DRender::QLayer *>(component);
        Qt3DRender::QLayer *newComponent = new Qt3DRender::QLayer();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setNames(source->names());
        return newComponent;
    }
    case Logic: {
        Qt3DLogic::QLogicComponent *newComponent = new Qt3DLogic::QLogicComponent();
        nameDuplicate(newComponent, source, parent, sceneModel);
        return newComponent;
    }
    case MouseInput: {
        Qt3DInput::QMouseInput *newComponent = new Qt3DInput::QMouseInput();
        nameDuplicate(newComponent, source, parent, sceneModel);
        return newComponent;
    }
        */
    case ObjectPicker: {
        QDummyObjectPicker *source = qobject_cast<QDummyObjectPicker *>(component);
        QDummyObjectPicker *newComponent = new QDummyObjectPicker();
        nameDuplicate(newComponent, source, parent, sceneModel);
        newComponent->setHoverEnabled(source->hoverEnabled());
        return newComponent;
    }
    case Unknown:
        qWarning() << "Unsupported component:" << component;
        break;
    }

    return Q_NULLPTR;
}

void EditorUtils::nameDuplicate(QObject *duplicate, QObject *original, Qt3DCore::QEntity *parent,
                                EditorSceneItemModel *sceneModel)
{
    if (original->objectName().isEmpty())
        return;

    QString newName = sceneModel->generateValidName(original->objectName() + QObject::tr("_Copy"),
                                                    parent);
    duplicate->setObjectName(newName);
}

Qt3DRender::QGeometryRenderer *EditorUtils::createWireframeBoxMesh()
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
    QVector<QVector3D> vertices;

    vertexBufferData.resize(8 * 3 * sizeof(float));
    indexBufferData.resize(12 * 2 * sizeof(ushort));

    float *vPtr = reinterpret_cast<float *>(vertexBufferData.data());
    vPtr[0]  = -0.5f; vPtr[1]  = -0.5f; vPtr[2]  = -0.5f;
    vPtr[3]  = 0.5f;  vPtr[4]  = -0.5f; vPtr[5]  = -0.5f;
    vPtr[6]  = 0.5f;  vPtr[7]  = -0.5f; vPtr[8]  = 0.5f;
    vPtr[9]  = -0.5f; vPtr[10] = -0.5f; vPtr[11] = 0.5f;
    vPtr[12] = -0.5f; vPtr[13] = 0.5f;  vPtr[14] = -0.5f;
    vPtr[15] = 0.5f;  vPtr[16] = 0.5f;  vPtr[17] = -0.5f;
    vPtr[18] = 0.5f;  vPtr[19] = 0.5f;  vPtr[20] = 0.5f;
    vPtr[21] = 0.5f;  vPtr[22] = 0.5f;  vPtr[23] = 0.5f;
    vPtr[24] = -0.5f; vPtr[25] = 0.5f;  vPtr[26] = 0.5f;

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

    Qt3DRender::QAttribute *boxPosAttribute = new Qt3DRender::QAttribute();
    boxPosAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    boxPosAttribute->setBuffer(boxDataBuffer);
    boxPosAttribute->setDataType(Qt3DRender::QAttribute::Float);
    boxPosAttribute->setDataSize(3);
    boxPosAttribute->setByteOffset(0);
    boxPosAttribute->setByteStride(0);
    boxPosAttribute->setCount(8);
    boxPosAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    Qt3DRender::QAttribute *boxIndexAttribute = new Qt3DRender::QAttribute();
    boxIndexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    boxIndexAttribute->setBuffer(indexDataBuffer);
    boxIndexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedShort);
    boxIndexAttribute->setDataSize(1);
    boxIndexAttribute->setByteOffset(0);
    boxIndexAttribute->setByteStride(0);
    boxIndexAttribute->setCount(24);

    boxGeometry->addAttribute(boxPosAttribute);
    boxGeometry->addAttribute(boxIndexAttribute);

    boxMesh->setInstanceCount(1);
    boxMesh->setBaseVertex(0);
    boxMesh->setBaseInstance(0);
    boxMesh->setPrimitiveCount(12);
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

    Qt3DRender::QAttribute *planePosAttribute = new Qt3DRender::QAttribute();
    planePosAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    planePosAttribute->setBuffer(planeDataBuffer);
    planePosAttribute->setDataType(Qt3DRender::QAttribute::Float);
    planePosAttribute->setDataSize(3);
    planePosAttribute->setByteOffset(0);
    planePosAttribute->setByteStride(0);
    planePosAttribute->setCount(lineCount * 4);
    planePosAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    planeGeometry->addAttribute(planePosAttribute);

    planeMesh->setInstanceCount(1);
    planeMesh->setBaseVertex(0);
    planeMesh->setBaseInstance(0);
    planeMesh->setPrimitiveCount(lineCount * 4);
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
    } else if (qobject_cast<Qt3DCore::QCameraLens *>(component)) {
        componentType = CameraLens;
    } else if (qobject_cast<Qt3DRender::QFrameGraph *>(component)) {
        componentType = FrameGraph;
    } else if (qobject_cast<Qt3DInput::QKeyboardInput *>(component)) {
        componentType = KeyboardInput;
    } else if (qobject_cast<Qt3DRender::QLayer *>(component)) {
        componentType = Layer;
    } else if (qobject_cast<Qt3DLogic::QLogicComponent *>(component)) {
        componentType = Logic;
    } else if (qobject_cast<Qt3DInput::QMouseInput *>(component)) {
        componentType = MouseInput;
    } else if (qobject_cast<QDummyObjectPicker *>(component)) {
        componentType = ObjectPicker;
    }

    return componentType;
}
