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
    class QCamera;
    class QTransform;
    class QCameraLens;
    class QComponent;
}

namespace Qt3DRender {
    class QPickEvent;
    class QObjectPicker;
    class QFrameGraph;
    class QMaterial;
    class QGeometryRenderer;
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

private:
    struct CameraFrustumData {
        CameraFrustumData() :
            frustumEntity(Q_NULLPTR)
          , viewVectorEntity(Q_NULLPTR)
          , viewCenterEntity(Q_NULLPTR)
          , frustumTransform(Q_NULLPTR)
          , viewVectorTransform(Q_NULLPTR)
          , viewCenterTransform(Q_NULLPTR)
          , frustumMesh(Q_NULLPTR)
          , viewCenterPicker(Q_NULLPTR)
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
            cameraEntity(Q_NULLPTR)
          , visibleEntity(Q_NULLPTR)
          , visibleTransform(Q_NULLPTR)
          , cameraPicker(Q_NULLPTR)
        {}
        CameraData(Qt3DCore::QCamera *camera,
                   Qt3DCore::QEntity *visible,
                   Qt3DCore::QTransform *visibleTrans,
                   Qt3DRender::QObjectPicker *picker) :
            cameraEntity(camera)
          , visibleEntity(visible)
          , visibleTransform(visibleTrans)
          , cameraPicker(picker)
        {}

        Qt3DCore::QCamera *cameraEntity;
        Qt3DCore::QEntity *visibleEntity;
        Qt3DCore::QTransform *visibleTransform;
        Qt3DRender::QObjectPicker *cameraPicker;
    };

    struct DragHandleData {
        DragHandleData() :
            entity(Q_NULLPTR)
          , transform(Q_NULLPTR)
          , picker(Q_NULLPTR)
        {}

        Qt3DCore::QEntity *entity;
        Qt3DCore::QTransform *transform;
        Qt3DRender::QObjectPicker *picker;
    };

    struct PlaceholderEntityData {
        PlaceholderEntityData() :
            entity(Q_NULLPTR)
          , transform(Q_NULLPTR)
          , material(Q_NULLPTR)
          , mesh(Q_NULLPTR)
          , type(EditorUtils::GenericEntity)
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

    Qt3DCore::QEntity *rootEntity();
    EditorSceneItem *rootItem();
    EditorSceneItem *sceneEntityItem();

    EditorSceneItemModel *sceneModel() const;

    void addEntity(Qt3DCore::QEntity *entity, int index = -1, Qt3DCore::QEntity *parent = Q_NULLPTR);

    void moveEntity(Qt3DCore::QEntity *entity, Qt3DCore::QEntity *newParent = Q_NULLPTR);

    void removeEntity(Qt3DCore::QEntity *entity);

    const QMap<Qt3DCore::QNodeId, EditorSceneItem *> &items() const;

    Q_INVOKABLE void resetScene();
    Q_INVOKABLE bool saveScene(const QUrl &fileUrl, bool autosave = false);
    Q_INVOKABLE bool loadScene(const QUrl &fileUrl);
    Q_INVOKABLE void deleteScene(const QUrl &fileUrl, bool autosave = false);
    Q_INVOKABLE QString cameraName(int index) const;
    Q_INVOKABLE void resetFreeViewCamera();
    Q_INVOKABLE void copyFreeViewToNewSceneCamera();
    Q_INVOKABLE void moveActiveSceneCameraToFreeView();
    Q_INVOKABLE void snapFreeViewCameraToActiveSceneCamera();
    Q_INVOKABLE void duplicateEntity(Qt3DCore::QEntity *entity);
    Q_INVOKABLE QVector3D getWorldPosition(int xPos, int yPos);
    Q_INVOKABLE void showPlaceholderEntity(const QString &name, int type);
    Q_INVOKABLE void movePlaceholderEntity(const QString &name, const QVector3D &worldPos);
    Q_INVOKABLE void hidePlaceholderEntity(const QString &name);
    Q_INVOKABLE void destroyPlaceholderEntity(const QString &name);

    bool isRemovable(Qt3DCore::QEntity *entity) const;

    void setSelection(Qt3DCore::QEntity *entity);
    Qt3DCore::QEntity *selection() const;
    const QString &error() const;

    void setActiveSceneCameraIndex(int index);
    int activeSceneCameraIndex() const;

    void setFreeView(bool enable);
    bool freeView() const;

    void setLanguage(const QString &language);
    const QString language() const;

    const QString emptyString() const;

