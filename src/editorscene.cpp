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
#include "editorsceneparser.h"
#include "editorsceneitemcomponentsmodel.h"
#include "editorviewportitem.h"
#include "undohandler.h"
#include "draghandleeffect.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCuboidMesh>
#include <Qt3DRender/QDiffuseSpecularMapMaterial>
#include <Qt3DRender/QPhongAlphaMaterial>
#include <Qt3DRender/QPhongMaterial>
#include <Qt3DRender/QLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QForwardRenderer>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QPickingSettings>

#include <Qt3DInput/QInputSettings>

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/QKeySequence>

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QtMath>

#include <QtQml/QQmlEngine>

//#define TEST_SCENE // If a test scene is wanted instead of the default scene

#ifdef TEST_SCENE
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QNormalDiffuseSpecularMapMaterial>
#endif

static const QString cameraVisibleEntityName = QStringLiteral("__internal camera visible entity");
static const QString lightVisibleEntityName = QStringLiteral("__internal light visible entity");
static const QString autoSavePostfix = QStringLiteral(".autosave");
static const QVector3D defaultLightDirection(0.0f, -1.0f, 0.0f);
static const float freeViewCameraNearPlane = 0.1f;
static const float freeViewCameraFarPlane = 10000.0f;
static const float freeViewCameraFov = 45.0f;

EditorScene::EditorScene(QObject *parent)
    : QObject(parent)
    , m_rootEntity(Q_NULLPTR)
    , m_componentCache(Q_NULLPTR)
    , m_rootItem(Q_NULLPTR)
    , m_sceneModel(new EditorSceneItemModel(this))
    , m_sceneParser(new EditorSceneParser(this))
    , m_renderSettings(Q_NULLPTR)
    , m_renderer(Q_NULLPTR)
    , m_sceneEntity(Q_NULLPTR)
    , m_sceneEntityItem(Q_NULLPTR)
    , m_selectedEntity(Q_NULLPTR)
    , m_selectedEntityTransform(Q_NULLPTR)
    , m_activeSceneCameraIndex(-1)
    , m_freeView(false)
    , m_freeViewCameraEntity(Q_NULLPTR)
    , m_viewport(Q_NULLPTR)
    , m_undoHandler(new UndoHandler(this))
    , m_helperPlane(Q_NULLPTR)
    , m_helperPlaneTransform(Q_NULLPTR)
    , m_qtTranslator(new QTranslator(this))
    , m_appTranslator(new QTranslator(this))
    , m_dragMode(DragNone)
    , m_dragEntity(Q_NULLPTR)
    , m_ignoringInitialDrag(true)
    , m_viewCenterLocked(false)
    , m_pickedEntity(Q_NULLPTR)
    , m_pickedDistance(-1.0f)
    , m_gridSize(3)
    , m_duplicateCount(0)
{
    retranslateUi();
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
    removeEntity(m_sceneEntity);

    // TODO: Check if it is necessary to delete rootentity and associated components, or do they get
    // TODO: properly deleted by aspect engine shutdown?

    delete m_componentCache;
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
        EditorSceneItem *item = new EditorSceneItem(this, entity,
                                                    m_sceneItems.value(entity->parentEntity()->id(),
                                                                       Q_NULLPTR), index, this);

        if (entity == m_sceneEntity)
            m_sceneEntityItem = item;

        m_sceneItems.insert(entity->id(), item);
        connect(entity, &QObject::objectNameChanged,
                this, &EditorScene::handleEntityNameChange);

        Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(entity);
        if (camera)
            handleCameraAdded(camera);
        else if (item->itemType() == EditorSceneItem::Light)
            handleLightAdded(entity);
        else
            createObjectPickerForEntity(entity);

        item->componentsModel()->initializeModel();
    }

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

// Removed entity is deleted
void EditorScene::removeEntity(Qt3DCore::QEntity *entity)
{
    if (entity == Q_NULLPTR || entity == m_rootEntity)
        return;

    if (entity == m_sceneEntity) {
        m_sceneEntity = Q_NULLPTR;
        m_sceneEntityItem = Q_NULLPTR;
    }

    disconnect(entity, 0, this, 0);

    foreach (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        // Don't deparent child entities to preserve removed entity tree
        removeEntity(childEntity);
    }

    Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(entity);
    if (camera)
        handleCameraRemoved(camera);

    EditorSceneItem *item = m_sceneItems.value(entity->id());
    if (item && item->itemType() == EditorSceneItem::Light)
        handleLightRemoved(entity);

    m_sceneItems.remove(entity->id());
    delete item;
    delete entity;
}

void EditorScene::resetScene()
{
    m_selectedEntity = Q_NULLPTR;
    // Clear the existing scene
    setFrameGraphCamera(Q_NULLPTR);
    m_undoHandler->clear();
    clearSceneCamerasAndLights();
    removeEntity(m_sceneEntity);

    // Create new scene root
    m_sceneEntity = new Qt3DCore::QEntity();
    m_sceneEntity->setObjectName(m_sceneRootString);
    addEntity(m_sceneEntity);

    // Set up default scene
    setupDefaultScene();

    // Set other defaults
    setActiveSceneCameraIndex(0);
    m_freeView = true;
    resetFreeViewCamera();
    setFrameGraphCamera(m_freeViewCameraEntity);
    enableVisibleCameras(m_freeView);
    enableVisibleLights(m_freeView);

    emit freeViewChanged(m_freeView);

    // Reset entity tree
    m_sceneModel->resetModel();

    setSelection(m_sceneEntity);
}

bool EditorScene::saveScene(const QUrl &fileUrl, bool autosave)
{
    Qt3DCore::QEntity *camera = Q_NULLPTR;
    if (m_activeSceneCameraIndex >= 0 && m_activeSceneCameraIndex < m_sceneCameras.size())
        camera = m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity;
    bool retval = m_sceneParser->exportQmlScene(m_sceneEntity, fileUrl, camera, autosave);
    if (retval) {
        m_undoHandler->setClean();
    } else {
        m_errorString = m_saveFailString;
        emit errorChanged(m_errorString);
        qWarning() << m_errorString;
    }
    return retval;
}

