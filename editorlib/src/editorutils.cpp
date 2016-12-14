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
#include "ontopeffect.h"
#include "qdummyobjectpicker.h"

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QTextureImage>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QRenderState>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QAbstractTextureImage>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DCore/QTransform>

#include <Qt3DRender/QMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTorusMesh>

#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QDiffuseSpecularMapMaterial>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DExtras/QNormalDiffuseMapMaterial>
#include <Qt3DExtras/QNormalDiffuseMapAlphaMaterial>
#include <Qt3DExtras/QNormalDiffuseSpecularMapMaterial>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DExtras/QPhongMaterial>

#include <Qt3DRender/QAbstractLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QSpotLight>

#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QBufferDataGenerator>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometryFactory>

#include <Qt3DRender/QAlphaCoverage>
#include <Qt3DRender/QAlphaTest>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QColorMask>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QMultiSampleAntiAliasing>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QDithering>
#include <Qt3DRender/QFrontFace>
#include <Qt3DRender/QPointSize>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/QScissorTest>
#include <Qt3DRender/QStencilTest>
#include <Qt3DRender/QStencilTestArguments>
#include <Qt3DRender/QStencilMask>
#include <Qt3DRender/QStencilOperation>
#include <Qt3DRender/QStencilOperationArguments>
#include <Qt3DRender/QClipPlane>
#include <Qt3DRender/QSeamlessCubemap>
#include <Qt3DRender/private/qrenderstate_p.h>

#include <QtCore/QtMath>

static const QString internalPrefix = QStringLiteral("__internal");

bool EditorUtils::isObjectInternal(QObject *obj)
{
    if (obj)
        return obj->objectName().startsWith(internalPrefix);
    else
        return false;
}

