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
#include "editorscene.h"
#include "editorutils.h"
#include "editorsceneitem.h"
#include "editorsceneitemmodel.h"
#include "editorsceneparser.h"
#include "editorsceneitemcomponentsmodel.h"
#include "editorviewportitem.h"
#include "undohandler.h"

#include <Qt3DCore/QCamera>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QCameraLens>

#include <Qt3DRender/QTexture>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QAttribute>

#include <Qt3DRender/QFrameGraph>
#include <Qt3DRender/QForwardRenderer>

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCuboidMesh>
#include <Qt3DRender/QDiffuseSpecularMapMaterial>
#include <Qt3DRender/QPhongAlphaMaterial>
#include <Qt3DRender/QPhongMaterial>
#include <Qt3DRender/QLight>

#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/QKeySequence>

#include <QtCore/QDir>

//#define TEST_SCENE // If a test scene is wanted instead of the default scene

#ifdef TEST_SCENE
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QNormalDiffuseSpecularMapMaterial>
#endif

static const QString cameraConeName = QStringLiteral("__internal camera cone");
static const QString internalPrefix = QStringLiteral("__internal");
static const QString autoSavePostfix = QStringLiteral(".autosave");

EditorScene::EditorScene(QObject *parent)
    : QObject(parent)
    , m_rootEntity(Q_NULLPTR)
    , m_componentCache(Q_NULLPTR)
    , m_rootItem(Q_NULLPTR)
    , m_sceneModel(new EditorSceneItemModel(this))
    , m_sceneParser(new EditorSceneParser(this))
    , m_frameGraph(Q_NULLPTR)
    , m_sceneEntity(Q_NULLPTR)
    , m_sceneEntityItem(Q_NULLPTR)
    , m_selectedEntity(Q_NULLPTR)
    , m_activeSelection(false)
    , m_activeSceneCameraIndex(-1)
    , m_freeView(false)
    , m_freeViewCameraEntity(Q_NULLPTR)
    , m_viewport(Q_NULLPTR)
    , m_undoHandler(new UndoHandler(this))
    , m_helperPlane(Q_NULLPTR)
    , m_helperPlaneTransform(Q_NULLPTR)
    , m_editorUtils(new EditorUtils(this))
{
    createRootEntity();
    setupDefaultScene();

    // Install event filter to handle undo/redo globally, instead of each TextField having
    // their own stack.
    // TODO: This might need to be done differently if we make this Creator plugin
    qGuiApp->installEventFilter(this);
}

EditorScene::~EditorScene()
{
    // Remove all entities recursively to ensure the root item is last one to be deleted
    removeEntity(m_sceneEntity, true);

    // TODO: Check if it is necessary to delete rootentity and associated components, or do they get
    // TODO: properly deleted by aspect engine shutdown?

    delete m_componentCache;
}

Qt3DCore::QEntity *EditorScene::rootEntity()
{
    return m_rootEntity;
}

EditorSceneItem *EditorScene::rootItem()
{
    return m_rootItem;
}

EditorSceneItem *EditorScene::sceneEntityItem()
{
    return m_sceneEntityItem;
}

EditorSceneItemModel *EditorScene::sceneModel() const
{
    return m_sceneModel;
}

void EditorScene::addEntity(Qt3DCore::QEntity *entity, int index, Qt3DCore::QEntity *parent)
{
    if (entity == Q_NULLPTR)
        return;

    if (parent == Q_NULLPTR) {
        //make sure that entity has a parent, otherwise make its parent the root entity
        if (entity->parentEntity() == Q_NULLPTR)
            entity->setParent(m_rootEntity);
    } else if (entity->parentEntity() != parent) {
        entity->setParent(parent);
    }

    if (m_sceneItems.value(entity->id(), Q_NULLPTR) == Q_NULLPTR) {
        EditorSceneItem *item =
                new EditorSceneItem(this, entity,
                                    m_sceneItems.value(entity->parentEntity()->id(),
                                                       Q_NULLPTR), index, m_freeView, this);

        if (entity == m_sceneEntity)
            m_sceneEntityItem = item;

        m_sceneItems.insert(entity->id(), item);
        connect(this, &EditorScene::freeViewChanged,
                item, &EditorSceneItem::freeViewChanged);
        connect(item, &EditorSceneItem::cameraAdded,
                this, &EditorScene::handleCameraAdded);
        connect(item, &EditorSceneItem::cameraRemoved,
                this, &EditorScene::handleCameraRemoved);
        connect(item, &EditorSceneItem::cameraTransformChanged,
                this, &EditorScene::handleCameraTransformChange);
        connect(entity, &EditorSceneItem::objectNameChanged,
                this, &EditorScene::handleEntityNameChange);

        Qt3DCore::QCameraLens *lens = cameraLensForEntity(entity);
        if (lens)
            handleCameraAdded(entity);

        item->componentsModel()->initializeModel();
    }

    createObjectPickerForEntity(entity);

    foreach (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            addEntity(childEntity);
    }
}

void EditorScene::moveEntity(Qt3DCore::QEntity *entity, Qt3DCore::QEntity *newParent)
{
    if (entity == Q_NULLPTR || entity == m_rootEntity)
        return;

    Qt3DCore::QEntity *targetParent = newParent;

    if (newParent == Q_NULLPTR)
        targetParent = m_rootEntity;

    entity->setParent(targetParent);
}