bool EditorScene::loadScene(const QUrl &fileUrl)
{
    Qt3DCore::QEntity *camera = Q_NULLPTR;
    Qt3DCore::QEntity *newSceneEntity = m_sceneParser->importQmlScene(fileUrl, camera);

    if (newSceneEntity) {
        m_selectedEntity = Q_NULLPTR;
        if (!m_freeView)
            setFrameGraphCamera(Q_NULLPTR);
        m_undoHandler->clear();
        clearSceneCamerasAndLights();
        removeEntity(m_sceneEntity);
        m_sceneEntity = newSceneEntity;
        addEntity(newSceneEntity);
        enableVisibleCameras(m_freeView);
        enableVisibleLights(m_freeView);
        m_activeSceneCameraIndex--; // To force change
        setActiveSceneCameraIndex(cameraIndexForEntity(camera));

        m_sceneModel->resetModel();
    } else {
        m_errorString = m_loadFailString;
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

void EditorScene::importEntity(const QUrl &fileUrl)
{
    // TODO: Scene loading doesn't work, pending QTBUG-51577
    Qt3DCore::QEntity *sceneLoaderEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QSceneLoader *sceneLoader = new Qt3DRender::QSceneLoader(sceneLoaderEntity);
    QObject::connect(sceneLoader, &Qt3DRender::QSceneLoader::statusChanged,
                     this, &EditorScene::handleSceneLoaderStatusChanged);
    sceneLoader->setSource(fileUrl);
    sceneLoaderEntity->addComponent(sceneLoader);
}

QString EditorScene::cameraName(int index) const
{
    if (m_sceneCameras.size() < index)
        return m_sceneCameras.at(index).cameraEntity->objectName();
    else
        return QString();
}

void EditorScene::resetFreeViewCamera()
{
    if (m_viewport)
        m_freeViewCameraEntity->setAspectRatio(m_viewport->width() / qMax(m_viewport->height(), 1.0));
    else
        m_freeViewCameraEntity->setAspectRatio(16.0f / 9.0f);
    m_freeViewCameraEntity->setBottom(-0.5f);
    m_freeViewCameraEntity->setFarPlane(freeViewCameraFarPlane);
    m_freeViewCameraEntity->setFieldOfView(freeViewCameraFov);
    m_freeViewCameraEntity->setLeft(-0.5f);
    m_freeViewCameraEntity->setNearPlane(freeViewCameraNearPlane);
    m_freeViewCameraEntity->setPosition(QVector3D(20.0f, 20.0f, 20.0f));
    m_freeViewCameraEntity->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
    m_freeViewCameraEntity->setRight(0.5f);
    m_freeViewCameraEntity->setTop(0.5f);
    m_freeViewCameraEntity->setUpVector(QVector3D(0, 1, 0));
    m_freeViewCameraEntity->setViewCenter(QVector3D(0, 0, 0));
}

void EditorScene::copyFreeViewToNewSceneCamera()
{
    // Set the new scene camera to freeview camera position
    Qt3DRender::QCamera *newCam = qobject_cast<Qt3DRender::QCamera *>(m_sceneCameras.last().cameraEntity);
    EditorUtils::copyCameraProperties(newCam, m_freeViewCameraEntity);
}

void EditorScene::moveActiveSceneCameraToFreeView()
{
    // Set the active scene camera to freeview camera position
    Qt3DRender::QCamera *newCam = qobject_cast<Qt3DRender::QCamera *>(
                m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity);
    EditorUtils::copyCameraProperties(newCam, m_freeViewCameraEntity);
}

void EditorScene::snapFreeViewCameraToActiveSceneCamera()
{
    // Set the freeview camera position and viewCenter to the active scene camera values
    Qt3DRender::QCamera *activeCam = m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity;
    m_freeViewCameraEntity->setViewCenter(activeCam->viewCenter());
    m_freeViewCameraEntity->setPosition(activeCam->position());
    // Need to reset upVector as well, as camera controls will keep updating it to actual
    // value, which won't work anymore if you move both camera viewCenter and position.
    m_freeViewCameraEntity->setUpVector(QVector3D(0, 1, 0));
}

void EditorScene::duplicateEntity(Qt3DCore::QEntity *entity)
{
    QVector3D duplicateOffset =
            m_helperPlaneTransform->rotation().rotatedVector(QVector3D(0.5f, 0.5f, 0.0f)
                                                             * ++m_duplicateCount);

    Qt3DCore::QEntity *newEntity =
            EditorUtils::duplicateEntity(entity, m_sceneEntity, duplicateOffset);

    // Set name and add to scene
    EditorUtils::nameDuplicate(newEntity, entity, m_sceneModel);
    addEntity(newEntity);

    // Refresh entity tree
    m_sceneModel->resetModel();
}

// Resolves a world position for given viewport position.
// The world position is the intersection of the eye ray at specified position and the active
// helper plane. If there is no intersection, (0, 0, 0) position is returned.
QVector3D EditorScene::getWorldPosition(int xPos, int yPos)
{
    QVector3D retVec;
    if (xPos >= 0 && yPos >= 0 && xPos < m_viewport->width() && yPos < m_viewport->height()) {
        QPoint pos(xPos, yPos);
        Qt3DRender::QCamera *camera = frameGraphCamera();
        if (camera) {
            QVector3D planeOrigin;
            QVector3D planeNormal = helperPlaneNormal();
            float cosAngle = QVector3D::dotProduct(planeOrigin.normalized(), planeNormal);
            float planeOffset = planeOrigin.length() * cosAngle;

            QVector3D ray = EditorUtils::unprojectRay(camera->viewMatrix(), camera->projectionMatrix(),
                                                      m_viewport->width(), m_viewport->height(),
                                                      pos);
            float t = 0.0f;
            QVector3D intersection = EditorUtils::findIntersection(camera->position(), ray,
                                                                   planeOffset, planeNormal, t);
            if (t > camera->nearPlane())
                retVec = intersection;
        }
    }

    return retVec;
}

// For some reason EditorUtils::InsertableEntities doesn't work as parameter type from QML here,
// so we use int and cast it.
void EditorScene::showPlaceholderEntity(const QString &name, int type)
{
    PlaceholderEntityData *data = m_placeholderEntityMap.value(name);
    if (!data) {
        data = new PlaceholderEntityData();
        data->entity = new Qt3DCore::QEntity(m_rootEntity);
        data->transform = new Qt3DCore::QTransform();
        Qt3DRender::QPhongAlphaMaterial *material = new Qt3DRender::QPhongAlphaMaterial();
        material->setAlpha(0.4f);
        material->setAmbient(Qt::blue);
        data->material = material;
        data->entity->addComponent(data->transform);
        data->entity->addComponent(material);
        m_placeholderEntityMap.insert(name, data);
    }

    EditorUtils::InsertableEntities insertableType = EditorUtils::InsertableEntities(type);
    if (data->type != insertableType) {
        data->type = insertableType;
        delete data->mesh;
        data->mesh = EditorUtils::createMeshForInsertableType(insertableType);
        if (!data->mesh) {
            if (insertableType == EditorUtils::LightEntity)
                data->mesh = EditorUtils::createLightMesh(EditorUtils::LightBasic);
            else if (insertableType == EditorUtils::CameraEntity)
                data->mesh = EditorUtils::createVisibleCameraMesh();
        }
        if (data->mesh)
            data->entity->addComponent(data->mesh);
    }

    data->transform->setTranslation(QVector3D());
    data->entity->setEnabled(true);
}

void EditorScene::movePlaceholderEntity(const QString &name, const QVector3D &worldPos)
{
    PlaceholderEntityData *data = m_placeholderEntityMap.value(name);
    if (data)
        data->transform->setTranslation(worldPos);
}

void EditorScene::hidePlaceholderEntity(const QString &name)
{
    PlaceholderEntityData *data = m_placeholderEntityMap.value(name);
    if (data)
        data->entity->setEnabled(false);
}

void EditorScene::destroyPlaceholderEntity(const QString &name)
{
    PlaceholderEntityData *data = m_placeholderEntityMap.value(name);
    if (data) {
        delete data->entity;
        delete data;
    }
}

int EditorScene::gridSize() const
{
    return m_gridSize;
}

void EditorScene::setGridSize(int size)
{
    if (m_gridSize != size) {
        delete m_helperPlane;
        m_gridSize = size;
        createHelperPlane();
        emit gridSizeChanged(size);
    }
}

const QString EditorScene::language() const
{
    if (m_language.isEmpty())
        return QLocale::system().name().left(2);
    else
        return m_language;
}

void EditorScene::setLanguage(const QString &language)
{
    if (!m_qtTranslator->isEmpty())
        QCoreApplication::removeTranslator(m_qtTranslator);
    if (!m_appTranslator->isEmpty())
        QCoreApplication::removeTranslator(m_appTranslator);

    if (m_qtTranslator->load("qt_" + language,
                             QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QCoreApplication::installTranslator(m_qtTranslator);
    }

    if (m_appTranslator->load(":/qt3dsceneeditor_" + language)) {
        QCoreApplication::installTranslator(m_appTranslator);
        m_language = language;
    } else {
        m_language = "C";
    }

    emit languageChanged(m_language);
    emit translationChanged("");
    retranslateUi();
}

void EditorScene::retranslateUi()
{
    m_sceneRootString = tr("Scene root");
    m_saveFailString = tr("Failed to save the scene");
    m_loadFailString = tr("Failed to load a new scene");
    m_importFailString = tr("Failed to import an Entity");
    m_cameraString = tr("Camera");
    m_cubeString = tr("Cube");
    m_lightString = tr("Light");
}

const QString EditorScene::emptyString() const
{
    return QStringLiteral("");
}

void EditorScene::enableVisibleCameras(bool enable)
{
    for (int i = 0; i < m_sceneCameras.size(); i++)
        enableVisibleCamera(m_sceneCameras[i], enable, i == m_activeSceneCameraIndex);
}

void EditorScene::enableVisibleCamera(EditorScene::CameraData &cameraData,
                                      bool enable, bool isActiveCamera)
{
    enable = enable && cameraData.cameraEntity->isEnabled();
    cameraData.visibleEntity->setEnabled(enable);

    if (isActiveCamera) {
        m_activeSceneCameraFrustumData.frustumEntity->setEnabled(enable);
        m_activeSceneCameraFrustumData.viewCenterEntity->setEnabled(enable);
        m_activeSceneCameraFrustumData.viewVectorEntity->setEnabled(enable);
        if (enable) {
            if (!m_activeSceneCameraFrustumData.viewCenterPicker) {
                m_activeSceneCameraFrustumData.viewCenterPicker =
                        createObjectPickerForEntity(m_activeSceneCameraFrustumData.viewCenterEntity);
            }
        } else {
            delete m_activeSceneCameraFrustumData.viewCenterPicker;
            m_activeSceneCameraFrustumData.viewCenterPicker = Q_NULLPTR;
        }
    }

    // Picker doesn't get disabled with the entity - we have to delete it to disable
    if (enable) {
        if (!cameraData.cameraPicker)
            cameraData.cameraPicker = createObjectPickerForEntity(cameraData.visibleEntity);
    } else {
        delete cameraData.cameraPicker;
        cameraData.cameraPicker = Q_NULLPTR;
    }
}

void EditorScene::enableVisibleLights(bool enable)
{
    Q_FOREACH (LightData *lightData, m_sceneLights.values())
        enableVisibleLight(*lightData, enable);
}

void EditorScene::enableVisibleLight(EditorScene::LightData &lightData, bool enable)
{
    enable = enable && lightData.lightEntity->isEnabled();
    lightData.visibleEntity->setEnabled(enable);

    // Picker doesn't get disabled with the entity - we have to delete it to disable
    if (enable) {
        if (!lightData.visiblePicker)
            lightData.visiblePicker = createObjectPickerForEntity(lightData.visibleEntity);
    } else {
        delete lightData.visiblePicker;
        lightData.visiblePicker = Q_NULLPTR;
    }
}

void EditorScene::clearSceneCamerasAndLights()
{
    Q_FOREACH (LightData *lightData, m_sceneLights.values()) {
        delete lightData->visibleEntity;
        delete lightData;
    }
    m_sceneLights.clear();

    for (int i = 0; i < m_sceneCameras.size(); i++)
        delete m_sceneCameras.at(i).visibleEntity;
    m_sceneCameras.clear();

    m_activeSceneCameraIndex = -1;
    m_sceneCamerasModel.setStringList(QStringList());
}

Qt3DRender::QObjectPicker * EditorScene::createObjectPickerForEntity(Qt3DCore::QEntity *entity)
{
    Qt3DRender::QObjectPicker *picker = new Qt3DRender::QObjectPicker(entity);
    picker->setHoverEnabled(false);
    picker->setObjectName(QStringLiteral("__internal object picker ") + entity->objectName());
    entity->addComponent(picker);
    connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &EditorScene::handlePickerPress);

    return picker;
}

int EditorScene::cameraIndexForEntity(Qt3DCore::QEntity *entity)
{
    int index = -1;
    if (entity) {
        for (int i = 0; i < m_sceneCameras.size(); i++) {
            if (m_sceneCameras.at(i).cameraEntity == entity) {
                index = i;
                break;
            }
        }
    }
    return index;
}

void EditorScene::updateVisibleSceneCameraMatrix(const EditorScene::CameraData &cameraData)
{
    QMatrix4x4 matrix = calculateVisibleSceneCameraMatrix(cameraData.cameraEntity);
    cameraData.visibleTransform->setMatrix(matrix);

    if (m_activeSceneCameraIndex >= 0
            && cameraData.cameraEntity == m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity) {
        m_activeSceneCameraFrustumData.viewVectorTransform->setScale3D(
                    QVector3D(1.0f, 1.0f, cameraData.cameraEntity->viewVector().length()));
        m_activeSceneCameraFrustumData.viewVectorTransform->setTranslation(
                    cameraData.cameraEntity->position());
        m_activeSceneCameraFrustumData.viewVectorTransform->setRotation(
                    cameraData.visibleTransform->rotation());

        EditorUtils::updateCameraFrustumMesh(m_activeSceneCameraFrustumData.frustumMesh,
                                             cameraData.cameraEntity);
        m_activeSceneCameraFrustumData.frustumTransform->setTranslation(
                    cameraData.cameraEntity->position());
        m_activeSceneCameraFrustumData.frustumTransform->setRotation(
                    cameraData.visibleTransform->rotation());

        m_activeSceneCameraFrustumData.viewCenterTransform->setTranslation(
                    cameraData.cameraEntity->viewCenter());
        resizeCameraViewCenterEntity();
    }
}

void EditorScene::connectDragHandles(EditorSceneItem *item, bool enable)
{
    if (item) {
        if (enable) {
            connect(item, &EditorSceneItem::selectionBoxTransformChanged,
                    this, &EditorScene::handleSelectionTransformChange);
        } else {
            disconnect(item, &EditorSceneItem::selectionBoxTransformChanged,
                       this, &EditorScene::handleSelectionTransformChange);
        }
    }
}

void EditorScene::dragTranslateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown)
{
    Q_UNUSED(ctrlDown)

    // By default, translate along helper plane
    // When shift is pressed, translate along camera plane
    // When ctrl is pressed, snap to grid

    Qt3DRender::QCamera *camera = frameGraphCamera();
    if (camera && m_selectedEntityTransform) {
        // For cameras, we need to use position instead of translation for correct results
        QVector3D entityTranslation = m_selectedEntityTransform->translation();
        Qt3DRender::QCamera *cameraEntity = qobject_cast<Qt3DRender::QCamera *>(m_selectedEntity);
        if (cameraEntity) {
            if (m_cameraViewCenterSelected)
                entityTranslation = cameraEntity->viewCenter();
            else
                entityTranslation = cameraEntity->position();
        }

        QVector3D planeOrigin = m_dragInitialTranslationValue;
        QVector3D planeNormal;
        if (shiftDown)
            planeNormal = frameGraphCameraNormal();
        else
            planeNormal = helperPlaneNormal();

        float cosAngle = QVector3D::dotProduct(planeOrigin.normalized(), planeNormal);
        float planeOffset = planeOrigin.length() * cosAngle;

        QVector3D ray = EditorUtils::unprojectRay(camera->viewMatrix(), camera->projectionMatrix(),
                                                  m_viewport->width(), m_viewport->height(),
                                                  newPos);
        float t = 0.0f;
        QVector3D intersection = EditorUtils::findIntersection(camera->position(), ray,
                                                               planeOffset, planeNormal, t);
        if (t > camera->nearPlane()) {
            EditorSceneItemComponentsModel::EditorSceneItemComponentTypes componentType =
                    EditorSceneItemComponentsModel::Transform;
            QString propertyName;

            if (cameraEntity) {
                componentType = EditorSceneItemComponentsModel::CameraEntity;
                if (m_cameraViewCenterSelected)
                    propertyName = QStringLiteral("viewCenter");
                else
                    propertyName = QStringLiteral("position");
            } else {
                propertyName = QStringLiteral("translation");
            }

            // If entity has parents with transfroms, those need to be applied in inverse
            QMatrix4x4 totalTransform = EditorUtils::totalAncestralTransform(m_selectedEntity);
            intersection = totalTransform.inverted() * intersection;

            if (ctrlDown) {
                m_snapToGridIntersection.setX(qRound(intersection.x() / m_gridSize) * m_gridSize);
                m_snapToGridIntersection.setY(qRound(intersection.y() / m_gridSize) * m_gridSize);
                m_snapToGridIntersection.setZ(qRound(intersection.z() / m_gridSize) * m_gridSize);
            } else {
                m_snapToGridIntersection = intersection;
            }

            m_undoHandler->createChangePropertyCommand(m_selectedEntity->objectName(), componentType,
                                                       propertyName, m_snapToGridIntersection,
                                                       entityTranslation, true);
        }
    }
}

void EditorScene::dragScaleSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown)
{
    // By default, scale each dimension individually
    // When shift is pressed, scale uniformly
    // When ctrl is pressed, scale in integers

    QVector3D posOffset = dragHandlePositionOffset(newPos);
    if (!posOffset.isNull()) {
        QVector3D moveFactors =
                EditorUtils::absVector3D(
                    QVector3D(m_dragInitialHandleCornerTranslation
                              + (m_dragHandles.transform->rotation().inverted() * posOffset)));

        // Divide by zero may cause an INFINITY. Fix it.
        if (m_dragInitialHandleCornerTranslation.x() != 0.0f)
            moveFactors.setX(moveFactors.x() / qAbs(m_dragInitialHandleCornerTranslation.x()));
        else
            moveFactors.setX(1.0f);
        if (m_dragInitialHandleCornerTranslation.y() != 0.0f)
            moveFactors.setY(moveFactors.y() / qAbs(m_dragInitialHandleCornerTranslation.y()));
        else
            moveFactors.setY(1.0f);
        if (m_dragInitialHandleCornerTranslation.z() != 0.0f)
            moveFactors.setZ(moveFactors.z() / qAbs(m_dragInitialHandleCornerTranslation.z()));
        else
            moveFactors.setZ(1.0f);

        if (shiftDown) {
            float averageFactor = (moveFactors.x() + moveFactors.y() + moveFactors.z()) / 3.0f;
            moveFactors = QVector3D(averageFactor, averageFactor, averageFactor);
        }

        QVector3D newScale = m_dragInitialScaleValue * EditorUtils::maxVector3D(moveFactors, 0.0001f);

        if (ctrlDown) {
            newScale.setX(qMax(qRound(newScale.x()), 1));
            newScale.setY(qMax(qRound(newScale.y()), 1));
            newScale.setZ(qMax(qRound(newScale.z()), 1));
        }

        m_undoHandler->createChangePropertyCommand(m_selectedEntity->objectName(),
                                                   EditorSceneItemComponentsModel::Transform,
                                                   QStringLiteral("scale3D"), newScale,
                                                   m_selectedEntityTransform->scale3D(), true);
    }
}

