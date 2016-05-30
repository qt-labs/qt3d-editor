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
import QtQuick.Controls.Styles 1.4
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
    visible: false
    color: paneBackgroundColor
    minimumHeight: 400
    minimumWidth: 640

    Item {
        // This item is used to map global mouse position
        id: applicationArea
        anchors.fill: parent
    }

    DragEntity {
        id: dragEntityItem
    }

    property var selectedEntity: null
    property string selectedEntityName: ""
    property var sceneModel: EditorSceneItemComponentsModel
    property url saveFileUrl: ""
    property int currentHelperPlane: 1
    property alias selectedEntityType: generalPropertyView.entityType

    property color textColor: "#ffffff"
    property color disabledTextColor: "#a0a1a2"
    property color selectionColor: "#43adee"
    property color listHighlightColor: "#585a5c"
    property color paneBackgroundColor: "#2e2f30"
    property color paneColor: "#373839"
    property color viewBorderColor: "#000000"
    property color itemBackgroundColor: "#46484a"
    property color menutItemColor: "#a0a1a2"
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

    property var selectionList: []

    property string systemLanguage: editorScene.language

    MenuBar {
        id: mainMenuBar
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
                onTriggered: {
                    if (checkUnsavedChanges())
                        mainwindow.close()
                }
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
                            editorScene.undoHandler.createChangeGenericPropertyCommand(
                                        editorScene, "activeSceneCameraIndex",
                                        index, editorScene.activeSceneCameraIndex,
                                        qsTr("Change active camera"))
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
                        editorScene.undoHandler.beginMacro(text)
                        entityTree.addNewEntity(EditorUtils.CameraEntity)
                        // When a new camera is added, it is automatically selected
                        editorScene.undoHandler.createCopyCameraPropertiesCommand(
                                    selectedEntityName);
                        editorScene.undoHandler.endMacro()
                    }
                }
                MenuItem {
                    enabled: freeViewCamera.checked
                    text: qsTr("Move active camera here") + editorScene.emptyString
                    onTriggered: {
                        editorScene.undoHandler.createCopyCameraPropertiesCommand(
                                    editorScene.cameraName(editorScene.activeSceneCameraIndex),
                                    "", text);
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

    menuBar: mainMenuBar

    FileDialog {
        id: loadFileDialog
        selectMultiple: false
        selectExisting: true
        title: qsTr("Load Scene") + editorScene.emptyString
        nameFilters: [qsTr("Qt3D Scenes (*.qt3d.qrc)") + editorScene.emptyString]
        onAccepted: {
            if (editorScene.loadScene(fileUrl)) {
                entityTree.selectSceneRoot()
                saveFileUrl = fileUrl
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
            saveFileUrl = fileUrl
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
        enabled: !editorScene.sceneModel.importEntityInProgress
        onTriggered: {
            importEntityDialog.open();
        }
    }

    Action {
        id: undoAction
        text: editorScene.undoHandler.undoText === ""
              ? qsTr ("Undo") + editorScene.emptyString
              : qsTr ("Undo '%1'").arg(editorScene.undoHandler.undoText) + editorScene.emptyString
        enabled: editorScene.undoHandler.canUndo
        shortcut: StandardKey.Undo
        onTriggered: editorScene.undoHandler.undo()
    }

    Action {
        id: redoAction
        text: editorScene.undoHandler.redoText === ""
              ? qsTr ("Redo") + editorScene.emptyString
              : qsTr ("Redo '%1'").arg(editorScene.undoHandler.redoText) + editorScene.emptyString
        enabled: editorScene.undoHandler.canRedo
        shortcut: StandardKey.Redo
        onTriggered: editorScene.undoHandler.redo()
    }

    toolBar: ToolBar {
        id: mainToolBar
        height: normalXButton.height
        style: ToolBarStyle {
            padding.top: 0
            padding.bottom: 0
            padding.right: 0
            padding.left: 0
            background: Rectangle {
                implicitHeight: normalXButton.height
                color: mainwindow.itemBackgroundColor
            }
        }

        RowLayout {
            spacing: 0
            EnableButton {
                id: normalXButton
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "images/helperplane_x_deselected.png"
                disabledIconSource: "images/helperplane_x_selected.png"
                hoveredBgColor: mainwindow.listHighlightColor
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Normal X (Ctrl + 1)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 0 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalXPlane()
            }
            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "images/helperplane_y_deselected.png"
                disabledIconSource: "images/helperplane_y_selected.png"
                hoveredBgColor: mainwindow.listHighlightColor
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Normal Y (Ctrl + 2)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 1 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalYPlane()
            }
            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "images/helperplane_z_deselected.png"
                disabledIconSource: "images/helperplane_z_selected.png"
                hoveredBgColor: mainwindow.listHighlightColor
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Normal Z (Ctrl + 3)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 2 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalZPlane()
            }
            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "images/helperplane_none_deselected.png"
                disabledIconSource: "images/helperplane_none_selected.png"
                hoveredBgColor: mainwindow.listHighlightColor
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Hide helper plane (Ctrl + 4)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 3 ? false : true
                onEnabledButtonClicked: mainwindow.hideHelperPlane()
            }
            Rectangle {
                // menu item separator
                height: 24
                width: 3
                color: mainwindow.listHighlightColor
                border.color: mainwindow.itemBackgroundColor
                border.width: 1
            }

            EnableButton {
                height: 32
                width: 32
                anchors.verticalCenter: parent.verticalCenter
                enabledIconSource: "images/reset_camera_to_default.png"
                disabledIconSource: "images/reset_camera_to_default.png"
                pressedIconSource: "images/reset_camera_to_default_pressed.png"
                hoveredBgColor: mainwindow.listHighlightColor
                selectedBgColor: mainwindow.iconHighlightColor
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
            if (multiSelection.length >= 1)
                entityTree.multiSelect = true
            else
                entityTree.multiSelect = false
            restoreSelection(selection)
        }

        onMultiSelectionChanged: {
            selectionList = multiSelection
            entityTree.multiSelectedCamera = false
            // Deselect old ones
            entityTree.view.selection.clear()
            // Dig indexes of all selected entities and pass the selections to entitytree
            // Note: We don't call setCurrentIndex, so no selection box will be shown during
            // multiselect
            for (var i = 0; i < multiSelection.length; ++i) {
                var index = editorScene.sceneModel.getModelIndexByName(multiSelection[i])
                entityTree.view.selection.select(index, ItemSelectionModel.Select)
                if (editorScene.sceneModel.editorSceneItemFromIndex(index).itemType()
                        === EditorSceneItem.Camera) {
                    entityTree.multiSelectedCamera = true
                }
            }
        }

        onErrorChanged: {
            notification.title = qsTr("Error") + editorScene.emptyString
            notification.text = error
            notification.open()
        }

        onMouseRightButtonReleasedWithoutDragging: {
            entityTree.multiSelect = multiSelect
            entityTree.menu.popup()
        }

        function restoreSelection(entity) {
            var index = editorScene.sceneModel.getModelIndex(entity)
            selectIndex(index)
         }

        function selectIndex(index) {
            expandTo(index)
            entityTree.view.forceActiveFocus()
            if (!entityTree.multiSelect)
                entityTree.view.selection.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect)
        }

        function expandTo(index) {
            var target = index
            do {
                target = target.parent
                entityTree.view.expand(target)
            } while (target.valid)
        }
    }

    MessageDialog {
        id: notification
        icon: StandardIcon.Warning
    }

    Dialog {
        id: gridSizeDialog
        property int previousSize: editorScene.gridSize
        title: qsTr("Grid Size") + editorScene.emptyString
        width: buttonRow.width

        contentItem: Rectangle {
            color: mainwindow.paneBackgroundColor
            StyledSpinBox {
                id: gridSizeSpinBox
                anchors.centerIn: parent
                implicitWidth: 140
                to: 20
                stepSize: 1
                from: 1
                contentItem: StyledTextInput {
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                }
            }

            RowLayout {
                id: buttonRow
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 4
                spacing: 4

                StyledButton {
                    text: qsTr("Apply")
                    onButtonClicked: {
                        editorScene.gridSize = gridSizeSpinBox.value
                    }
                }
                StyledButton {
                    text: qsTr("Cancel")
                    onButtonClicked: {
                        editorScene.gridSize = gridSizeDialog.previousSize
                        gridSizeDialog.close()
                    }
                }
                StyledButton {
                    text: qsTr("Ok")
                    onButtonClicked: {
                        editorScene.gridSize = gridSizeSpinBox.value
                        gridSizeDialog.close()
                    }
                }
            }
        }
    }

    MessageDialog {
        id: closingDialog
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Save | StandardButton.Cancel | StandardButton.Discard
        title: qsTr("Exit Qt 3D Scene Editor") + editorScene.emptyString
        text: qsTr("There are unsaved changes.\nQuit anyway?")
              + editorScene.emptyString

        onAccepted: {
            if (saveFileUrl == "") {
                saveFileDialog.exiting = true
                saveFileDialog.open()
                // No previous autosave file, no need to delete anything
                saveFileDialog.exiting = false
            } else {
                editorScene.saveScene(saveFileUrl)
                editorScene.deleteScene(saveFileUrl, true)
                Qt.quit()
            }
        }

        onDiscard: {
            Qt.quit()
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
            Layout.minimumWidth: 100
            Layout.maximumWidth: mainwindow.width - 10
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
                }
                DropArea {
                    anchors.fill: parent
                    keys: [ "insertEntity" ]
                    onContainsDragChanged: {
                        dragEntityItem.visible = !containsDrag
                    }
                }
                DragHandle {
                    id: dragTranslateHandle
                    handleType: EditorScene.DragTranslate
                    color: "red"
                }
                DragHandle {
                    id: dragRotateHandle
                    handleType: EditorScene.DragRotate
                    color: "blue"
                }
                DragHandle {
                    id: dragScaleHandle
                    handleType: EditorScene.DragScale
                    color: "green"
                }
            }
        }
    }

    SplitView {
        id: propertyPane
        orientation: Qt.Vertical
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: mainwindow.width / 4.5
        x: mainwindow.width - visibleWidth

        property int visibleWidth: width

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

    onWidthChanged: {
        var maximumWidth = mainwindow.width - entityLibrary.width - 10
        if (propertyPane.visibleWidth > maximumWidth) {
            propertyPane.visibleWidth = maximumWidth
            propertyPane.width = propertyPane.visibleWidth
        } else if (propertyPane.visibleWidth > resizeRectangle.paneMinimumWidth) {
            propertyPane.width = propertyPane.visibleWidth
        }
        propertyPane.x = mainwindow.width - propertyPane.visibleWidth
    }

    Rectangle {
        id: resizeRectangle
        width: 2
        height: parent.height
        anchors.right: propertyPane.left
        color: mainwindow.viewBorderColor

        property int paneMinimumWidth: 250
        property int paneMaximumWidth: mainwindow.width - entityLibrary.width - 10

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
                        if (newXPos < mainwindow.width) {
                            propertyPane.visibleWidth = mainwindow.width - newXPos
                            propertyPane.x = newXPos
                        }
                    }
                    else if (newPaneWidth > resizeRectangle.paneMinimumWidth
                             && newPaneWidth < resizeRectangle.paneMaximumWidth) {
                        propertyPane.x = mainwindow.width - newPaneWidth
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
            editorScene.saveScene(saveFileUrl, true)
        }
    }

    function resetCameraToDefault() {
        editorScene.freeView = true
        editorScene.resetFreeViewCamera()
    }

    function checkUnsavedChanges() {
        if (!editorScene.undoHandler.isClean()) {
            closingDialog.open()
            return false
        }
        return true
    }

    onClosing: {
        close.accepted = checkUnsavedChanges()
    }

    Component.onCompleted: {
        // Redraw everything to get rid of artifacts
        showMaximized()
        show()
        visible = true
    }
}
