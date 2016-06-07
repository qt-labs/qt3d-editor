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
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.2
import QtQml.Models 2.2
import QtQml 2.2
import Qt3D.Core 2.0
import Qt.labs.settings 1.0
import com.theqtcompany.SceneEditor3D 1.0

Item {
    id: editorContent
    anchors.fill: parent

    property var selectedEntity: null
    property string selectedEntityName: ""
    property url saveFileUrl: ""
    property url defaultFolder: "file:///"
    property url saveFolder: "file:///"
    property url textureFolder: "file:///"
    property url importFolder: "file:///"
    property string saveFileTitleAddition: {
        if (saveFileUrl != "")
            " - " + saveFileUrl.toString().substring(saveFileUrl.toString().lastIndexOf("/") + 1)
        else
            saveFileUrl.toString()
    }

    property int currentHelperPlane: 1
    property alias selectedEntityType: generalPropertyView.entityType

    property bool trackMousePosition: false
    property int mousePosY: -1
    property int mousePosX: -1

    property color textColor: "#ffffff"
    property color disabledTextColor: "#a0a1a2"
    property color selectionColor: "#43adee"
    property color listHighlightColor: "#585a5c"
    property color paneBackgroundColor: "#2e2f30"
    property color paneColor: "#373839"
    property color viewBorderColor: "#000000"
    property color itemBackgroundColor: "#46484a"
    property color itemColor: "#cccccc"
    property color iconHighlightColor: "#26282a"
    property string labelFontFamily: "Open Sans"
    property int labelFontWeight: Font.Normal
    property int labelFontPixelSize: 12
    property int maximumControlWidth: 200
    property int controlMargin: 4

    property bool transformViewVisible: true
    property bool materialViewVisible: true
    property bool meshViewVisible: true
    property bool lightViewVisible: true
    property bool cameraViewVisible: true

    property real qlcControlHeight: 28

    property string systemLanguage: editorScene.language
    property alias editorScene: editorScene

    function checkUnsavedChanges() {
        if (!editorScene.undoHandler.isClean()) {
            closingDialog.open()
            return false
        }
        return true
    }

    function fileLoad() {
        if (!editorScene.undoHandler.isClean()) {
            saveUnsavedDialog.newFile = false
            saveUnsavedDialog.open()
        } else {
            loadFileDialog.folder = saveFolder
            loadFileDialog.open()
        }
    }

    function fileSave() {
        if (saveFileUrl == "") {
            saveFileDialog.folder = saveFolder
            saveFileDialog.open()
            // No previous autosave file, no need to delete anything
        } else {
            editorScene.saveScene(saveFileUrl)
            editorScene.deleteScene(saveFileUrl, true)
        }
    }

    function fileSaveAs() {
        if (editorContent.saveFileUrl != "")
            editorScene.deleteScene(editorContent.saveFileUrl, true)
        saveFileDialog.folder = saveFolder
        saveFileDialog.open()
    }

    function undo() {
        editorScene.undoHandler.undo()
    }

    function redo() {
        editorScene.undoHandler.redo()
    }

    function fileNew() {
        if (!editorScene.undoHandler.isClean()) {
            saveUnsavedDialog.newFile = true
            saveUnsavedDialog.open()
        } else {
            editorScene.resetScene()
            showNormalYPlane()
            saveFileUrl = ""
        }
    }

    function showNormalXPlane() {
        editorScene.helperPlane.enabled = true
        editorScene.helperPlaneTransform.rotation =
                editorScene.helperPlaneTransform.fromAxisAndAngle(0, 1, 0, 90)
        currentHelperPlane = 0
    }

    function showNormalYPlane() {
        editorScene.helperPlane.enabled = true
        editorScene.helperPlaneTransform.rotation =
                editorScene.helperPlaneTransform.fromAxisAndAngle(1, 0, 0, 90)
        currentHelperPlane = 1
    }

    function showNormalZPlane() {
        editorScene.helperPlane.enabled = true
        editorScene.helperPlaneTransform.rotation =
                editorScene.helperPlaneTransform.fromAxisAndAngle(0, 0, 1, 90)
        currentHelperPlane = 2
    }

    function hideHelperPlane() {
        editorScene.helperPlane.enabled = false
        currentHelperPlane = 3
    }

    function copyEntity(entityName) {
        if (entityName !== editorScene.sceneRootName()) {
            // Disable possible previous cut operation
            if (editorScene.clipboardOperation === EditorScene.ClipboardCut) {
                var index = editorScene.sceneModel.getModelIndexByName(
                            editorScene.clipboardContent)
                var sceneItem = editorScene.sceneModel.editorSceneItemFromIndex(index)
                sceneItem.entity().enabled = true
            }
            trackMousePosition = true
            editorScene.clipboardContent = entityName
            editorScene.clipboardOperation = EditorScene.ClipboardCopy
            // TODO: When to stop reading mouse movements when copy-pasting?
            // TODO: When any other operation is done?
        }
    }

    function cutEntity(entityName, entity) {
        if (entityName !== editorScene.sceneRootName()) {
            trackMousePosition = true
            editorScene.clipboardContent = entityName
            editorScene.clipboardOperation = EditorScene.ClipboardCut
            entity.entity().enabled = false
        }
    }

    function pasteEntity() {
        if (editorScene.clipboardContent.length) {
            var parentName = ""
            if (entityTree.treeviewPasting) {
                parentName = selectedEntityName
                // Prevent pasting an entity under itself in tree
                if (!editorScene.sceneModel.canReparent(
                            editorScene.sceneModel.editorSceneItemFromIndex(
                                editorScene.sceneModel.getModelIndexByName(
                                    selectedEntityName)),
                            editorScene.sceneModel.editorSceneItemFromIndex(
                                editorScene.sceneModel.getModelIndexByName(
                                    editorScene.clipboardContent)), true)) {
                    return
                }
            }
            // When pasting to tree, world position is not used, and parent entity name is passed
            editorScene.undoHandler.createPasteEntityCommand(editorScene.getWorldPosition(
                                                                 mousePosX, mousePosY), parentName)
            if (editorScene.clipboardOperation === EditorScene.ClipboardCut) {
                trackMousePosition = false
                editorScene.clipboardOperation = EditorScene.ClipboardNone
            }
        }
    }

    Settings {
        // Save view panel sizes
        // Use detailed category name, as plugin saves settings under QtCreator application
        category: "Qt 3D SceneEditor Content Geometry"
        property alias leftPaneWidth: entityLibrary.width
        property alias rightPaneWidth: propertyPane.width
        property alias entityTreeHeight: entityTree.height
    }

    Settings {
        category: "Qt 3D SceneEditor Folders"
        property alias saveFolder: editorContent.saveFolder
        property alias textureFolder: editorContent.textureFolder
        property alias importFolder: editorContent.importFolder
        property alias defaultFolder: editorContent.defaultFolder
    }

    EditorToolbar {
        id: editorToolbar
    }
    Item {
        anchors.top: editorToolbar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Item {
            // This item is used to map global mouse position
            id: applicationArea
            anchors.fill: parent
        }

        DragEntity {
            id: dragEntityItem
        }


        FileDialog {
            id: loadFileDialog
            selectMultiple: false
            selectExisting: true
            title: qsTr("Load Scene") + editorScene.emptyString
            nameFilters: [qsTr("Qt3D Scenes (*.qt3d.qrc)") + editorScene.emptyString]
            onAccepted: {
                if (editorScene.loadScene(fileUrl)) {
                    entityTree.selectSceneRoot()
                    editorContent.saveFolder = folder
                    editorContent.saveFileUrl = fileUrl
                }
            }
        }

        FileDialog {
            id: saveFileDialog
            selectMultiple: false
            selectExisting: false
            property bool exiting: false
            title: qsTr("Save Scene") + editorScene.emptyString
            nameFilters: [qsTr("Qt3D Scenes (*.qt3d.qrc)") + editorScene.emptyString]
            onAccepted: {
                editorScene.saveScene(fileUrl)
                editorContent.saveFolder = folder
                editorContent.saveFileUrl = fileUrl
                if (exiting)
                    Qt.quit()
            }
        }

        FileDialog {
            id: importEntityDialog
            selectMultiple: false
            selectExisting: true
            title: qsTr("Import Entity") + editorScene.emptyString
            nameFilters: [qsTr("All files (*)") + editorScene.emptyString]
            onAccepted: {
                editorScene.undoHandler.createImportEntityCommand(fileUrl)
                editorContent.importFolder = folder
            }
        }


        Shortcut {
            id: copyShortcut
            sequence: StandardKey.Copy
            onActivated: {
                // Prevent copying multiselection (for now, at least)
                if (!editorScene.multiSelection)
                    editorContent.copyEntity(editorContent.selectedEntityName)
            }
        }

        Shortcut {
            id: cutShortcut
            sequence: StandardKey.Cut
            onActivated: {
                // Prevent cutting multiselection (for now, at least)
                if (!editorScene.multiSelection)
                    editorContent.cutEntity(editorContent.selectedEntityName, editorContent.selectedEntity)
            }
        }

        Shortcut {
            id: pasteShortcut
            sequence: StandardKey.Paste
            onActivated: {
                editorContent.pasteEntity()
            }
        }

        EditorScene {
            id: editorScene
            viewport: editorViewport
            freeView: true

            onSelectionChanged: {
                restoreSelection(selection)
            }

            onMultiSelectionListChanged: {
                restoreMultiSelection(editorScene.multiSelectionList)
            }

            onErrorChanged: {
                notification.title = qsTr("Error") + editorScene.emptyString
                notification.text = error
                notification.open()
            }

            onMouseRightButtonReleasedWithoutDragging: {
                entityTree.menu.popup()
            }

            onClipboardOperationChanged: {
                if (clipboardOperation === EditorScene.ClipboardNone)
                    editorContent.trackMousePosition = false
            }

            function restoreSelection(entity) {
                var index = editorScene.sceneModel.getModelIndex(entity)
                selectIndex(index)
            }

            function restoreMultiSelection(selectionList) {
                // Deselect old ones
                entityTree.view.selection.clear()
                // Dig indexes of all selected entities and pass the selections to entitytree
                for (var i = 0; i < selectionList.length; ++i) {
                    var index = editorScene.sceneModel.getModelIndexByName(multiSelectionList[i])
                    entityTree.view.selection.select(index, ItemSelectionModel.Select)
                    expandTo(index)
                }
            }

            function selectIndex(index) {
                expandTo(index)
                entityTree.focusTree()
                entityTree.view.selection.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect)
            }

            function expandTo(index) {
                var target = index
                do {
                    entityTree.view.expand(target)
                    target = target.parent
                } while (target.valid)
            }
        }

        MessageDialog {
            id: notification
            icon: StandardIcon.Warning
        }

        SettingsDialog {
            id: settingsDialog
        }

        MessageDialog {
            id: closingDialog
            icon: StandardIcon.Warning
            standardButtons: StandardButton.Yes | StandardButton.No | StandardButton.Cancel
            title: qsTr("Exit Qt 3D Scene Editor") + editorScene.emptyString
            text: qsTr("There are unsaved changes.\nSave changes before exiting?")
                  + editorScene.emptyString

            onYes: {
                if (editorContent.saveFileUrl == "") {
                    saveFileDialog.exiting = true
                    saveFileDialog.open()
                    // No previous autosave file, no need to delete anything
                    saveFileDialog.exiting = false
                } else {
                    editorScene.saveScene(editorContent.saveFileUrl)
                    editorScene.deleteScene(editorContent.saveFileUrl, true)
                    Qt.quit()
                }
            }

            onNo: {
                Qt.quit()
            }

            // Cancel doesn't need to do anything
        }

        MessageDialog {
            id: saveUnsavedDialog
            property bool newFile: false
            icon: StandardIcon.Warning
            standardButtons: StandardButton.Yes | StandardButton.No | StandardButton.Cancel
            title: qsTr("Save changes?") + editorScene.emptyString
            text: qsTr("There are unsaved changes.\nSave changes?")
                  + editorScene.emptyString

            onYes: {
                if (editorContent.saveFileUrl == "") {
                    saveFileDialog.folder = editorContent.saveFolder
                    saveFileDialog.open()
                    // No previous autosave file, no need to delete anything
                } else {
                    editorScene.saveScene(editorContent.saveFileUrl)
                    editorScene.deleteScene(editorContent.saveFileUrl, true)
                }
            }

            onNo: {
                if (newFile) {
                    editorScene.resetScene()
                    showNormalYPlane()
                    editorContent.saveFileUrl = ""
                } else {
                    loadFileDialog.folder = editorContent.saveFolder
                    loadFileDialog.open()
                }
            }

            // Cancel doesn't need to do anything
        }


        SplitView {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            orientation: Qt.Horizontal
            width: parent.width - propertyPane.visibleWidth

            // Entity library
            EntityLibrary {
                id: entityLibrary
                z: 4
                Layout.minimumWidth: 100
                Layout.maximumWidth: editorContent.width - 10
                onCreateNewEntity: {
                    entityTree.selectSceneRoot()
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

                    MouseArea {
                        id: viewportMouseArea
                        anchors.fill: parent
                        onPressed: {
                            entityTree.focusTree()
                            mouse.accepted = false
                        }
                        onEntered: entityTree.treeviewPasting = false
                        hoverEnabled: editorContent.trackMousePosition
                        onMouseYChanged: editorContent.mousePosY = mouseY
                        onMouseXChanged: editorContent.mousePosX = mouseX
                    }

                    DropArea {
                        anchors.fill: parent
                        keys: [ "insertEntity" ]
                        onContainsDragChanged: {
                            dragEntityItem.visible = !containsDrag
                        }
                    }
                    DragHandle {
                        // This is the translate handle at the center of selection box
                        handleType: EditorScene.DragTranslate
                        baseZ: 3 // Make sure translate handle is on top of scale handles
                        color: "#c22555"
                        radius: height / 2
                        handleIndex: 0
                    }
                    DragHandle {
                        // This is the group/mesh center indicator handle
                        handleType: EditorScene.DragTranslate
                        baseZ: 3 // Make sure center handle is on top of scale handles
                        color: "blue"
                        radius: height / 2
                        handleIndex: 1
                    }
                    DragHandle {
                        handleType: EditorScene.DragRotate
                        baseZ: 3 // Make sure rotate handle is on top of scale handles
                        Image {
                            anchors.fill: parent
                            source: "images/rotate_handle.png"
                        }
                        color: "transparent"
                    }
                    // Each corner will have a separate drag handle
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 0
                    }
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 1
                    }
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 2
                    }
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 3
                    }
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 4
                    }
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 5
                    }
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 6
                    }
                    DragHandle {
                        handleType: EditorScene.DragScale
                        handleIndex: 7
                    }

                    // Debug handle for visualizing world positions, uncomment to use.
                    // You can show debug handles with EditorScene::showDebugHandle().
    //                DragHandle {
    //                    handleType: EditorScene.DragDebug
    //                    color: "green"
    //                    baseZ: 4
    //                }
                }
            }
        }

        SplitView {
            id: propertyPane
            orientation: Qt.Vertical
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: editorContent.width / 4.5
            x: editorContent.width - visibleWidth

            property int visibleWidth: width

            // Entity list
            EntityTree {
                id: entityTree
            }

            GeneralPropertyView {
                id: generalPropertyView
                entityName: editorContent.selectedEntityName
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

        onWidthChanged: {
            var maximumWidth = editorContent.width - entityLibrary.width - 10
            if (propertyPane.visibleWidth > maximumWidth) {
                propertyPane.visibleWidth = maximumWidth
                propertyPane.width = propertyPane.visibleWidth
            } else if (propertyPane.visibleWidth > resizeRectangle.paneMinimumWidth) {
                propertyPane.width = propertyPane.visibleWidth
            }
            propertyPane.x = editorContent.width - propertyPane.visibleWidth
        }

        Rectangle {
            id: resizeRectangle
            width: 2
            height: propertyPane.height
            anchors.top: parent.top
            anchors.right: propertyPane.left
            color: editorContent.viewBorderColor

            property int paneMinimumWidth: 250
            property int paneMaximumWidth: editorContent.width - entityLibrary.width - 10

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                drag {
                    target: parent;
                    axis: Drag.XAxis
                }

                onEntered: {
                    cursorShape = Qt.SplitHCursor
                }

                onExited: {
                    cursorShape = Qt.ArrowCursor
                }

                onMouseXChanged: {
                    if (drag.active) {
                        var newPaneWidth = propertyPane.width - mouseX
                        if (propertyPane.visibleWidth < resizeRectangle.paneMinimumWidth
                                || newPaneWidth < resizeRectangle.paneMinimumWidth) {
                            var newXPos = propertyPane.x + mouseX
                            if (newXPos < editorContent.width) {
                                propertyPane.visibleWidth = editorContent.width - newXPos
                                propertyPane.x = newXPos
                            }
                        }
                        else if (newPaneWidth > resizeRectangle.paneMinimumWidth
                                 && newPaneWidth < resizeRectangle.paneMaximumWidth) {
                            propertyPane.x = editorContent.width - newPaneWidth
                            propertyPane.width = newPaneWidth
                            propertyPane.visibleWidth = propertyPane.width
                        }
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
                editorScene.saveScene(editorContent.saveFileUrl, true)
            }
        }

        function resetCameraToDefault() {
            editorScene.freeView = true
            editorScene.resetFreeViewCamera()
        }

    }
}

