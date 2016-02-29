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
import com.theqtcompany.SceneEditor3D 1.0

ApplicationWindow {
    id: mainwindow
    title: qsTr("Qt 3D Scene Editor") + editorScene.emptyString
    width: 1280
    height: 700
    visible: true

    MouseArea {
        id: applicationMouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    property var selectedEntity: null
    property string selectedEntityName: ""
    property var sceneModel: EditorSceneItemComponentsModel
    property url saveFileUrl: ""

    property color labelTextColor: "white"

    property bool transformViewVisible: true
    property bool materialViewVisible: true
    property bool meshViewVisible: true
    property bool lightViewVisible: true
    property bool cameraViewVisible: true

    property string systemLanguage: editorScene.language

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File") + editorScene.emptyString
            MenuItem {
                text: qsTr("&New") + editorScene.emptyString
                onTriggered: {
                    editorScene.resetScene()
                    planeOrientationY.checked = true
                    saveFileUrl = ""
                    autoSave.checked = false
                }
            }
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
                id: autoSave
                text: qsTr("Enable autosave") + editorScene.emptyString
                checkable: true
                checked: false
                onTriggered: {
                    if (checked) {
                        if (saveFileUrl == "")
                            saveFileDialog.open()
                        autoSaveTimer.start()
                    } else {
                        autoSaveTimer.stop()
                    }
                }
            }
            MenuSeparator {
            }
            MenuItem {
                text: qsTr("E&xit") + editorScene.emptyString
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: qsTr("&Edit") + editorScene.emptyString
            MenuItem {
                action: undoAction
            }
            MenuItem {
                action: redoAction
            }
        }
        Menu {
            id: viewMenu
            title: qsTr("&View") + editorScene.emptyString
            Menu {
                id: cameraMenu
                title: qsTr("&Camera") + editorScene.emptyString
                ExclusiveGroup {
                    id: sceneCamerasGroup
                }

                Instantiator {
                    model: editorScene.sceneCamerasModel
                    MenuItem {
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
                    text: qsTr("Free viewing mode") + editorScene.emptyString
                    checkable: true
                    checked: editorScene.freeView
                    onTriggered: {
                        editorScene.freeView = checked
                    }
                }
                MenuItem {
                    enabled: freeViewCamera.checked
                    text: qsTr("Reset") + editorScene.emptyString
                    onTriggered: {
                        editorScene.resetFreeViewCamera()
                    }
                }
                MenuItem {
                    enabled: freeViewCamera.checked
                    text: qsTr("Add scene camera here") + editorScene.emptyString
                    onTriggered: {
                        entityTree.selectSceneRoot()
                        entityTree.addNewEntity(EditorSceneItemModel.CameraEntity)
                        editorScene.copyFreeViewToNewSceneCamera()
                        // TODO: Needs undo/redo support.
                        // TODO: Also, adding and copying should be undone/redone in a single step
                    }
                }
                MenuItem {
                    enabled: freeViewCamera.checked
                    text: qsTr("Move active camera here") + editorScene.emptyString
                    onTriggered: {
                        editorScene.moveActiveSceneCameraToFreeView()
                        // TODO: Needs undo/redo support.
                    }
                }
                MenuItem {
                    enabled: freeViewCamera.checked
                    text: qsTr("Snap to active camera") + editorScene.emptyString
                    onTriggered: {
                        editorScene.snapFreeViewCameraToActiveSceneCamera()
                    }
                }
            }
            Menu {
                id: helperPlaneMenu
                title: qsTr("&Helper Plane") + editorScene.emptyString
                ExclusiveGroup {
                    id: helperPlaneOrientationGroup
                }
                MenuItem {
                    id: planeOrientationX
                    text: qsTr("Normal &X") + editorScene.emptyString
                    checkable: true
                    checked: helperPlaneComboBox.currentIndex === 0 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            helperPlaneComboBox.currentIndex = 0
                    }
                }
                MenuItem {
                    id: planeOrientationY
                    text: qsTr("Normal &Y") + editorScene.emptyString
                    checkable: true
                    checked: helperPlaneComboBox.currentIndex === 1 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            helperPlaneComboBox.currentIndex = 1
                    }
                }
                MenuItem {
                    id: planeOrientationZ
                    text: qsTr("Normal &Z") + editorScene.emptyString
                    checkable: true
                    checked: helperPlaneComboBox.currentIndex === 2 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            helperPlaneComboBox.currentIndex = 2
                    }
                }
                MenuItem {
                    id: planeDisabled
                    text: qsTr("&Hide") + editorScene.emptyString
                    checkable: true
                    checked: helperPlaneComboBox.currentIndex === 3 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            helperPlaneComboBox.currentIndex = 3
                    }
                }
            }
            Menu {
                id: languageMenu
                title: qsTr("&Language") + editorScene.emptyString
                ExclusiveGroup {
                    id: languageGroup
                }
                MenuItem {
                    id: languageEnglish
                    text: qsTr("English") + editorScene.emptyString
                    checkable: true
                    checked: (systemLanguage == "en")
                    exclusiveGroup: languageGroup
                    onCheckedChanged: {
                        if (checked) {
                            editorScene.language = "en"
                        }
                    }
                }
                MenuItem {
                    id: languageFinnish
                    text: qsTr("Finnish") + editorScene.emptyString
                    checkable: true
                    checked: (systemLanguage == "fi")
                    exclusiveGroup: languageGroup
                    onCheckedChanged: {
                        if (checked) {
                            editorScene.language = "fi"
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: loadFileDialog
        selectMultiple: false
        selectExisting: true
        title: qsTr("Load Scene") + editorScene.emptyString
        nameFilters: [qsTr("Qt3D Scenes (*.qml)") + editorScene.emptyString]
        onAccepted: {
            if (editorScene.loadScene(fileUrl))
                entityTree.selectSceneRoot()
        }
    }

    FileDialog {
        id: saveFileDialog
        selectMultiple: false
        selectExisting: false
        title: qsTr("Save Scene") + editorScene.emptyString
        nameFilters: [qsTr("Qt3D Scenes (*.qml)") + editorScene.emptyString]
        onAccepted: {
            editorScene.saveScene(fileUrl)
            saveFileUrl = fileUrl
        }
    }

    Action {
        id: fileLoadAction
        text: qsTr("L&oad") + editorScene.emptyString
        shortcut: StandardKey.Open
        onTriggered: loadFileDialog.open()
    }

    Action {
        id: fileSaveAction
        text: qsTr("&Save") + editorScene.emptyString
        shortcut: StandardKey.Save
        onTriggered: {
            if (saveFileUrl == "") {
                saveFileDialog.open()
                // No previous autosave file, no need to delete anything
            } else {
                editorScene.saveScene(saveFileUrl)
                editorScene.deleteScene(saveFileUrl, true)
            }
        }
    }

    Action {
        id: fileSaveAsAction
        text: qsTr("Save As") + editorScene.emptyString
        shortcut: StandardKey.SaveAs
        onTriggered: {
            if (saveFileUrl != "")
                editorScene.deleteScene(saveFileUrl, true)
            saveFileDialog.open()
        }
    }

    Action {
        id: undoAction
        text: editorScene.undoHandler.undoText === "" ? qsTr ("Undo") + editorScene.emptyString : qsTr ("Undo '%1'").arg(_undoHandler.undoText) + editorScene.emptyString
        enabled: editorScene.undoHandler.canUndo
        shortcut: StandardKey.Undo
        onTriggered: editorScene.undoHandler.undo()
    }

    Action {
        id: redoAction
        text: editorScene.undoHandler.redoText === "" ? qsTr ("Redo") + editorScene.emptyString : qsTr ("Redo '%1'").arg(_undoHandler.redoText) + editorScene.emptyString
        enabled: editorScene.undoHandler.canRedo
        shortcut: StandardKey.Redo
        onTriggered: editorScene.undoHandler.redo()
    }

    toolBar: ToolBar {
        height: 24
        RowLayout {
            ComboBox {
                id: helperPlaneComboBox
                model: ListModel {
                    property string language: systemLanguage

                    function retranslateUi() {
                        // Repopulate list to change the current text as well
                        clear()
                        append({text: qsTr("Normal X")})
                        append({text: qsTr("Normal Y")})
                        append({text: qsTr("Normal Z")})
                        append({text: qsTr("Hide Helper Plane")})
                    }

                    Component.onCompleted: retranslateUi()

                    onLanguageChanged: retranslateUi()
                }

                currentIndex: 1

                onCurrentIndexChanged: {
                    if (editorScene) {
                        if (currentIndex === 3) {
                            editorScene.helperPlane.enabled = false
                        } else {
                            editorScene.helperPlane.enabled = true
                            if (currentIndex === 0) {
                                editorScene.helperPlaneTransform.rotation =
                                    editorScene.helperPlaneTransform.fromAxisAndAngle(0, 1, 0, 90)
                            } else if (currentIndex === 1) {
                                editorScene.helperPlaneTransform.rotation =
                                    editorScene.helperPlaneTransform.fromAxisAndAngle(1, 0, 0, 90)
                            } else if (currentIndex === 2) {
                                editorScene.helperPlaneTransform.rotation =
                                    editorScene.helperPlaneTransform.fromAxisAndAngle(0, 0, 1, 90)
                            }
                        }
                    }
                }
            }
            ToolButton {
                text: qsTr("Reset to Default") + editorScene.emptyString
                onClicked: editorScene.resetFreeViewCamera()
            }
        }
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
            notification.title = qsTr("Error") + editorScene.emptyString
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

        // Entity library
        EntityLibrary {
            id: entityLibrary
            Layout.minimumWidth: 100
            Layout.maximumWidth: mainwindow.width - 10
            onCreateNewEntity: {
                entityTree.selectSceneRoot() //TODO: check where the entity is really added to
                entityTree.addNewEntity(entityType)
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

                GeneralPropertyView {
                    id: generalPropertyView
                    entityName: selectedEntityName
                    entityType: editorScene.sceneModel.editorSceneItemFromIndex(entityTree.view.selection.currentIndex).itemType()
                    propertiesButtonVisible: {
                        (entityTree.view.selection.currentIndex
                         !== editorScene.sceneModel.sceneEntityIndex())
                                ? true : false
                    }
                }

                // Property (transform, material, etc.) list
                ListView {
                    id: componentPropertiesView
                    Layout.fillHeight: true
                    delegate: ComponentPropertiesDelegate {}
                    flickableDirection: Flickable.VerticalFlick
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    visible: generalPropertyView.viewTitleVisible
                }
            }
        }
    }

    Timer {
        id: autoSaveTimer
        running: false
        interval: 600000 // 10 minutes
        repeat: true
        onTriggered: {
            editorScene.saveScene(saveFileUrl, true)
        }
    }
}
