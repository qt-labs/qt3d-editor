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

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QUrl>
#include <Qt3DCore/QNodeId>

#include <QStringListModel>

namespace Qt3DCore {
    class QEntity;
    class QCamera;
    class QTransform;
    class QCameraLens;
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

class EditorScene : public QObject
{
    Q_OBJECT
    Q_PROPERTY(EditorSceneItemModel *sceneModel READ sceneModel CONSTANT)
    Q_PROPERTY(Qt3DCore::QEntity *selection READ selection NOTIFY selectionChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(int activeSceneCameraIndex READ activeSceneCameraIndex WRITE setActiveSceneCameraIndex NOTIFY activeSceneCameraIndexChanged)
    Q_PROPERTY(EditorViewportItem *viewport READ viewport WRITE setViewport NOTIFY viewportChanged)
    Q_PROPERTY(bool freeView READ freeView WRITE setFreeView NOTIFY freeViewChanged)
    Q_PROPERTY(QAbstractItemModel *sceneCamerasModel READ sceneCamerasModel NOTIFY sceneCamerasModelChanged)
    Q_PROPERTY(UndoHandler *undoHandler READ undoHandler CONSTANT)
    Q_PROPERTY(Qt3DCore::QEntity *helperPlane READ helperPlane CONSTANT)
    Q_PROPERTY(Qt3DCore::QTransform *helperPlaneTransform READ helperPlaneTransform CONSTANT)

public:
    explicit EditorScene(QObject *parent = 0);
    ~EditorScene();

    Qt3DCore::QEntity *rootEntity();
    EditorSceneItem *rootItem();
    EditorSceneItem *sceneEntityItem();

    EditorSceneItemModel *sceneModel() const;

    void addEntity(Qt3DCore::QEntity *entity, int index = -1, Qt3DCore::QEntity *parent = Q_NULLPTR);

    void moveEntity(Qt3DCore::QEntity *entity, Qt3DCore::QEntity *newParent = Q_NULLPTR);

    void removeEntity(Qt3DCore::QEntity *entity, bool deleteEntity, bool recursiveCall = false);

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

    bool isRemovable(Qt3DCore::QEntity *entity) const;

    Qt3DCore::QEntity *selection() const;
    const QString &error() const;

    void setActiveSceneCameraIndex(int index);
    int activeSceneCameraIndex() const;

    void setFreeView(bool enable);
    bool freeView() const;

    void setViewport(EditorViewportItem *viewport);
    EditorViewportItem *viewport() const;

    QAbstractItemModel *sceneCamerasModel();
    UndoHandler *undoHandler();

    Qt3DCore::QEntity *helperPlane() const;
    Qt3DCore::QTransform *helperPlaneTransform() const;

    Qt3DRender::QMaterial *selectionBoxMaterial() const;
    Qt3DRender::QGeometryRenderer *selectionBoxMesh() const;

    QMatrix4x4 calculateCameraConeMatrix(Qt3DCore::QTransform *sourceTransform) const;

public slots:
    void handleClick(Qt3DRender::QPickEvent *event);
    void handlePress(Qt3DRender::QPickEvent *event);
    void handleRelease(Qt3DRender::QPickEvent *event);
    void handleEnter();
    void handleExit();
    void handleCameraAdded(Qt3DCore::QEntity *camera);
    void handleCameraRemoved(Qt3DCore::QEntity *camera);
    void handleCameraTransformChange(Qt3DCore::QEntity *camera);
    void handleCameraMatrixChange();
    void handleViewportSizeChange();
    void handleEntityNameChange();
    void clearSelectionBoxes();

signals:
    void selectionChanged(Qt3DCore::QEntity *selection);
    void errorChanged(const QString &error);
    void freeViewChanged(bool enabled);
    void activeSceneCameraIndexChanged(int index);
    void viewportChanged(EditorViewportItem *viewport);
    void sceneCamerasModelChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void removeEntityItem(const Qt3DCore::QNodeId &id);
    void setupDefaultScene();
    void createRootEntity();
    void setFrameGraphCamera(Qt3DCore::QEntity *cameraEntity);
    Qt3DCore::QEntity *frameGraphCamera() const;
    void enableCameraCones(bool enable);
    void clearSceneCameras();
    void resetSceneCamera(Qt3DCore::QEntity *sceneCameraEntity);
    Qt3DRender::QObjectPicker *createObjectPickerForEntity(Qt3DCore::QEntity *entity);
    int cameraIndexForEntity(Qt3DCore::QEntity *entity);
    Qt3DCore::QCameraLens *cameraLensForEntity(Qt3DCore::QEntity *entity);
    void updateCameraConeMatrix(Qt3DCore::QTransform *sourceTransform,
                                Qt3DCore::QTransform *coneTransform);
    void copyCameraProperties(Qt3DCore::QCamera *target, Qt3DCore::QCamera *source);

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
    bool m_activeSelection;

    QString m_errorString;

    struct CameraData {
        CameraData() :
            entity(Q_NULLPTR)
          , cone(Q_NULLPTR)
          , coneTransform(Q_NULLPTR)
          , lens(Q_NULLPTR)
          , picker(Q_NULLPTR)
        {}
        CameraData(Qt3DCore::QEntity *cameraEntity, Qt3DCore::QEntity *cameraCone,
                   Qt3DCore::QTransform *cameraConeTransform, Qt3DCore::QCameraLens *cameraLens,
                   Qt3DRender::QObjectPicker *cameraPicker) :
            entity(cameraEntity)
          , cone(cameraCone)
          , coneTransform(cameraConeTransform)
          , lens(cameraLens)
          , picker(cameraPicker)
        {}

        Qt3DCore::QEntity *entity;
        Qt3DCore::QEntity *cone;
        Qt3DCore::QTransform *coneTransform;
        Qt3DCore::QCameraLens *lens;
        Qt3DRender::QObjectPicker *picker;
    };

    QList<CameraData> m_sceneCameras;
    int m_activeSceneCameraIndex;
    bool m_freeView;
    Qt3DCore::QCamera *m_freeViewCameraEntity;

    EditorViewportItem *m_viewport; // Not owned

    // m_sceneCamerasModel is simply a list of camera entity names for UI.
    // Indexes match m_sceneCameras list indexes.
    QStringListModel m_sceneCamerasModel;
    UndoHandler *m_undoHandler;
    struct EntityRelationship {
        EntityRelationship() : parent(Q_NULLPTR), child(Q_NULLPTR) {}
        EntityRelationship(Qt3DCore::QEntity *p, Qt3DCore::QEntity *c)
            : parent(p), child(c) {}

        Qt3DCore::QEntity *parent;
        Qt3DCore::QEntity *child;
    };
    QVector<EntityRelationship> m_entityRelations;
    Qt3DCore::QEntity *m_helperPlane;
    Qt3DCore::QTransform *m_helperPlaneTransform;

    Qt3DRender::QMaterial *m_selectionBoxMaterial;
    Qt3DRender::QGeometryRenderer *m_selectionBoxMesh;
};

#endif // EDITORSCENE_H