// When deleteEntity is false, caller assumes ownership of the removed entity
void EditorScene::removeEntity(Qt3DCore::QEntity *entity, bool deleteEntity, bool recursiveCall)
{
    if (entity == Q_NULLPTR || entity == m_rootEntity)
        return;

    if (entity == m_sceneEntity) {
        m_sceneEntity = Q_NULLPTR;
        m_sceneEntityItem = Q_NULLPTR;
    }

    // Top level removal, initialize relation map
    if (!recursiveCall)
        m_entityRelations.clear();

    foreach (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        // Don't deparent child entities to preserve removed entity tree
        removeEntity(childEntity, deleteEntity, true);
    }

    if (cameraLensForEntity(entity))
        handleCameraRemoved(entity);

    removeEntityItem(entity->id());

    disconnect(entity, 0, this, 0);

    // Remove and delete any internal components
    foreach (Qt3DCore::QComponent *component, entity->components()) {
        if (component) {
            if (component->objectName().startsWith(internalPrefix)) {
                entity->removeComponent(component);
                delete component;
            } else {
                disconnect(component, 0, this, 0);
            }
        }
    }

    if (deleteEntity) {
        delete entity;
    } else {
        Qt3DCore::QEntity *oldParent = entity->parentEntity();
        // We need to deparent every entity to properly remove them from the scene, since
        // we are not actually deleting them
        entity->setParent(static_cast<Qt3DCore::QEntity *>(Q_NULLPTR));

        if (recursiveCall) {
            // Add relation to map so we can reconstruct the parent/child tree later
            m_entityRelations.append(EntityRelationship(oldParent, entity));
        } else {
            // Top level entity, reconstruct the parent-child tree for possible undo later
            foreach (EntityRelationship relationship, m_entityRelations)
                relationship.child->setParent(relationship.parent);
            m_entityRelations.clear();
        }
    }
}

void EditorScene::removeEntityItem(const Qt3DCore::QNodeId &id)
{
    EditorSceneItem *item = m_sceneItems.value(id);
    delete item;
    m_sceneItems.remove(id);
}

const QMap<Qt3DCore::QNodeId, EditorSceneItem *> &EditorScene::items() const
{
    return m_sceneItems;
}

void EditorScene::resetScene()
{
    // Clear the existing scene
    setFrameGraphCamera(Q_NULLPTR);
    m_undoHandler->clear();
    clearSceneCameras();
    removeEntity(m_sceneEntity, true);

    // Create new scene root
    m_sceneEntity = new Qt3DCore::QEntity();
    m_sceneEntity->setObjectName(tr("Scene root"));
    addEntity(m_sceneEntity);

    // Set up default scene
    setupDefaultScene();

    // Set other defaults
    setActiveSceneCameraIndex(0);
    m_freeView = true;
    resetFreeViewCamera();
    setFrameGraphCamera(m_freeViewCameraEntity);
    enableCameraCones(m_freeView);
    emit freeViewChanged(m_freeView);

    // Reset entity tree
    m_sceneModel->resetModel();
}

bool EditorScene::saveScene(const QUrl &fileUrl, bool autosave)
{
    Qt3DCore::QEntity *camera = Q_NULLPTR;
    if (m_activeSceneCameraIndex >= 0 && m_activeSceneCameraIndex < m_sceneCameras.size())
        camera = m_sceneCameras.at(m_activeSceneCameraIndex).entity;
    bool retval = m_sceneParser->exportScene(m_sceneEntity, fileUrl, camera, autosave);
    if (retval) {
        m_undoHandler->setClean();
    } else {
        m_errorString = tr("Failed to save the scene");
        emit errorChanged(m_errorString);
        qWarning() << m_errorString;
    }
    return retval;
}

bool EditorScene::loadScene(const QUrl &fileUrl)
{
    Qt3DCore::QEntity *camera = Q_NULLPTR;
    Qt3DCore::QEntity *newSceneEntity = m_sceneParser->importScene(fileUrl, camera);

    if (newSceneEntity) {
        if (!m_freeView)
            setFrameGraphCamera(Q_NULLPTR);
        m_undoHandler->clear();
        clearSceneCameras();
        removeEntity(m_sceneEntity, true);
        m_sceneEntity = newSceneEntity;
        addEntity(newSceneEntity);
        enableCameraCones(m_freeView);
        m_activeSceneCameraIndex--; // To force change
        setActiveSceneCameraIndex(cameraIndexForEntity(camera));

        // Enable/disable light meshes
        emit freeViewChanged(m_freeView);

        m_sceneModel->resetModel();
    } else {
        m_errorString = tr("Failed to load a new scene");
        emit errorChanged(m_errorString);
        qWarning() << m_errorString;
    }

    return bool(newSceneEntity);
}

void EditorScene::deleteScene(const QUrl &fileUrl, bool autosave)
{
    // Remove qml file
    QString fileName = fileUrl.toLocalFile();
    if (autosave)
        fileName.append(autoSavePostfix);
    QFile::remove(fileName);

    // Remove resource directory
    QString qmlFinalFileAbsoluteFilePath = fileUrl.toLocalFile();
    QFile qmlFinalFile(qmlFinalFileAbsoluteFilePath);
    QFileInfo qmlFinalFileInfo(qmlFinalFile);
    QString resourceDirName = qmlFinalFileInfo.baseName() + QStringLiteral("_scene_res");
    if (autosave)
        resourceDirName.append(autoSavePostfix);
    QDir dir = QDir(resourceDirName);
    dir.removeRecursively();
}