void EditorScene::dragRotateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown)
{
    // By default, rotate around helper plane
    // When shift is pressed, rotate around camera plane.
    // When ctrl is pressed, rotate in 22.5 degree increments

    QVector3D posOffset = dragHandlePositionOffset(newPos);
    if (!posOffset.isNull()) {
        QVector3D unrotatedHandlePos = m_dragInitialHandleTranslation;
        QVector3D desiredPos = unrotatedHandlePos + posOffset;
        unrotatedHandlePos = projectVectorOnCameraPlane(unrotatedHandlePos);
        desiredPos = projectVectorOnCameraPlane(desiredPos);
        unrotatedHandlePos.normalize();
        desiredPos.normalize();
        QQuaternion newRotation;
        float d = QVector3D::dotProduct(unrotatedHandlePos, desiredPos) + 1.0f;
        if (ctrlDown) {
            // Rotate in larger increments
            // We need an additional check vector to determine which way the angle points
            QVector3D checkVec = EditorUtils::rotateVector(unrotatedHandlePos,
                                                           frameGraphCameraNormal(),
                                                           M_PI / 2.0);
            bool largeAngle = QVector3D::dotProduct(checkVec, desiredPos) > 0.0f;
            qreal radsOrig = qAcos(d - 1.0f);
            if (largeAngle)
                radsOrig = (2.0 * M_PI) - radsOrig;
            qreal radsAdjusted = -(qreal(qRound(radsOrig * 8.0 / M_PI)) / 8.0) * M_PI;
            if (radsAdjusted == 0.0) {
                // Indicate rotation of 0 degrees
                d = 2.0f;
            } else if (radsAdjusted == -M_PI) {
                // Indicate rotation of 180 degrees
                d = 0.0f;
            } else {
                desiredPos = EditorUtils::rotateVector(unrotatedHandlePos,
                                                       frameGraphCameraNormal(),
                                                       radsAdjusted);
            }
        }
        EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id());
        Qt3DRender::QCamera *cameraEntity = qobject_cast<Qt3DRender::QCamera *>(m_selectedEntity);
        if (cameraEntity) {
            QVector3D newUpVector;
            if (qFuzzyIsNull(d)) {
                // Rotation of 180 degrees
                newUpVector = -m_dragInitialRotateCustomVector;
            } else if (qFuzzyCompare(d, 2.0f)) {
                // Rotation of zero degrees
                newUpVector = m_dragInitialRotateCustomVector;
            } else {
                // In case of camera, we rotate the upvector
                QVector3D cameraNormal = cameraEntity->viewVector().normalized();
                if (cameraNormal.distanceToPlane(QVector3D(), frameGraphCameraNormal()) < 0.0f)
                    cameraNormal = -cameraNormal;
                QVector3D initialUpVector =
                        EditorUtils::projectVectorOnPlane(m_dragInitialRotateCustomVector.normalized(),
                                                          cameraNormal);
                QQuaternion planeRotation = QQuaternion::rotationTo(frameGraphCameraNormal(),
                                                                    cameraNormal);
                unrotatedHandlePos = planeRotation.rotatedVector(unrotatedHandlePos);
                desiredPos = planeRotation.rotatedVector(desiredPos);
                newRotation = QQuaternion::rotationTo(unrotatedHandlePos, desiredPos);
                newUpVector = newRotation.rotatedVector(initialUpVector).normalized();
            }
            m_undoHandler->createChangePropertyCommand(m_selectedEntity->objectName(),
                                                       EditorSceneItemComponentsModel::CameraEntity,
                                                       QStringLiteral("upVector"), newUpVector,
                                                       cameraEntity->upVector(), true);
        } else if (selectedItem && selectedItem->itemType() == EditorSceneItem::Light) {
            QVector3D newDirection;
            QVector3D oldDirection;
            LightData *lightData = m_sceneLights.value(m_selectedEntity->id());
            if (lightData)
                oldDirection = EditorUtils::lightDirection(lightData->lightComponent);
            if (qFuzzyIsNull(d)) {
                // Rotation of 180 degrees
                QVector3D rotationAxis;
                if (shiftDown)
                    rotationAxis = frameGraphCameraNormal();
                else
                    rotationAxis = helperPlaneNormal();
                newRotation = QQuaternion::fromAxisAndAngle(rotationAxis, 180.0f)
                        * m_dragInitialRotationValue;
                newDirection = newRotation.rotatedVector(
                            m_dragInitialRotateCustomVector.normalized()).normalized();
            } else if (qFuzzyCompare(d, 2.0f)) {
                // Rotation of zero degrees
                newDirection = m_dragInitialRotateCustomVector;
            } else {
                QVector3D rotationAxis;
                if (shiftDown) {
                    rotationAxis = frameGraphCameraNormal();
                } else {
                    // Rotate vectors so that they lie on helper plane instead of camera plane
                    QQuaternion planeRotation = QQuaternion::rotationTo(frameGraphCameraNormal(),
                                                                        helperPlaneNormal());
                    unrotatedHandlePos = planeRotation.rotatedVector(unrotatedHandlePos);
                    desiredPos = planeRotation.rotatedVector(desiredPos);
                    rotationAxis = helperPlaneNormal();
                }
                QVector3D checkVector =
                        EditorUtils::projectVectorOnPlane(m_dragInitialRotateCustomVector.normalized(),
                                                          rotationAxis);
                if (checkVector.length() > 0.001) {
                    newRotation = QQuaternion::rotationTo(unrotatedHandlePos, desiredPos);
                    newDirection = newRotation.rotatedVector(
                                m_dragInitialRotateCustomVector.normalized()).normalized();
                } else {
                    // Don't rotate at all if direction is paraller to rotation axis
                    newDirection = m_dragInitialRotateCustomVector;
                }
            }
            // In case of camera, we rotate the upvector, and in case of lights, the direction
            QString propertyName =
                    qobject_cast<Qt3DRender::QDirectionalLight *>(lightData->lightComponent)
                    ? QStringLiteral("worldDirection") : QStringLiteral("direction");
            m_undoHandler->createChangePropertyCommand(m_selectedEntity->objectName(),
                                                       EditorSceneItemComponentsModel::Light,
                                                       propertyName, newDirection,
                                                       oldDirection, true);
        } else {
            QQuaternion ancestralRotation =
                    EditorUtils::totalAncestralRotation(m_selectedEntity).inverted();
            if (qFuzzyIsNull(d)) {
                // Rotation of 180 degrees
                QVector3D rotationAxis;
                if (shiftDown)
                    rotationAxis = frameGraphCameraNormal();
                else
                    rotationAxis = helperPlaneNormal();
                rotationAxis = ancestralRotation.rotatedVector(rotationAxis);
                newRotation = QQuaternion::fromAxisAndAngle(rotationAxis, 180.0f)
                        * m_dragInitialRotationValue;
            } else if (qFuzzyCompare(d, 2.0f)) {
                // Rotation of zero degrees
                newRotation = m_dragInitialRotationValue;
            } else {
                if (!shiftDown) {
                    // Rotate vectors so that they lie on helper plane instead of camera plane
                    QQuaternion planeRotation = QQuaternion::rotationTo(frameGraphCameraNormal(),
                                                                        helperPlaneNormal());

                    planeRotation = ancestralRotation * planeRotation;
                    unrotatedHandlePos = planeRotation.rotatedVector(unrotatedHandlePos);
                    desiredPos = planeRotation.rotatedVector(desiredPos);
                } else {
                    unrotatedHandlePos = ancestralRotation.rotatedVector(unrotatedHandlePos);
                    desiredPos = ancestralRotation.rotatedVector(desiredPos);
                }
                newRotation = QQuaternion::rotationTo(unrotatedHandlePos, desiredPos)
                        * m_dragInitialRotationValue;
            }
            m_undoHandler->createChangePropertyCommand(m_selectedEntity->objectName(),
                                                       EditorSceneItemComponentsModel::Transform,
                                                       QStringLiteral("rotation"), newRotation,
                                                       m_selectedEntityTransform->rotation(), true);
        }
    }
}

