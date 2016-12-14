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
#include "editorsceneitemcomponentsmodel.h"
#include "editorviewportitem.h"
#include "ontopeffect.h"
#include "undohandler.h"

#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QDiffuseSpecularMapMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QPickingSettings>

#include <Qt3DInput/QInputSettings>

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/QKeySequence>
#include <QtGui/QMatrix4x4>

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QtMath>

#include <QtQml/QQmlEngine>

#ifdef GLTF_EXPORTER_AVAILABLE
#include <Qt3DRender/private/qsceneexportfactory_p.h>
#include <Qt3DRender/private/qsceneexporter_p.h>
#include "editorscenesaver.h"
#else
#include "editorsceneparser.h"
#endif

#include <cfloat>

//#define TEST_SCENE // If a test scene is wanted instead of the default scene

#ifdef TEST_SCENE
#include <Qt3DRender/QCylinderMesh>
#include <Qt3DRender/QNormalDiffuseSpecularMapMaterial>
#endif

static const QString cameraVisibleEntityName = QStringLiteral("__internal camera visible entity");
static const QString lightVisibleEntityName = QStringLiteral("__internal light visible entity");
static const QString sceneLoaderSubEntityName = QStringLiteral("__internal sceneloader sub entity");
static const QString helperArrowName = QStringLiteral("__internal helper arrow");
static const QVector3D defaultLightDirection(0.0f, -1.0f, 0.0f);
static const float freeViewCameraNearPlane = 0.1f;
static const float freeViewCameraFarPlane = 10000.0f;
static const float freeViewCameraFov = 45.0f;
static const int dragCornerHandleCount = 8; // One handle for each selection box corner
static const QColor selectionBoxColor("#43adee");
static const QColor cameraFrustumColor("#c22555");
static const QColor helperPlaneColor("#585a5c");
static const QColor helperArrowColorX("red");
static const QColor helperArrowColorY("green");
static const QColor helperArrowColorZ("blue");
#ifndef GLTF_EXPORTER_AVAILABLE
static const QString autoSavePostfix = QStringLiteral(".autosave");
#endif

EditorScene::EditorScene(QObject *parent)
    : QObject(parent)
    , m_rootEntity(nullptr)
    , m_componentCache(nullptr)
    , m_rootItem(nullptr)
    , m_sceneModel(new EditorSceneItemModel(this))
    , m_renderSettings(nullptr)
    , m_renderer(nullptr)
    , m_sceneEntity(nullptr)
    , m_sceneEntityItem(nullptr)
    , m_selectedEntity(nullptr)
    , m_selectedEntityTransform(nullptr)
    , m_activeSceneCameraIndex(-1)
    , m_freeView(false)
    , m_freeViewCameraEntity(nullptr)
    , m_viewport(nullptr)
    , m_undoHandler(new UndoHandler(this))
    , m_helperPlane(nullptr)
    , m_helperPlaneTransform(nullptr)
    , m_helperArrows(nullptr)
    , m_helperArrowsTransform(nullptr)
    , m_helperArrowsLocal(false)
    , m_meshCenterIndicatorLine(nullptr)
    , m_meshCenterIndicatorLineTransform(nullptr)
    , m_qtTranslator(new QTranslator(this))
    , m_appTranslator(new QTranslator(this))
    , m_dragHandlesTransform(nullptr)
    , m_dragHandleRotateTransform(nullptr)
    , m_dragHandleTranslateTransform(nullptr)
    , m_dragHandleIndex(0)
    , m_dragMode(DragNone)
    , m_ignoringInitialDrag(true)
    , m_viewCenterLocked(false)
    , m_pickedEntity(nullptr)
    , m_pickedDistance(-1.0f)
    , m_gridSize(3)
    , m_duplicateCount(0)
    , m_previousDuplicate(nullptr)
    , m_ctrlDownOnLastLeftPress(false)
    , m_clipboardOperation(ClipboardNone)
    , m_groupBoxUpdatePending(false)
#ifdef GLTF_EXPORTER_AVAILABLE
    , m_gltfExporter(nullptr)
    , m_sceneSaver(new EditorSceneSaver(this))
#else
    , m_sceneParser(new EditorSceneParser(this))
#endif
{
    setLanguage(language());
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

    delete m_dragHandlesTransform;
    delete m_dragHandleRotateTransform;
    delete m_dragHandleTranslateTransform;
    qDeleteAll(m_dragHandleScaleTransforms);
}

EditorSceneItem *EditorScene::entityItem(Qt3DCore::QEntity *entity) const
{
    return m_sceneItems.value(entity->id());
}

void EditorScene::addEntity(Qt3DCore::QEntity *entity, int index, Qt3DCore::QEntity *parent)
{
    if (entity == nullptr)
        return;

    if (parent == nullptr) {
        //make sure that entity has a parent, otherwise make its parent the root entity
        if (entity->parentEntity() == nullptr)
            entity->setParent(m_rootEntity);
    } else if (entity->parentEntity() != parent) {
        entity->setParent(parent);
    }

    EditorSceneItem *item = m_sceneItems.value(entity->id(), nullptr);
    if (!item) {
        item = new EditorSceneItem(this, entity, m_sceneItems.value(entity->parentEntity()->id(),
                                                                    nullptr), index, this);

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
        else if (entity->isEnabled() && item->itemType() != EditorSceneItem::SceneLoader)
            createObjectPickerForEntity(entity);
        // Note: Scene loader pickers are created asynchronously after scene is loaded fully

        item->componentsModel()->initializeModel();
    }

    if (item->itemType() != EditorSceneItem::SceneLoader) {
        foreach (QObject *child, entity->children()) {
            Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
            if (childEntity)
                addEntity(childEntity);
        }
    }
}

// Removed entity is deleted
void EditorScene::removeEntity(Qt3DCore::QEntity *entity)
{
    if (entity == nullptr || entity == m_rootEntity)
        return;

    if (entity->objectName() == m_clipboardEntityName)
        setClipboardOperation(ClipboardNone);

    if (entity == m_sceneEntity) {
        m_sceneEntity = nullptr;
        m_sceneEntityItem = nullptr;
    }

    disconnect(entity, 0, this, 0);

    EditorSceneItem *item = m_sceneItems.value(entity->id());

    if (item->itemType() != EditorSceneItem::SceneLoader) {
        foreach (QObject *child, entity->children()) {
            Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
            removeEntity(childEntity);
        }
    }

    Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(entity);
    if (camera)
        handleCameraRemoved(camera);

    if (item && item->itemType() == EditorSceneItem::Light)
        handleLightRemoved(entity);

    m_sceneItems.remove(entity->id());

    if (m_selectedEntity == entity || multiSelection())
        queueEnsureSelection();

    queueUpdateGroupSelectionBoxes();

    delete item;
    delete entity;
}

void EditorScene::resetScene()
{
    clearSingleSelection();

    // Clear the existing scene
    setFrameGraphCamera(nullptr);
    m_undoHandler->clear();
    clearSceneCamerasAndLights();
    removeEntity(m_sceneEntity);

    // Create new scene root
    setSceneEntity();

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
    m_sceneModel->clearExpandedItems();
    m_sceneModel->resetModel();

    setSelection(m_sceneEntity);
}

bool EditorScene::saveScene(const QUrl &fileUrl, bool autosave)
{
    if (!fileUrl.isValid())
        return false;

#ifdef GLTF_EXPORTER_AVAILABLE
    QString camera;
    if (m_activeSceneCameraIndex >= 0 && m_activeSceneCameraIndex < m_sceneCameras.size())
        camera = m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity->objectName();

    bool retval = m_sceneSaver->saveScene(m_sceneEntity, camera, fileUrl.toLocalFile(), autosave);
#else
    QUrl url = fileUrl;
    if (!url.toString().endsWith(QStringLiteral(".qt3d.qrc"))) {
        QString filePath = url.toString() + QStringLiteral(".qt3d.qrc");
        url.setUrl(filePath);
    }

    Qt3DCore::QEntity *camera = nullptr;
    if (m_activeSceneCameraIndex >= 0 && m_activeSceneCameraIndex < m_sceneCameras.size())
        camera = m_sceneCameras.at(m_activeSceneCameraIndex).cameraEntity;
    bool retval = m_sceneParser->exportQmlScene(m_sceneEntity, url, camera, autosave);
#endif

    if (retval)
        m_undoHandler->setClean();
    else
        setError(m_saveFailString);
    return retval;
}

bool EditorScene::loadScene(const QUrl &fileUrl)
{
    if (!fileUrl.isValid())
        return false;

    Qt3DCore::QEntity *camera = nullptr;
#ifdef GLTF_EXPORTER_AVAILABLE
    Qt3DCore::QEntity *newSceneEntity = m_sceneSaver->loadScene(fileUrl.toLocalFile(), camera);
#else
    Qt3DCore::QEntity *newSceneEntity = m_sceneParser->importQmlScene(fileUrl, camera);
#endif

    if (newSceneEntity) {
        clearSingleSelection();
        if (!m_freeView)
            setFrameGraphCamera(nullptr);
        m_undoHandler->clear();
        clearSceneCamerasAndLights();
        removeEntity(m_sceneEntity);
        m_sceneEntity = newSceneEntity;
        addEntity(newSceneEntity);
        enableVisibleCameras(m_freeView);
        enableVisibleLights(m_freeView);
        m_activeSceneCameraIndex--; // To force change
        setActiveSceneCameraIndex(cameraIndexForEntity(camera));

        m_sceneModel->clearExpandedItems();
        m_sceneModel->resetModel();
    } else {
        setError(m_loadFailString);
    }

    return bool(newSceneEntity);
}

void EditorScene::deleteScene(const QUrl &fileUrl, bool autosave)
{
    if (!fileUrl.isValid())
        return;

#ifdef GLTF_EXPORTER_AVAILABLE
    m_sceneSaver->deleteSavedScene(fileUrl.toLocalFile(), autosave);
#else
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
#endif
}

