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
#include <Qt3DCore/QEntity>
#include <QtQml/QJSValue>

namespace Qt3DCore {
    class QTransform;
    class QComponent;
}

namespace Qt3DRender {
    class QPickEvent;
    class QObjectPicker;
    class QRenderSettings;
    class QMaterial;
    class QGeometryRenderer;
    class QCamera;
    class QCameraLens;
    class QSceneExporter;
}

namespace Qt3DExtras {
    class QPhongAlphaMaterial;
    class QForwardRenderer;
}

class EditorSceneItemModel;
class EditorSceneItem;
class EditorViewportItem;
class UndoHandler;
class QMouseEvent;

#ifdef GLTF_EXPORTER_AVAILABLE
class EditorSceneSaver;
#else
class EditorSceneParser;
#endif

class EditorScene : public QObject
{
    Q_OBJECT
    Q_PROPERTY(EditorSceneItemModel *sceneModel READ sceneModel CONSTANT)
    Q_PROPERTY(Qt3DCore::QEntity *selection READ selection WRITE setSelection NOTIFY selectionChanged)
    Q_PROPERTY(QStringList multiSelectionList READ multiSelectionList NOTIFY multiSelectionListChanged)
    Q_PROPERTY(bool multiSelection READ multiSelection NOTIFY multiSelectionChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(int activeSceneCameraIndex READ activeSceneCameraIndex WRITE setActiveSceneCameraIndex NOTIFY activeSceneCameraIndexChanged)
    Q_PROPERTY(EditorViewportItem *viewport READ viewport WRITE setViewport NOTIFY viewportChanged)
    Q_PROPERTY(bool freeView READ freeView WRITE setFreeView NOTIFY freeViewChanged)
    Q_PROPERTY(bool helperArrowsLocal READ helperArrowsLocal WRITE setHelperArrowsLocal NOTIFY helperArrowsLocalChanged)
    Q_PROPERTY(QAbstractItemModel *sceneCamerasModel READ sceneCamerasModel NOTIFY sceneCamerasModelChanged)
    Q_PROPERTY(UndoHandler *undoHandler READ undoHandler CONSTANT)
    Q_PROPERTY(Qt3DCore::QEntity *helperPlane READ helperPlane CONSTANT)
    Q_PROPERTY(Qt3DCore::QTransform *helperPlaneTransform READ helperPlaneTransform CONSTANT)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString emptyString READ emptyString NOTIFY translationChanged)
    Q_PROPERTY(QString lockPropertySuffix READ lockPropertySuffix CONSTANT)
    Q_PROPERTY(QString lockTransformPropertyName READ lockTransformPropertyName CONSTANT)
    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(ClipboardOperation clipboardOperation READ clipboardOperation WRITE setClipboardOperation NOTIFY clipboardOperationChanged)
    Q_PROPERTY(QString clipboardContent READ clipboardContent WRITE setClipboardContent NOTIFY clipboardContentChanged)
    Q_PROPERTY(bool canExportGltf READ canExportGltf CONSTANT)

public:
    enum DragMode {
        DragNone = 0,
        DragTranslate,
        DragScale,
        DragRotate,
        DragDebug  // Can be used to debugging positions
    };
    Q_ENUM(DragMode)

    enum TranslateHandleIndex {
        TranslateHandleBoxCenter,
        TranslateHandleMeshCenter,
        TranslateHandleArrowX,
        TranslateHandleArrowY,
        TranslateHandleArrowZ
    };
    Q_ENUM(TranslateHandleIndex)

    enum ClipboardOperation {
        ClipboardNone = 0,
        ClipboardCopy,
        ClipboardCut
    };
    Q_ENUM(ClipboardOperation)

    enum CameraPosition {
        CameraPositionNone = 0,
        CameraPositionTop,
        CameraPositionBottom,
        CameraPositionLeft,
        CameraPositionRight,
        CameraPositionFront,
        CameraPositionBack
    };
    Q_ENUM(CameraPosition)

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
                  Qt3DRender::QAbstractLight *component,
                  Qt3DCore::QTransform *transform,
                  Qt3DCore::QEntity *visEntity,
                  Qt3DCore::QTransform *visTransform,
                  Qt3DExtras::QPhongAlphaMaterial *visMaterial,
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
        Qt3DRender::QAbstractLight *lightComponent;
        Qt3DCore::QTransform *lightTransform;
        Qt3DCore::QEntity *visibleEntity;
        Qt3DCore::QTransform *visibleTransform;
        Qt3DExtras::QPhongAlphaMaterial *visibleMaterial;
        Qt3DRender::QGeometryRenderer *visibleMesh;
        Qt3DRender::QObjectPicker *visiblePicker;
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

public:
    explicit EditorScene(QObject *parent = 0);
    ~EditorScene();