// Returns world coordinate offset from drag handle position to cursor position on a plane
// that is defined by middle of selection box and reverse camera view direction.
QVector3D EditorScene::dragHandlePositionOffset(const QPoint &newPos)
{
    QVector3D posOffset;
    Qt3DRender::QCamera *camera = frameGraphCamera();
    if (camera) {
        // Find out a camera oriented plane that intersects middle of selection box
        QVector3D planeNormal = camera->position() - camera->viewCenter();
        planeNormal.normalize();

        QVector3D planeOrigin = m_dragHandles.transform->translation();

        float cosAngle = QVector3D::dotProduct(planeOrigin.normalized(), planeNormal);
        float planeOffset = planeOrigin.length() * cosAngle;

        // Calculate intersection with plane and newPos
        QVector3D rayToNewPos = EditorUtils::unprojectRay(camera->viewMatrix(),
                                                          camera->projectionMatrix(),
                                                          m_viewport->width(), m_viewport->height(),
                                                          newPos);
        float t = 0.0f;
        QVector3D intersection = EditorUtils::findIntersection(camera->position(), rayToNewPos,
                                                               planeOffset, planeNormal, t);

        if (t > 0.0f) {
            posOffset = intersection - (m_dragHandles.transform->translation()
                                        + m_dragInitialHandleTranslation);
        }
    }
    return posOffset;
}

QMatrix4x4 EditorScene::calculateVisibleSceneCameraMatrix(Qt3DRender::QCamera *camera) const
{
    QMatrix4x4 matrix = EditorUtils::totalAncestralTransform(camera);

    QQuaternion rotation = QQuaternion::fromDirection(-camera->viewVector(),
                                                      camera->upVector());

    matrix.translate(camera->position());
    matrix.rotate(rotation);
    return matrix;
}

QMatrix4x4 EditorScene::calculateVisibleLightMatrix(Qt3DCore::QEntity *lightEntity) const
{
    QMatrix4x4 matrix;

    LightData *lightData = m_sceneLights.value(lightEntity->id());
    if (lightData) {
        QMatrix4x4 ancestralMatrix = EditorUtils::totalAncestralTransform(lightData->lightEntity);
        QMatrix4x4 lightMatrix;
        lightMatrix.translate(lightData->lightTransform->translation());
        QVector3D newPos = ancestralMatrix * lightMatrix * QVector3D();
        QVector3D direction = EditorUtils::lightDirection(lightData->lightComponent);
        matrix.translate(newPos);
        if (!direction.isNull()) {
            // Rotate using only pitch and yaw to keep light sensibly oriented
            direction.normalize();
            float pitch = qAsin(-direction.y()) * 180.0f / M_PI;
            float yaw = qAtan2(direction.x(), direction.z()) * 180.0f / M_PI;
            QQuaternion rotation = QQuaternion::fromEulerAngles(pitch, yaw, 0.0f);
            matrix.rotate(rotation);

        }
    }
    return matrix;
}

void EditorScene::handlePropertyLocking(EditorSceneItem *item, const QString &lockProperty,
                                        bool locked)
{
    // Disable/enable relevant drag handles when properties are locked/unlocked
    EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id(), Q_NULLPTR);
    if (item && item == selectedItem) {
        if (item->itemType() == EditorSceneItem::Camera) {
            QString upVectorLock = QStringLiteral("upVector") + lockPropertySuffix();
            QString positionLock = QStringLiteral("position") + lockPropertySuffix();
            QString viewCenterLock = QStringLiteral("viewCenter") + lockPropertySuffix();
            if (lockProperty == upVectorLock)
                m_dragHandleRotate.entity->setEnabled(!locked);
            else if (lockProperty == positionLock)
                m_dragHandleTranslate.entity->setEnabled(!locked);
            else if (lockProperty == viewCenterLock)
                m_viewCenterLocked = locked;
        } else {
            QString translateLock = QStringLiteral("translation") + lockPropertySuffix();
            if (lockProperty == translateLock) {
                m_dragHandleTranslate.entity->setEnabled(!locked);
            } else if (item->itemType() == EditorSceneItem::Light) {
                if (item->canRotate()) {
                    QString directionLock = QStringLiteral("direction") + lockPropertySuffix();
                    QString worldDirectionLock = QStringLiteral("worldDirection") + lockPropertySuffix();
                    if (lockProperty == directionLock || lockProperty == worldDirectionLock)
                        m_dragHandleRotate.entity->setEnabled(!locked);
                }
            } else {
                QString scaleLock = QStringLiteral("scale3D") + lockPropertySuffix();
                QString rotateLock = QStringLiteral("rotation") + lockPropertySuffix();
                if (lockProperty == scaleLock)
                    m_dragHandleScale.entity->setEnabled(!locked);
                else if (lockProperty == rotateLock)
                    m_dragHandleRotate.entity->setEnabled(!locked);
            }
        }
        handleSelectionTransformChange();
        updateDragHandlePickers();
    }
}

void EditorScene::handleLightTypeChanged(EditorSceneItem *item)
{
    if (item) {
        Qt3DRender::QLight *light = EditorUtils::entityLight(item->entity());
        if (light) {
            LightData *lightData = m_sceneLights.value(item->entity()->id());
            if (lightData) {
                lightData->lightComponent = light;
                connect(light, &Qt3DRender::QLight::colorChanged,
                        lightData->visibleMaterial, &Qt3DRender::QPhongAlphaMaterial::setAmbient);
                delete lightData->visibleMesh;
                Qt3DRender::QDirectionalLight *dirLight =
                        qobject_cast<Qt3DRender::QDirectionalLight *>(light);
                Qt3DRender::QSpotLight *spotLight = qobject_cast<Qt3DRender::QSpotLight *>(light);
                if (dirLight) {
                    lightData->visibleMesh = EditorUtils::createLightMesh(EditorUtils::LightDirectional);
                    connect(dirLight, &Qt3DRender::QDirectionalLight::worldDirectionChanged,
                            this, &EditorScene::handleLightTransformChange);
                    connect(dirLight, &Qt3DRender::QDirectionalLight::worldDirectionChanged,
                            item, &EditorSceneItem::updateSelectionBoxTransform);
                } else if (spotLight) {
                    lightData->visibleMesh = EditorUtils::createLightMesh(EditorUtils::LightSpot);
                    connect(spotLight, &Qt3DRender::QSpotLight::localDirectionChanged,
                            this, &EditorScene::handleLightTransformChange);
                    connect(spotLight, &Qt3DRender::QSpotLight::localDirectionChanged,
                            item, &EditorSceneItem::updateSelectionBoxTransform);
                } else if (qobject_cast<Qt3DRender::QPointLight *>(light)) {
                    lightData->visibleMesh = EditorUtils::createLightMesh(EditorUtils::LightPoint);
                } else {
                    lightData->visibleMesh = EditorUtils::createLightMesh(EditorUtils::LightBasic);
                }
                lightData->visibleEntity->addComponent(lightData->visibleMesh);
            }
            if (item->entity() == m_selectedEntity) {
                if (item->canRotate()) {
                    QString lockProperty =
                            qobject_cast<Qt3DRender::QDirectionalLight *>(lightData->lightComponent)
                            ? QStringLiteral("worldDirection") : QStringLiteral("direction");
                    m_dragHandleRotate.entity->setEnabled(
                                !isPropertyLocked(lockProperty, light));
                } else {
                    m_dragHandleRotate.entity->setEnabled(false);
                }
                item->updateSelectionBoxTransform();
                updateLightVisibleTransform(item->entity());
                updateDragHandlePickers();
            }
        }
    }
}