QString EditorScene::cameraName(int index) const
{
    if (m_sceneCameras.size() > index)
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

QString EditorScene::duplicateEntity(Qt3DCore::QEntity *entity)
{
    QString duplicateName;

    if (m_previousDuplicate != entity) {
        m_duplicateCount = 0;
        m_previousDuplicate = entity;
    }

    QVector3D duplicateOffset =
            m_helperPlaneTransform->rotation().rotatedVector(QVector3D(0.5f, 0.5f, 0.0f)
                                                             * ++m_duplicateCount);

    Qt3DCore::QEntity *newEntity =
            m_sceneModel->duplicateEntity(entity, entity->parentEntity(), duplicateOffset);

    // Set name and add to scene
    duplicateName = EditorUtils::nameDuplicate(newEntity, entity, m_sceneModel);
    addEntity(newEntity);

    // Refresh entity tree
    m_sceneModel->resetModel();

    return duplicateName;
}

Qt3DRender::QCamera *EditorScene::inputCamera() const
{
    Qt3DRender::QCamera *inputCamera = nullptr;

    if (m_freeView)
        inputCamera =  m_freeViewCameraEntity;

    return inputCamera;
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
    EditorUtils::InsertableEntities insertableType = EditorUtils::InsertableEntities(type);
    if (!data) {
        data = new PlaceholderEntityData();
        data->entity = new Qt3DCore::QEntity(m_rootEntity);
        data->transform = new Qt3DCore::QTransform();
        Qt3DExtras::QPhongAlphaMaterial *material = new Qt3DExtras::QPhongAlphaMaterial();
        if (insertableType == EditorUtils::InsertableEntities::GroupEntity) {
            material->setAlpha(0.2f);
            material->setAmbient(selectionBoxColor);
        } else {
            material->setAlpha(0.4f);
            material->setAmbient("#53adee");
        }
        data->material = material;
        data->entity->addComponent(data->transform);
        data->entity->addComponent(material);
        m_placeholderEntityMap.insert(name, data);
    }

    if (data->type != insertableType) {
        data->type = insertableType;
        delete data->mesh;
        data->mesh = EditorUtils::createMeshForInsertableType(insertableType);
        if (!data->mesh) {
            if (insertableType == EditorUtils::LightEntity)
                data->mesh = EditorUtils::createLightMesh(EditorUtils::LightPoint);
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

void EditorScene::dragHandlePress(EditorScene::DragMode dragMode, const QPoint &pos, int handleIndex)
{
    Q_ASSERT(handleIndex >= 0 && handleIndex < m_dragHandleScaleTransforms.size());

    cancelDrag();
    m_previousMousePosition = pos;
    EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id(), nullptr);
    if (selectedItem) {
        m_dragHandleIndex = handleIndex;
        if (dragMode == DragTranslate && m_dragHandleTranslateTransform->isEnabled()) {
            Qt3DRender::QCamera *cameraEntity =
                    qobject_cast<Qt3DRender::QCamera *>(m_selectedEntity);
            if (cameraEntity) {
                if (m_cameraViewCenterSelected && !m_viewCenterLocked)
                    m_dragInitialWorldTranslationValue = cameraEntity->viewCenter();
                else
                    m_dragInitialWorldTranslationValue = cameraEntity->position();
            } else {
                if (handleIndex == TranslateHandleMeshCenter
                        || (handleIndex >= TranslateHandleArrowX && m_helperArrowsLocal)) {
                    m_dragInitialWorldTranslationValue = m_dragHandlesTransform->matrix()
                            * m_dragHandleTranslateTransform->matrix() * QVector3D();
                } else {
                    m_dragInitialWorldTranslationValue = m_dragHandlesTransform->translation();
                }
            }
            m_dragInitialEntityTranslationValue = m_selectedEntityTransform->translation();
            m_dragInitialHandleRotationValue = m_dragHandlesTransform->rotation();

            // Calculate snap point offset in world coordinates
            for (int i = 0; i < dragCornerHandleCount; ++i) {
                if (cameraEntity) {
                    m_dragEntitySnapOffsets[i] = QVector3D();
                } else {
                    QVector3D centerHandleAdj =
                            (handleIndex == TranslateHandleMeshCenter
                            || (handleIndex >= TranslateHandleArrowX && m_helperArrowsLocal))
                            ? selectedItem->entityMeshCenter()
                              * m_selectedEntityTransform->scale3D()
                              / selectedItem->selectionTransform()->scale3D()
                            : QVector3D();
                    QVector3D snapPos = (selectedItem->unadjustedSelectionBoxMatrix()
                                         * (m_dragHandleCornerAdjustments.at(i)
                                            * QVector3D(0.5f, 0.5f, 0.5f) + centerHandleAdj));
                    snapPos -= selectedItem->selectionBoxCenter();
                    m_dragEntitySnapOffsets[i] = snapPos;
                }
            }

            m_dragEntity = m_selectedEntity;
            m_dragMode = DragTranslate;
        } else if (dragMode == DragRotate && m_dragHandleRotateTransform->isEnabled()) {
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
            m_dragInitialRotationValue = m_selectedEntityTransform->rotation();
            m_dragInitialHandleTranslation = m_dragHandlesTransform->rotation()
                    * m_dragHandleRotateTransform->translation();
            m_dragInitialCenterTranslation = m_dragHandlesTransform->translation();
        } else if (dragMode == DragScale
                   && m_dragHandleScaleTransforms.at(0)->isEnabled()) {
            m_dragMode = DragScale;
            m_dragEntity = m_selectedEntity;
            m_dragInitialScaleValue = m_selectedEntityTransform->scale3D();
            m_dragInitialCenterTranslation = m_dragHandlesTransform->translation();
            m_dragInitialHandleTranslation = m_dragHandlesTransform->rotation()
                    * m_dragHandleScaleTransforms.at(m_dragHandleIndex)->translation();
            m_dragHandleCornerTranslation = selectedItem->entityMeshExtents()
                    * m_dragHandleCornerAdjustments.at(m_dragHandleIndex) / 2.0f;
            m_dragInitialHandleCornerTranslation =
                    EditorUtils::totalAncestralScale(m_selectedEntity)
                    * m_dragInitialScaleValue * m_dragHandleCornerTranslation;
            m_dragInitialHandleMatrix = m_dragHandlesTransform->matrix();
        }
    }
}

void EditorScene::dragHandleMove(const QPoint &pos, bool shiftDown, bool ctrlDown, bool altDown)
{
    // Ignore initial minor drags
    if (m_ignoringInitialDrag) {
        QPoint delta = pos - m_previousMousePosition;
        if (delta.manhattanLength() > 10)
            m_ignoringInitialDrag = false;
    }
    if (!m_ignoringInitialDrag) {
        // If selected entity changes mid-drag, cancel drag.
        if (m_dragMode != DragNone && m_dragEntity != m_selectedEntity)
            cancelDrag();
        switch (m_dragMode) {
        case DragTranslate: {
            dragTranslateSelectedEntity(pos, shiftDown, ctrlDown, altDown);
            break;
        }
        case DragScale: {
            dragScaleSelectedEntity(pos, shiftDown, ctrlDown, altDown);
            break;
        }
        case DragRotate: {
            dragRotateSelectedEntity(pos, shiftDown, ctrlDown);
            break;
        }
        default:
            break;
        }
    }
}

void EditorScene::dragHandleRelease()
{
    cancelDrag();
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

    if (m_appTranslator->load(":/qt3deditorlib/editorlib_" + language)) {
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
    //: This string is entity name, no non-ascii characters allowed
    m_sceneRootString = tr("Scene root");
    m_saveFailString = tr("Failed to save the scene");
    m_loadFailString = tr("Failed to load a new scene");
    m_cameraString = tr("Camera");
    m_cubeString = tr("Cube");
    m_lightString = tr("Light");
    m_gltfExportFailString = tr("Failed to export GLTF scene");
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
            m_activeSceneCameraFrustumData.viewCenterPicker = nullptr;
        }
    }

    // Picker doesn't get disabled with the entity - we have to delete it to disable
    if (enable) {
        if (!cameraData.cameraPicker)
            cameraData.cameraPicker = createObjectPickerForEntity(cameraData.visibleEntity);
    } else {
        delete cameraData.cameraPicker;
        cameraData.cameraPicker = nullptr;
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
        lightData.visiblePicker = nullptr;
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

Qt3DRender::QObjectPicker *EditorScene::createObjectPickerForEntity(Qt3DCore::QEntity *entity)
{
    Qt3DRender::QObjectPicker *picker = nullptr;
    EditorSceneItem *item = m_sceneItems.value(entity->id());
    if (item && item->itemType() == EditorSceneItem::SceneLoader) {
        // Scene loaders need multiple pickers. Null picker is returned.
        createSceneLoaderChildPickers(entity, item->internalPickers());
    } else if (!item || item->itemType() != EditorSceneItem::Group) {
        // Group is not visible by itself (has no mesh), so no picker is needed
        picker = new Qt3DRender::QObjectPicker(entity);
        picker->setHoverEnabled(false);
        picker->setObjectName(QStringLiteral("__internal object picker ") + entity->objectName());
        entity->addComponent(picker);
        connect(picker, &Qt3DRender::QObjectPicker::pressed, this, &EditorScene::handlePickerPress);
    }

    return picker;
}

// Debug handle is useful for visually debugging calculated world positions
void EditorScene::showDebugHandle(bool show, int handleIndex, const QVector3D &worldPosition)
{
    QVector3D screenPoint;
    if (show) {
        Qt3DRender::QCamera *camera = frameGraphCamera();
        screenPoint = EditorUtils::projectRay(
                    camera->viewMatrix(), camera->projectionMatrix(),
                    m_viewport->width(), m_viewport->height(), worldPosition);
    }

    emit repositionDragHandle(DragDebug, QPoint(screenPoint.x(), screenPoint.y()),
                              show, handleIndex, 0);
}

void EditorScene::queueEnsureSelection()
{
    // Ensure that something is selected after the current pending events have executed
    if (m_sceneEntity && m_ensureSelectionEntityName.isEmpty()) {
        if (m_selectedEntity) {
            m_ensureSelectionEntityName = m_selectedEntity->objectName();
            clearSingleSelection();
        } else {
            m_ensureSelectionEntityName = m_sceneEntity->objectName();
        }
        QMetaObject::invokeMethod(this, "doEnsureSelection", Qt::QueuedConnection);
    }
}

void EditorScene::doEnsureSelection()
{
    if (!m_selectedEntity) {
        // If we have multiselection active, update the multiselection list
        const int count = m_selectedEntityNameList.size();
        if (count > 0) {
            QStringList newList;
            newList.reserve(count);
            for (int i = 0; i < count; ++i) {
                QString entityName = m_selectedEntityNameList.at(i);
                if (itemByName(entityName))
                    newList.append(entityName);
            }
            if (newList.size() == 1) {
                m_ensureSelectionEntityName = newList.at(0);
                newList.clear();
            }

            if (count != newList.size()) {
                m_selectedEntityNameList = newList;
                if (newList.size() == 0)
                    emit multiSelectionChanged(false);
            }
            checkMultiSelectionHighlights();
            emit multiSelectionListChanged();
        }
        if (!m_selectedEntityNameList.size()) {
            EditorSceneItem *item = itemByName(m_ensureSelectionEntityName);
            if (item)
                setSelection(item->entity());
            else
                setSelection(m_sceneEntity);
        }
    }
    m_ensureSelectionEntityName.clear();
}

void EditorScene::queueUpdateGroupSelectionBoxes()
{
    // Queue asynchronous update to group selection boxes
    if (!m_groupBoxUpdatePending) {
        m_groupBoxUpdatePending = true;
        QMetaObject::invokeMethod(this, "doUpdateGroupSelectionBoxes", Qt::QueuedConnection);
    }
}

bool EditorScene::canExportGltf()
{
#ifdef GLTF_EXPORTER_AVAILABLE
    static int canExportGltf = -1;

    if (canExportGltf < 0) {
        canExportGltf++;
        // Try to find the export plugin
        QStringList keys = Qt3DRender::QSceneExportFactory::keys();
        for (const QString &key : keys) {
            Qt3DRender::QSceneExporter *exporter =
                    Qt3DRender::QSceneExportFactory::create(key, QStringList());
            if (exporter != nullptr && key == QStringLiteral("gltfexport")) {
                m_gltfExporter = exporter;
                canExportGltf++;
                break;
            }
        }
    }

    return (canExportGltf > 0);
#else
    return false;
#endif
}

void EditorScene::doUpdateGroupSelectionBoxes()
{
    const QList<EditorSceneItem *> items = m_sceneItems.values();
    for (int i = 0; i < items.size(); ++i) {
        EditorSceneItem *item = items.at(i);
        if (item && item->itemType() == EditorSceneItem::Group && item->isSelectionBoxShowing())
            item->updateGroupExtents();
    }
    m_groupBoxUpdatePending = false;
}

void EditorScene::enableHelperArrows(bool enable)
{
    // TODO: Remove this function once disabling parent properly hides the children
    Q_FOREACH (QObject *child, m_helperArrows->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            childEntity->setEnabled(enable);
    }
    m_helperArrows->setEnabled(enable);
}

EditorSceneItem *EditorScene::itemByName(const QString &name)
{
    const QList<EditorSceneItem *> items = m_sceneItems.values();
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i)->entity()->objectName() == name)
            return items.at(i);
    }
    return nullptr;
}

void EditorScene::clearSingleSelection()
{
    if (m_selectedEntity) {
        EditorSceneItem *oldItem = m_sceneItems.value(m_selectedEntity->id(), nullptr);
        if (oldItem) {
            connectDragHandles(oldItem, false);
            oldItem->setShowSelectionBox(false);
        }
        m_selectedEntity = nullptr;
    }
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
        resizeConstantScreenSizeEntities();
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

void EditorScene::dragTranslateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown,
                                              bool altDown)
{
    // By default, translate along helper plane
    // When shift is pressed, translate along camera plane
    // When ctrl is pressed, snap to grid
    // When alt is pressed, translate along helper plane normal (lock to one axis)
    // Dragging helper arrows only translates along arrow vector. Ctrl translates in units of grid.

    Qt3DRender::QCamera *camera = frameGraphCamera();
    if (camera && m_selectedEntityTransform) {
        // For cameras, we need to use position instead of translation for correct results
        QVector3D entityTranslation = m_selectedEntityTransform->translation();
        Qt3DRender::QCamera *cameraEntity = qobject_cast<Qt3DRender::QCamera *>(m_selectedEntity);
        if (cameraEntity) {
            if (m_cameraViewCenterSelected && !m_viewCenterLocked)
                entityTranslation = cameraEntity->viewCenter();
            else
                entityTranslation = cameraEntity->position();
        }

        QVector3D helperNormal = helperPlaneNormal();
        QVector3D planeOrigin = m_dragInitialWorldTranslationValue;
        QVector3D planeNormal;
        const bool useCameraNormal =
                shiftDown || altDown || m_dragHandleIndex >= TranslateHandleArrowX;
        if (useCameraNormal)
            planeNormal = EditorUtils::cameraNormal(frameGraphCamera());
        else
            planeNormal = helperNormal;

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
                if (m_cameraViewCenterSelected && !m_viewCenterLocked)
                    propertyName = QStringLiteral("viewCenter");
                else
                    propertyName = QStringLiteral("position");
            } else {
                propertyName = QStringLiteral("translation");
            }

            QVector3D newPosition = intersection;
            if (ctrlDown && m_dragHandleIndex < TranslateHandleArrowX) {
                newPosition = snapPosition(intersection,
                                           useCameraNormal || helperNormal.x() < 0.5,
                                           useCameraNormal || helperNormal.y() < 0.5,
                                           useCameraNormal || helperNormal.z() < 0.5);
            }
            if (altDown && m_dragHandleIndex < TranslateHandleArrowX) {
                QVector3D snapPos = newPosition;
                newPosition = m_dragInitialEntityTranslationValue;
                if (helperNormal.x() > 0.5)
                    newPosition.setX(snapPos.x());
                else if (helperNormal.y() > 0.5)
                    newPosition.setY(snapPos.y());
                else if (helperNormal.z() > 0.5)
                    newPosition.setZ(snapPos.z());
            } else {
                if (m_dragHandleIndex >= TranslateHandleArrowX) {
                    QVector3D arrowVector = m_dragHandleIndex == TranslateHandleArrowX
                                ? QVector3D(1.0f, 0.0f, 0.0f)
                              : m_dragHandleIndex == TranslateHandleArrowY
                                  ? QVector3D(0.0f, 1.0f, 0.0f) : QVector3D(0.0f, 0.0f, 1.0f);

                    if (m_helperArrowsLocal)
                        arrowVector = m_dragInitialHandleRotationValue.rotatedVector(arrowVector);

                    QVector3D planeOffset = EditorUtils::projectVectorOnPlane(
                                m_helperArrowGrabOffset, planeNormal);

                    QVector3D initialPos = m_dragInitialWorldTranslationValue + planeOffset;
                    float distance = newPosition.distanceToPlane(initialPos,
                                                                 planeOffset.normalized());
                    if (ctrlDown)
                        distance = qRound(distance / m_gridSize) * m_gridSize;

                    newPosition = m_dragInitialWorldTranslationValue
                            + distance * arrowVector;
                }
                // If entity has parents with transfroms, those need to be applied in inverse
                QMatrix4x4 totalTransform = EditorUtils::totalAncestralTransform(m_selectedEntity);
                if ((m_dragHandleIndex == TranslateHandleBoxCenter
                        || (m_dragHandleIndex >= TranslateHandleArrowX && !m_helperArrowsLocal))
                        && !cameraEntity) {
                    newPosition = totalTransform.inverted()
                            * (newPosition + m_dragHandlesTransform->rotation()
                               * m_dragHandleTranslateTransform->translation());
                } else {
                    newPosition = totalTransform.inverted() * newPosition;
                }
            }

            m_undoHandler->createChangePropertyCommand(m_selectedEntity->objectName(), componentType,
                                                       propertyName, newPosition,
                                                       entityTranslation, true);
        }
    }
}

