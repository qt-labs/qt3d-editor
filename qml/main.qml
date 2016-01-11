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
import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQml.Models 2.2
import Qt3D.Core 2.0
import com.theqtcompany.Editor3d 1.0

ApplicationWindow {
    id: mainwindow
    title: qsTr("Qt 3D Editor")
    width: 1280
    height: 700
    visible: true

    MouseArea {
        id: applicationMouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    property string selectedEntityName: ""
    property var sceneModel: EditorSceneItemComponentsModel
    property url saveFileUrl: ""

    property color labelTextColor: "white"

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem {
                action: fileLoadAction
            }
            MenuItem {
                action: fileSaveAsAction
            }
            MenuItem {
                action: fileSaveAction
            }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: qsTr("&Edit")
            MenuItem {
                action: undoAction
            }
            MenuItem {
                action: redoAction
            }
        }
        Menu {
            id: viewMenu
            title: qsTr("&View")
            Menu {
                id: cameraMenu
                title: qsTr("&Camera")
                ExclusiveGroup {
                    id: sceneCamerasGroup
                }

                Instantiator {
                    model: editorScene.sceneCamerasModel
                    MenuItem {
                        enabled: !freeViewCamera.checked
                        text: model.display
                        checkable: true
                        checked: editorScene.activeSceneCameraIndex === index
                        exclusiveGroup: sceneCamerasGroup
                        onTriggered: {
                            editorScene.activeSceneCameraIndex = index
                        }
                    }
                    onObjectAdded: cameraMenu.insertItem(index, object)
                    onObjectRemoved: cameraMenu.removeItem(object)
                }

                MenuSeparator {
                }

                MenuItem {
                    id: freeViewCamera
                    text: qsTr("Free viewing mode")
                    checkable: true
                    checked: editorScene.freeView
                    onTriggered: {
                        editorScene.freeView = checked
                    }
                }
            }
            Menu {
                id: helperPlaneMenu
                title: qsTr("&Helper Plane")
                ExclusiveGroup {
                    id: helperPlaneOrientationGroup
                }
                MenuItem {
                    id: planeOrientationX
                    text: qsTr("Normal &X")
                    checkable: true
                    checked: false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked) {
                            editorScene.helperPlaneTransform.rotation =
                                editorScene.helperPlaneTransform.fromAxisAndAngle(0, 1, 0, 90)
                        }
                    }
                }
                MenuItem {
                    id: planeOrientationY
                    text: qsTr("Normal &Y")
                    checkable: true
                    checked: true
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked) {
                            editorScene.helperPlaneTransform.rotation =
                                editorScene.helperPlaneTransform.fromAxisAndAngle(1, 0, 0, 90)
                        }
                    }
                }
                MenuItem {
                    id: planeOrientationZ
                    text: qsTr("Normal &Z")
                    checkable: true
                    checked: false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked) {
                            editorScene.helperPlaneTransform.rotation =
                                editorScene.helperPlaneTransform.fromAxisAndAngle(0, 0, 1, 90)
                        }
                    }
                }
                MenuItem {
                    id: planeDisabled
                    text: qsTr("&Hide")
                    checkable: true
                    checked: false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        editorScene.helperPlane.enabled = !checked
                    }
                }
            }
        }
    }

    FileDialog {
        id: loadFileDialog
        selectMultiple: false
        selectExisting: true
        title: qsTr("Load Scene")
        nameFilters: [qsTr("Qt3D Scenes (*.qml)")]
        onAccepted: {
            if (editorScene.loadScene(fileUrl))
                entityTree.selectSceneRoot()
        }
    }

    FileDialog {
        id: saveFileDialog
        selectMultiple: false
        selectExisting: false
        title: qsTr("Save Scene")
        nameFilters: [qsTr("Qt3D Scenes (*.qml)")]
        onAccepted: {
            editorScene.saveScene(fileUrl)
            saveFileUrl = fileUrl
        }
    }

    Action {
        id: fileLoadAction
        text: qsTr("L&oad")
        shortcut: StandardKey.Open
        onTriggered: loadFileDialog.open()
//        onTriggered: {
//            var success = editorScene.loadScene("file:///D:/dev/qt/apps/qt3deditor/Qt3DEditorTestApp/GeneratedScene.qml") // Debug
//            if (success)
//                entityTree.selectSceneRoot()
//        }
}

    Action {
        id: fileSaveAction
        text: qsTr("&Save")
        shortcut: StandardKey.Save
//        onTriggered: editorScene.saveScene("file:///D:/dev/qt/apps/qt3deditor/Qt3DEditorTestApp/GeneratedScene.qml") // Debug
        onTriggered: {
            if (saveFileUrl == "")
                saveFileDialog.open()
            else
                editorScene.saveScene(saveFileUrl)
        }
    }

    Action {
        id: fileSaveAsAction
        text: qsTr("Save As")
        shortcut: StandardKey.SaveAs
        onTriggered: saveFileDialog.open()
    }

    Action {
        id: undoAction
        text: editorScene.undoHandler.undoText === "" ? qsTr ("Undo") : qsTr ("Undo '%1'").arg(_undoHandler.undoText)
        enabled: editorScene.undoHandler.canUndo
        shortcut: StandardKey.Undo
        onTriggered: editorScene.undoHandler.undo()
    }

    Action {
        id: redoAction
        text: editorScene.undoHandler.redoText === "" ? qsTr ("Redo") : qsTr ("Redo '%1'").arg(_undoHandler.redoText)
        enabled: editorScene.undoHandler.canRedo
        shortcut: StandardKey.Redo
        onTriggered: editorScene.undoHandler.redo()
    }

    EditorScene {
        id: editorScene
        viewport: editorViewport
        freeView: true

        onSelectionChanged: {
            var index = editorScene.sceneModel.getModelIndex(selection)
            // Expand tree view to the selection
            var previous = index
            do {
                previous = previous.parent
                entityTree.view.expand(previous)
            } while (previous.valid)
            // Highlight the selection
            entityTree.view.forceActiveFocus()
            entityTree.view.selection.setCurrentIndex(index, ItemSelectionModel.SelectCurrent)
        }

        onErrorChanged: {
            notification.title = qsTr("Error")
            notification.icon = StandardIcon.Warning
            notification.text = error
            notification.open()
        }
    }

    MessageDialog {
        id: notification
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        SplitView {
            orientation: Qt.Vertical
            Layout.minimumWidth: 100
            Layout.maximumWidth: mainwindow.width - 10
            width: mainwindow.width / 5

            // Entity library
            EntityLibrary {
                id: entityLibrary
                onCreateNewEntity: {
                    entityTree.selectSceneRoot() //TODO: check where the entity is really added to
                    entityTree.addNewEntity(entityType)
                }
            }

            Rectangle {
                Layout.fillHeight: true
                color: "lightgray"
            }

        }

        Item {
            Layout.fillWidth: true

            EditorViewport {
                id: editorViewport
                // TODO: Picking doesn't work correctly, as view is smaller than the window
                //anchors.fill: parent
                // TODO: Use this until picking is fixed
                width: mainwindow.width
                height: mainwindow.height
                scene: editorScene
                inputEnabled: editorScene.freeView
            }
        }

        Rectangle { // TODO: remove rectangle once picking works correctly
            width: 250
            Layout.minimumWidth: 250
            color: "lightGray"

            SplitView {
                anchors.fill: parent // TODO: remove once picking works correctly
                orientation: Qt.Vertical
                Layout.minimumWidth: 250
                Layout.maximumWidth: mainwindow.width - 10
                //width: mainwindow.width / 5 // TODO: Use once picking works correctly

                // Entity list
                EntityTree {
                    id: entityTree
                }

                // Property (transform, material, etc.) list
                ListView {
                    id: componentPropertiesView
                    Layout.fillHeight: true
                    delegate: ComponentPropertiesDelegate {}
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                }
            }
        }
    }
}