void EditorScene::updateLightVisibleTransform(Qt3DCore::QEntity *lightEntity)
{
    if (lightEntity) {
        LightData *lightData = m_sceneLights.value(lightEntity->id());
        if (lightData)
            lightData->visibleTransform->setMatrix(calculateVisibleLightMatrix(lightEntity));
    }
}

void EditorScene::handleEnabledChanged(Qt3DCore::QEntity *entity, bool enabled)
{
    bool freeViewEnabled = enabled && m_freeView;
    Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(entity);
    if (camera != Q_NULLPTR) {
        int cameraIndex = cameraIndexForEntity(camera);
        if (cameraIndex >= 0) {
            enableVisibleCamera(m_sceneCameras[cameraIndex], freeViewEnabled,
                                cameraIndex == m_activeSceneCameraIndex);
        }

    } else if (EditorUtils::entityLight(entity) != Q_NULLPTR) {
        LightData *lightData = m_sceneLights.value(entity->id());
        if (lightData)
            enableVisibleLight(*lightData, freeViewEnabled);

    } else {
        // Picker doesn't get disabled with the entity - we have to delete it to disable
        Qt3DRender::QObjectPicker *picker = EditorUtils::entityPicker(entity);
        // Other objects aren't affected by m_freeView, so just check enabled flag
        if (enabled) {
            if (!picker)
                createObjectPickerForEntity(entity);
        } else {
            delete picker;
        }
    }
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
    Qt3DRender::QCamera *sceneCameraEntity = new Qt3DRender::QCamera(m_sceneEntity);
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
    Qt3DRender::QCamera *sceneCameraEntity = new Qt3DRender::QCamera(m_sceneEntity);
    sceneCameraEntity->setObjectName(m_cameraString);

    sceneCameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 50.0f);
    sceneCameraEntity->setPosition(QVector3D(0, 0, -15.0f));
    sceneCameraEntity->setUpVector(QVector3D(0, 1, 0));
    sceneCameraEntity->setViewCenter(QVector3D(0, 0, 0));

    setFrameGraphCamera(sceneCameraEntity);
    addEntity(sceneCameraEntity);

    // Cube
    Qt3DCore::QEntity *cubeEntity = new Qt3DCore::QEntity(m_sceneEntity);
    cubeEntity->setObjectName(m_cubeString);
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
    lightEntity->setObjectName(m_lightString);
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
    // Grab explicit ownership of the root entity, otherwise QML garbage collector may
    // clean it up.
    QQmlEngine::setObjectOwnership(m_rootEntity, QQmlEngine::CppOwnership);

    m_rootEntity->setObjectName(QStringLiteral("__internal root entity"));

    // Create a component cache for components that are needed after Load/New/possible other
    // reason for deleting scene root (m_sceneEntity)
    m_componentCache = new Qt3DCore::QEntity(m_rootEntity);
    m_componentCache->setObjectName("__internal component cache");
    m_componentCache->setEnabled(false);

    // Selection box material and mesh need to be created before any
    // EditorSceneItem are created
    Qt3DRender::QPhongMaterial *selectionBoxMaterial = new Qt3DRender::QPhongMaterial();
    selectionBoxMaterial->setAmbient(QColor(Qt::yellow));
    selectionBoxMaterial->setDiffuse(QColor(Qt::black));
    selectionBoxMaterial->setSpecular(QColor(Qt::black));
    selectionBoxMaterial->setShininess(0);
    m_selectionBoxMaterial = selectionBoxMaterial;
    m_selectionBoxMesh = EditorUtils::createWireframeBoxMesh();

    // Save to cache, as these are needed after Load/New
    m_componentCache->addComponent(m_selectionBoxMesh);
    m_componentCache->addComponent(m_selectionBoxMaterial);

    m_rootItem = new EditorSceneItem(this, m_rootEntity, Q_NULLPTR, -1, this);

    m_sceneItems.insert(m_rootEntity->id(), m_rootItem);

    m_renderSettings = new Qt3DRender::QRenderSettings();
    // TODO: TrianglePicking doesn't work for current camera model as it is just GL_LINES.
    // TODO: We need a proper camera mesh to enable it.
    //m_renderSettings->pickingSettings()->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
    m_renderSettings->pickingSettings()->setPickResultMode(Qt3DRender::QPickingSettings::AllPicks);
    m_renderSettings->setObjectName(QStringLiteral("__internal Scene frame graph"));
    m_renderer = new Qt3DRender::QForwardRenderer();
    m_renderer->setClearColor(Qt::lightGray);
    m_renderSettings->setActiveFrameGraph(m_renderer);

    // Setting the FrameGraph to actual root entity to protect it from accidental removal
    m_rootEntity->addComponent(m_renderSettings);

    m_rootEntity->addComponent(new Qt3DInput::QInputSettings());

    // Scene entity (i.e. the visible root)
    m_sceneEntity = new Qt3DCore::QEntity();
    m_sceneEntity->setObjectName(m_sceneRootString);

    // Free view camera
    m_freeViewCameraEntity = new Qt3DRender::QCamera(m_rootEntity);
    m_freeViewCameraEntity->setObjectName(QStringLiteral("__internal free view camera"));
    resetFreeViewCamera();

    addEntity(m_sceneEntity);

    // Helper plane
    createHelperPlane();

    // Drag handles for selected item
    m_dragHandles.entity = new Qt3DCore::QEntity(m_rootEntity);
    m_dragHandles.entity->setObjectName(QStringLiteral("__internal draghandles root entity"));
    m_dragHandleScale.entity = new Qt3DCore::QEntity(m_dragHandles.entity);
    m_dragHandleRotate.entity = new Qt3DCore::QEntity(m_dragHandles.entity);
    m_dragHandleTranslate.entity = new Qt3DCore::QEntity(m_dragHandles.entity);
    m_dragHandleScale.entity->setObjectName(QStringLiteral("__internal draghandle: scale"));
    m_dragHandleRotate.entity->setObjectName(QStringLiteral("__internal draghandle: rotate"));
    m_dragHandleTranslate.entity->setObjectName(QStringLiteral("__internal draghandle: translate"));

    // The drag handles translation is same as the selection box + a specified distance
    // depending on the scale of the box.
    m_dragHandles.transform = new Qt3DCore::QTransform();
    m_dragHandleScale.transform = new Qt3DCore::QTransform();
    m_dragHandleRotate.transform = new Qt3DCore::QTransform();
    m_dragHandleTranslate.transform = new Qt3DCore::QTransform();
    m_dragHandleScale.entity->addComponent(m_dragHandleScale.transform);
    m_dragHandleRotate.entity->addComponent(m_dragHandleRotate.transform);
    m_dragHandleTranslate.entity->addComponent(m_dragHandleTranslate.transform);
    m_dragHandles.entity->addComponent(m_dragHandles.transform);

    Qt3DRender::QMaterial *dragHandleMaterial = new Qt3DRender::QMaterial();
    dragHandleMaterial->setEffect(new DragHandleEffect());
    dragHandleMaterial->addParameter(new Qt3DRender::QParameter(QStringLiteral("handleColor"),
                                                                QColor(Qt::yellow)));

    m_dragHandleScale.entity->addComponent(dragHandleMaterial);
    m_dragHandleRotate.entity->addComponent(dragHandleMaterial);
    m_dragHandleTranslate.entity->addComponent(dragHandleMaterial);

    Qt3DRender::QGeometryRenderer *scaleHandleMesh =
            EditorUtils::createScaleHandleMesh(1.0f);
    Qt3DRender::QGeometryRenderer *rotateHandleMesh =
            EditorUtils::createRotateHandleMesh(1.0f);
    Qt3DRender::QGeometryRenderer *translateHandleMesh =
            EditorUtils::createTranslateHandleMesh(1.0f);
    m_dragHandleScale.entity->addComponent(scaleHandleMesh);
    m_dragHandleRotate.entity->addComponent(rotateHandleMesh);
    m_dragHandleTranslate.entity->addComponent(translateHandleMesh);

    m_dragHandles.entity->setEnabled(false);
    updateDragHandlePickers();

    // Active scene camera frustum visualization
    m_activeSceneCameraFrustumData.frustumEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_activeSceneCameraFrustumData.viewVectorEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_activeSceneCameraFrustumData.viewCenterEntity = new Qt3DCore::QEntity(m_rootEntity);

    m_activeSceneCameraFrustumData.frustumMesh = EditorUtils::createWireframeBoxMesh();
    Qt3DRender::QGeometryRenderer *viewVectorMesh = EditorUtils::createCameraViewVectorMesh();
    Qt3DRender::QGeometryRenderer *viewCenterMesh = EditorUtils::createCameraViewCenterMesh(1.0f);

    Qt3DRender::QPhongMaterial *frustumMaterial = new Qt3DRender::QPhongMaterial();
    frustumMaterial->setAmbient(QColor(Qt::magenta));
    frustumMaterial->setDiffuse(QColor(Qt::black));
    frustumMaterial->setSpecular(QColor(Qt::black));
    frustumMaterial->setShininess(0);

    m_activeSceneCameraFrustumData.frustumTransform = new Qt3DCore::QTransform();
    m_activeSceneCameraFrustumData.viewVectorTransform = new Qt3DCore::QTransform();
    m_activeSceneCameraFrustumData.viewCenterTransform = new Qt3DCore::QTransform();

    m_activeSceneCameraFrustumData.frustumEntity->addComponent(frustumMaterial);
    m_activeSceneCameraFrustumData.frustumEntity->addComponent(
                m_activeSceneCameraFrustumData.frustumMesh);
    m_activeSceneCameraFrustumData.frustumEntity->addComponent(
                m_activeSceneCameraFrustumData.frustumTransform);

    m_activeSceneCameraFrustumData.viewVectorEntity->addComponent(frustumMaterial);
    m_activeSceneCameraFrustumData.viewVectorEntity->addComponent(viewVectorMesh);
    m_activeSceneCameraFrustumData.viewVectorEntity->addComponent(
                m_activeSceneCameraFrustumData.viewVectorTransform);

    m_activeSceneCameraFrustumData.viewCenterEntity->addComponent(frustumMaterial);
    m_activeSceneCameraFrustumData.viewCenterEntity->addComponent(viewCenterMesh);
    m_activeSceneCameraFrustumData.viewCenterEntity->addComponent(
                m_activeSceneCameraFrustumData.viewCenterTransform);
}