    Qt3DCore::QEntity *rootEntity() const { return m_rootEntity; }
    EditorSceneItem *rootItem() const { return m_rootItem; }
    EditorSceneItem *sceneEntityItem() const { return m_sceneEntityItem; }
    EditorSceneItem *entityItem(Qt3DCore::QEntity *entity) const;
    Qt3DExtras::QForwardRenderer *renderer() const { return m_renderer; }
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
    Q_INVOKABLE void dragHandlePress(DragMode dragMode, const QPoint &pos, int handleIndex = 0);
    Q_INVOKABLE void dragHandleMove(const QPoint &pos, bool shiftDown, bool ctrlDown, bool altDown);
    Q_INVOKABLE void dragHandleRelease();
    Q_INVOKABLE QString sceneRootName() const { return m_sceneEntity->objectName(); }
    Q_INVOKABLE void toggleEntityMultiSelection(const QString &name);
    Q_INVOKABLE void clearMultiSelection();
    Q_INVOKABLE QVector3D getMultiSelectionCenter();
    Q_INVOKABLE void updateWorldPositionLabel(int xPos, int yPos);
    Q_INVOKABLE void updateWorldPositionLabelToDragHandle(DragMode dragMode, int handleIndex = 0);
    Q_INVOKABLE void changeCameraPosition(CameraPosition preset);
    Q_INVOKABLE bool exportGltfScene(const QUrl &folder, const QString &exportName,
                                     bool exportSelected, const QJSValue &options);

    void removeEntityFromMultiSelection(const QString &name);
    void addEntityToMultiSelection(const QString &name);
    void renameEntityInMultiSelectionList(const QString &oldName, const QString &newName);

    ClipboardOperation clipboardOperation() { return m_clipboardOperation; }
    void setClipboardOperation(ClipboardOperation operation);

    QString clipboardContent() const { return m_clipboardEntityName; }
    void setClipboardContent(const QString &entityName);

    QString duplicateEntity(Qt3DCore::QEntity *entity);
    void decrementDuplicateCount() { m_duplicateCount--; }
    Qt3DRender::QCamera *freeViewCamera() const { return m_freeViewCameraEntity; }
    Qt3DRender::QCamera *inputCamera() const;

    bool isRemovable(Qt3DCore::QEntity *entity) const;

    void setSelection(Qt3DCore::QEntity *entity);
    Qt3DCore::QEntity *selection() const { return m_selectedEntity; }

    QStringList multiSelectionList() { return m_selectedEntityNameList; }

    bool multiSelection() const { return m_selectedEntityNameList.size() > 0; }

    const QString &error() const { return m_errorString; }

    void setActiveSceneCameraIndex(int index);
    int activeSceneCameraIndex() const { return m_activeSceneCameraIndex; }

    void setFreeView(bool enable);
    bool freeView() const { return m_freeView; }
    void setHelperArrowsLocal(bool enable);
    bool helperArrowsLocal() const { return m_helperArrowsLocal; }

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

    void showDebugHandle(bool show, int handleIndex = 0, const QVector3D &worldPosition = QVector3D());
    void queueEnsureSelection();
    void queueUpdateGroupSelectionBoxes();

    bool canExportGltf();

public slots:
    void clearSelectionBoxes(Qt3DCore::QEntity *skipEntity = nullptr);

signals:
    void selectionChanged(Qt3DCore::QEntity *selection);
    void multiSelectionListChanged();
    void multiSelectionChanged(bool multiSelection);
    void errorChanged(const QString &error);
    void freeViewChanged(bool enabled);
    void activeSceneCameraIndexChanged(int index);
    void viewportChanged(EditorViewportItem *viewport);
    void sceneCamerasModelChanged();
    void languageChanged(const QString &language);
    void translationChanged(const QString &translation);
    void gridSizeChanged(int gridSize);
    void mouseRightButtonReleasedWithoutDragging();
    void repositionDragHandle(DragMode dragMode, const QPoint &pos, bool visible, int handleIndex,
                              float handleZ);
    void beginDragHandlesRepositioning();
    void endDragHandlesRepositioning();
    void clipboardOperationChanged(ClipboardOperation clipboardOperation);
    void clipboardContentChanged(const QString &clipboardContent);
    void worldPositionLabelUpdate(const QString &wpX, const QString &wpY, const QString &wpZ);
    void helperArrowsLocalChanged(bool enable);

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
    void createHelperArrows();
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
    void dragScaleSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown, bool altDown);
    void dragRotateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown);
    QVector3D dragHandlePositionOffset(const QPoint &newPos);
    bool handleMousePress(QMouseEvent *event);
    bool handleMouseRelease(QMouseEvent *event);
    bool handleMouseMove(QMouseEvent *event);
    QVector3D helperPlaneNormal() const;
    QVector3D projectVectorOnCameraPlane(const QVector3D &vector) const;
    void resizeConstantScreenSizeEntities();
    bool isPropertyLocked(const QString &propertyName, QObject *obj);
    void cancelDrag();
    void setSceneEntity(Qt3DCore::QEntity *newSceneEntity = nullptr);
    void createSceneLoaderChildPickers(Qt3DCore::QEntity *entity,
                                       QList<Qt3DRender::QObjectPicker *> *pickers);
    void checkMultiSelectionHighlights();
    QVector3D snapPosition(const QVector3D &worldPos, bool x, bool y, bool z);
    Q_INVOKABLE void doEnsureSelection();
    EditorSceneItem *itemByName(const QString &name);
    void clearSingleSelection();
    Q_INVOKABLE void doUpdateGroupSelectionBoxes();
    void enableHelperArrows(bool enable);
    void updateWorldPositionLabel(const QVector3D &worldPos);