QString EditorScene::cameraName(int index) const
{
    if (m_sceneCameras.size() < index)
        return m_sceneCameras.at(index).entity->objectName();
    else
        return QString();
}

void EditorScene::resetFreeViewCamera()
{
    if (m_viewport)
        m_freeViewCameraEntity->setAspectRatio(m_viewport->width() / m_viewport->height());
    else
        m_freeViewCameraEntity->setAspectRatio(16.0f / 9.0f);
    m_freeViewCameraEntity->setBottom(-0.5f);
    m_freeViewCameraEntity->setFarPlane(1000.0f);
    m_freeViewCameraEntity->setFieldOfView(45.0f);
    m_freeViewCameraEntity->setLeft(-0.5f);
    m_freeViewCameraEntity->setNearPlane(0.1f);
    m_freeViewCameraEntity->setPosition(QVector3D(20.0f, 20.0f, 20.0f));
    m_freeViewCameraEntity->setProjectionType(Qt3DCore::QCameraLens::PerspectiveProjection);
    m_freeViewCameraEntity->setRight(0.5f);
    m_freeViewCameraEntity->setTop(0.5f);
    m_freeViewCameraEntity->setUpVector(QVector3D(0, 1, 0));
    m_freeViewCameraEntity->setViewCenter(QVector3D(0, 0, 0));
}

void EditorScene::copyFreeViewToNewSceneCamera()
{
    // Set the new scene camera to freeview camera position
    Qt3DCore::QCamera *newCam = qobject_cast<Qt3DCore::QCamera *>(m_sceneCameras.last().entity);
    copyCameraProperties(newCam, m_freeViewCameraEntity);
}

void EditorScene::moveActiveSceneCameraToFreeView()
{
    // Set the active scene camera to freeview camera position
    Qt3DCore::QCamera *newCam = qobject_cast<Qt3DCore::QCamera *>(
                m_sceneCameras.at(m_activeSceneCameraIndex).entity);
    copyCameraProperties(newCam, m_freeViewCameraEntity);
}

void EditorScene::snapFreeViewCameraToActiveSceneCamera()
{
    // Set the freeview camera position to the active scene camera position
    Qt3DCore::QCamera *activeCam = qobject_cast<Qt3DCore::QCamera *>(
                m_sceneCameras.at(m_activeSceneCameraIndex).entity);
    copyCameraProperties(m_freeViewCameraEntity, activeCam);
}

void EditorScene::duplicateEntity(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QEntity *newEntity = Q_NULLPTR;

    // Check if it's a camera
    if (qobject_cast<Qt3DCore::QCamera *>(entity)) {
        Qt3DCore::QCamera *newCam = new Qt3DCore::QCamera(m_sceneEntity);
        copyCameraProperties(newCam, qobject_cast<Qt3DCore::QCamera *>(entity));
        newEntity = newCam;
    } else {
        newEntity = new Qt3DCore::QEntity(m_sceneEntity);
        // Duplicate components
        Q_FOREACH (Qt3DCore::QComponent *component, entity->components()) {
            Qt3DCore::QComponent *newComponent = m_editorUtils->duplicateComponent(component,
                                                                                   newEntity,
                                                                                   m_sceneModel);
            if (newComponent)
                newEntity->addComponent(newComponent);
        }
    }

    // Set name and add to scene
    m_editorUtils->nameDuplicate(newEntity, entity, newEntity, m_sceneModel);
    addEntity(newEntity);

    // Refresh entity tree
    m_sceneModel->resetModel();
}

void EditorScene::enableCameraCones(bool enable)
{
    for (int i = 0; i < m_sceneCameras.size(); i++) {
        m_sceneCameras.at(i).cone->setEnabled(enable);
        // Odd that picker doesn't get disabled with the entity - we have to delete it to disable
        if (enable) {
            m_sceneCameras[i].picker = createObjectPickerForEntity(m_sceneCameras.at(i).cone);
        } else {
            delete m_sceneCameras.at(i).picker;
            m_sceneCameras[i].picker = Q_NULLPTR;
        }
    }
}

void EditorScene::clearSceneCameras()
{
    for (int i = 0; i < m_sceneCameras.size(); i++)
        delete m_sceneCameras.at(i).cone;
    m_sceneCameras.clear();
    m_activeSceneCameraIndex = -1;
    m_sceneCamerasModel.setStringList(QStringList());
}

Qt3DRender::QObjectPicker * EditorScene::createObjectPickerForEntity(Qt3DCore::QEntity *entity)
{
    Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(entity);
    picker->setHoverEnabled(true);
    picker->setObjectName(QStringLiteral("__internal object picker ") + entity->objectName());
    entity->addComponent(picker);
    connect(picker, &Qt3DRender::QObjectPicker::clicked, this, &EditorScene::handleClick);
    connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &EditorScene::handlePress);
    connect(picker, &Qt3DRender::QObjectPicker::released, this,
            &EditorScene::handleRelease);
    connect(picker, &Qt3DRender::QObjectPicker::entered, this, &EditorScene::handleEnter);

    return picker;
}

int EditorScene::cameraIndexForEntity(Qt3DCore::QEntity *entity)
{
    int index = -1;
    if (entity) {
        for (int i = 0; i < m_sceneCameras.size(); i++) {
            if (m_sceneCameras.at(i).entity == entity) {
                index = i;
                break;
            }
        }
    }
    return index;
}