void EditorScene::createHelperPlane()
{
    m_helperPlane = new Qt3DCore::QEntity();

    m_helperPlane->setObjectName(QStringLiteral("__internal helper plane"));

    // Helper plane origin must be at the meeting point of lines, hence the odd lineCount
    Qt3DRender::QGeometryRenderer *planeMesh = EditorUtils::createWireframePlaneMesh(51);

    Qt3DRender::QPhongMaterial *helperPlaneMaterial = new Qt3DRender::QPhongMaterial();
    helperPlaneMaterial->setAmbient(QColor(Qt::darkGray));
    helperPlaneMaterial->setDiffuse(QColor(Qt::black));
    helperPlaneMaterial->setSpecular(QColor(Qt::black));
    helperPlaneMaterial->setShininess(0);

    m_helperPlaneTransform = new Qt3DCore::QTransform();
    m_helperPlaneTransform->setScale3D(QVector3D(m_gridSize * 25.0f, m_gridSize * 25.0f, 1.0f));
    m_helperPlaneTransform->setRotation(
                m_helperPlaneTransform->fromAxisAndAngle(1.0f, 0.0f, 0.0f, 90.0f));
    m_helperPlane->addComponent(planeMesh);
    m_helperPlane->addComponent(helperPlaneMaterial);
    m_helperPlane->addComponent(m_helperPlaneTransform);
    m_helperPlane->setParent(m_rootEntity);
}

void EditorScene::setFrameGraphCamera(Qt3DCore::QEntity *cameraEntity)
{
    if (m_renderer) {
        Qt3DRender::QCamera *currentCamera =
                qobject_cast<Qt3DRender::QCamera *>(m_renderer->camera());
        if (currentCamera) {
            disconnect(currentCamera, &Qt3DRender::QCamera::viewMatrixChanged,
                       this, &EditorScene::handleSelectionTransformChange);
        }
        m_renderer->setCamera(cameraEntity);
        currentCamera = qobject_cast<Qt3DRender::QCamera *>(cameraEntity);
        if (cameraEntity) {
            connect(currentCamera, &Qt3DRender::QCamera::viewMatrixChanged,
                    this, &EditorScene::handleSelectionTransformChange);
        }
    }
}

Qt3DRender::QCamera *EditorScene::frameGraphCamera() const
{
    if (m_renderer)
        return qobject_cast<Qt3DRender::QCamera *>(m_renderer->camera());
    else
        return Q_NULLPTR;
}

void EditorScene::setSelection(Qt3DCore::QEntity *entity)
{
    EditorSceneItem *item = m_sceneItems.value(entity->id(), Q_NULLPTR);
    if (item) {
        if (entity != m_selectedEntity) {
            if (m_selectedEntity)
                connectDragHandles(m_sceneItems.value(m_selectedEntity->id(), Q_NULLPTR), false);

            m_selectedEntity = entity;

            if (m_selectedEntity) {
                connectDragHandles(item, true);
                m_selectedEntityTransform = EditorUtils::entityTransform(m_selectedEntity);
            }

            m_duplicateCount = 0;

            // Emit signal to highlight the entity from the list
            emit selectionChanged(m_selectedEntity);
        }
        m_dragHandles.entity->setEnabled(item->isSelectionBoxShowing());

        if (item->itemType() == EditorSceneItem::Camera) {
            // Disable scale handles for cameras
            m_dragHandleScale.entity->setEnabled(false);
            m_dragHandleRotate.entity->setEnabled(!isPropertyLocked(QStringLiteral("upVector"),
                                                                    m_selectedEntity));
            m_dragHandleTranslate.entity->setEnabled(
                        !isPropertyLocked(QStringLiteral("position"), m_selectedEntity));
            m_viewCenterLocked = isPropertyLocked(QStringLiteral("viewCenter"), m_selectedEntity);
        } else {
            Qt3DCore::QTransform *transform = EditorUtils::entityTransform(m_selectedEntity);
            m_dragHandleTranslate.entity->setEnabled(
                        !isPropertyLocked(QStringLiteral("translation"), transform));
            if (item->itemType() == EditorSceneItem::Light) {
                // Disable scale handles for lights
                m_dragHandleScale.entity->setEnabled(false);
                // Some lights can rotate
                if (item->canRotate()) {
                    Qt3DRender::QLight *light = EditorUtils::entityLight(m_selectedEntity);
                    QString lockProperty =
                            qobject_cast<Qt3DRender::QDirectionalLight *>(light)
                            ? QStringLiteral("worldDirection") : QStringLiteral("direction");
                    m_dragHandleRotate.entity->setEnabled(
                                !isPropertyLocked(lockProperty, light));
                } else {
                    m_dragHandleRotate.entity->setEnabled(false);
                }
            } else {
                m_dragHandleScale.entity->setEnabled(!isPropertyLocked(QStringLiteral("scale3D"),
                                                                       transform));
                m_dragHandleRotate.entity->setEnabled(!isPropertyLocked(QStringLiteral("rotation"),
                                                                        transform));
            }
        }

        // Update drag handles transforms to initial state
        handleSelectionTransformChange();
    } else {
        m_dragHandles.entity->setEnabled(false);
    }
    updateDragHandlePickers();
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
            setFrameGraphCamera(m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity);
        updateVisibleSceneCameraMatrix(m_sceneCameras.at(m_activeSceneCameraIndex));
    } else {
        setFreeView(true);
    }

    if (m_freeView)
        enableVisibleCameras(bool(m_sceneCameras.size()));

    if (previousIndex != m_activeSceneCameraIndex)
        emit activeSceneCameraIndexChanged(m_activeSceneCameraIndex);
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
            setFrameGraphCamera(m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity);
        enableVisibleCameras(m_freeView);
        enableVisibleLights(m_freeView);
    }
    // Show / hide light meshes, and notify UI. Need to be emitted always even if it doesn't change,
    // as otherwise the UI can change the checked status of the menu item on click even if
    // the status doesn't really change.
    emit freeViewChanged(m_freeView);
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

        // Set the viewport up as a source of input events for the input aspect
        Qt3DInput::QInputSettings *inputSettings =
                m_rootEntity->findChild<Qt3DInput::QInputSettings *>();
        if (inputSettings) {
            inputSettings->setEventSource(viewport);
        } else {
            qWarning() << "No Input Settings found, keyboard and mouse events won't be handled";
        }

        emit viewportChanged(viewport);
    }
}

void EditorScene::clearSelectionBoxes()
{
    Q_FOREACH (EditorSceneItem *item, m_sceneItems.values())
        item->setShowSelectionBox(false);
}

void EditorScene::endSelectionHandling()
{
    if (m_dragMode == DragNone && m_pickedEntity) {
        setSelection(m_pickedEntity);

        // Selecting an object also starts drag, if translate handle is enabled
        Qt3DRender::QCamera *cameraEntity = qobject_cast<Qt3DRender::QCamera *>(m_pickedEntity);
        bool viewCenterDrag = cameraEntity && m_cameraViewCenterSelected && !m_viewCenterLocked;
        bool entityDrag = m_dragHandleTranslate.entity->isEnabled()
                && (!cameraEntity || !m_cameraViewCenterSelected);
        if (viewCenterDrag || entityDrag) {
            m_dragMode = DragTranslate;
            m_dragEntity = m_pickedEntity;
            m_previousMousePosition = QCursor::pos();
            if (cameraEntity) {
                if (viewCenterDrag)
                    m_dragInitialTranslationValue = cameraEntity->viewCenter();
                else
                    m_dragInitialTranslationValue = cameraEntity->position();
            } else {
                m_dragInitialTranslationValue = m_dragHandles.transform->translation();
            }
        }
        m_pickedEntity = Q_NULLPTR;
        m_pickedDistance = -1.0f;
    }
}

