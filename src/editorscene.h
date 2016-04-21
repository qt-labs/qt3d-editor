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
#ifndef EDITORSCENE_H
#define EDITORSCENE_H

#include "editorutils.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QUrl>
#include <QtCore/QStringListModel>
#include <QtCore/QTranslator>
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <Qt3DCore/QNodeId>

namespace Qt3DCore {
    class QEntity;
    class QTransform;
    class QComponent;
}

namespace Qt3DRender {
    class QPickEvent;
    class QObjectPicker;
    class QRenderSettings;
    class QMaterial;
    class QPhongAlphaMaterial;
    class QGeometryRenderer;
    class QCamera;
    class QCameraLens;
    class QForwardRenderer;
}

class EditorSceneItemModel;
class EditorSceneItem;
class EditorSceneParser;
class EditorViewportItem;
class UndoHandler;
class QMouseEvent;

class EditorScene : public QObject
{
    Q_OBJECT
    Q_PROPERTY(EditorSceneItemModel *sceneModel READ sceneModel CONSTANT)
    Q_PROPERTY(Qt3DCore::QEntity *selection READ selection WRITE setSelection NOTIFY selectionChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(int activeSceneCameraIndex READ activeSceneCameraIndex WRITE setActiveSceneCameraIndex NOTIFY activeSceneCameraIndexChanged)
    Q_PROPERTY(EditorViewportItem *viewport READ viewport WRITE setViewport NOTIFY viewportChanged)
    Q_PROPERTY(bool freeView READ freeView WRITE setFreeView NOTIFY freeViewChanged)
    Q_PROPERTY(QAbstractItemModel *sceneCamerasModel READ sceneCamerasModel NOTIFY sceneCamerasModelChanged)
    Q_PROPERTY(UndoHandler *undoHandler READ undoHandler CONSTANT)
    Q_PROPERTY(Qt3DCore::QEntity *helperPlane READ helperPlane CONSTANT)
    Q_PROPERTY(Qt3DCore::QTransform *helperPlaneTransform READ helperPlaneTransform CONSTANT)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString emptyString READ emptyString NOTIFY translationChanged)
    Q_PROPERTY(QString lockPropertySuffix READ lockPropertySuffix CONSTANT)
    Q_PROPERTY(QString lockTransformPropertyName READ lockTransformPropertyName CONSTANT)
    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)