Qt3DCore::QCameraLens *EditorScene::cameraLensForEntity(Qt3DCore::QEntity *entity)
{
    Qt3DCore::QComponentList components = entity->components();
    for (int i = 0; i < components.size(); i++) {
        Qt3DCore::QCameraLens *lens = qobject_cast<Qt3DCore::QCameraLens *>(components.value(i));
        if (lens)
            return lens;
    }

    return Q_NULLPTR;
}

void EditorScene::updateCameraConeMatrix(Qt3DCore::QTransform *sourceTransform,
                                         Qt3DCore::QTransform *coneTransform)
{
    QMatrix4x4 coneMat = calculateCameraConeMatrix(sourceTransform);
    coneTransform->setMatrix(coneMat);
}

void EditorScene::copyCameraProperties(Qt3DCore::QCamera *target, Qt3DCore::QCamera *source)
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

Qt3DRender::QMaterial *EditorScene::selectionBoxMaterial() const
{
    return m_selectionBoxMaterial;
}

Qt3DRender::QGeometryRenderer *EditorScene::selectionBoxMesh() const
{
    return m_selectionBoxMesh;
}

QMatrix4x4 EditorScene::calculateCameraConeMatrix(Qt3DCore::QTransform *sourceTransform) const
{
    QMatrix4x4 coneMat = QMatrix4x4();
    float x(0);
    float y(0);
    float z(0);
    float angle(0);
    sourceTransform->rotation().getAxisAndAngle(&x, &y, &z, &angle);
    coneMat.rotate(180, 0, 1, 0);
    coneMat.rotate(angle, x, -y, z);
    coneMat.translate(sourceTransform->translation());
    return coneMat;
}

bool EditorScene::isRemovable(Qt3DCore::QEntity *entity) const
{
    if (entity == m_sceneEntity || entity == m_rootEntity)
        return false;

    return true;
}

void EditorScene::setupDefaultScene()
{
    // NOTE: Do not add components to an entity after addEntity call.
#ifdef TEST_SCENE
    // Camera
    Qt3DCore::QCamera *sceneCameraEntity = new Qt3DCore::QCamera(m_sceneEntity);
    sceneCameraEntity->setObjectName(QStringLiteral("camera"));

    sceneCameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    sceneCameraEntity->setPosition(QVector3D(0, 0, -20.0f));
    sceneCameraEntity->setUpVector(QVector3D(0, 1, 0));
    sceneCameraEntity->setViewCenter(QVector3D(0, 0, 0));

    setFrameGraphCamera(sceneCameraEntity);
    addEntity(sceneCameraEntity);

    // Cylinder shape data
    Qt3DRender::QCylinderMesh *cylinder = new Qt3DRender::QCylinderMesh();
    cylinder->setRadius(1);
    cylinder->setLength(3);
    cylinder->setRings(100);
    cylinder->setSlices(20);

    // CylinderMesh Transform
    Qt3DCore::QTransform *cylinderTransform = new Qt3DCore::QTransform();
    cylinderTransform->setScale3D(QVector3D(1.5f, 1.5f, 1.5f));
    cylinderTransform->setRotation(QQuaternion::fromAxisAndAngle(
                                       QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
    cylinderTransform->setTranslation(QVector3D(-2.0f, -5.0f, 0.0f));

    // Cylinder 1
    Qt3DCore::QEntity *cylinderEntity = new Qt3DCore::QEntity(m_sceneEntity);
    cylinderEntity->setObjectName(QStringLiteral("cylinder 1"));
    cylinderEntity->addComponent(cylinder);
    cylinderEntity->addComponent(cylinderTransform);

    Qt3DRender::QPhongMaterial *mat = new Qt3DRender::QPhongMaterial();
    mat->setDiffuse(Qt::red);
    mat->setSpecular(Qt::white);
    mat->setShininess(150.0f);
    cylinderEntity->addComponent(mat);

    Qt3DCore::QTransform *cylinderTransform2 = new Qt3DCore::QTransform();
    cylinderTransform2->setTranslation(QVector3D(5.0f, 5.0f, 0.0f));

    // Cylinder 2
    Qt3DCore::QEntity *cylinderEntity2 = new Qt3DCore::QEntity(cylinderEntity);
    cylinderEntity2->setObjectName(QStringLiteral("cylinder 2"));
    cylinderEntity2->addComponent(cylinder);
    cylinderEntity2->addComponent(cylinderTransform2);
    addEntity(cylinderEntity);

    // Cube 1
    Qt3DCore::QEntity *cubeEntity1 = new Qt3DCore::QEntity(m_sceneEntity);
    cubeEntity1->setObjectName(QStringLiteral("Cube 1"));

    //Cube matrix transform
    QMatrix4x4 cubeMatrix;
    cubeMatrix.rotate(90.0f, 1.0f, 0.0f, 1.0f);
    cubeMatrix.scale(1.4f);
    cubeMatrix.translate(0.0f, -3.0f, -4.0f);
    Qt3DCore::QTransform *cubeTransform = new Qt3DCore::QTransform();
    cubeTransform->setMatrix(cubeMatrix);

    //Cube Mesh
    Qt3DRender::QCuboidMesh *cubeMesh = new Qt3DRender::QCuboidMesh();

    Qt3DRender::QNormalDiffuseSpecularMapMaterial *diffuseMat
            = new Qt3DRender::QNormalDiffuseSpecularMapMaterial();
    Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
    diffuseMat->diffuse()->addTextureImage(diffuseTextureImage);
    diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo.png")));
    Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
    diffuseMat->normal()->addTextureImage(normalTextureImage);
    normalTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_normal.png")));
    Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
    diffuseMat->specular()->addTextureImage(specularTextureImage);
    specularTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_specular.png")));
    //diffuseMat->setSpecular(Qt::white);
    diffuseMat->setAmbient(Qt::black);
    diffuseMat->setShininess(150.0f);

    cubeEntity1->addComponent(diffuseMat);
    cubeEntity1->addComponent(cubeTransform);
    cubeEntity1->addComponent(cubeMesh);
    addEntity(cubeEntity1);

    // Light
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_sceneEntity);
    lightEntity->setObjectName(QStringLiteral("Light 1"));
    Qt3DRender::QLight *light = new Qt3DRender::QLight(m_sceneEntity);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 10.0f, -10.0f));
    lightEntity->addComponent(light);
    lightEntity->addComponent(lightTransform);
    addEntity(lightEntity);