    void setViewport(EditorViewportItem *viewport);
    EditorViewportItem *viewport() const;

    QAbstractItemModel *sceneCamerasModel();
    UndoHandler *undoHandler();

    Qt3DCore::QEntity *helperPlane() const;
    Qt3DCore::QTransform *helperPlaneTransform() const;

    Qt3DRender::QMaterial *selectionBoxMaterial() const;
    Qt3DRender::QGeometryRenderer *selectionBoxMesh() const;

    QMatrix4x4 calculateVisibleSceneCameraMatrix(Qt3DCore::QCamera *camera) const;

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

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void handlePress(Qt3DRender::QPickEvent *event);
    void handleCameraMatrixChange();
    void handleViewportSizeChange();
    void handleEntityNameChange();
    void endSelectionHandling(Qt3DCore::QEntity *selectedEntity);
    void handleSelectionTransformChange();

private:
    void handleCameraAdded(Qt3DCore::QCamera *camera);
    void handleCameraRemoved(Qt3DCore::QCamera *camera);
    void connectSceneCamera(const CameraData &cameraData);
    void removeEntityItem(const Qt3DCore::QNodeId &id);
    void setupDefaultScene();
    void createRootEntity();
    void setFrameGraphCamera(Qt3DCore::QEntity *cameraEntity);
    Qt3DCore::QCamera *frameGraphCamera() const;
    void enableCameraCones(bool enable);
    void clearSceneCameras();
    void resetSceneCamera(Qt3DCore::QEntity *sceneCameraEntity);
    Qt3DRender::QObjectPicker *createObjectPickerForEntity(Qt3DCore::QEntity *entity);
    int cameraIndexForEntity(Qt3DCore::QEntity *entity);
    void updateVisibleSceneCameraMatrix(const CameraData &cameraData);
    void retranslateUi();
    void connectDragHandles(EditorSceneItem *item, bool enable);
    void dragTranslateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown);
    void dragScaleSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown);
    void dragRotateSelectedEntity(const QPoint &newPos, bool shiftDown, bool ctrlDown);
    QVector3D dragHandlePositionOffset(const QPoint &newPos);
    bool handleMousePress(QMouseEvent *event);
    bool handleMouseRelease(QMouseEvent *event);
    bool handleMouseMove(QMouseEvent *event);
    QVector3D helperPlaneNormal() const;
    QVector3D projectVectorOnCameraPlane(const QVector3D &vector) const;
    QVector3D frameGraphCameraNormal() const;
    void updateDragHandlePickers();
    void resizeCameraViewCenterEntity();

private:
    friend class EditorViewportItem;

    Qt3DCore::QEntity *m_rootEntity;
    Qt3DCore::QEntity *m_componentCache;
    EditorSceneItem *m_rootItem;
    EditorSceneItemModel* m_sceneModel;

    QMap<Qt3DCore::QNodeId, EditorSceneItem *> m_sceneItems;

    EditorSceneParser *m_sceneParser;
    Qt3DRender::QFrameGraph *m_frameGraph;
    Qt3DCore::QEntity *m_sceneEntity;
    EditorSceneItem *m_sceneEntityItem;
    Qt3DCore::QEntity *m_selectedEntity;
    Qt3DCore::QTransform *m_selectedEntityTransform;
    bool m_handlingSelection;
    bool m_cameraViewCenterSelected;

    QString m_errorString;

    QList<CameraData> m_sceneCameras;
    CameraFrustumData m_activeSceneCameraFrustumData;
    int m_activeSceneCameraIndex;
    bool m_freeView;
    Qt3DCore::QCamera *m_freeViewCameraEntity;

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
    QString m_cameraString;
    QString m_cubeString;
    QString m_lightString;

    DragHandleData m_dragHandles;
    DragHandleData m_dragHandleScale;
    DragHandleData m_dragHandleRotate;
    DragMode m_dragMode;
    QPoint m_previousMousePosition;
    QVector3D m_dragHandleScaleCornerTranslation;
    QVector3D m_dragInitialTranslationValue;
    QVector3D m_dragInitialScaleValue;
    QQuaternion m_dragInitialRotationValue;
    QVector3D m_dragInitialUpVector;
    QVector3D m_dragInitialHandleTranslation;
    QVector3D m_dragInitialHandleCornerTranslation;
    bool m_ignoringInitialDrag;

    QMap<QString, PlaceholderEntityData *> m_placeholderEntityMap;
};

#endif // EDITORSCENE_H