private:
    struct CameraFrustumData {
        CameraFrustumData() :
            frustumEntity(nullptr)
          , viewVectorEntity(nullptr)
          , viewCenterEntity(nullptr)
          , frustumTransform(nullptr)
          , viewVectorTransform(nullptr)
          , viewCenterTransform(nullptr)
          , frustumMesh(nullptr)
          , viewCenterPicker(nullptr)
        {}

        Qt3DCore::QEntity *frustumEntity;
        Qt3DCore::QEntity *viewVectorEntity;
        Qt3DCore::QEntity *viewCenterEntity;
        Qt3DCore::QTransform *frustumTransform;
        Qt3DCore::QTransform *viewVectorTransform;
        Qt3DCore::QTransform *viewCenterTransform;
        Qt3DRender::QGeometryRenderer *frustumMesh;
        Qt3DRender::QObjectPicker *viewCenterPicker;
    };

    struct CameraData {
        CameraData() :
            cameraEntity(nullptr)
          , visibleEntity(nullptr)
          , visibleTransform(nullptr)
          , cameraPicker(nullptr)
        {}
        CameraData(Qt3DRender::QCamera *camera,
                   Qt3DCore::QEntity *visible,
                   Qt3DCore::QTransform *visibleTrans,
                   Qt3DRender::QObjectPicker *picker) :
            cameraEntity(camera)
          , visibleEntity(visible)
          , visibleTransform(visibleTrans)
          , cameraPicker(picker)
        {}

        Qt3DRender::QCamera *cameraEntity;
        Qt3DCore::QEntity *visibleEntity;
        Qt3DCore::QTransform *visibleTransform;
        Qt3DRender::QObjectPicker *cameraPicker;
    };

    struct LightData {
        LightData() :
            lightEntity(nullptr)
          , lightComponent(nullptr)
          , lightTransform(nullptr)
          , visibleEntity(nullptr)
          , visibleTransform(nullptr)
          , visibleMaterial(nullptr)
          , visibleMesh(nullptr)
          , visiblePicker(nullptr)
        {}
        LightData(Qt3DCore::QEntity *entity,
                  Qt3DRender::QLight *component,
                  Qt3DCore::QTransform *transform,
                  Qt3DCore::QEntity *visEntity,
                  Qt3DCore::QTransform *visTransform,
                  Qt3DRender::QPhongAlphaMaterial *visMaterial,
                  Qt3DRender::QGeometryRenderer *visMesh,
                  Qt3DRender::QObjectPicker *picker) :
            lightEntity(entity)
          , lightComponent(component)
          , lightTransform(transform)
          , visibleEntity(visEntity)
          , visibleTransform(visTransform)
          , visibleMaterial(visMaterial)
          , visibleMesh(visMesh)
          , visiblePicker(picker)
        {}

        Qt3DCore::QEntity *lightEntity;
        Qt3DRender::QLight *lightComponent;
        Qt3DCore::QTransform *lightTransform;
        Qt3DCore::QEntity *visibleEntity;
        Qt3DCore::QTransform *visibleTransform;
        Qt3DRender::QPhongAlphaMaterial *visibleMaterial;
        Qt3DRender::QGeometryRenderer *visibleMesh;
        Qt3DRender::QObjectPicker *visiblePicker;
    };

    struct DragHandleData {
        DragHandleData() :
            entity(nullptr)
          , transform(nullptr)
          , picker(nullptr)
        {}

        Qt3DCore::QEntity *entity;
        Qt3DCore::QTransform *transform;
        Qt3DRender::QObjectPicker *picker;
    };

    struct PlaceholderEntityData {
        PlaceholderEntityData() :
            entity(nullptr)
          , transform(nullptr)
          , material(nullptr)
          , mesh(nullptr)
          , type(EditorUtils::InvalidEntity)
        {}
        Qt3DCore::QEntity *entity;
        Qt3DCore::QTransform *transform;
        Qt3DRender::QMaterial *material;
        Qt3DRender::QGeometryRenderer *mesh;
        EditorUtils::InsertableEntities type;
    };

    enum DragMode {
        DragNone = 0,
        DragTranslate,
        DragScale,
        DragRotate
    };