#else
    // Camera
    Qt3DCore::QCamera *sceneCameraEntity = new Qt3DCore::QCamera(m_sceneEntity);
    sceneCameraEntity->setObjectName(tr("Camera"));

    sceneCameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    sceneCameraEntity->setPosition(QVector3D(0, 0, -15.0f));
    sceneCameraEntity->setUpVector(QVector3D(0, 1, 0));
    sceneCameraEntity->setViewCenter(QVector3D(0, 0, 0));

    setFrameGraphCamera(sceneCameraEntity);
    addEntity(sceneCameraEntity);

    // Cube
    Qt3DCore::QEntity *cubeEntity = new Qt3DCore::QEntity(m_sceneEntity);
    cubeEntity->setObjectName(tr("Cube"));
    Qt3DRender::QCuboidMesh *cubeMesh = new Qt3DRender::QCuboidMesh();
    Qt3DCore::QTransform *cubeTransform = new Qt3DCore::QTransform();
    cubeTransform->setTranslation(QVector3D(0.0f, 0.0f, 5.0f));
    cubeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), 180.0f));
    Qt3DRender::QDiffuseSpecularMapMaterial *cubeMaterial
            = new Qt3DRender::QDiffuseSpecularMapMaterial();
    Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
    cubeMaterial->diffuse()->addTextureImage(diffuseTextureImage);
    diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo.png")));
    Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
    cubeMaterial->specular()->addTextureImage(specularTextureImage);
    specularTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_specular.png")));
    cubeMaterial->setAmbient(Qt::black);
    cubeMaterial->setShininess(150.0f);
    cubeEntity->addComponent(cubeMesh);
    cubeEntity->addComponent(cubeTransform);
    cubeEntity->addComponent(cubeMaterial);
    addEntity(cubeEntity);

    // Light
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_sceneEntity);
    lightEntity->setObjectName(tr("Light"));
    Qt3DRender::QLight *light = new Qt3DRender::QLight(m_sceneEntity);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 10.0f, -5.0f));
    lightEntity->addComponent(light);
    lightEntity->addComponent(lightTransform);
    addEntity(lightEntity);
#endif
    setActiveSceneCameraIndex(0);
    m_sceneModel->resetModel();
}