private:
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *m_componentCache;
    EditorSceneItem *m_rootItem;
    EditorSceneItemModel* m_sceneModel;

    QMap<Qt3DCore::QNodeId, EditorSceneItem *> m_sceneItems;

    Qt3DRender::QRenderSettings *m_renderSettings;
    Qt3DExtras::QForwardRenderer *m_renderer;
    Qt3DCore::QEntity *m_sceneEntity;
    EditorSceneItem *m_sceneEntityItem;
    Qt3DCore::QEntity *m_selectedEntity;
    Qt3DCore::QTransform *m_selectedEntityTransform;
    QString m_ensureSelectionEntityName;
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
    Qt3DCore::QEntity *m_helperArrows;
    Qt3DCore::QTransform *m_helperArrowsTransform;
    bool m_helperArrowsLocal;
    QMap<Qt3DCore::QEntity *, int> m_helperArrowHandleIndexMap;
    QVector3D m_helperArrowGrabOffset;

    Qt3DRender::QMaterial *m_selectionBoxMaterial;
    Qt3DRender::QGeometryRenderer *m_selectionBoxMesh;
    Qt3DCore::QEntity *m_meshCenterIndicatorLine;
    Qt3DCore::QTransform *m_meshCenterIndicatorLineTransform;

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
    QString m_gltfExportFailString;

    Qt3DCore::QTransform *m_dragHandlesTransform;
    Qt3DCore::QTransform *m_dragHandleRotateTransform;
    Qt3DCore::QTransform *m_dragHandleTranslateTransform;
    QVector<Qt3DCore::QTransform *> m_dragHandleScaleTransforms;
    QVector<QVector3D> m_dragHandleCornerAdjustments;
    QVector3D m_dragHandleRotationAdjustment;
    int m_dragHandleIndex;
    DragMode m_dragMode;
    QPoint m_previousMousePosition;
    QVector3D m_dragInitialEntityTranslationValue;
    QVector3D m_dragInitialWorldTranslationValue;
    QVector3D m_dragInitialScaleValue;
    QQuaternion m_dragInitialRotationValue;
    QVector3D m_dragInitialRotateCustomVector;
    QVector3D m_dragInitialHandleTranslation;
    QVector3D m_dragInitialHandleCornerTranslation;
    QQuaternion m_dragInitialHandleRotationValue;
    QVector3D m_dragInitialCenterTranslation;
    QVector3D m_dragHandleCornerTranslation;
    QVector<QVector3D> m_dragEntitySnapOffsets;
    QMatrix4x4 m_dragInitialHandleMatrix;
    Qt3DCore::QEntity *m_dragEntity;
    bool m_ignoringInitialDrag;
    bool m_viewCenterLocked;
    Qt3DCore::QEntity *m_pickedEntity;
    float m_pickedDistance;
    int m_gridSize;
    int m_duplicateCount;
    Qt3DCore::QEntity *m_previousDuplicate;
    bool m_ctrlDownOnLastLeftPress;
    QStringList m_selectedEntityNameList;
    Qt::MouseButton m_mouseButton;
    QString m_clipboardEntityName;
    ClipboardOperation m_clipboardOperation;

    QMap<QString, PlaceholderEntityData *> m_placeholderEntityMap;
    bool m_groupBoxUpdatePending;
#ifdef GLTF_EXPORTER_AVAILABLE
    Qt3DRender::QSceneExporter *m_gltfExporter;
    EditorSceneSaver *m_sceneSaver;
#else
    EditorSceneParser *m_sceneParser;
#endif
};

#endif // EDITORSCENE_H