public:
    explicit EditorScene(QObject *parent = 0);
    ~EditorScene();

    Qt3DCore::QEntity *rootEntity() const { return m_rootEntity; }
    EditorSceneItem *rootItem() const { return m_rootItem; }
    EditorSceneItem *sceneEntityItem() const { return m_sceneEntityItem; }
    Qt3DRender::QForwardRenderer *renderer() const { return m_renderer; }
    EditorSceneItemModel *sceneModel() const { return m_sceneModel; }
    const QMap<Qt3DCore::QNodeId, EditorSceneItem *> &items() const { return m_sceneItems; }

    void addEntity(Qt3DCore::QEntity *entity, int index = -1, Qt3DCore::QEntity *parent = nullptr);
    void removeEntity(Qt3DCore::QEntity *entity);

    Q_INVOKABLE void resetScene();
    Q_INVOKABLE bool saveScene(const QUrl &fileUrl, bool autosave = false);
    Q_INVOKABLE bool loadScene(const QUrl &fileUrl);
    Q_INVOKABLE void deleteScene(const QUrl &fileUrl, bool autosave = false);
    Q_INVOKABLE QString cameraName(int index) const;
    Q_INVOKABLE void resetFreeViewCamera();
    Q_INVOKABLE void snapFreeViewCameraToActiveSceneCamera();
    Q_INVOKABLE QVector3D getWorldPosition(int xPos, int yPos);
    Q_INVOKABLE void showPlaceholderEntity(const QString &name, int type);
    Q_INVOKABLE void movePlaceholderEntity(const QString &name, const QVector3D &worldPos);
    Q_INVOKABLE void hidePlaceholderEntity(const QString &name);
    Q_INVOKABLE void destroyPlaceholderEntity(const QString &name);

    QString duplicateEntity(Qt3DCore::QEntity *entity);
    void decrementDuplicateCount() { m_duplicateCount--; }
    Qt3DRender::QCamera *freeViewCamera() const { return m_freeViewCameraEntity; }
    Qt3DRender::QCamera *inputCamera() const;

    bool isRemovable(Qt3DCore::QEntity *entity) const;

    void setSelection(Qt3DCore::QEntity *entity);
    Qt3DCore::QEntity *selection() const { return m_selectedEntity; }
    const QString &error() const { return m_errorString; }

    void setActiveSceneCameraIndex(int index);
    int activeSceneCameraIndex() const { return m_activeSceneCameraIndex; }

    void setFreeView(bool enable);
    bool freeView() const { return m_freeView; }

    int gridSize() const;
    void setGridSize(int size);

    void setLanguage(const QString &language);
    const QString language() const;

    const QString emptyString() const;
    const QString lockPropertySuffix() const { return EditorUtils::lockPropertySuffix(); }
    const QString lockTransformPropertyName() const {
        return EditorUtils::lockTransformPropertyName();
    }

    void setViewport(EditorViewportItem *viewport);
    EditorViewportItem *viewport() const { return m_viewport; }

    QAbstractItemModel *sceneCamerasModel() { return &m_sceneCamerasModel; }
    UndoHandler *undoHandler() const { return m_undoHandler; }

    Qt3DCore::QEntity *helperPlane() const { return m_helperPlane; }
    Qt3DCore::QTransform *helperPlaneTransform() const { return m_helperPlaneTransform; }

    Qt3DRender::QMaterial *selectionBoxMaterial() const { return m_selectionBoxMaterial; }
    Qt3DRender::QGeometryRenderer *selectionBoxMesh() const { return m_selectionBoxMesh; }

    QMatrix4x4 calculateVisibleSceneCameraMatrix(Qt3DRender::QCamera *camera) const;
    QMatrix4x4 calculateVisibleLightMatrix(Qt3DCore::QEntity *lightEntity) const;

    void handlePropertyLocking(EditorSceneItem *item, const QString &lockProperty, bool locked);
    void handleLightTypeChanged(EditorSceneItem *item);
    void updateLightVisibleTransform(Qt3DCore::QEntity *lightEntity);

    void handleEnabledChanged(Qt3DCore::QEntity *entity, bool enabled);
    void setError(const QString &errorString);
    Qt3DRender::QObjectPicker *createObjectPickerForEntity(Qt3DCore::QEntity *entity);

public slots:
    void clearSelectionBoxes();

signals:
    void selectionChanged(Qt3DCore::QEntity *selection);
    void errorChanged(const QString &error);
    void freeViewChanged(bool enabled);
    void activeSceneCameraIndexChanged(int index);
    void viewportChanged(EditorViewportItem *viewport);
    void sceneCamerasModelChanged();
    void languageChanged(const QString &language);
    void translationChanged(const QString &translation);
    void gridSizeChanged(int gridSize);
    void mouseRightButtonReleasedWithoutDragging();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void handlePickerPress(Qt3DRender::QPickEvent *event);
    void handleCameraMatrixChange();
    void handleLightTransformChange();
    void handleViewportSizeChange();
    void handleEntityNameChange();
    void endSelectionHandling();
    void handleSelectionTransformChange();