void EditorScene::createRootEntity()
{
    m_rootEntity = new Qt3DCore::QEntity();
    m_rootEntity->setObjectName(QStringLiteral("__internal root entity"));

    // Create a component cache for components that are needed after Load/New/possible other
    // reason for deleting scene root (m_sceneEntity)
    m_componentCache = new Qt3DCore::QEntity(m_rootEntity);
    m_componentCache->setObjectName("__internal component cache");
    m_componentCache->setEnabled(false);

    // Selection box material and mesh need to be created before any
    // EditorSceneItem are created
    Qt3DRender::QPhongAlphaMaterial *selectionBoxMaterial = new Qt3DRender::QPhongAlphaMaterial();
    selectionBoxMaterial->setAmbient(QColor(Qt::darkGray));
    selectionBoxMaterial->setDiffuse(QColor(Qt::black));
    selectionBoxMaterial->setSpecular(QColor(Qt::black));
    selectionBoxMaterial->setShininess(0);
    selectionBoxMaterial->setAlpha(0.2f);
    m_selectionBoxMaterial = selectionBoxMaterial;

    m_selectionBoxMesh = new Qt3DRender::QCuboidMesh();

    // Save to cache, as these are needed after Load/New
    m_componentCache->addComponent(m_selectionBoxMesh);
    m_componentCache->addComponent(m_selectionBoxMaterial);

    //    // TODO: Prototype is done with transparent cuboids, todo a proper wireframe material
    //    Qt3DRender::QMaterial *wireframeMaterial = new Qt3DRender::QMaterial();
    //    Qt3DRender::QEffect *wireframeEffect = new Qt3DRender::QEffect();
    //    Qt3DRender::QTechnique *wireframeTechnique = new Qt3DRender::QTechnique();
    //    Qt3DRender::QRenderPass *wireframeRenderPass = new Qt3DRender::QRenderPass();
    //    Qt3DRender::QShaderProgram *wireframeShader = new Qt3DRender::QShaderProgram();
    //    Qt3DRender::QBlendState *wireframeBlend = new Qt3DRender::QBlendState();
    //    Qt3DRender::QBlendEquation *wireframeBlendEq = new Qt3DRender::QBlendEquation();

    //    wireframeShader->setVertexShaderCode(Qt3DRender::QShaderProgram::loadSource(
    //                                        QUrl(QStringLiteral("qrc:/shaders/wireframe.vert"))));
    //    wireframeShader->setFragmentShaderCode(Qt3DRender::QShaderProgram::loadSource(
    //                                          QUrl(QStringLiteral("qrc:/shaders/wireframe.frag"))));

    //    wireframeTechnique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
    //    wireframeTechnique->graphicsApiFilter()->setMajorVersion(2);
    //    wireframeTechnique->graphicsApiFilter()->setMinorVersion(0);
    //    wireframeTechnique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::NoProfile);

    //    wireframeBlend->setSrcRGB(Qt3DRender::QBlendState::SrcAlpha);
    //    wireframeBlend->setDstRGB(Qt3DRender::QBlendState::OneMinusSrcAlpha);
    //    wireframeBlendEq->setMode(Qt3DRender::QBlendEquation::FuncAdd);

    //    wireframeRenderPass->setShaderProgram(wireframeShader);
    //    wireframeRenderPass->addRenderState(wireframeBlend);
    //    wireframeRenderPass->addRenderState(wireframeBlendEq);
    //    wireframeTechnique->addPass(wireframeRenderPass);
    //    wireframeEffect->addTechnique(wireframeTechnique);
    //    wireframeMaterial->setEffect(wireframeEffect);

    //    wireframeMaterial->addParameter(new Qt3DRender::QParameter(QStringLiteral("color"),
    //                                                               QColor(Qt::darkGray)));

    m_rootItem = new EditorSceneItem(this, m_rootEntity, Q_NULLPTR, -1, m_freeView, this);

    m_sceneItems.insert(m_rootEntity->id(), m_rootItem);

    m_frameGraph = new Qt3DRender::QFrameGraph();
    m_frameGraph->setObjectName(QStringLiteral("__internal Scene frame graph"));
    Qt3DRender::QForwardRenderer *forwardRenderer = new Qt3DRender::QForwardRenderer();

    forwardRenderer->setClearColor(Qt::lightGray);

    m_frameGraph->setActiveFrameGraph(forwardRenderer);

    // Setting the FrameGraph to actual root entity to protect it from accidental removal
    m_rootEntity->addComponent(m_frameGraph);

    // Scene entity (i.e. the visible root)
    m_sceneEntity = new Qt3DCore::QEntity();
    m_sceneEntity->setObjectName(tr("Scene root"));

    // Free view camera
    m_freeViewCameraEntity = new Qt3DCore::QCamera(m_rootEntity);
    m_freeViewCameraEntity->setObjectName(QStringLiteral("__internal free view camera"));
    resetFreeViewCamera();

    addEntity(m_sceneEntity);

    // Helper plane
    // Implemented as two identical planes in same position, with one rotated 180 degrees,
    // creating a two sided plane. Having a two sided material would be better, but it doesn't
    // seem to be easily achievable with current Qt3D implementation.
    m_helperPlane = new Qt3DCore::QEntity();
    Qt3DCore::QEntity *frontPlane = new Qt3DCore::QEntity(m_helperPlane);
    Qt3DCore::QEntity *backPlane = new Qt3DCore::QEntity(m_helperPlane);

    m_helperPlane->setObjectName(QStringLiteral("__internal helper plane"));
    frontPlane->setObjectName(QStringLiteral("__internal helper plane front"));
    backPlane->setObjectName(QStringLiteral("__internal helper plane back"));

    Qt3DCore::QTransform *flipTransform = new Qt3DCore::QTransform();
    flipTransform->setMatrix(Qt3DCore::QTransform::rotateAround(QVector3D(), 180.0f, QVector3D(1.0f, 0, 0)));
    backPlane->addComponent(flipTransform);

    int lineCount = 50;
    Qt3DRender::QGeometryRenderer *planeMesh = new Qt3DRender::QGeometryRenderer();
    Qt3DRender::QGeometry *planeGeometry = new Qt3DRender::QGeometry(planeMesh);
    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer,
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

    vertexDataBuffer->setData(vertexBufferData);

    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute();
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexDataBuffer);
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
    positionAttribute->setDataSize(3);
    positionAttribute->setByteOffset(0);
    positionAttribute->setByteStride(0);
    positionAttribute->setCount(lineCount * 4);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    planeGeometry->addAttribute(positionAttribute);

    planeMesh->setInstanceCount(1);
    planeMesh->setBaseVertex(0);
    planeMesh->setBaseInstance(0);
    planeMesh->setPrimitiveCount(lineCount * 4);
    planeMesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    planeMesh->setGeometry(planeGeometry);

    Qt3DRender::QPhongMaterial *helperPlaneMaterial = new Qt3DRender::QPhongMaterial();
    helperPlaneMaterial->setAmbient(QColor(Qt::darkGray));
    helperPlaneMaterial->setDiffuse(QColor(Qt::black));
    helperPlaneMaterial->setSpecular(QColor(Qt::black));
    helperPlaneMaterial->setShininess(0);

    m_helperPlaneTransform = new Qt3DCore::QTransform();
    m_helperPlaneTransform->setScale3D(QVector3D(100.0f, 100.0f, 1.0f));
    m_helperPlaneTransform->setRotation(
                m_helperPlaneTransform->fromAxisAndAngle(1.0f, 0.0f, 0.0f, 90.0f));
    frontPlane->addComponent(planeMesh);
    frontPlane->addComponent(helperPlaneMaterial);
    backPlane->addComponent(planeMesh);
    backPlane->addComponent(helperPlaneMaterial);
    m_helperPlane->addComponent(m_helperPlaneTransform);
    m_helperPlane->setParent(m_rootEntity);
}