void EditorScene::handleSelectionTransformChange()
{
    EditorSceneItem *item = m_sceneItems.value(m_selectedEntity->id(), Q_NULLPTR);
    if (item) {
        QVector3D dragHandleScaleAdjustment(1.0f, 1.0f, 1.0f);
        QVector3D dragHandleRotationAdjustment(1.0f, -1.0f, -1.0f);
        // Update drag handles rotation so that they are always facing camera the same way
        Qt3DRender::QCamera *camera = frameGraphCamera();
        QVector3D cameraPos;
        if (camera) {
            // Drag handles should be on the side of the selection box that is most
            // towards the camera.
            cameraPos = camera->position();
            QVector3D ray = item->selectionTransform()->translation() - cameraPos;
            ray = item->selectionTransform()->rotation().inverted().rotatedVector(ray);
            float max = qMax(qAbs(ray.x()), qMax(qAbs(ray.y()), qAbs(ray.z())));
            if (qAbs(ray.x()) == max) {
                if (ray.x() > 0.0f) {
                    dragHandleScaleAdjustment = QVector3D(-1.0f, 1.0f, -1.0f);
                    dragHandleRotationAdjustment = QVector3D(-1.0f, -1.0f, 1.0f);
                } else {
                    dragHandleScaleAdjustment = QVector3D(1.0f, 1.0f, 1.0f);
                    dragHandleRotationAdjustment = QVector3D(1.0f, -1.0f, -1.0f);
                }
            } else if (qAbs(ray.y()) == max) {
                if (ray.y() > 0.0f) {
                    dragHandleScaleAdjustment = QVector3D(1.0f, -1.0f, -1.0f);
                    dragHandleRotationAdjustment = QVector3D(-1.0f, -1.0f, 1.0f);
                } else {
                    dragHandleScaleAdjustment = QVector3D(-1.0f, 1.0f, -1.0f);
                    dragHandleRotationAdjustment = QVector3D(1.0f, 1.0f, 1.0f);
                }
            } else {
                if (ray.z() > 0.0f) {
                    dragHandleScaleAdjustment = QVector3D(-1.0f, 1.0f, -1.0f);
                    dragHandleRotationAdjustment = QVector3D(1.0f, -1.0f, -1.0f);
                } else {
                    dragHandleScaleAdjustment = QVector3D(1.0f, 1.0f, 1.0f);
                    dragHandleRotationAdjustment = QVector3D(-1.0f, -1.0f, 1.0f);
                }
            }
        }

        m_dragHandles.transform->setTranslation(item->selectionBoxCenter());
        m_dragHandles.transform->setRotation(item->selectionTransform()->rotation());

        // Scale handles so that they look okay and are usable on any distance the object
        // itself can reasonably be manipulated.
        // - Handle rendered exactly the same size regardless of distance
        // - Handle edge distance from corner is constant

        QVector3D translation = (item->selectionBoxExtents() / 2.0f);

        // These are approximate distances to drag handles used for sizing of the handles
        float distanceToScale = ((translation * dragHandleScaleAdjustment
                                  + m_dragHandles.transform->translation())
                                 - cameraPos).length();
        float distanceToRotate = ((translation * dragHandleRotationAdjustment
                                   + m_dragHandles.transform->translation())
                                  - cameraPos).length();
        float distanceToTranslate = (m_dragHandles.transform->translation() - cameraPos).length();

        // We want the size to be constant on screen, so the angle for the handle radius
        // must be constant.
        const float dragHandleAngle = 0.006f;
        float scaleScale = dragHandleAngle * distanceToScale;
        float rotateScale = dragHandleAngle * distanceToRotate;
        float translateScale = dragHandleAngle * distanceToTranslate;

        m_dragHandleScale.transform->setScale(scaleScale * 2.0f);
        m_dragHandleRotate.transform->setScale(rotateScale * 2.0f);
        m_dragHandleTranslate.transform->setScale(translateScale * 2.0f);

        m_dragHandleScale.transform->setTranslation(
                    (QVector3D(scaleScale, scaleScale, scaleScale) + translation)
                    * dragHandleScaleAdjustment);
        m_dragHandleScaleCornerTranslation =
                item->entityMeshExtents() * dragHandleScaleAdjustment / 2.0f;
        m_dragHandleRotate.transform->setTranslation(
                    (QVector3D(rotateScale, rotateScale, rotateScale) + translation)
                    * dragHandleRotationAdjustment);

        resizeCameraViewCenterEntity();
    }
}

void EditorScene::handleSceneLoaderStatusChanged()
{
    Qt3DRender::QSceneLoader *sceneLoader = qobject_cast<Qt3DRender::QSceneLoader *>(sender());
    if (sceneLoader) {
        QVector<Qt3DCore::QEntity *> entities = sceneLoader->entities();
        if (!entities.isEmpty()) {
            Qt3DCore::QEntity *importedEntity = entities[0];
            if (sceneLoader->status() == Qt3DRender::QSceneLoader::Ready) {
                // TODO do we need to do something else here?
                importedEntity->setParent(m_sceneEntity);
                addEntity(importedEntity);
            } else if (sceneLoader->status() == Qt3DRender::QSceneLoader::Error) {
                importedEntity->deleteLater();
                m_errorString = m_importFailString;
                emit errorChanged(m_errorString);
                qWarning() << m_errorString;
            }
        }
    }
}

void EditorScene::handlePickerPress(Qt3DRender::QPickEvent *event)
{
    if (m_dragMode == DragNone) {
        Qt3DCore::QEntity *pressedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        m_ignoringInitialDrag = true;
        // If pressedEntity is not enabled, it typically means the pressedEntity is a drag handle
        // and the selection has changed to a different type of entity since the mouse press was
        // registered. Since the new entity is not the one we wanted to modify anyway, just
        // skip handling the pick event.
        if (pressedEntity->isEnabled()) {
            if (pressedEntity == m_dragHandleScale.entity) {
                EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id(),
                                                                   Q_NULLPTR);
                if (selectedItem) {
                    m_dragMode = DragScale;
                    m_dragEntity = m_selectedEntity;
                    m_dragInitialScaleValue = selectedItem->entityTransform()->scale3D();
                    m_dragInitialHandleTranslation = m_dragHandles.transform->rotation()
                            * m_dragHandleScale.transform->translation();
                    m_dragInitialHandleCornerTranslation =
                            EditorUtils::totalAncestralScale(m_selectedEntity) *
                            m_dragInitialScaleValue * m_dragHandleScaleCornerTranslation;
                }
            } else if (pressedEntity == m_dragHandleRotate.entity) {
                EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id(),
                                                                   Q_NULLPTR);
                if (selectedItem) {
                    Qt3DRender::QCamera *cameraEntity =
                            qobject_cast<Qt3DRender::QCamera *>(m_selectedEntity);
                    if (cameraEntity) {
                        // Store the initial upvector
                        m_dragInitialRotateCustomVector = cameraEntity->upVector();
                    } else if (selectedItem->itemType() == EditorSceneItem::Light) {
                        LightData *lightData = m_sceneLights.value(m_selectedEntity->id());
                        if (lightData) {
                            m_dragInitialRotateCustomVector =
                                    EditorUtils::lightDirection(lightData->lightComponent);
                            if (m_dragInitialRotateCustomVector.isNull()) {
                                // Have some valid vector to rotate in case direction is 0,0,0
                                m_dragInitialRotateCustomVector = defaultLightDirection;
                            }
                        }
                    }
                    m_dragEntity = m_selectedEntity;
                    m_dragMode = DragRotate;
                    m_dragInitialRotationValue = selectedItem->entityTransform()->rotation();
                    m_dragInitialHandleTranslation = m_dragHandles.transform->rotation()
                            * m_dragHandleRotate.transform->translation();
                }
            } else if (pressedEntity == m_dragHandleTranslate.entity) {
                EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id(),
                                                                   Q_NULLPTR);
                if (selectedItem) {
                    m_cameraViewCenterSelected = false;
                    m_previousMousePosition = QCursor::pos();
                    Qt3DRender::QCamera *cameraEntity =
                            qobject_cast<Qt3DRender::QCamera *>(m_selectedEntity);
                    if (cameraEntity)
                        m_dragInitialTranslationValue = cameraEntity->position();
                    else
                        m_dragInitialTranslationValue = m_dragHandles.transform->translation();
                    m_dragEntity = m_selectedEntity;
                    m_dragMode = DragTranslate;
                }
            } else if (pressedEntity && (!m_pickedEntity || m_pickedDistance > event->distance())) {
                // Ignore presses that are farther away than the closest one
                m_pickedDistance = event->distance();
                bool select = false;
                EditorSceneItem *item = m_sceneItems.value(pressedEntity->id(), Q_NULLPTR);
                if (item) {
                    select = true;
                } else if (m_freeView) {
                    if (pressedEntity == m_activeSceneCameraFrustumData.viewCenterEntity) {
                        // Select the active scene camera instead if clicked on view center
                        pressedEntity = m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity;
                        select = true;
                        m_cameraViewCenterSelected = true;
                    } else if (pressedEntity->objectName() == cameraVisibleEntityName) {
                        // Select the camera instead if clicked on camera cone
                        for (int i = 0; i < m_sceneCameras.size(); i++) {
                            if (m_sceneCameras.at(i).visibleEntity == pressedEntity) {
                                pressedEntity = m_sceneCameras.at(i).cameraEntity;
                                select = true;
                                m_cameraViewCenterSelected = false;
                                break;
                            }
                        }
                    } else if (pressedEntity->objectName() == lightVisibleEntityName) {
                        // Select the light instead if clicked on visible light mesh
                        Q_FOREACH (LightData *lightData, m_sceneLights.values()) {
                            if (lightData->visibleEntity == pressedEntity) {
                                pressedEntity = lightData->lightEntity;
                                select = true;
                            }
                        }
                    }
                }
                if (select && !m_pickedEntity)
                    QMetaObject::invokeMethod(this, "endSelectionHandling", Qt::QueuedConnection);
                m_pickedEntity = pressedEntity;
                // Get the position of the picked entity, and copy it to m_snapToGridIntersection
                m_snapToGridIntersection = EditorUtils::entityTransform(m_pickedEntity)->translation();
            }
        }
    }
    event->setAccepted(true);
}

bool EditorScene::handleMousePress(QMouseEvent *event)
{
    Q_UNUSED(event)
    cancelDrag();
    return false; // Never consume press event
}

bool EditorScene::handleMouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event)
    cancelDrag();
    return false; // Never consume release event
}

bool EditorScene::handleMouseMove(QMouseEvent *event)
{
    // Ignore initial minor drags
    if (m_ignoringInitialDrag) {
        QPoint delta = event->globalPos() - m_previousMousePosition;
        if (delta.manhattanLength() > 10)
            m_ignoringInitialDrag = false;
    }
    if (!m_ignoringInitialDrag) {
        bool shiftDown = event->modifiers() & Qt::ShiftModifier;
        bool ctrlDown = event->modifiers() & Qt::ControlModifier;
        // If selected entity changes mid-drag, cancel drag.
        if (m_dragMode != DragNone && m_dragEntity != m_selectedEntity)
            cancelDrag();
        switch (m_dragMode) {
        case DragTranslate: {
            dragTranslateSelectedEntity(event->pos(), shiftDown, ctrlDown);
            break;
        }
        case DragScale: {
            dragScaleSelectedEntity(event->pos(), shiftDown, ctrlDown);
            break;
        }
        case DragRotate: {
            dragRotateSelectedEntity(event->pos(), shiftDown, ctrlDown);
            break;
        }
        default:
            break;
        }
        m_previousMousePosition = event->globalPos();
    }

    return (m_dragMode != DragNone);
}