private:
    void handleCameraAdded(Qt3DRender::QCamera *camera);
    void handleCameraRemoved(Qt3DRender::QCamera *camera);
    void handleLightAdded(Qt3DCore::QEntity *lightEntity);
    void handleLightRemoved(Qt3DCore::QEntity *lightEntity);
    void connectSceneCamera(const CameraData &cameraData);
    void setupDefaultScene();
    void createRootEntity();
    void createHelperPlane();
    void setFrameGraphCamera(Qt3DCore::QEntity *cameraEntity);
    Qt3DRender::QCamera *frameGraphCamera() const;
    void enableVisibleCameras(bool enable);
    void enableVisibleCamera(CameraData &cameraData, bool enable, bool isActiveCamera);
    void enableVisibleLights(bool enable);
    void enableVisibleLight(LightData &lightData, bool enable);
    void clearSceneCamerasAndLights();
    void resetSceneCamera(Qt3DCore::QEntity *sceneCameraEntity);
    int cameraIndexForEntity(Qt3DCore::QEntity *entity);
    void updateVisibleSceneCameraMatrix(const CameraData &cameraData);
    void retranslateUi();
    void connectDragHandles(EditorSceneItem *item, bool enable);
    void dragTranslateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown,
                                     bool altDown);
    void dragScaleSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown);
    void dragRotateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown);
    QVector3D dragHandlePositionOffset(const QPoint &newPos);
    bool handleMousePress(QMouseEvent *event);
    bool handleMouseRelease(QMouseEvent *event);
    bool handleMouseMove(QMouseEvent *event);
    QVector3D helperPlaneNormal() const;
    QVector3D projectVectorOnCameraPlane(const QVector3D &vector) const;
    void updateDragHandlePickers();
    void updateDragHandlePicker(DragHandleData &handleData);
    void resizeCameraViewCenterEntity();
    bool isPropertyLocked(const QString &propertyName, QObject *obj);
    void cancelDrag();
    void setSceneEntity(Qt3DCore::QEntity *newSceneEntity = nullptr);
    void createSceneLoaderChildPickers(Qt3DCore::QEntity *entity,
                                       QList<Qt3DRender::QObjectPicker *> *pickers);

private:
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *m_componentCache;
    EditorSceneItem *m_rootItem;
    EditorSceneItemModel* m_sceneModel;

    QMap<Qt3DCore::QNodeId, EditorSceneItem *> m_sceneItems;

    EditorSceneParser *m_sceneParser;
    Qt3DRender::QRenderSettings *m_renderSettings;
    Qt3DRender::QForwardRenderer *m_renderer;
    Qt3DCore::QEntity *m_sceneEntity;
    EditorSceneItem *m_sceneEntityItem;
    Qt3DCore::QEntity *m_selectedEntity;
    Qt3DCore::QTransform *m_selectedEntityTransform;
    bool m_cameraViewCenterSelected;

    QString m_errorString;

    QList<CameraData> m_sceneCameras;
    QMap<Qt3DCore::QNodeId, LightData *> m_sceneLights;
    CameraFrustumData m_activeSceneCameraFrustumData;
    int m_activeSceneCameraIndex;
    bool m_freeView;
    Qt3DRender::QCamera *m_freeViewCameraEntity;

    EditorViewportItem *m_viewport; // Not owned

    // m_sceneCamerasModel is simply a list of camera entity names for UI.
    // Indexes match m_sceneCameras list indexes.
    QStringListModel m_sceneCamerasModel;
    UndoHandler *m_undoHandler;
    Qt3DCore::QEntity *m_helperPlane;
    Qt3DCore::QTransform *m_helperPlaneTransform;

    Qt3DRender::QMaterial *m_selectionBoxMaterial;
    Qt3DRender::QGeometryRenderer *m_selectionBoxMesh;

    QTranslator *m_qtTranslator;
    QTranslator *m_appTranslator;

    QString m_language;

    QString m_sceneRootString;
    QString m_saveFailString;
    QString m_loadFailString;
    QString m_importFailString;
    QString m_cameraString;
    QString m_cubeString;
    QString m_lightString;

    DragHandleData m_dragHandles;
    DragHandleData m_dragHandleScale;
    DragHandleData m_dragHandleRotate;
    DragHandleData m_dragHandleTranslate;
    DragMode m_dragMode;
    QPoint m_previousMousePosition;
    QPoint m_mousePressPosition;
    QVector3D m_dragHandleScaleCornerTranslation;
    QVector3D m_dragInitialTranslationValue;
    QVector3D m_dragInitialScaleValue;
    QQuaternion m_dragInitialRotationValue;
    QVector3D m_dragInitialRotateCustomVector;
    QVector3D m_dragInitialHandleTranslation;
    QVector3D m_dragInitialHandleCornerTranslation;
    Qt3DCore::QEntity *m_dragEntity;
    bool m_ignoringInitialDrag;
    bool m_viewCenterLocked;
    Qt3DCore::QEntity *m_pickedEntity;
    float m_pickedDistance;
    QVector3D m_snapToGridIntersection;
    int m_gridSize;
    int m_duplicateCount;
    Qt3DCore::QEntity *m_previousDuplicate;

    QMap<QString, PlaceholderEntityData *> m_placeholderEntityMap;
};

#endif // EDITORSCENE_H