void EditorScene::setFrameGraphCamera(Qt3DCore::QEntity *cameraEntity)
{
    Qt3DRender::QForwardRenderer *forwardRenderer =
            qobject_cast<Qt3DRender::QForwardRenderer *>(m_frameGraph->activeFrameGraph());
    if (forwardRenderer)
        forwardRenderer->setCamera(cameraEntity);
}

Qt3DCore::QEntity *EditorScene::frameGraphCamera() const
{
    Qt3DRender::QForwardRenderer *forwardRenderer =
            qobject_cast<Qt3DRender::QForwardRenderer *>(m_frameGraph->activeFrameGraph());
    if (forwardRenderer)
        return forwardRenderer->camera();
    else
        return Q_NULLPTR;
}

Qt3DCore::QEntity *EditorScene::selection() const
{
    return m_selectedEntity;
}

const QString &EditorScene::error() const
{
    return m_errorString;
}

void EditorScene::setActiveSceneCameraIndex(int index)
{
    int previousIndex = m_activeSceneCameraIndex;
    if (index >= 0 && index < m_sceneCameras.size())
        m_activeSceneCameraIndex = index;
    else if (m_sceneCameras.size())
        m_activeSceneCameraIndex = 0;
    else
        m_activeSceneCameraIndex = -1;

    // Reset camera even if index didn't change, as it might point to a different camera
    if (m_activeSceneCameraIndex >= 0) {
        if (!m_freeView)
            setFrameGraphCamera(m_sceneCameras.at(m_activeSceneCameraIndex).entity);
    } else {
        setFreeView(true);
    }

    if (previousIndex != m_activeSceneCameraIndex)
        emit activeSceneCameraIndexChanged(m_activeSceneCameraIndex);
}

int EditorScene::activeSceneCameraIndex() const
{
    return m_activeSceneCameraIndex;
}

void EditorScene::setFreeView(bool enable)
{
    // Force freeview if no active scene cameras available
    if (!enable && (m_activeSceneCameraIndex < 0 || m_activeSceneCameraIndex >= m_sceneCameras.size()))
        enable = true;

    if (m_freeView != enable) {
        m_freeView = enable;

        // Set free view when trying to change to invalid camera
        if (m_freeView)
            setFrameGraphCamera(m_freeViewCameraEntity);
        else
            setFrameGraphCamera(m_sceneCameras.at(m_activeSceneCameraIndex).entity);
        enableCameraCones(m_freeView);
    }
    // Show / hide light meshes, and notify UI. Need to be emitted always even if it doesn't change,
    // as otherwise the UI can change the checked status of the menu item on click even if
    // the status doesn't really change.
    emit freeViewChanged(m_freeView);
}

bool EditorScene::freeView() const
{
    return m_freeView;
}

void EditorScene::setViewport(EditorViewportItem *viewport)
{
    if (m_viewport != viewport) {
        if (m_viewport)
            disconnect(m_viewport, 0, this, 0);

        m_viewport = viewport;
        connect(viewport, &EditorViewportItem::heightChanged,
                this, &EditorScene::handleViewportSizeChange);
        connect(viewport, &EditorViewportItem::widthChanged,
                this, &EditorScene::handleViewportSizeChange);
        handleViewportSizeChange();
        emit viewportChanged(viewport);
    }
}

EditorViewportItem *EditorScene::viewport() const
{
    return m_viewport;
}

QAbstractItemModel *EditorScene::sceneCamerasModel()
{
    return &m_sceneCamerasModel;
}

UndoHandler *EditorScene::undoHandler()
{
    return m_undoHandler;
}

Qt3DCore::QEntity *EditorScene::helperPlane() const
{
    return m_helperPlane;
}

Qt3DCore::QTransform *EditorScene::helperPlaneTransform() const
{
    return m_helperPlaneTransform;
}

void EditorScene::clearSelectionBoxes()
{
    Q_FOREACH (EditorSceneItem *item, m_sceneItems.values())
        item->setShowSelectionBox(false);
}

void EditorScene::handleClick(Qt3DRender::QPickEvent *event)
{
    Q_UNUSED(event)
    //qDebug() << __FUNCTION__ << sender() << sender()->parent();
}

void EditorScene::handlePress(Qt3DRender::QPickEvent *event)
{
    Q_UNUSED(event)
    //qDebug() << __FUNCTION__ << sender() << sender()->parent();
    if (!m_activeSelection) {
        m_activeSelection = true;
        Qt3DCore::QEntity *selectedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        if (selectedEntity) {
            // Check if it's the camera cone. Select camera if it is.
            if (m_freeView && (selectedEntity->objectName() == cameraConeName)) {
                for (int i = 0; i < m_sceneCameras.size(); i++) {
                    if (m_sceneCameras.at(i).cone == selectedEntity) {
                        selectedEntity = m_sceneCameras.at(i).entity;
                        break;
                    }
                }
            }
        }

        if (selectedEntity) {
            // Emit signal to highlight the entity from the list
            m_selectedEntity = selectedEntity;
            emit selectionChanged(m_selectedEntity);
        }
    }
}

void EditorScene::handleRelease(Qt3DRender::QPickEvent *event)
{
    Q_UNUSED(event)
    //qDebug() << __FUNCTION__ << sender() << sender()->parent();
    m_activeSelection = false;
}