// Find out the normal of the helper plane.
QVector3D EditorScene::helperPlaneNormal() const
{
    QVector3D helperPlaneNormal = m_helperPlaneTransform->matrix() * QVector3D(0.0f, 0.0f, 1.0f);
    helperPlaneNormal.setX(qAbs(helperPlaneNormal.x()));
    helperPlaneNormal.setY(qAbs(helperPlaneNormal.y()));
    helperPlaneNormal.setZ(qAbs(helperPlaneNormal.z()));
    return helperPlaneNormal.normalized();
}

// Projects vector to a plane defined by active frame graph camera
QVector3D EditorScene::projectVectorOnCameraPlane(const QVector3D &vector) const
{
    QVector3D projectionVector;
    Qt3DRender::QCamera *camera = frameGraphCamera();
    if (camera) {
        QVector3D planeNormal = camera->position() - camera->viewCenter();
        planeNormal.normalize();
        projectionVector = EditorUtils::projectVectorOnPlane(vector, planeNormal);
        // Have some valid vector at least if vector is too close to zero
        if (projectionVector.length() < 0.00001f) {
            projectionVector = QVector3D::crossProduct(planeNormal,
                                                       camera->upVector().normalized());
        }
    }
    return projectionVector;
}

QVector3D EditorScene::frameGraphCameraNormal() const
{
    QVector3D planeNormal;
    Qt3DRender::QCamera *camera = frameGraphCamera();
    if (camera) {
        planeNormal = camera->position() - camera->viewCenter();
        planeNormal.normalize();
    }
    return planeNormal;
}

void EditorScene::updateDragHandlePickers()
{
    updateDragHandlePicker(m_dragHandleScale);
    updateDragHandlePicker(m_dragHandleRotate);
    updateDragHandlePicker(m_dragHandleTranslate);
}

void EditorScene::updateDragHandlePicker(EditorScene::DragHandleData &handleData)
{
    // Odd that picker doesn't get disabled with the entity - we have to delete it to disable
    if (m_dragHandles.entity->isEnabled()) {
        if (handleData.entity->isEnabled() && !handleData.picker) {
            handleData.picker = createObjectPickerForEntity(handleData.entity);
        } else if (!handleData.entity->isEnabled()) {
            delete handleData.picker;
            handleData.picker = Q_NULLPTR;
        }
    } else {
        delete handleData.picker;
        handleData.picker = Q_NULLPTR;
    }
}

void EditorScene::resizeCameraViewCenterEntity()
{
    // Rescale the camera viewcenter entity according to distance, as it is draggable
    const float vcEntityAngle = 0.006f;
    QVector3D vcPos = m_activeSceneCameraFrustumData.viewCenterTransform->translation();
    float distanceToVc = (vcPos - frameGraphCamera()->position()).length();
    float vcScale = vcEntityAngle * distanceToVc;
    m_activeSceneCameraFrustumData.viewCenterTransform->setScale(vcScale * 2.0f);
}

bool EditorScene::isPropertyLocked(const QString &propertyName, QObject *obj)
{
    if (!obj)
        return false;
    QString lockProperty = propertyName + lockPropertySuffix();
    QByteArray nameArray = lockProperty.toLatin1();
    const char *namePtr = nameArray.constData();
    QVariant propertyVariant = obj->property(namePtr);
    if (propertyVariant.isValid())
        return propertyVariant.toBool();
    else
        return false;
}

void EditorScene::cancelDrag()
{
    m_dragMode = DragNone;
    m_pickedEntity = Q_NULLPTR;
    m_pickedDistance = -1.0f;
    m_dragEntity = Q_NULLPTR;
}

void EditorScene::handleCameraAdded(Qt3DRender::QCamera *camera)
{
    Qt3DCore::QEntity *visibleEntity = new Qt3DCore::QEntity(m_rootEntity);

    visibleEntity->setObjectName(cameraVisibleEntityName);

    Qt3DRender::QGeometryRenderer *visibleMesh = EditorUtils::createVisibleCameraMesh();

    Qt3DRender::QPhongMaterial *cameraMaterial = new Qt3DRender::QPhongMaterial();
    cameraMaterial->setAmbient(QColor(Qt::magenta));
    cameraMaterial->setDiffuse(QColor(Qt::black));
    cameraMaterial->setSpecular(QColor(Qt::black));
    cameraMaterial->setShininess(0);

    Qt3DCore::QTransform *visibleTransform = new Qt3DCore::QTransform();

    visibleEntity->addComponent(visibleMesh);
    visibleEntity->addComponent(cameraMaterial);
    visibleEntity->addComponent(visibleTransform);

    CameraData newData(camera, visibleEntity, visibleTransform, Q_NULLPTR);
    enableVisibleCamera(newData, m_freeView, false);
    m_sceneCameras.append(newData);

    connectSceneCamera(newData);
    updateVisibleSceneCameraMatrix(newData);

    int newRow = m_sceneCamerasModel.rowCount();
    m_sceneCamerasModel.insertRow(newRow);
    m_sceneCamerasModel.setData(m_sceneCamerasModel.index(newRow),
                                QVariant::fromValue(camera->objectName()),
                                Qt::DisplayRole);

    // Activate the newly added camera if it is the only existing scene camera
    if (m_sceneCameras.size() == 1)
        setActiveSceneCameraIndex(0);
}

void EditorScene::handleCameraRemoved(Qt3DRender::QCamera *camera)
{
    int removeIndex = cameraIndexForEntity(camera);

    if (removeIndex >= 0) {
        delete m_sceneCameras.at(removeIndex).visibleEntity;
        m_sceneCameras.removeAt(removeIndex);
        m_sceneCamerasModel.removeRow(removeIndex);
        if (removeIndex <= m_activeSceneCameraIndex)
            setActiveSceneCameraIndex(m_activeSceneCameraIndex - 1);
    }
}

void EditorScene::handleLightAdded(Qt3DCore::QEntity *lightEntity)
{
    Qt3DCore::QEntity *visibleEntity = new Qt3DCore::QEntity(m_rootEntity);

    visibleEntity->setObjectName(lightVisibleEntityName);

    Qt3DCore::QTransform *visibleTransform = new Qt3DCore::QTransform();

    Qt3DRender::QLight *lightComponent = EditorUtils::entityLight(lightEntity);

    Qt3DRender::QPhongAlphaMaterial *visibleMaterial = new Qt3DRender::QPhongAlphaMaterial();
    visibleMaterial->setDiffuse(Qt::black);
    visibleMaterial->setSpecular(Qt::black);
    visibleMaterial->setAmbient(lightComponent->color());
    visibleMaterial->setAlpha(0.5f);

    visibleEntity->addComponent(visibleMaterial);
    visibleEntity->addComponent(visibleTransform);

    Qt3DCore::QTransform *lightTransform = EditorUtils::entityTransform(lightEntity);
    if (lightTransform) {
        connect(lightTransform, &Qt3DCore::QTransform::translationChanged,
                this, &EditorScene::handleLightTransformChange);
    }

    LightData *newData = new LightData(lightEntity, lightComponent, lightTransform, visibleEntity,
                                       visibleTransform, visibleMaterial, Q_NULLPTR, Q_NULLPTR);
    enableVisibleLight(*newData, m_freeView);
    m_sceneLights.insert(lightEntity->id(), newData);

    QMatrix4x4 matrix = EditorUtils::totalAncestralTransform(lightEntity);
    matrix.translate(lightTransform->translation());
    visibleTransform->setMatrix(matrix);

    handleLightTypeChanged(m_sceneItems.value(lightEntity->id()));
}

void EditorScene::handleLightRemoved(Qt3DCore::QEntity *lightEntity)
{
    LightData *lightData = m_sceneLights.value(lightEntity->id());
    if (lightData) {
        m_sceneLights.remove(lightEntity->id());
        delete lightData->visibleEntity;
        delete lightData;
    }
}

void EditorScene::connectSceneCamera(const CameraData &cameraData)
{
    connect(cameraData.cameraEntity, &Qt3DRender::QCamera::projectionMatrixChanged,
            this, &EditorScene::handleCameraMatrixChange);
    connect(cameraData.cameraEntity, &Qt3DRender::QCamera::viewMatrixChanged,
            this, &EditorScene::handleCameraMatrixChange);
    connect(cameraData.cameraEntity, &Qt3DRender::QCamera::viewVectorChanged,
            this, &EditorScene::handleCameraMatrixChange);
}

void EditorScene::handleCameraMatrixChange()
{
    Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(sender());
    if (camera) {
        int changedIndex = cameraIndexForEntity(camera);
        if (changedIndex >= 0)
            updateVisibleSceneCameraMatrix(m_sceneCameras[changedIndex]);
    }
}

void EditorScene::handleLightTransformChange()
{
    Qt3DCore::QComponent *component = qobject_cast<Qt3DCore::QComponent *>(sender());
    if (component) {
        QVector<Qt3DCore::QEntity *> entities = component->entities();
        Qt3DCore::QEntity *entity = entities.size() ? entities.at(0) : Q_NULLPTR;
        updateLightVisibleTransform(entity);
    }
}

void EditorScene::handleViewportSizeChange()
{
    qreal aspectRatio = m_viewport->width() / qMax(m_viewport->height(), 1.0);
    m_freeViewCameraEntity->lens()->setPerspectiveProjection(
                freeViewCameraFov, aspectRatio, freeViewCameraNearPlane, freeViewCameraFarPlane);
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
    switch (event->type()) {
    case QEvent::KeyPress: {
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
        break;
    }
    case QEvent::MouseButtonPress:
        if (obj == m_viewport)
            return handleMousePress(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseButtonRelease:
        if (obj == m_viewport)
            return handleMouseRelease(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseMove:
        if (obj == m_viewport)
            return handleMouseMove(static_cast<QMouseEvent *>(event));
        break;
    default:
        break;
    }

    return false;
}
