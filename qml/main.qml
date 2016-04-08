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
import Qt.labs.controls 1.0 as QLC
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.2
import Qt3D.Core 2.0
import com.theqtcompany.SceneEditor3D 1.0

ApplicationWindow {
    id: mainwindow
    title: qsTr("Qt 3D Scene Editor") + editorScene.emptyString
    width: 1280
    height: 800
    visible: true
    color: "lightGray"
    minimumHeight: 400
    minimumWidth: 640

    Item {
        id: applicationArea
        anchors.fill: parent
    }

    property var selectedEntity: null
    property string selectedEntityName: ""
    property var sceneModel: EditorSceneItemComponentsModel
    property url saveFileUrl: ""
    property int currentHelperPlane: 1
    property alias selectedEntityType: generalPropertyView.entityType

    property color labelTextColor: "white"

    property bool transformViewVisible: true
    property bool materialViewVisible: true
    property bool meshViewVisible: true
    property bool lightViewVisible: true
    property bool cameraViewVisible: true

    property real qlcControlHeight: 28

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
            MenuItem {
                action: entityImportAction
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
                    text: qsTr("Reset") + editorScene.emptyString
                    onTriggered: {
                        resetCameraToDefault()
                    }
                }
                MenuItem {
                    enabled: freeViewCamera.checked
                    text: qsTr("Add scene camera here") + editorScene.emptyString
                    onTriggered: {
                        entityTree.selectSceneRoot()
                        entityTree.addNewEntity(EditorUtils.CameraEntity)
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
                    checked: currentHelperPlane === 0 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            mainwindow.showNormalXPlane()
                    }
                }
                MenuItem {
                    id: planeOrientationY
                    text: qsTr("Normal &Y") + editorScene.emptyString
                    checkable: true
                    checked: currentHelperPlane === 1 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            mainwindow.showNormalYPlane()
                    }
                }
                MenuItem {
                    id: planeOrientationZ
                    text: qsTr("Normal &Z") + editorScene.emptyString
                    checkable: true
                    checked: currentHelperPlane === 2 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            mainwindow.showNormalZPlane()
                    }
                }
                MenuItem {
                    id: gridSize
                    text: qsTr("Change Grid Size") + editorScene.emptyString
                    onTriggered: {
                        gridSizeSpinBox.value = editorScene.gridSize
                        gridSizeDialog.open()
                    }
                }
                MenuItem {
                    id: planeDisabled
                    text: qsTr("&Hide") + editorScene.emptyString
                    checkable: true
                    checked: currentHelperPlane === 3 ? true : false
                    exclusiveGroup: helperPlaneOrientationGroup
                    onCheckedChanged: {
                        if (checked)
                            mainwindow.hideHelperPlane()
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

    FileDialog {
        id: importEntityDialog
        selectMultiple: false
        selectExisting: true
        title: qsTr("Import Entity") + editorScene.emptyString
        nameFilters: [qsTr("All files (*)") + editorScene.emptyString]
        onAccepted: {
            editorScene.importEntity(fileUrl)
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
        id: entityImportAction
        text: qsTr("&Import Entity") + editorScene.emptyString
        onTriggered: {
            importEntityDialog.open();
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
        id: mainToolBar
        height: normalXButton.height + 4
        RowLayout {
            EnableButton {
                id: normalXButton
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "/images/helperplane_x_deselected.png"
                disabledIconSource: "/images/helperplane_x_selected.png"
                tooltip: qsTr("Normal X (Ctrl + 1)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 0 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalXPlane()
            }
            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "/images/helperplane_y_deselected.png"
                disabledIconSource: "/images/helperplane_y_selected.png"
                tooltip: qsTr("Normal Y (Ctrl + 2)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 1 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalYPlane()
            }
            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "/images/helperplane_z_deselected.png"
                disabledIconSource: "/images/helperplane_z_selected.png"
                tooltip: qsTr("Normal Z (Ctrl + 3)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 2 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalZPlane()
            }
            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "/images/helperplane_none_deselected.png"
                disabledIconSource: "/images/helperplane_none_selected.png"
                tooltip: qsTr("Hide helper plane (Ctrl + 4)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 3 ? false : true
                onEnabledButtonClicked: mainwindow.hideHelperPlane()
            }
            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "/images/reset_camera_to_default.png"
                disabledIconSource: "/images/reset_camera_to_default.png"
                tooltip: qsTr("Reset to Default (Ctrl + R)") + editorScene.emptyString
                buttonEnabled: true
                onEnabledButtonClicked: resetCameraToDefault()
            }
        }
    }
    Shortcut {
        id: normalXShortcut
        sequence: "Ctrl+1"
        onActivated: mainwindow.showNormalXPlane()
    }
    function showNormalXPlane() {
        editorScene.helperPlane.enabled = true
        editorScene.helperPlaneTransform.rotation =
                editorScene.helperPlaneTransform.fromAxisAndAngle(0, 1, 0, 90)
        currentHelperPlane = 0
    }
    Shortcut {
        id: normalYShortcut
        sequence: "Ctrl+2"
        onActivated: mainwindow.showNormalYPlane()
    }
    function showNormalYPlane() {
        editorScene.helperPlane.enabled = true
        editorScene.helperPlaneTransform.rotation =
                editorScene.helperPlaneTransform.fromAxisAndAngle(1, 0, 0, 90)
        currentHelperPlane = 1
    }
    Shortcut {
        id: normalZShortcut
        sequence: "Ctrl+3"
        onActivated: mainwindow.showNormalZPlane()
    }
    function showNormalZPlane() {
        editorScene.helperPlane.enabled = true
        editorScene.helperPlaneTransform.rotation =
                editorScene.helperPlaneTransform.fromAxisAndAngle(0, 0, 1, 90)
        currentHelperPlane = 2
    }
    Shortcut {
        id: hideHelperPlaneShortcut
        sequence: "Ctrl+4"
        onActivated: mainwindow.hideHelperPlane()
    }
    function hideHelperPlane() {
        editorScene.helperPlane.enabled = false
        currentHelperPlane = 3
    }
    Shortcut {
        id: resetCameraShortcut
        sequence: "Ctrl+R"
        onActivated: resetCameraToDefault()
    }

    EditorScene {
        id: editorScene
        viewport: editorViewport
        freeView: true

        onSelectionChanged: {
            restoreSelection(selection)
        }

        onErrorChanged: {
            notification.title = qsTr("Error") + editorScene.emptyString
            notification.icon = StandardIcon.Warning
            notification.text = error
            notification.open()
        }

        function restoreSelection(entity) {
            var index = editorScene.sceneModel.getModelIndex(entity)
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
    }

    MessageDialog {
        id: notification
    }

    Dialog {
        id: gridSizeDialog
        property int previousSize: editorScene.gridSize
        title: qsTr("Grid Size") + editorScene.emptyString
        standardButtons: StandardButton.Cancel | StandardButton.Apply | StandardButton.Ok

        QLC.SpinBox {
            id: gridSizeSpinBox
            anchors.horizontalCenter: parent.horizontalCenter
            to: 20
            stepSize: 1
            from: 1
        }

        onRejected: {
            editorScene.gridSize = previousSize
        }

        onApply: {
            editorScene.gridSize = gridSizeSpinBox.value
        }

        onAccepted: {
            editorScene.gridSize = gridSizeSpinBox.value
        }
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
                entityTree.addNewEntity(entityType, xPos, yPos)
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 100

            EditorViewport {
                id: editorViewport
                anchors.fill: parent
                scene: editorScene
                //inputEnabled: editorScene.freeView // TODO: Why was editing disabled for scene cameras?

                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        entityTree.focusTree()
                        mouse.accepted = false
                    }
                }
            }
        }

        SplitView {
            orientation: Qt.Vertical
            Layout.minimumWidth: 250
            Layout.maximumWidth: mainwindow.width - 10
            width: mainwindow.width / 5

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

    Timer {
        id: autoSaveTimer
        running: false
        interval: 600000 // 10 minutes
        repeat: true
        onTriggered: {
            editorScene.saveScene(saveFileUrl, true)
        }
    }

    function resetCameraToDefault() {
        editorScene.freeView = true
        editorScene.resetFreeViewCamera()
    }
}