void EditorScene::handleEnter()
{
    //qDebug() << __FUNCTION__ << sender() << sender()->parent();
}

void EditorScene::handleExit()
{
    //qDebug() << __FUNCTION__ << sender() << sender()->parent();
}

void EditorScene::handleCameraAdded(Qt3DCore::QEntity *camera)
{
    Qt3DCore::QEntity *cameraCone = new Qt3DCore::QEntity(m_rootEntity);
    cameraCone->setObjectName(cameraConeName);
    Qt3DRender::QMesh *cone = new Qt3DRender::QMesh();
    cone->setSource(QUrl("qrc:meshes/cameracone.obj"));
    cone->setObjectName(QStringLiteral("__internal camera mesh"));
    Qt3DRender::QPhongAlphaMaterial *coneMaterial = new Qt3DRender::QPhongAlphaMaterial();
    coneMaterial->setObjectName(QStringLiteral("__internal camera material"));
    coneMaterial->setDiffuse(Qt::white);
    coneMaterial->setAlpha(0.9f);
    Qt3DCore::QTransform *coneTransform = new Qt3DCore::QTransform();
    coneTransform->setObjectName(QStringLiteral("__internal camera transform"));

    cameraCone->setEnabled(m_freeView);
    cameraCone->addComponent(cone);
    cameraCone->addComponent(coneMaterial);
    cameraCone->addComponent(coneTransform);

    CameraData newData(camera, cameraCone, coneTransform, cameraLensForEntity(camera), Q_NULLPTR);
    m_sceneCameras.append(newData);
    handleCameraTransformChange(camera);

    int newRow = m_sceneCamerasModel.rowCount();
    m_sceneCamerasModel.insertRow(newRow);
    m_sceneCamerasModel.setData(m_sceneCamerasModel.index(newRow),
                                QVariant::fromValue(camera->objectName()),
                                Qt::DisplayRole);

    // Activate the newly added camera if it is the only existing scene camera
    if (m_sceneCameras.size() == 1)
        setActiveSceneCameraIndex(0);
}

void EditorScene::handleCameraRemoved(Qt3DCore::QEntity *camera)
{
    int removeIndex = cameraIndexForEntity(camera);

    if (removeIndex >= 0) {
        delete m_sceneCameras.at(removeIndex).cone;
        m_sceneCameras.removeAt(removeIndex);
        m_sceneCamerasModel.removeRow(removeIndex);
        if (removeIndex <= m_activeSceneCameraIndex)
            setActiveSceneCameraIndex(m_activeSceneCameraIndex - 1);
    }
}

void EditorScene::handleCameraTransformChange(Qt3DCore::QEntity *camera)
{
    int changedIndex = cameraIndexForEntity(camera);

    if (changedIndex >= 0) {
        Qt3DCore::QComponentList components = camera->components();
        Qt3DCore::QTransform *transform = Q_NULLPTR;
        for (int i = 0; i < components.size(); i++) {
            transform = qobject_cast<Qt3DCore::QTransform *>(components.value(i));
            if (transform)
                break;
        }
        if (transform) {
            connect(transform, &Qt3DCore::QTransform::matrixChanged,
                    this, &EditorScene::handleCameraMatrixChange);
            connect(transform, &Qt3DCore::QTransform::rotationChanged,
                    this, &EditorScene::handleCameraMatrixChange);
            connect(transform, &Qt3DCore::QTransform::translationChanged,
                    this, &EditorScene::handleCameraMatrixChange);
            updateCameraConeMatrix(transform, m_sceneCameras[changedIndex].coneTransform);
        } else {
            Qt3DCore::QTransform dummyTransform;
            updateCameraConeMatrix(&dummyTransform, m_sceneCameras[changedIndex].coneTransform);
        }
    }
}

void EditorScene::handleCameraMatrixChange()
{
    Qt3DCore::QTransform *transform = qobject_cast<Qt3DCore::QTransform *>(sender());
    if (transform) {
        QVector<Qt3DCore::QEntity *> entities = transform->entities();
        // Note: This works only when transform is used for single entity
        if (entities.size()) {
            int changedIndex = cameraIndexForEntity(entities.at(0));
            if (changedIndex >= 0)
                updateCameraConeMatrix(transform, m_sceneCameras[changedIndex].coneTransform);
        }
    }
}

void EditorScene::handleViewportSizeChange()
{
    qreal aspectRatio = m_viewport->width() / m_viewport->height();
    m_freeViewCameraEntity->lens()->setPerspectiveProjection(
                45.0f, aspectRatio, 0.1f, 1000.0f);
}

void EditorScene::handleEntityNameChange()
{
    Qt3DCore::QEntity *entity = qobject_cast<Qt3DCore::QEntity *>(sender());
    int cameraIndex = cameraIndexForEntity(entity);
    if (cameraIndex >= 0) {
        m_sceneCamerasModel.setData(m_sceneCamerasModel.index(cameraIndex),
                                    QVariant::fromValue(entity->objectName()),
                                    Qt::DisplayRole);
    }
}

bool EditorScene::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    // Filter undo and redo keysequences so TextFields don't get them
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke == QKeySequence::Redo) {
            if (m_undoHandler->canRedo())
                m_undoHandler->redo();
            return true;
        } else  if (ke == QKeySequence::Undo) {
            if (m_undoHandler->canUndo())
                m_undoHandler->undo();
            return true;
        }
    }
    return false;
}