void EditorUtils::copyCameraProperties(Qt3DRender::QCamera *target, Qt3DCore::QEntity *source)
{
    Qt3DRender::QCamera *sourceCamera = qobject_cast<Qt3DRender::QCamera *>(source);
    if (sourceCamera) {
        target->setAspectRatio(sourceCamera->aspectRatio());
        target->setBottom(sourceCamera->bottom());
        target->setFarPlane(sourceCamera->farPlane());
        target->setFieldOfView(sourceCamera->fieldOfView());
        target->setLeft(sourceCamera->left());
        target->setNearPlane(sourceCamera->nearPlane());
        target->setPosition(sourceCamera->position());
        target->setProjectionType(sourceCamera->projectionType());
        target->setRight(sourceCamera->right());
        target->setTop(sourceCamera->top());
        target->setUpVector(sourceCamera->upVector());
        target->setViewCenter(sourceCamera->viewCenter());
    } else {
        Qt3DRender::QCameraLens *sourceCameraLens = EditorUtils::entityCameraLens(source);
        Qt3DCore::QTransform *sourceTransform = EditorUtils::entityTransform(source);
        target->setAspectRatio(sourceCameraLens->aspectRatio());
        target->setBottom(sourceCameraLens->bottom());
        target->setFarPlane(sourceCameraLens->farPlane());
        target->setFieldOfView(sourceCameraLens->fieldOfView());
        target->setLeft(sourceCameraLens->left());
        target->setNearPlane(sourceCameraLens->nearPlane());
        target->setProjectionType(sourceCameraLens->projectionType());
        target->setRight(sourceCameraLens->right());
        target->setTop(sourceCameraLens->top());

        QMatrix4x4 m;
        if (sourceTransform)
            m = sourceTransform->matrix();
        target->setPosition(m.map(QVector3D(0.0f, 0.0f, 0.0f)));
        target->setUpVector(QVector3D(m(0,1), m(1,1), m(2,1)));
        target->setViewCenter(m.mapVector(QVector3D(0.0f, 0.0f, 1.0f)));
    }
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
    case MaterialDiffuseMap: {
        Qt3DExtras::QDiffuseMapMaterial *source =
                qobject_cast<Qt3DExtras::QDiffuseMapMaterial *>(component);
        Qt3DExtras::QDiffuseMapMaterial *newComponent = new Qt3DExtras::QDiffuseMapMaterial();
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
        Qt3DExtras::QDiffuseSpecularMapMaterial *source =
                qobject_cast<Qt3DExtras::QDiffuseSpecularMapMaterial *>(component);
        Qt3DExtras::QDiffuseSpecularMapMaterial *newComponent =
                new Qt3DExtras::QDiffuseSpecularMapMaterial();
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
        Qt3DExtras::QGoochMaterial *source = qobject_cast<Qt3DExtras::QGoochMaterial *>(component);
        Qt3DExtras::QGoochMaterial *newComponent = new Qt3DExtras::QGoochMaterial();
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
        Qt3DExtras::QNormalDiffuseMapAlphaMaterial *source =
                qobject_cast<Qt3DExtras::QNormalDiffuseMapAlphaMaterial *>(component);
        Qt3DExtras::QNormalDiffuseMapAlphaMaterial *newComponent =
                new Qt3DExtras::QNormalDiffuseMapAlphaMaterial();
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
        Qt3DExtras::QNormalDiffuseMapMaterial *source =
                qobject_cast<Qt3DExtras::QNormalDiffuseMapMaterial *>(component);
        Qt3DExtras::QNormalDiffuseMapMaterial *newComponent =
                new Qt3DExtras::QNormalDiffuseMapMaterial();
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
        Qt3DExtras::QNormalDiffuseSpecularMapMaterial *source =
                qobject_cast<Qt3DExtras::QNormalDiffuseSpecularMapMaterial *>(component);
        Qt3DExtras::QNormalDiffuseSpecularMapMaterial *newComponent =
                new Qt3DExtras::QNormalDiffuseSpecularMapMaterial();
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
        Qt3DExtras::QPerVertexColorMaterial *newComponent =
                new Qt3DExtras::QPerVertexColorMaterial();
        duplicate = newComponent;
        break;
    }
    case MaterialPhongAlpha: {
        Qt3DExtras::QPhongAlphaMaterial *source =
                qobject_cast<Qt3DExtras::QPhongAlphaMaterial *>(component);
        Qt3DExtras::QPhongAlphaMaterial *newComponent = new Qt3DExtras::QPhongAlphaMaterial();
        newComponent->setAlpha(source->alpha());
        newComponent->setAmbient(source->ambient());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        duplicate = newComponent;
        break;
    }
    case MaterialPhong: {
        Qt3DExtras::QPhongMaterial *source = qobject_cast<Qt3DExtras::QPhongMaterial *>(component);
        Qt3DExtras::QPhongMaterial *newComponent = new Qt3DExtras::QPhongMaterial();
        newComponent->setAmbient(source->ambient());
        newComponent->setDiffuse(source->diffuse());
        newComponent->setShininess(source->shininess());
        newComponent->setSpecular(source->specular());
        duplicate = newComponent;
        break;
    }
    case MaterialGeneric: {
        Qt3DRender::QMaterial *source = qobject_cast<Qt3DRender::QMaterial *>(component);
        Qt3DRender::QMaterial *newComponent = new Qt3DRender::QMaterial();
        Qt3DRender::QEffect *newEffect = new Qt3DRender::QEffect;

        copyRenderParameters(source, newComponent);
        copyRenderParameters(source->effect(), newEffect);

        Q_FOREACH (Qt3DRender::QTechnique *tech, source->effect()->techniques()) {
            Qt3DRender::QTechnique *newTech = new Qt3DRender::QTechnique;
            copyFilterKeys(tech, newTech);
            copyRenderParameters(tech, newTech);

            Q_FOREACH (Qt3DRender::QRenderPass *pass, tech->renderPasses()) {
                Qt3DRender::QRenderPass *newPass = new Qt3DRender::QRenderPass;
                copyFilterKeys(pass, newPass);
                copyRenderParameters(pass, newPass);
                copyRenderStates(pass, newPass);

                Qt3DRender::QShaderProgram *newProgram = new Qt3DRender::QShaderProgram;
                newProgram->setVertexShaderCode(pass->shaderProgram()->vertexShaderCode());
                newProgram->setTessellationControlShaderCode(
                            pass->shaderProgram()->tessellationControlShaderCode());
                newProgram->setTessellationEvaluationShaderCode(
                            pass->shaderProgram()->tessellationEvaluationShaderCode());
                newProgram->setGeometryShaderCode(pass->shaderProgram()->geometryShaderCode());
                newProgram->setFragmentShaderCode(pass->shaderProgram()->fragmentShaderCode());
                newProgram->setComputeShaderCode(pass->shaderProgram()->computeShaderCode());

                newPass->setShaderProgram(newProgram);
                newTech->addRenderPass(newPass);
            }
            newEffect->addTechnique(newTech);
        }
        newComponent->setEffect(newEffect);

        duplicate = newComponent;
        break;
    }
    case MeshCuboid: {
        Qt3DExtras::QCuboidMesh *source = qobject_cast<Qt3DExtras::QCuboidMesh *>(component);
        Qt3DExtras::QCuboidMesh *newComponent = new Qt3DExtras::QCuboidMesh();
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
        Qt3DExtras::QCylinderMesh *source = qobject_cast<Qt3DExtras::QCylinderMesh *>(component);
        Qt3DExtras::QCylinderMesh *newComponent = new Qt3DExtras::QCylinderMesh();
        newComponent->setLength(source->length());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        duplicate = newComponent;
        break;
    }
    case MeshPlane: {
        Qt3DExtras::QPlaneMesh *source = qobject_cast<Qt3DExtras::QPlaneMesh *>(component);
        Qt3DExtras::QPlaneMesh *newComponent = new Qt3DExtras::QPlaneMesh();
        newComponent->setHeight(source->height());
        newComponent->setMeshResolution(source->meshResolution());
        newComponent->setWidth(source->width());
        duplicate = newComponent;
        break;
    }
    case MeshSphere: {
        Qt3DExtras::QSphereMesh *source = qobject_cast<Qt3DExtras::QSphereMesh *>(component);
        Qt3DExtras::QSphereMesh *newComponent = new Qt3DExtras::QSphereMesh();
        newComponent->setGenerateTangents(source->generateTangents());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        duplicate = newComponent;
        break;
    }
    case MeshTorus: {
        Qt3DExtras::QTorusMesh *source = qobject_cast<Qt3DExtras::QTorusMesh *>(component);
        Qt3DExtras::QTorusMesh *newComponent = new Qt3DExtras::QTorusMesh();
        newComponent->setMinorRadius(source->minorRadius());
        newComponent->setRadius(source->radius());
        newComponent->setRings(source->rings());
        newComponent->setSlices(source->slices());
        duplicate = newComponent;
        break;
    }
    case MeshGeneric: {
        Qt3DRender::QGeometryRenderer *source = qobject_cast<Qt3DRender::QGeometryRenderer *>(component);
        Qt3DRender::QGeometryRenderer *newComponent = new Qt3DRender::QGeometryRenderer();
        newComponent->setInstanceCount(source->instanceCount());
        newComponent->setVertexCount(source->vertexCount());
        newComponent->setIndexOffset(source->indexOffset());
        newComponent->setFirstInstance(source->firstInstance());
        newComponent->setRestartIndexValue(source->restartIndexValue());
        newComponent->setVerticesPerPatch(source->verticesPerPatch());
        newComponent->setPrimitiveRestartEnabled(source->primitiveRestartEnabled());
        newComponent->setPrimitiveType(source->primitiveType());

        Qt3DRender::QGeometry *sourceGeometry = source->geometry();
        Qt3DRender::QGeometry *newGeometry = new Qt3DRender::QGeometry;
        if (!sourceGeometry) {
            Qt3DRender::QGeometryFactoryPtr geometryFunctorPtr = source->geometryFactory();
            if (geometryFunctorPtr.data())
                sourceGeometry = geometryFunctorPtr.data()->operator()();
        }
        if (sourceGeometry) {
            QMap<Qt3DRender::QBuffer *, Qt3DRender::QBuffer *> bufferMap;
            Q_FOREACH (Qt3DRender::QAttribute *oldAtt, sourceGeometry->attributes()) {
                Qt3DRender::QAttribute *newAtt = copyAttribute(oldAtt, bufferMap);
                if (newAtt)
                    newGeometry->addAttribute(newAtt);
            }

            newGeometry->setBoundingVolumePositionAttribute(
                        copyAttribute(sourceGeometry->boundingVolumePositionAttribute(),
                                      bufferMap));

            newComponent->setGeometry(newGeometry);
        }
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

Qt3DRender::QGeometryRenderer *EditorUtils::createSingleLineMesh()
{
    Qt3DRender::QGeometryRenderer *mesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *geometry = new Qt3DRender::QGeometry(mesh);
    Qt3DRender::QBuffer *dataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer,
                                                              geometry);
    QByteArray vertexBufferData;
    QVector<QVector3D> vertices;

    vertices.resize(2);
    vertexBufferData.resize(vertices.size() * 3 * sizeof(float));

    vertices[0] = QVector3D(0.0f, 0.0f, 0.0f);
    vertices[1] = QVector3D(0.0f, 0.0f, 1.0f);

    float *rawVertexArray = reinterpret_cast<float *>(vertexBufferData.data());
    int idx = 0;
    Q_FOREACH (const QVector3D &v, vertices) {
        rawVertexArray[idx++] = v.x();
        rawVertexArray[idx++] = v.y();
        rawVertexArray[idx++] = v.z();
    }

    dataBuffer->setData(vertexBufferData);

    addPositionAttributeToGeometry(geometry, dataBuffer, 2);

    mesh->setInstanceCount(1);
    mesh->setIndexOffset(0);
    mesh->setFirstInstance(0);
    mesh->setVertexCount(2);
    mesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    mesh->setGeometry(geometry);

    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createDefaultCustomMesh()
{
    Qt3DRender::QMesh *customMesh = new Qt3DRender::QMesh();
    customMesh->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/meshes/defaultmesh.obj")));
    return customMesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createVisibleCameraMesh()
{
    Qt3DRender::QMesh *customMesh = new Qt3DRender::QMesh();
    customMesh->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/meshes/camera.obj")));
    return customMesh;
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
    Qt3DExtras::QSphereMesh *mesh = new Qt3DExtras::QSphereMesh;
    mesh->setRadius(size / 2.0f);
    return mesh;
}

Qt3DRender::QGeometryRenderer *EditorUtils::createLightMesh(EditorUtils::ComponentTypes type)
{
    Qt3DRender::QGeometryRenderer *mesh = nullptr;

    switch (type) {
    case LightDirectional: {
        Qt3DRender::QMesh *directionalMesh = new Qt3DRender::QMesh();
        directionalMesh->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/meshes/directionallight.obj")));
        mesh = directionalMesh;
        break;
    }
    case LightPoint: {
        Qt3DExtras::QSphereMesh *pointMesh = new Qt3DExtras::QSphereMesh();
        pointMesh->setRadius(0.2f);
        pointMesh->setRings(10);
        pointMesh->setSlices(10);
        mesh = pointMesh;
        break;
    }
    case LightSpot: {
        Qt3DRender::QMesh *spotMesh = new Qt3DRender::QMesh();
        spotMesh->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/meshes/spotlight.obj")));
        mesh = spotMesh;
        break;
    }
    default: {
        qCritical("Should not get here.");
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
        mesh = new Qt3DExtras::QCuboidMesh();
        break;
    }
    case CylinderEntity: {
        mesh = new Qt3DExtras::QCylinderMesh();
        break;
    }
    case PlaneEntity: {
        mesh = new Qt3DExtras::QPlaneMesh();
        break;
    }
    case SphereEntity: {
        mesh = new Qt3DExtras::QSphereMesh();
        break;
    }
    case TorusEntity: {
        mesh = new Qt3DExtras::QTorusMesh();
        break;
    }
    case CustomEntity: {
        mesh = createDefaultCustomMesh();
        break;
    }
    case GroupEntity: {
        // Group entity mesh is only used for drag-insert placeholder
        mesh = new Qt3DExtras::QCuboidMesh();
        break;
    }
    default:
        break;
    }

    return mesh;
}

// Creates a single arrow
Qt3DRender::QGeometryRenderer *EditorUtils::createArrowMesh()
{
    Qt3DRender::QMesh *customMesh = new Qt3DRender::QMesh();
    customMesh->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/meshes/arrow.obj")));
    return customMesh;
}

Qt3DCore::QEntity *EditorUtils::createArrowEntity(const QColor &color,
                                                  Qt3DCore::QEntity *parent,
                                                  const QMatrix4x4 &matrix,
                                                  const QString &name)
{
    Qt3DCore::QEntity *arrow = new Qt3DCore::QEntity(parent);
    arrow->setObjectName(name);

    Qt3DRender::QGeometryRenderer *mesh = EditorUtils::createArrowMesh();

    Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial();
    material->setEffect(new OnTopEffect());
    material->addParameter(new Qt3DRender::QParameter(QStringLiteral("handleColor"), color));

    Qt3DCore::QTransform *transform = new Qt3DCore::QTransform();
    transform->setMatrix(matrix);

    arrow->addComponent(mesh);
    arrow->addComponent(material);
    arrow->addComponent(transform);

    return arrow;
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

Qt3DRender::QAbstractLight *EditorUtils::entityLight(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DRender::QAbstractLight *light = qobject_cast<Qt3DRender::QAbstractLight *>(components.value(i));
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

Qt3DRender::QCameraLens *EditorUtils::entityCameraLens(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DRender::QCameraLens *lens
                = qobject_cast<Qt3DRender::QCameraLens *>(components.value(i));
        if (lens)
            return lens;
    }

    return nullptr;
}

bool EditorUtils::isGroupEntity(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentVector components = entity->components();
    return (components.size() == 0 || (components.size() == 1 && entityTransform(entity)));
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

// Returns a viewport position for a ray from camera origin to world position
// If returned z-value is negative, the position is behind camera
QVector3D EditorUtils::projectRay(const QMatrix4x4 &viewMatrix,
                                  const QMatrix4x4 &projectionMatrix,
                                  int viewPortWidth, int viewPortHeight,
                                  const QVector3D &worldPos)
{
    QVector3D localPos = projectionMatrix * viewMatrix * worldPos;
    localPos *= QVector3D(0.5f, -0.5f, -0.5f);
    localPos += QVector3D(0.5f, 0.5f, 0.5f);
    return QVector3D(viewPortWidth * localPos.x(), viewPortHeight * localPos.y(), localPos.z());
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

    if (qobject_cast<Qt3DRender::QAbstractLight *>(component)) {
        if (qobject_cast<Qt3DRender::QDirectionalLight *>(component))
            componentType = LightDirectional;
        else if (qobject_cast<Qt3DRender::QPointLight *>(component))
            componentType = LightPoint;
        else if (qobject_cast<Qt3DRender::QSpotLight *>(component))
            componentType = LightSpot;
    } else if (qobject_cast<Qt3DRender::QMaterial *>(component)) {
        if (qobject_cast<Qt3DExtras::QDiffuseMapMaterial *>(component))
            componentType = MaterialDiffuseMap;
        else if (qobject_cast<Qt3DExtras::QDiffuseSpecularMapMaterial *>(component))
            componentType = MaterialDiffuseSpecularMap;
        else if (qobject_cast<Qt3DExtras::QGoochMaterial *>(component))
            componentType = MaterialGooch;
        // Inherits QNormalDiffuseMapMaterial, so must be tested first
        else if (qobject_cast<Qt3DExtras::QNormalDiffuseMapAlphaMaterial *>(component))
            componentType = MaterialNormalDiffuseMapAlpha;
        else if (qobject_cast<Qt3DExtras::QNormalDiffuseMapMaterial *>(component))
            componentType = MaterialNormalDiffuseMap;
        else if (qobject_cast<Qt3DExtras::QNormalDiffuseSpecularMapMaterial *>(component))
            componentType = MaterialNormalDiffuseSpecularMap;
        else if (qobject_cast<Qt3DExtras::QPerVertexColorMaterial *>(component))
            componentType = MaterialPerVertexColor;
        else if (qobject_cast<Qt3DExtras::QPhongAlphaMaterial *>(component))
            componentType = MaterialPhongAlpha;
        else if (qobject_cast<Qt3DExtras::QPhongMaterial *>(component))
            componentType = MaterialPhong;
        else
            componentType = MaterialGeneric;
    } else if (qobject_cast<Qt3DRender::QGeometryRenderer *>(component)) {
        if (qobject_cast<Qt3DRender::QMesh *>(component))
            componentType = MeshCustom;
        else if (qobject_cast<Qt3DExtras::QCuboidMesh *>(component))
            componentType = MeshCuboid;
        else if (qobject_cast<Qt3DExtras::QCylinderMesh *>(component))
            componentType = MeshCylinder;
        else if (qobject_cast<Qt3DExtras::QPlaneMesh *>(component))
            componentType = MeshPlane;
        else if (qobject_cast<Qt3DExtras::QSphereMesh *>(component))
            componentType = MeshSphere;
        else if (qobject_cast<Qt3DExtras::QTorusMesh *>(component))
            componentType = MeshTorus;
        else
            componentType = MeshGeneric;
    } else if (qobject_cast<Qt3DCore::QTransform *>(component)) {
        componentType = Transform;
    } else if (qobject_cast<QDummyObjectPicker *>(component)) {
        componentType = ObjectPicker;
    } else if (qobject_cast<Qt3DRender::QSceneLoader *>(component)) {
        componentType = SceneLoader;
    }

    return componentType;
}

Qt3DRender::QAttribute *EditorUtils::copyAttribute(
        Qt3DRender::QAttribute *oldAtt,
        QMap<Qt3DRender::QBuffer *, Qt3DRender::QBuffer *> &bufferMap)
{
    Qt3DRender::QAttribute *newAtt = nullptr;
    if (oldAtt) {
        newAtt = new Qt3DRender::QAttribute;

        newAtt->setName(oldAtt->name());
        newAtt->setDataType(oldAtt->vertexBaseType());
        newAtt->setDataSize(oldAtt->vertexSize());
        newAtt->setCount(oldAtt->count());
        newAtt->setByteStride(oldAtt->byteStride());
        newAtt->setByteOffset(oldAtt->byteOffset());
        newAtt->setDivisor(oldAtt->divisor());
        newAtt->setAttributeType(oldAtt->attributeType());

        Qt3DRender::QBuffer *oldBuf = oldAtt->buffer();
        if (oldBuf) {
            Qt3DRender::QBuffer *newBuf = bufferMap.value(oldBuf);
            if (!newBuf) {
                newBuf = new Qt3DRender::QBuffer;
                bufferMap.insert(oldBuf, newBuf);

                if (oldBuf->data().isEmpty())
                    newBuf->setData(oldBuf->dataGenerator()->operator()());
                else
                    newBuf->setData(oldBuf->data());
                newBuf->setType(oldBuf->type());
                newBuf->setUsage(oldBuf->usage());
                newBuf->setSyncData(oldBuf->isSyncData());
            }

            newAtt->setBuffer(newBuf);
        }
    }

    return newAtt;
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

QVector3D EditorUtils::lightDirection(const Qt3DRender::QAbstractLight *light)
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
    EditorSceneItem *parent = descendantItem->parentItem();

    while (parent) {
        if (parent == ancestor)
            return true;
        parent = parent->parentItem();
    }
    return false;
}

EditorUtils::InsertableEntities EditorUtils::insertableEntityType(Qt3DCore::QEntity *entity)
{
    InsertableEntities insertableType = InvalidEntity;

    Qt3DRender::QAbstractLight *light = entityLight(entity);
    Qt3DRender::QGeometryRenderer *mesh = entityMesh(entity);

    if (light) {
        insertableType = LightEntity;
    } else if (mesh) {
        if (qobject_cast<Qt3DRender::QMesh *>(mesh))
            insertableType = CustomEntity;
        else if (qobject_cast<Qt3DExtras::QCuboidMesh *>(mesh))
            insertableType = CuboidEntity;
        else if (qobject_cast<Qt3DExtras::QCylinderMesh *>(mesh))
            insertableType = CylinderEntity;
        else if (qobject_cast<Qt3DExtras::QPlaneMesh *>(mesh))
            insertableType = PlaneEntity;
        else if (qobject_cast<Qt3DExtras::QSphereMesh *>(mesh))
            insertableType = SphereEntity;
        else if (qobject_cast<Qt3DExtras::QTorusMesh *>(mesh))
            insertableType = TorusEntity;
    } else if (qobject_cast<Qt3DRender::QCamera *>(entity)) {
        insertableType = CameraEntity;
    } else if (entity->children().count() == 1
               && qobject_cast<Qt3DCore::QTransform *>(entity->children().at(0))) {
        insertableType = GroupEntity;
    }

    return insertableType;
}

void EditorUtils::setEnabledToSubtree(Qt3DCore::QEntity *entity, bool enable)
{
    entity->setEnabled(enable);
    Q_FOREACH (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            setEnabledToSubtree(childEntity, enable);
    }
}

Qt3DCore::QEntity *EditorUtils::findEntityByName(Qt3DCore::QEntity *entity, const QString &name)
{
    if (entity->objectName() == name) {
        return entity;
    } else {
        for (QObject *child : entity->children()) {
            Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
            if (childEntity) {
                Qt3DCore::QEntity *foundEntity = findEntityByName(childEntity, name);
                if (foundEntity)
                    return foundEntity;
            }
        }
    }
    return nullptr;
}

template <typename T>
void EditorUtils::copyRenderParameters(T *source, T *target)
{
    Q_FOREACH (Qt3DRender::QParameter *param, source->parameters()) {
        Qt3DRender::QParameter *newParam = new Qt3DRender::QParameter;
        newParam->setName(param->name());

        // Textures need special handling
        Qt3DRender::QAbstractTexture *texture =
                param->value().value<Qt3DRender::QAbstractTexture *>();
        if (texture) {
            Qt3DRender::QTexture2D *texture2D = qobject_cast<Qt3DRender::QTexture2D *>(texture);
            if (texture2D) {
                // TODO: Only support texture2D for now (as that's what qgltf supports), todo rest
                Qt3DRender::QTexture2D *newTexture = new Qt3DRender::QTexture2D;

                Q_FOREACH (Qt3DRender::QAbstractTextureImage *ti, texture->textureImages()) {
                    Qt3DRender::QTextureImage *sourceImage =
                            qobject_cast<Qt3DRender::QTextureImage *>(ti);
                    if (sourceImage) {
                        Qt3DRender::QTextureImage *newImage = new Qt3DRender::QTextureImage;
                        newImage->setMipLevel(sourceImage->mipLevel());
                        newImage->setLayer(sourceImage->layer());
                        newImage->setFace(sourceImage->face());
                        newImage->setSource(sourceImage->source());
                        newTexture->addTextureImage(newImage);
                    }
                }

                newTexture->setFormat(texture->format());
                newTexture->setGenerateMipMaps(texture->generateMipMaps());
                newTexture->setWidth(texture->width());
                newTexture->setHeight(texture->height());
                newTexture->setDepth(texture->depth());
                newTexture->setMagnificationFilter(texture->magnificationFilter());
                newTexture->setMinificationFilter(texture->minificationFilter());
                newTexture->setMaximumAnisotropy(texture->maximumAnisotropy());
                newTexture->setComparisonFunction(texture->comparisonFunction());
                newTexture->setComparisonMode(texture->comparisonMode());
                newTexture->setLayers(texture->layers());
                newTexture->wrapMode()->setX(texture->wrapMode()->x());
                newTexture->wrapMode()->setY(texture->wrapMode()->y());
                newTexture->wrapMode()->setZ(texture->wrapMode()->z());

                newParam->setValue(QVariant::fromValue(newTexture));
            }
        } else {
            newParam->setValue(param->value());
        }

        target->addParameter(newParam);
    }
}

template <typename T>
void EditorUtils::copyFilterKeys(T *source, T *target) {
    Q_FOREACH (Qt3DRender::QFilterKey *key, source->filterKeys()) {
        Qt3DRender::QFilterKey *newKey = new Qt3DRender::QFilterKey;
        newKey->setName(key->name());
        newKey->setValue(key->value());
        target->addFilterKey(newKey);
    }
}

void EditorUtils::copyRenderStates(Qt3DRender::QRenderPass *source,
                                   Qt3DRender::QRenderPass *target)
{
    Q_FOREACH (Qt3DRender::QRenderState *state, source->renderStates()) {
        Qt3DRender::QRenderState *newState = nullptr;

        Qt3DRender::QRenderStatePrivate *stateP =
                static_cast<Qt3DRender::QRenderStatePrivate *>(
                    Qt3DRender::QRenderStatePrivate::get(state));

        switch (stateP->m_type) {
        case Qt3DRender::Render::AlphaCoverageStateMask: {
            newState = new Qt3DRender::QAlphaCoverage;
            break;
        }
        case Qt3DRender::Render::AlphaTestMask: {
            Qt3DRender::QAlphaTest *sourceState =
                    qobject_cast<Qt3DRender::QAlphaTest *>(state);
            Qt3DRender::QAlphaTest *targetState = new Qt3DRender::QAlphaTest;
            targetState->setAlphaFunction(sourceState->alphaFunction());
            targetState->setReferenceValue(sourceState->referenceValue());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::BlendStateMask: {
            Qt3DRender::QBlendEquation *sourceState =
                    qobject_cast<Qt3DRender::QBlendEquation *>(state);
            Qt3DRender::QBlendEquation *targetState = new Qt3DRender::QBlendEquation;
            targetState->setBlendFunction(sourceState->blendFunction());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::BlendEquationArgumentsMask: {
            Qt3DRender::QBlendEquationArguments *sourceState =
                    qobject_cast<Qt3DRender::QBlendEquationArguments *>(state);
            Qt3DRender::QBlendEquationArguments *targetState =
                    new Qt3DRender::QBlendEquationArguments;
            targetState->setSourceRgb(sourceState->sourceRgb());
            targetState->setDestinationRgb(sourceState->destinationRgb());
            targetState->setSourceAlpha(sourceState->sourceAlpha());
            targetState->setDestinationAlpha(sourceState->destinationAlpha());
            targetState->setBufferIndex(sourceState->bufferIndex());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::ColorStateMask: {
            Qt3DRender::QColorMask *sourceState = qobject_cast<Qt3DRender::QColorMask *>(state);
            Qt3DRender::QColorMask *targetState = new Qt3DRender::QColorMask;
            targetState->setRedMasked(sourceState->isRedMasked());
            targetState->setGreenMasked(sourceState->isGreenMasked());
            targetState->setBlueMasked(sourceState->isBlueMasked());
            targetState->setAlphaMasked(sourceState->isAlphaMasked());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::CullFaceStateMask: {
            Qt3DRender::QCullFace *sourceState = qobject_cast<Qt3DRender::QCullFace *>(state);
            Qt3DRender::QCullFace *targetState = new Qt3DRender::QCullFace;
            targetState->setMode(sourceState->mode());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::MSAAEnabledStateMask: {
            newState = new Qt3DRender::QMultiSampleAntiAliasing;
            break;
        }
        case Qt3DRender::Render::DepthWriteStateMask: {
            newState = new Qt3DRender::QNoDepthMask;
            break;
        }
        case Qt3DRender::Render::DepthTestStateMask: {
            Qt3DRender::QDepthTest *sourceState = qobject_cast<Qt3DRender::QDepthTest *>(state);
            Qt3DRender::QDepthTest *targetState = new Qt3DRender::QDepthTest;
            targetState->setDepthFunction(sourceState->depthFunction());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::DitheringStateMask: {
            newState = new Qt3DRender::QDithering;
            break;
        }
        case Qt3DRender::Render::FrontFaceStateMask: {
            Qt3DRender::QFrontFace *sourceState = qobject_cast<Qt3DRender::QFrontFace *>(state);
            Qt3DRender::QFrontFace *targetState = new Qt3DRender::QFrontFace;
            targetState->setDirection(sourceState->direction());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::PointSizeMask: {
            Qt3DRender::QPointSize *sourceState = qobject_cast<Qt3DRender::QPointSize *>(state);
            Qt3DRender::QPointSize *targetState = new Qt3DRender::QPointSize;
            targetState->setSizeMode(sourceState->sizeMode());
            targetState->setValue(sourceState->value());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::PolygonOffsetStateMask: {
            Qt3DRender::QPolygonOffset *sourceState =
                    qobject_cast<Qt3DRender::QPolygonOffset *>(state);
            Qt3DRender::QPolygonOffset *targetState = new Qt3DRender::QPolygonOffset;
            targetState->setScaleFactor(sourceState->scaleFactor());
            targetState->setDepthSteps(sourceState->depthSteps());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::ScissorStateMask: {
            Qt3DRender::QScissorTest *sourceState =
                    qobject_cast<Qt3DRender::QScissorTest *>(state);
            Qt3DRender::QScissorTest *targetState = new Qt3DRender::QScissorTest;
            targetState->setLeft(sourceState->left());
            targetState->setBottom(sourceState->bottom());
            targetState->setWidth(sourceState->width());
            targetState->setHeight(sourceState->height());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::StencilTestStateMask: {
            Qt3DRender::QStencilTest *sourceState =
                    qobject_cast<Qt3DRender::QStencilTest *>(state);
            Qt3DRender::QStencilTest *targetState = new Qt3DRender::QStencilTest;
            targetState->front()->setComparisonMask(sourceState->front()->comparisonMask());
            targetState->front()->setReferenceValue(sourceState->front()->referenceValue());
            targetState->front()->setStencilFunction(sourceState->front()->stencilFunction());
            targetState->back()->setComparisonMask(sourceState->back()->comparisonMask());
            targetState->back()->setReferenceValue(sourceState->back()->referenceValue());
            targetState->back()->setStencilFunction(sourceState->back()->stencilFunction());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::StencilWriteStateMask: {
            Qt3DRender::QStencilMask *sourceState =
                    qobject_cast<Qt3DRender::QStencilMask *>(state);
            Qt3DRender::QStencilMask *targetState = new Qt3DRender::QStencilMask;
            targetState->setFrontOutputMask(sourceState->frontOutputMask());
            targetState->setBackOutputMask(sourceState->backOutputMask());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::StencilOpMask: {
            Qt3DRender::QStencilOperation *sourceState =
                    qobject_cast<Qt3DRender::QStencilOperation *>(state);
            Qt3DRender::QStencilOperation *targetState = new Qt3DRender::QStencilOperation;
            targetState->front()->setStencilTestFailureOperation(
                        sourceState->front()->stencilTestFailureOperation());
            targetState->front()->setDepthTestFailureOperation(
                        sourceState->front()->depthTestFailureOperation());
            targetState->front()->setAllTestsPassOperation(
                        sourceState->front()->allTestsPassOperation());
            targetState->back()->setStencilTestFailureOperation(
                        sourceState->back()->stencilTestFailureOperation());
            targetState->back()->setDepthTestFailureOperation(
                        sourceState->back()->depthTestFailureOperation());
            targetState->back()->setAllTestsPassOperation(
                        sourceState->back()->allTestsPassOperation());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::ClipPlaneMask: {
            Qt3DRender::QClipPlane *sourceState = qobject_cast<Qt3DRender::QClipPlane *>(state);
            Qt3DRender::QClipPlane *targetState = new Qt3DRender::QClipPlane;
            targetState->setPlaneIndex(sourceState->planeIndex());
            targetState->setNormal(sourceState->normal());
            targetState->setDistance(sourceState->distance());
            newState = targetState;
            break;
        }
        case Qt3DRender::Render::SeamlessCubemapMask: {
            newState = new Qt3DRender::QSeamlessCubemap;
            break;
        }
        default:
            qWarning() << __FUNCTION__ << QStringLiteral("Unknown render state");
            break;
        }

        if (newState)
            target->addRenderState(newState);
    }
}