void EditorScene::dragScaleSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown,
                                          bool altDown)
{
    // By default, scale each dimension individually.
    // Only the dragged corner moves freely when scaling, the opposite corner is anchored.
    // When shift is pressed, scale uniformly
    // When ctrl is pressed, scale in integers
    // When alt is pressed, scale along helper plane normal (lock to one axis)

    QVector3D posOffset = dragHandlePositionOffset(newPos);
    if (!posOffset.isNull()) {
        QVector3D rotatedPosOffset = m_dragHandlesTransform->rotation().inverted() * posOffset;
        QVector3D moveFactors =
                EditorUtils::absVector3D(
                    QVector3D((2.0f * m_dragInitialHandleCornerTranslation)
                              + rotatedPosOffset)) / 2.0f;

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

        QVector3D newScale = m_dragInitialScaleValue * EditorUtils::maxVector3D(moveFactors,
                                                                                0.0001f);
        if (ctrlDown) {
            newScale.setX(qMax(qRound(newScale.x()), 1));
            newScale.setY(qMax(qRound(newScale.y()), 1));
            newScale.setZ(qMax(qRound(newScale.z()), 1));
        }
        if (altDown) {
            QVector3D helperNormal = helperPlaneNormal();
            QVector3D snapScale = newScale;
            newScale = m_dragInitialScaleValue;
            if (helperNormal.x() > 0.5f)
                newScale.setX(snapScale.x());
            else if (helperNormal.y() > 0.5f)
                newScale.setY(snapScale.y());
            else if (helperNormal.z() > 0.5f)
                newScale.setZ(snapScale.z());
        }

        // Calculate the translate needed to keep opposite corner anchored
        QMatrix4x4 ancestralTransform = EditorUtils::totalAncestralTransform(m_selectedEntity);
        QVector3D ancestralScale =
                EditorUtils::totalAncestralScale(m_selectedEntity) * newScale;
        QVector3D newHandleCornerTranslation = ancestralScale * m_dragHandleCornerTranslation;
        EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id(), nullptr);
        if (selectedItem)
            newHandleCornerTranslation -= ancestralScale * selectedItem->entityMeshCenter();
        QVector3D newTranslation = ancestralTransform.inverted() * m_dragInitialHandleMatrix
                * (newHandleCornerTranslation - m_dragInitialHandleCornerTranslation);

        m_undoHandler->createChangePropertyCommand(m_selectedEntity->objectName(),
                                                   EditorSceneItemComponentsModel::Transform,
                                                   QStringLiteral("scale3D"), newScale,
                                                   m_selectedEntityTransform->scale3D(),
                                                   QStringLiteral("translation"), newTranslation,
                                                   m_selectedEntityTransform->translation(), true);
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
            QVector3D checkVec = EditorUtils::rotateVector(
                        unrotatedHandlePos, EditorUtils::cameraNormal(frameGraphCamera()),
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
                desiredPos = EditorUtils::rotateVector(
                            unrotatedHandlePos,
                            EditorUtils::cameraNormal(frameGraphCamera()),
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
                if (cameraNormal.distanceToPlane(
                            QVector3D(), EditorUtils::cameraNormal(frameGraphCamera())) < 0.0f) {
                    cameraNormal = -cameraNormal;
                }
                QVector3D initialUpVector =
                        EditorUtils::projectVectorOnPlane(m_dragInitialRotateCustomVector.normalized(),
                                                          cameraNormal);
                QQuaternion planeRotation =
                        QQuaternion::rotationTo(EditorUtils::cameraNormal(frameGraphCamera()),
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
                    rotationAxis = EditorUtils::cameraNormal(frameGraphCamera());
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
                    rotationAxis = EditorUtils::cameraNormal(frameGraphCamera());
                } else {
                    // Rotate vectors so that they lie on helper plane instead of camera plane
                    QQuaternion planeRotation =
                            QQuaternion::rotationTo(EditorUtils::cameraNormal(frameGraphCamera()),
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
                    ? QStringLiteral("worldDirection") : QStringLiteral("localDirection");
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
                    rotationAxis = EditorUtils::cameraNormal(frameGraphCamera());
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
                    QQuaternion planeRotation =
                            QQuaternion::rotationTo(EditorUtils::cameraNormal(frameGraphCamera()),
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

        QVector3D planeOrigin = m_dragInitialCenterTranslation;

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
            posOffset = intersection - (m_dragInitialCenterTranslation
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
    EditorSceneItem *selectedItem = m_sceneItems.value(m_selectedEntity->id(), nullptr);
    if (item && item == selectedItem) {
        if (item->itemType() == EditorSceneItem::Camera) {
            QString upVectorLock = QStringLiteral("upVector") + lockPropertySuffix();
            QString positionLock = QStringLiteral("position") + lockPropertySuffix();
            QString viewCenterLock = QStringLiteral("viewCenter") + lockPropertySuffix();
            if (lockProperty == upVectorLock)
                m_dragHandleRotateTransform->setEnabled(!locked);
            else if (lockProperty == positionLock)
                m_dragHandleTranslateTransform->setEnabled(!locked);
            else if (lockProperty == viewCenterLock)
                m_viewCenterLocked = locked;
        } else {
            if (lockProperty == lockTransformPropertyName()) {
                Qt3DCore::QTransform *transform =
                        EditorUtils::entityTransform(m_selectedEntity);
                if (item->itemType() == EditorSceneItem::Light) {
                    if (locked) {
                        m_dragHandleTranslateTransform->setEnabled(false);
                    } else {
                        m_dragHandleTranslateTransform->setEnabled(
                                    !isPropertyLocked(QStringLiteral("translation"), transform));
                    }
                } else {
                    if (locked) {
                        m_dragHandleTranslateTransform->setEnabled(false);
                        m_dragHandleScaleTransforms.at(0)->setEnabled(false);
                        m_dragHandleRotateTransform->setEnabled(false);
                    } else {
                        m_dragHandleTranslateTransform->setEnabled(
                                    !isPropertyLocked(QStringLiteral("translation"), transform));
                        m_dragHandleScaleTransforms.at(0)->setEnabled(
                                    !isPropertyLocked(QStringLiteral("scale3D"), transform));
                        m_dragHandleRotateTransform->setEnabled(
                                    !isPropertyLocked(QStringLiteral("rotation"), transform));
                    }
                }
            } else {
                QString translateLock = QStringLiteral("translation") + lockPropertySuffix();
                if (lockProperty == translateLock) {
                    m_dragHandleTranslateTransform->setEnabled(!locked);
                } else if (item->itemType() == EditorSceneItem::Light) {
                    if (item->canRotate()) {
                        QString directionLock = QStringLiteral("localDirection") + lockPropertySuffix();
                        QString worldDirectionLock = QStringLiteral("worldDirection") + lockPropertySuffix();
                        if (lockProperty == directionLock || lockProperty == worldDirectionLock)
                            m_dragHandleRotateTransform->setEnabled(!locked);
                    }
                } else {
                    QString scaleLock = QStringLiteral("scale3D") + lockPropertySuffix();
                    QString rotateLock = QStringLiteral("rotation") + lockPropertySuffix();
                    if (lockProperty == scaleLock)
                        m_dragHandleScaleTransforms.at(0)->setEnabled(!locked);
                    else if (lockProperty == rotateLock)
                        m_dragHandleRotateTransform->setEnabled(!locked);
                }
            }
        }
        handleSelectionTransformChange();
    }
}

void EditorScene::handleLightTypeChanged(EditorSceneItem *item)
{
    if (item) {
        Qt3DRender::QAbstractLight *light = EditorUtils::entityLight(item->entity());
        if (light) {
            LightData *lightData = m_sceneLights.value(item->entity()->id());
            if (lightData) {
                lightData->lightComponent = light;
                connect(light, &Qt3DRender::QAbstractLight::colorChanged,
                        lightData->visibleMaterial, &Qt3DExtras::QPhongAlphaMaterial::setAmbient);
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
                }
                lightData->visibleEntity->addComponent(lightData->visibleMesh);
            }
            if (item->entity() == m_selectedEntity) {
                if (item->canRotate()) {
                    QString lockProperty =
                            qobject_cast<Qt3DRender::QDirectionalLight *>(lightData->lightComponent)
                            ? QStringLiteral("worldDirection") : QStringLiteral("localDirection");
                    m_dragHandleRotateTransform->setEnabled(
                                !isPropertyLocked(lockProperty, light));
                } else {
                    m_dragHandleRotateTransform->setEnabled(false);
                }
                item->updateSelectionBoxTransform();
                updateLightVisibleTransform(item->entity());
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
    if (camera != nullptr) {
        int cameraIndex = cameraIndexForEntity(camera);
        if (cameraIndex >= 0) {
            enableVisibleCamera(m_sceneCameras[cameraIndex], freeViewEnabled,
                                cameraIndex == m_activeSceneCameraIndex);
        }

    } else if (EditorUtils::entityLight(entity) != nullptr) {
        LightData *lightData = m_sceneLights.value(entity->id());
        if (lightData)
            enableVisibleLight(*lightData, freeViewEnabled);

    } else {
        EditorSceneItem *item = m_sceneItems.value(entity->id());
        if (item && item->itemType() == EditorSceneItem::SceneLoader) {
            if (enabled) {
                if (item->internalPickers()->size() == 0)
                    createObjectPickerForEntity(entity);
            } else {
                Q_FOREACH (Qt3DRender::QObjectPicker *picker, *item->internalPickers())
                    delete picker;
                item->internalPickers()->clear();
            }
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
}

void EditorScene::setError(const QString &errorString)
{
    m_errorString = errorString;
    emit errorChanged(m_errorString);
    qWarning() << m_errorString;
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
    diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/images/qtlogo.png")));
    Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
    diffuseMat->normal()->addTextureImage(normalTextureImage);
    normalTextureImage->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/images/qtlogo_normal.png")));
    Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
    diffuseMat->specular()->addTextureImage(specularTextureImage);
    specularTextureImage->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/images/qtlogo_specular.png")));
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
    Qt3DRender::QAbstractLight *light = new Qt3DRender::QAbstractLight(m_sceneEntity);
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
    Qt3DExtras::QCuboidMesh *cubeMesh = new Qt3DExtras::QCuboidMesh();
    Qt3DCore::QTransform *cubeTransform = new Qt3DCore::QTransform();
    cubeTransform->setTranslation(QVector3D(0.0f, 0.0f, 5.0f));
    cubeTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), 180.0f));
    Qt3DExtras::QDiffuseSpecularMapMaterial *cubeMaterial
            = new Qt3DExtras::QDiffuseSpecularMapMaterial();
    Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
    cubeMaterial->diffuse()->addTextureImage(diffuseTextureImage);
    diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/images/qtlogo.png")));
    Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
    cubeMaterial->specular()->addTextureImage(specularTextureImage);
    specularTextureImage->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/images/qtlogo_specular.png")));
    cubeMaterial->setAmbient(Qt::black);
    cubeMaterial->setShininess(150.0f);
    cubeEntity->addComponent(cubeMesh);
    cubeEntity->addComponent(cubeTransform);
    cubeEntity->addComponent(cubeMaterial);
    addEntity(cubeEntity);

    // Light
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_sceneEntity);
    lightEntity->setObjectName(m_lightString);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(m_sceneEntity);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 10.0f, -5.0f));
    lightEntity->addComponent(light);
    lightEntity->addComponent(lightTransform);
    addEntity(lightEntity);
#endif
    setActiveSceneCameraIndex(0);
    m_sceneModel->clearExpandedItems();
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
    Qt3DExtras::QPhongMaterial *selectionBoxMaterial = new Qt3DExtras::QPhongMaterial();
    selectionBoxMaterial->setAmbient(selectionBoxColor);
    selectionBoxMaterial->setDiffuse(QColor(Qt::black));
    selectionBoxMaterial->setSpecular(QColor(Qt::black));
    selectionBoxMaterial->setShininess(0);
    m_selectionBoxMaterial = selectionBoxMaterial;
    m_selectionBoxMesh = EditorUtils::createWireframeBoxMesh();

    m_meshCenterIndicatorLine = new Qt3DCore::QEntity();
    m_meshCenterIndicatorLine->setObjectName(QStringLiteral("__internal mesh center indicator line"));
    Qt3DRender::QGeometryRenderer *indicatorMesh = EditorUtils::createSingleLineMesh();

    Qt3DRender::QMaterial *meshCenterLineMaterial = new Qt3DRender::QMaterial();
    meshCenterLineMaterial->setEffect(new OnTopEffect());
    meshCenterLineMaterial->addParameter(new Qt3DRender::QParameter(QStringLiteral("handleColor"),
                                                                    selectionBoxColor));

    m_meshCenterIndicatorLineTransform = new Qt3DCore::QTransform();
    m_meshCenterIndicatorLine->addComponent(indicatorMesh);
    m_meshCenterIndicatorLine->addComponent(meshCenterLineMaterial);
    m_meshCenterIndicatorLine->addComponent(m_meshCenterIndicatorLineTransform);
    m_meshCenterIndicatorLine->setParent(m_rootEntity);
    m_meshCenterIndicatorLine->setEnabled(false);

    // Save to cache, as these are needed after Load/New
    m_componentCache->addComponent(m_selectionBoxMesh);
    m_componentCache->addComponent(m_selectionBoxMaterial);

    m_rootItem = new EditorSceneItem(this, m_rootEntity, nullptr, -1, this);

    m_sceneItems.insert(m_rootEntity->id(), m_rootItem);

    m_renderSettings = new Qt3DRender::QRenderSettings();
    m_renderSettings->pickingSettings()->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
    m_renderSettings->pickingSettings()->setPickResultMode(Qt3DRender::QPickingSettings::AllPicks);
    m_renderSettings->setObjectName(QStringLiteral("__internal Scene frame graph"));
    m_renderer = new Qt3DExtras::QForwardRenderer();
    m_renderer->setClearColor(Qt::lightGray);
    m_renderSettings->setActiveFrameGraph(m_renderer);

    // Setting the FrameGraph to actual root entity to protect it from accidental removal
    m_rootEntity->addComponent(m_renderSettings);

    m_rootEntity->addComponent(new Qt3DInput::QInputSettings());

    // Scene entity (i.e. the visible root)
    setSceneEntity();

    // Free view camera
    m_freeViewCameraEntity = new Qt3DRender::QCamera(m_rootEntity);
    m_freeViewCameraEntity->setObjectName(QStringLiteral("__internal free view camera"));
    resetFreeViewCamera();

    // Helper plane
    createHelperPlane();

    // Helper arrows
    createHelperArrows();

    // The drag handles translation is same as the selection box + a specified distance
    // depending on the scale of the box.
    m_dragHandlesTransform = new Qt3DCore::QTransform();
    m_dragHandleRotateTransform = new Qt3DCore::QTransform();
    m_dragHandleTranslateTransform = new Qt3DCore::QTransform();
    // Grab explicit ownership of drag transforms as they are not going to be part of the scene
    QQmlEngine::setObjectOwnership(m_dragHandlesTransform, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(m_dragHandleRotateTransform, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(m_dragHandleTranslateTransform, QQmlEngine::CppOwnership);

    m_dragHandleScaleTransforms.resize(dragCornerHandleCount);
    m_dragHandleCornerAdjustments.resize(dragCornerHandleCount);
    for (int i = 0; i < dragCornerHandleCount; i++) {
        m_dragHandleScaleTransforms[i] = new Qt3DCore::QTransform();
        QQmlEngine::setObjectOwnership(m_dragHandleScaleTransforms.at(i), QQmlEngine::CppOwnership);
    }
    m_dragHandleCornerAdjustments[0] = QVector3D(-1.0f, -1.0f, -1.0f);
    m_dragHandleCornerAdjustments[1] = QVector3D(-1.0f, -1.0f,  1.0f);
    m_dragHandleCornerAdjustments[2] = QVector3D(-1.0f,  1.0f, -1.0f);
    m_dragHandleCornerAdjustments[3] = QVector3D(-1.0f,  1.0f,  1.0f);
    m_dragHandleCornerAdjustments[4] = QVector3D( 1.0f, -1.0f, -1.0f);
    m_dragHandleCornerAdjustments[5] = QVector3D( 1.0f, -1.0f,  1.0f);
    m_dragHandleCornerAdjustments[6] = QVector3D( 1.0f,  1.0f, -1.0f);
    m_dragHandleCornerAdjustments[7] = QVector3D( 1.0f,  1.0f,  1.0f);

    m_dragEntitySnapOffsets.resize(dragCornerHandleCount);

    // Active scene camera frustum visualization
    m_activeSceneCameraFrustumData.frustumEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_activeSceneCameraFrustumData.viewVectorEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_activeSceneCameraFrustumData.viewCenterEntity = new Qt3DCore::QEntity(m_rootEntity);

    m_activeSceneCameraFrustumData.frustumMesh = EditorUtils::createWireframeBoxMesh();
    Qt3DRender::QGeometryRenderer *viewVectorMesh = EditorUtils::createCameraViewVectorMesh();
    Qt3DRender::QGeometryRenderer *viewCenterMesh = EditorUtils::createCameraViewCenterMesh(1.0f);

    Qt3DExtras::QPhongMaterial *frustumMaterial = new Qt3DExtras::QPhongMaterial();
    frustumMaterial->setAmbient(cameraFrustumColor);
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

    Qt3DExtras::QPhongMaterial *helperPlaneMaterial = new Qt3DExtras::QPhongMaterial();
    helperPlaneMaterial->setAmbient(helperPlaneColor);
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

void EditorScene::createHelperArrows()
{
    m_helperArrowHandleIndexMap.clear();
    m_helperArrows = new Qt3DCore::QEntity();
    m_helperArrows->setObjectName(QStringLiteral("__internal helper arrows"));

    QMatrix4x4 matrix;
    Qt3DCore::QEntity *arrow = EditorUtils::createArrowEntity(helperArrowColorY, m_helperArrows,
                                                              matrix, helperArrowName);
    createObjectPickerForEntity(arrow);
    m_helperArrowHandleIndexMap.insert(arrow, TranslateHandleArrowY);

    matrix.rotate(90.0f, QVector3D(1.0f, 0.0f, 0.0f));
    arrow = EditorUtils::createArrowEntity(helperArrowColorZ, m_helperArrows, matrix,
                                           helperArrowName);
    createObjectPickerForEntity(arrow);
    m_helperArrowHandleIndexMap.insert(arrow, TranslateHandleArrowZ);

    matrix = QMatrix();
    matrix.rotate(-90.0f, QVector3D(0.0f, 0.0f, 1.0f));
    arrow = EditorUtils::createArrowEntity(helperArrowColorX, m_helperArrows, matrix,
                                           helperArrowName);
    createObjectPickerForEntity(arrow);
    m_helperArrowHandleIndexMap.insert(arrow, TranslateHandleArrowX);

    m_helperArrowsTransform = new Qt3DCore::QTransform();
    m_helperArrows->addComponent(m_helperArrowsTransform);
    m_helperArrows->setParent(m_rootEntity);
    enableHelperArrows(false);
}

void EditorScene::setFrameGraphCamera(Qt3DCore::QEntity *cameraEntity)
{
    if (m_renderer) {
        Qt3DCore::QTransform *cameraTransform = nullptr;
        Qt3DRender::QCamera *currentCamera =
                qobject_cast<Qt3DRender::QCamera *>(m_renderer->camera());
        if (currentCamera) {
            cameraTransform = currentCamera->transform();
            if (cameraTransform) {
                disconnect(cameraTransform, &Qt3DCore::QTransform::matrixChanged,
                           this, &EditorScene::handleSelectionTransformChange);
            }
        }
        m_renderer->setCamera(cameraEntity);
        currentCamera = qobject_cast<Qt3DRender::QCamera *>(cameraEntity);
        if (currentCamera) {
            cameraTransform = currentCamera->transform();
            if (cameraTransform) {
                connect(cameraTransform, &Qt3DCore::QTransform::matrixChanged,
                        this, &EditorScene::handleSelectionTransformChange);
            }
        }
        // This will update drag handle positions if needed
        handleSelectionTransformChange();
    }
}

Qt3DRender::QCamera *EditorScene::frameGraphCamera() const
{
    if (m_renderer)
        return qobject_cast<Qt3DRender::QCamera *>(m_renderer->camera());
    else
        return nullptr;
}

void EditorScene::setSelection(Qt3DCore::QEntity *entity)
{
    // Setting selection implies end to multiSelection
    if (m_selectedEntityNameList.size())
        clearMultiSelection();
    EditorSceneItem *item = m_sceneItems.value(entity->id(), nullptr);
    if (item) {
        if (entity != m_selectedEntity) {
            clearSingleSelection();

            m_selectedEntity = entity;

            if (m_selectedEntity) {
                connectDragHandles(item, true);
                if (m_selectedEntity != m_sceneEntity)
                    item->setShowSelectionBox(true);
                m_selectedEntityTransform = EditorUtils::entityTransform(m_selectedEntity);
            }

            // Emit signal to highlight the entity from the list
            emit selectionChanged(m_selectedEntity);
        }
        m_dragHandlesTransform->setEnabled(item->isSelectionBoxShowing());
        enableHelperArrows(item->isSelectionBoxShowing());

        if (item->itemType() == EditorSceneItem::Camera) {
            // Disable scale handles for cameras
            m_dragHandleScaleTransforms.at(0)->setEnabled(false);
            m_dragHandleRotateTransform->setEnabled(!isPropertyLocked(QStringLiteral("upVector"),
                                                                      m_selectedEntity));
            m_dragHandleTranslateTransform->setEnabled(
                        !isPropertyLocked(QStringLiteral("position"), m_selectedEntity));
            m_viewCenterLocked = isPropertyLocked(QStringLiteral("viewCenter"), m_selectedEntity);
        } else {
            Qt3DCore::QTransform *transform = EditorUtils::entityTransform(m_selectedEntity);
            bool transformPropertiesLocked = item->customProperty(m_selectedEntity,
                                                                  lockTransformPropertyName()).toBool();
            if (transformPropertiesLocked) {
                m_dragHandleTranslateTransform->setEnabled(false);
            } else {
                m_dragHandleTranslateTransform->setEnabled(
                            !isPropertyLocked(QStringLiteral("translation"), transform));
            }
            if (item->itemType() == EditorSceneItem::Light) {
                // Disable scale handles for lights
                m_dragHandleScaleTransforms.at(0)->setEnabled(false);
                // Some lights can rotate
                if (item->canRotate()) {
                    Qt3DRender::QAbstractLight *light = EditorUtils::entityLight(m_selectedEntity);
                    QString lockProperty =
                            qobject_cast<Qt3DRender::QDirectionalLight *>(light)
                            ? QStringLiteral("worldDirection") : QStringLiteral("localDirection");
                    m_dragHandleRotateTransform->setEnabled(
                                !isPropertyLocked(lockProperty, light));
                } else {
                    m_dragHandleRotateTransform->setEnabled(false);
                }
            } else {
                if (transformPropertiesLocked) {
                    m_dragHandleScaleTransforms.at(0)->setEnabled(false);
                    m_dragHandleRotateTransform->setEnabled(false);
                } else {
                    m_dragHandleScaleTransforms.at(0)->setEnabled(!isPropertyLocked(QStringLiteral("scale3D"),
                                                                                    transform));
                    m_dragHandleRotateTransform->setEnabled(!isPropertyLocked(QStringLiteral("rotation"),
                                                                              transform));
                }
            }
        }

        // Update drag handles transforms to initial state
        handleSelectionTransformChange();
    } else {
        m_dragHandlesTransform->setEnabled(false);
        enableHelperArrows(false);
        if (m_selectedEntity != m_sceneEntity)
            setSelection(m_sceneEntity);
    }
}

void EditorScene::toggleEntityMultiSelection(const QString &name)
{
    // If the new one is already in, remove it. Otherwise add it.
    if (m_selectedEntityNameList.contains(name))
        removeEntityFromMultiSelection(name);
    else
        addEntityToMultiSelection(name);
}

void EditorScene::clearMultiSelection()
{
    if (m_selectedEntityNameList.size() > 0) {
        m_selectedEntityNameList.clear();
        checkMultiSelectionHighlights();
        emit multiSelectionChanged(false);
        emit multiSelectionListChanged();
        setSelection(m_sceneEntity);
    }
}

QVector3D EditorScene::getMultiSelectionCenter()
{
    QVector3D pos;
    for (int i = 0; i < m_selectedEntityNameList.size(); i++) {
        EditorSceneItem *item = itemByName(m_selectedEntityNameList.at(i));
        if (item) {
            item->doUpdateSelectionBoxTransform();
            pos += item->selectionBoxCenter();
        }
    }
    return m_selectedEntityNameList.size() ? (pos / m_selectedEntityNameList.size()) : QVector3D();
}

void EditorScene::updateWorldPositionLabel(int xPos, int yPos)
{
    updateWorldPositionLabel(getWorldPosition(xPos,yPos));
}

void EditorScene::updateWorldPositionLabelToDragHandle(EditorScene::DragMode dragMode,
                                                       int handleIndex)
{
    // All handles show actual handle position
    QMatrix4x4 matrix = m_dragHandlesTransform->matrix();
    switch (dragMode) {
    case EditorScene::DragTranslate:
        if (handleIndex == TranslateHandleMeshCenter
                || (handleIndex >= TranslateHandleArrowX && m_helperArrowsLocal)) {
            matrix *= m_dragHandleTranslateTransform->matrix();
        }
        break;
    case EditorScene::DragRotate:
        matrix *= m_dragHandleRotateTransform->matrix();
        break;
    case EditorScene::DragScale:
        matrix *= m_dragHandleScaleTransforms.at(handleIndex)->matrix();
        break;
    }

    updateWorldPositionLabel(matrix * QVector3D());
}

void EditorScene::changeCameraPosition(EditorScene::CameraPosition preset)
{
    Qt3DRender::QCamera *camera = frameGraphCamera();

    QVector3D cameraDirection;
    QVector3D up(0.0f, 0.0f, 0.0f);
    switch (preset) {
    case CameraPositionTop:
        cameraDirection.setY(1.0f);
        up.setZ(1.0f);
        break;
    case CameraPositionBottom:
        cameraDirection.setY(-1.0f);
        up.setZ(-1.0f);
        break;
    case CameraPositionLeft:
        cameraDirection.setX(1.0f);
        up.setY(1.0f);
        break;
    case CameraPositionRight:
        cameraDirection.setX(-1.0f);
        up.setY(1.0f);
        break;
    case CameraPositionFront:
        cameraDirection.setZ(-1.0f);
        up.setY(1.0f);
        break;
    case CameraPositionBack:
        cameraDirection.setZ(1.0f);
        up.setY(1.0f);
        break;
    default:
        return;
    }

    // Keep the current distance and viewcenter, but change upvector to properly orient the camera.
    float len = camera->viewVector().length();
    camera->setPosition(camera->viewCenter() + cameraDirection * len);
    camera->setUpVector(up);
}

bool EditorScene::exportGltfScene(const QUrl &folder, const QString &exportName,
                                  bool exportSelected, const QJSValue &options)
{
#ifdef GLTF_EXPORTER_AVAILABLE
    if (canExportGltf()) {
        QString exportDir = folder.toLocalFile();
        QVariantHash optionsHash;
        const QString binaryKey = QStringLiteral("binaryJson");
        const QString compactKey = QStringLiteral("compactJson");
        if (options.hasProperty(binaryKey))
            optionsHash.insert(binaryKey, options.property(binaryKey).toBool());
        if (options.hasProperty(compactKey))
            optionsHash.insert(compactKey, options.property(compactKey).toBool());
        if (exportDir.length() > 0 && exportName.length() > 0) {
            Qt3DCore::QEntity *exportEntity = m_selectedEntity;
            if (!exportSelected || !exportEntity)
                exportEntity = m_sceneEntity;
            if (!m_gltfExporter->exportScene(exportEntity, exportDir, exportName, optionsHash))
                setError(m_gltfExportFailString);
            else
                return true;
        } else {
            setError(m_gltfExportFailString);
        }
    }
#else
    Q_UNUSED(folder)
    Q_UNUSED(exportName)
    Q_UNUSED(exportSelected)
    Q_UNUSED(options)
#endif
    return false;
}

void EditorScene::updateWorldPositionLabel(const QVector3D &worldPos)
{
    emit worldPositionLabelUpdate(QString::number(qreal(worldPos.x()), 'f', 2),
                                  QString::number(qreal(worldPos.y()), 'f', 2),
                                  QString::number(qreal(worldPos.z()), 'f', 2));
}

void EditorScene::addEntityToMultiSelection(const QString &name)
{
    const int oldSize = m_selectedEntityNameList.size();
    if (oldSize == 0) {
        // Do not add if multiselecting the currently selected entity as the first entity
        if (m_selectedEntity->objectName() == name)
            return;

        if (m_selectedEntity != m_sceneEntity) {
            m_selectedEntityNameList.append(m_selectedEntity->objectName());
            m_dragHandlesTransform->setEnabled(false);
            enableHelperArrows(false);
            handleSelectionTransformChange();
            clearSingleSelection();
        } else {
            // Just single-select the new entity and return if the other entity was scene entity
            EditorSceneItem *item = m_sceneModel->getItemByName(name);
            if (item) {
                setSelection(item->entity());
                return;
            }
        }
    }
    m_selectedEntityNameList.append(name);

    checkMultiSelectionHighlights();

    if (oldSize == 0)
        emit multiSelectionChanged(true);
    emit multiSelectionListChanged();
}

void EditorScene::removeEntityFromMultiSelection(const QString &name)
{
    bool removed = m_selectedEntityNameList.removeOne(name);

    if (removed) {
        bool lastRemoved = m_selectedEntityNameList.size() == 1;
        EditorSceneItem *lastItem = nullptr;
        if (lastRemoved) {
            lastItem = m_sceneModel->getItemByName(m_selectedEntityNameList.at(0));
            m_selectedEntityNameList.clear();
            emit multiSelectionChanged(false);
        }
        checkMultiSelectionHighlights();
        emit multiSelectionListChanged();
        if (lastRemoved) {
            if (lastItem)
                setSelection(lastItem->entity());
            else
                setSelection(m_sceneEntity);
        }
    }
}

void EditorScene::renameEntityInMultiSelectionList(const QString &oldName, const QString &newName)
{
    int index = m_selectedEntityNameList.indexOf(oldName);
    if (index > 0)
        m_selectedEntityNameList.replace(index, newName);
}

void EditorScene::setClipboardOperation(ClipboardOperation operation)
{
    if (operation != m_clipboardOperation) {
        m_clipboardOperation = operation;
        emit clipboardOperationChanged(operation);
        if (operation == ClipboardNone) {
            m_clipboardEntityName.clear();
            emit clipboardContentChanged(m_clipboardEntityName);
        }
    }
}

void EditorScene::setClipboardContent(const QString &entityName)
{
    if (entityName != m_clipboardEntityName) {
        m_clipboardEntityName = entityName;
        emit clipboardContentChanged(entityName);
    }
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

void EditorScene::setHelperArrowsLocal(bool enable)
{
    if (enable != m_helperArrowsLocal) {
        m_helperArrowsLocal = enable;
        handleSelectionTransformChange();
        emit helperArrowsLocalChanged(m_helperArrowsLocal);
    }
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

void EditorScene::clearSelectionBoxes(Qt3DCore::QEntity *skipEntity)
{
    Q_FOREACH (EditorSceneItem *item, m_sceneItems.values()) {
        if (item->entity() != skipEntity)
            item->setShowSelectionBox(false);
    }
}

void EditorScene::endSelectionHandling()
{
    if (m_dragMode == DragNone && m_pickedEntity) {
        if (m_ctrlDownOnLastLeftPress) {
            // Multiselection handling
            toggleEntityMultiSelection(m_pickedEntity->objectName());
        } else {
            // Single selection handling
            setSelection(m_pickedEntity);

            // Selecting an object also starts drag, if translate handle is enabled
            Qt3DRender::QCamera *cameraEntity = qobject_cast<Qt3DRender::QCamera *>(m_pickedEntity);
            bool viewCenterDrag = cameraEntity && m_cameraViewCenterSelected && !m_viewCenterLocked;
            bool entityDrag = m_dragHandleTranslateTransform->isEnabled()
                    && m_dragHandlesTransform->isEnabled()
                    && (!cameraEntity || !m_cameraViewCenterSelected);
            if (viewCenterDrag || entityDrag) {
                // The mouse position passed to dragHandlePress is irrelevant in this case
                dragHandlePress(DragTranslate, QPoint(0, 0), 0);
            }
        }
        m_pickedEntity = nullptr;
        m_pickedDistance = -1.0f;
    }
}

void EditorScene::handleSelectionTransformChange()
{
    EditorSceneItem *item = nullptr;
    if (m_selectedEntity && m_selectedEntity != m_sceneEntity)
        item = m_sceneItems.value(m_selectedEntity->id(), nullptr);
    QPoint translatePoint;
    QPoint centerPoint;
    QVector3D translateHandlePos;
    QVector3D itemCenterHandlePos;
    QVector3D rotateHandlePos;
    QVector3D cornerHandlePositions[dragCornerHandleCount];
    bool showCenterHandle = false;

    if (item) {
        Qt3DRender::QCamera *camera = frameGraphCamera();

        m_dragHandlesTransform->setTranslation(item->selectionBoxCenter());
        m_dragHandlesTransform->setRotation(item->selectionTransform()->rotation());

        QVector3D translation = (item->unadjustedSelectionBoxExtents() / 2.0f);

        // m_dragHandleTranslateTransform indicates the mesh center position in drag handles
        // coordinates, i.e. the position of the secondary translate handle.
        // The primary handle at the center of the selection box always has zero translation.
        QMatrix4x4 entityMatrix = EditorUtils::totalAncestralTransform(item->entity())
                * item->entityTransform()->matrix();
        QVector3D meshCenter = m_dragHandlesTransform->matrix().inverted() * entityMatrix
                * QVector3D();
        m_dragHandleTranslateTransform->setTranslation(meshCenter);

        // Find out x/y viewport positions of drag handles

        translateHandlePos = EditorUtils::projectRay(
                    camera->viewMatrix(), camera->projectionMatrix(),
                    m_viewport->width(), m_viewport->height(),
                    m_dragHandlesTransform->matrix() * QVector3D());

        for (int i = 0; i < dragCornerHandleCount; i++) {
            m_dragHandleScaleTransforms.at(i)->setTranslation(
                        translation * m_dragHandleCornerAdjustments.at(i));

            cornerHandlePositions[i] = EditorUtils::projectRay(
                        camera->viewMatrix(), camera->projectionMatrix(),
                        m_viewport->width(), m_viewport->height(),
                        m_dragHandlesTransform->matrix()
                        * m_dragHandleScaleTransforms.at(i)->matrix() * QVector3D());
        }

        // Try to position rotate handle to the upper right corner of the selection box, as
        // drawn on the screen. However, we don't change the corner during drag-rotation.
        const float handleDistance = 12.0f;
        if (m_dragMode != DragRotate) {
            float maxDelta = 0.0f;
            int rotateHandleIndex = 0;
            for (int i = 0; i < dragCornerHandleCount; i++) {
                float delta = (cornerHandlePositions[i].x() - translateHandlePos.x())
                        + (translateHandlePos.y() - cornerHandlePositions[i].y());
                if (delta > maxDelta) {
                    rotateHandleIndex = i;
                    maxDelta = delta;
                }
            }
            m_dragHandleRotationAdjustment = m_dragHandleCornerAdjustments.at(rotateHandleIndex);
            rotateHandlePos = cornerHandlePositions[rotateHandleIndex]
                    + QVector3D(handleDistance, -handleDistance, 0.0f);
        }

        m_dragHandleRotateTransform->setTranslation(translation * m_dragHandleRotationAdjustment);

        if (m_dragMode == DragRotate) {
            // When drag-rotating, we can't rely on drag handle being on top right, so we need
            // to calculate the correct screen position so that it is always away from the
            // object center.
            rotateHandlePos = EditorUtils::projectRay(
                        camera->viewMatrix(), camera->projectionMatrix(),
                        m_viewport->width(), m_viewport->height(),
                        m_dragHandlesTransform->matrix() * m_dragHandleRotateTransform->matrix()
                        * QVector3D());

            // Get another point to help adjust rotation handle position
            QMatrix4x4 rotateHelperMatrix;
            rotateHelperMatrix.translate(translation * m_dragHandleRotationAdjustment * 1.1f);
            QVector3D rotateHelperPos = EditorUtils::projectRay(
                        camera->viewMatrix(), camera->projectionMatrix(),
                        m_viewport->width(), m_viewport->height(),
                        m_dragHandlesTransform->matrix() * rotateHelperMatrix * QVector3D());

            // Adjust rotate handle position number of pixels towards the helper point
            QVector3D adjustVector = rotateHelperPos - rotateHandlePos;
            adjustVector.normalize();
            adjustVector *= handleDistance * float(qSqrt(2.0));
            rotateHandlePos += adjustVector;
        }

        translatePoint = QPoint(translateHandlePos.x(), translateHandlePos.y());
        if (item->itemType() != EditorSceneItem::Camera
                && item->itemType() != EditorSceneItem::Light) {
            itemCenterHandlePos = EditorUtils::projectRay(
                        camera->viewMatrix(), camera->projectionMatrix(),
                        m_viewport->width(), m_viewport->height(),
                        m_dragHandlesTransform->matrix() * m_dragHandleTranslateTransform->matrix()
                        * QVector3D());
            centerPoint = QPoint(itemCenterHandlePos.x(), itemCenterHandlePos.y());
            showCenterHandle = translatePoint != centerPoint;
        }
        if (showCenterHandle) {
            QQuaternion rot = QQuaternion::rotationTo(QVector3D(0.0f, 0.0f, 1.0f),
                                                      meshCenter.normalized());
            m_meshCenterIndicatorLineTransform->setRotation(
                        m_dragHandlesTransform->rotation() * rot);
            m_meshCenterIndicatorLineTransform->setTranslation(
                        m_dragHandlesTransform->translation());
            m_meshCenterIndicatorLineTransform->setScale(meshCenter.length());
        }

        // Move the helper arrows to the center of the entity
        if (m_helperArrowsLocal) {
            if (showCenterHandle) {
                m_helperArrowsTransform->setMatrix(m_dragHandlesTransform->matrix()
                                                   * m_dragHandleTranslateTransform->matrix());
            } else {
                m_helperArrowsTransform->setMatrix(m_dragHandlesTransform->matrix());
            }
        } else {
            QMatrix4x4 matrix;
            matrix.translate(QVector3D(m_dragHandlesTransform->translation()));
            m_helperArrowsTransform->setMatrix(matrix);
        }
    }
    resizeConstantScreenSizeEntities();

    m_meshCenterIndicatorLine->setEnabled(showCenterHandle && m_dragHandlesTransform->isEnabled());

    // Signal UI to reposition drag handles
    emit beginDragHandlesRepositioning();
    emit repositionDragHandle(DragTranslate, translatePoint,
                              m_dragHandlesTransform->isEnabled()
                              ? m_dragHandleTranslateTransform->isEnabled()
                                && translateHandlePos.z() > 0.0f : false, 0,
                              translateHandlePos.z());
    emit repositionDragHandle(DragTranslate, centerPoint,
                              m_dragHandlesTransform->isEnabled()
                              ? m_dragHandleTranslateTransform->isEnabled()
                                && itemCenterHandlePos.z() > 0.0f && showCenterHandle : false,
                              1, itemCenterHandlePos.z());
    emit repositionDragHandle(DragRotate, QPoint(rotateHandlePos.x(), rotateHandlePos.y()),
                              m_dragHandlesTransform->isEnabled()
                              ? m_dragHandleRotateTransform->isEnabled()
                                && rotateHandlePos.z() > 0.0f : false, 0, rotateHandlePos.z());
    for (int i = 0; i < dragCornerHandleCount; i++) {
        emit repositionDragHandle(DragScale,
                                  QPoint(cornerHandlePositions[i].x(),
                                         cornerHandlePositions[i].y()),
                                  m_dragHandlesTransform->isEnabled()
                                  ? m_dragHandleScaleTransforms.at(0)->isEnabled()
                                    && cornerHandlePositions[i].z() > 0.0f : false, i,
                                  cornerHandlePositions[i].z());
    }
    emit endDragHandlesRepositioning();
}

void EditorScene::handlePickerPress(Qt3DRender::QPickEvent *event)
{
    if (m_dragMode == DragNone && m_mouseButton == Qt::LeftButton) {
        Qt3DCore::QEntity *pressedEntity = qobject_cast<Qt3DCore::QEntity *>(sender()->parent());
        // If pressedEntity is not enabled, it typically means the pressedEntity is a drag handle
        // and the selection has changed to a different type of entity since the mouse press was
        // registered. Since the new entity is not the one we wanted to modify anyway, just
        // skip handling the pick event.
        if (pressedEntity && pressedEntity->isEnabled()) {
            if (pressedEntity->objectName() == helperArrowName) {
                if (m_selectedEntity) {
                    m_helperArrowGrabOffset =
                            event->worldIntersection() - m_helperArrowsTransform->translation();
                    dragHandlePress(DragTranslate, m_previousMousePosition,
                                    m_helperArrowHandleIndexMap.value(pressedEntity));
                    m_pickedEntity = m_selectedEntity;
                }
            } else if (!m_pickedEntity || m_pickedDistance > event->distance()) {
                // Ignore presses that are farther away than the closest one
                m_pickedDistance = event->distance();
                bool select = false;
                EditorSceneItem *item = m_sceneItems.value(pressedEntity->id(), nullptr);
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
                                break;
                            }
                        }
                    } else if (pressedEntity->objectName() == sceneLoaderSubEntityName) {
                        // Select the scene loader entity instead when picking one of loader's
                        // internal mesh entites.
                        Qt3DCore::QEntity *parentEntity = pressedEntity->parentEntity();
                        while (parentEntity) {
                            EditorSceneItem *parentItem = m_sceneItems.value(parentEntity->id());
                            if (parentItem) {
                                pressedEntity = parentEntity;
                                select = true;
                                break;
                            } else {
                                parentEntity = parentEntity->parentEntity();
                            }
                        }
                    }
                }
                if (select && !m_pickedEntity)
                    QMetaObject::invokeMethod(this, "endSelectionHandling", Qt::QueuedConnection);
                m_pickedEntity = pressedEntity;
            }
        }
    }
    event->setAccepted(true);
}

bool EditorScene::handleMousePress(QMouseEvent *event)
{
    m_previousMousePosition = event->pos();
    m_mouseButton = event->button();
    if (m_mouseButton == Qt::LeftButton)
        m_ctrlDownOnLastLeftPress = event->modifiers() & Qt::ControlModifier;
    cancelDrag();
    updateWorldPositionLabel(event->pos().x(), event->pos().y());
    return false; // Never consume press event
}

bool EditorScene::handleMouseRelease(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QPoint delta = event->pos() - m_previousMousePosition;
        if (delta.manhattanLength() < 5 && (m_dragMode == DragNone || m_ignoringInitialDrag))
            emit mouseRightButtonReleasedWithoutDragging();
    }
    m_cameraViewCenterSelected = false;
    cancelDrag();
    updateWorldPositionLabel(event->pos().x(), event->pos().y());
    return false; // Never consume release event
}

bool EditorScene::handleMouseMove(QMouseEvent *event)
{
    dragHandleMove(event->pos(), event->modifiers() & Qt::ShiftModifier,
                   event->modifiers() & Qt::ControlModifier,
                   event->modifiers() & Qt::AltModifier);

    if (m_dragMode != DragNone) {
        // Selection dragging updates world position label to mesh center while dragging
        updateWorldPositionLabelToDragHandle(EditorScene::DragTranslate, 1);
    } else {
        updateWorldPositionLabel(event->pos().x(), event->pos().y());
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

// Rescales various entities that need to be constant size on the screen
void EditorScene::resizeConstantScreenSizeEntities()
{
    if (frameGraphCamera()) {
        // Camera viewcenter
        const float vcEntityAngle = 0.0045f;
        QVector3D vcPos = m_activeSceneCameraFrustumData.viewCenterTransform->translation();
        float distanceToVc = (vcPos - frameGraphCamera()->position()).length();
        float vcScale = vcEntityAngle * distanceToVc;
        m_activeSceneCameraFrustumData.viewCenterTransform->setScale(vcScale * 2.0f);

        // Helper arrows
        const float arrowsEntityAngle = 0.035f;
        QVector3D arrowsPos = m_helperArrowsTransform->translation();
        float distanceToArrows = (arrowsPos - frameGraphCamera()->position()).length();
        float arrowsScale = arrowsEntityAngle * distanceToArrows;
        m_helperArrowsTransform->setScale(arrowsScale * 2.0f);
    }
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
    m_pickedEntity = nullptr;
    m_pickedDistance = -1.0f;
    m_dragEntity = nullptr;
    m_ignoringInitialDrag = true;
    m_dragHandleIndex = 0;
}

void EditorScene::setSceneEntity(Qt3DCore::QEntity *newSceneEntity)
{
    if (newSceneEntity)
        m_sceneEntity = newSceneEntity;
    else
        m_sceneEntity = new Qt3DCore::QEntity();
    m_sceneEntity->setObjectName(m_sceneRootString);
    addEntity(m_sceneEntity);
}

void EditorScene::createSceneLoaderChildPickers(Qt3DCore::QEntity *entity,
                                                QList<Qt3DRender::QObjectPicker *> *pickers)
{
    if (EditorUtils::entityMesh(entity)) {
        pickers->append(createObjectPickerForEntity(entity));
        // Rename entity so we can identify it later
        entity->setObjectName(sceneLoaderSubEntityName);
    }

    Q_FOREACH (QObject *child, entity->children()) {
        Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
        if (childEntity)
            createSceneLoaderChildPickers(childEntity, pickers);
    }
}

void EditorScene::handleCameraAdded(Qt3DRender::QCamera *camera)
{
    Qt3DCore::QEntity *visibleEntity = new Qt3DCore::QEntity(m_rootEntity);

    visibleEntity->setObjectName(cameraVisibleEntityName);

    Qt3DRender::QGeometryRenderer *visibleMesh = EditorUtils::createVisibleCameraMesh();

    Qt3DExtras::QPhongMaterial *cameraMaterial = new Qt3DExtras::QPhongMaterial();
    cameraMaterial->setAmbient(QColor(Qt::black));
    cameraMaterial->setDiffuse(QColor(Qt::black));
    cameraMaterial->setSpecular(QColor(Qt::black));
    cameraMaterial->setShininess(0);

    Qt3DCore::QTransform *visibleTransform = new Qt3DCore::QTransform();

    visibleEntity->addComponent(visibleMesh);
    visibleEntity->addComponent(cameraMaterial);
    visibleEntity->addComponent(visibleTransform);

    CameraData newData(camera, visibleEntity, visibleTransform, nullptr);
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

    if (!m_sceneCameras.length()) {
        m_activeSceneCameraFrustumData.frustumEntity->setEnabled(false);
        m_activeSceneCameraFrustumData.viewCenterEntity->setEnabled(false);
        m_activeSceneCameraFrustumData.viewVectorEntity->setEnabled(false);
    }
}

void EditorScene::handleLightAdded(Qt3DCore::QEntity *lightEntity)
{
    Qt3DCore::QEntity *visibleEntity = new Qt3DCore::QEntity(m_rootEntity);

    visibleEntity->setObjectName(lightVisibleEntityName);

    Qt3DCore::QTransform *visibleTransform = new Qt3DCore::QTransform();

    Qt3DRender::QAbstractLight *lightComponent = EditorUtils::entityLight(lightEntity);

    Qt3DExtras::QPhongAlphaMaterial *visibleMaterial = new Qt3DExtras::QPhongAlphaMaterial();
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
                                       visibleTransform, visibleMaterial, nullptr, nullptr);
    enableVisibleLight(*newData, m_freeView);
    m_sceneLights.insert(lightEntity->id(), newData);

    QMatrix4x4 matrix = EditorUtils::totalAncestralTransform(lightEntity);
    matrix.translate(lightTransform->translation());
    visibleTransform->setMatrix(matrix);

    handleLightTypeChanged(m_sceneItems.value(lightEntity->id()));

    updateLightVisibleTransform(lightEntity);
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
        Qt3DCore::QEntity *entity = entities.size() ? entities.at(0) : nullptr;
        updateLightVisibleTransform(entity);
    }
}

void EditorScene::handleViewportSizeChange()
{
    qreal aspectRatio = m_viewport->width() / qMax(m_viewport->height(), 1.0);
    m_freeViewCameraEntity->lens()->setPerspectiveProjection(
                freeViewCameraFov, aspectRatio, freeViewCameraNearPlane, freeViewCameraFarPlane);
    // Need to update drag handle positions
    handleSelectionTransformChange();
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

void EditorScene::checkMultiSelectionHighlights()
{
    const QList<EditorSceneItem *> items = m_sceneItems.values();
    for (int i = 0; i < items.size(); ++i) {
        EditorSceneItem *item = items.at(i);
        if (m_selectedEntityNameList.contains(item->entity()->objectName()))
            item->setShowSelectionBox(true);
        else
            item->setShowSelectionBox(false);
    }
}

QVector3D EditorScene::snapPosition(const QVector3D &worldPos, bool x, bool y, bool z)
{
    QVector3D newPos = worldPos;
    float shortestLen = FLT_MAX;
    int index = 0;
    QVector3D snapPos;
    // Snap nearest corner to grid intersection
    for (int i = 0; i < dragCornerHandleCount; ++i) {
        QVector3D corner = worldPos;
        QVector3D currentPos = worldPos;
        if (x) {
            corner.setX(worldPos.x() + m_dragEntitySnapOffsets.at(i).x());
            currentPos.setX(qRound(corner.x() / m_gridSize) * m_gridSize);
        }
        if (y) {
            corner.setY(worldPos.y() + m_dragEntitySnapOffsets.at(i).y());
            currentPos.setY(qRound(corner.y() / m_gridSize) * m_gridSize);
        }
        if (z) {
            corner.setZ(worldPos.z() + m_dragEntitySnapOffsets.at(i).z());
            currentPos.setZ(qRound(corner.z() / m_gridSize) * m_gridSize);
        }
        float len = (corner - currentPos).length();
        if (len < shortestLen) {
            shortestLen = len;
            snapPos = currentPos;
            index = i;
        }
    }
    if (x)
        newPos.setX(snapPos.x() - m_dragEntitySnapOffsets.at(index).x());
    if (y)
        newPos.setY(snapPos.y() - m_dragEntitySnapOffsets.at(index).y());
    if (z)
        newPos.setZ(snapPos.z() - m_dragEntitySnapOffsets.at(index).z());

    return newPos;
}
