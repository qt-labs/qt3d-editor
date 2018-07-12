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
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as QQC2
import com.theqtcompany.SceneEditor3D 1.0

Item {
    property alias coordDisplayText: coordinateDisplayLabel.text
    property string coordDisplayTemplate: qsTr("x:%1 y:%2 z:%3") + editorScene.emptyString

    height: newButton.height
    ToolBar {
        id: mainToolBar
        anchors.fill: parent
        style: ToolBarStyle {
            padding.top: 0
            padding.bottom: 0
            padding.right: 0
            padding.left: 0
            background: Rectangle {
                implicitHeight: newButton.height
                color: editorContent.itemBackgroundColor
            }
        }

        RowLayout {
            spacing: 0

            ToolbarButton {
                id: newButton
                enabledIconSource: "images/new.png"
                tooltip: qsTr("New") + editorScene.emptyString
                onEnabledButtonClicked: editorContent.fileNew()
            }
            ToolbarButton {
                enabledIconSource: "images/load.png"
                tooltip: qsTr("Load (Ctrl + O)") + editorScene.emptyString
                onEnabledButtonClicked: editorContent.fileLoad()
            }
            ToolbarButton {
                enabledIconSource: "images/import.png"
                disabledIconSource: "images/import_disabled.png"
                tooltip: qsTr("Import Entity (Ctrl + I)") + editorScene.emptyString
                buttonEnabled: !editorScene.sceneModel.importEntityInProgress
                onEnabledButtonClicked: editorContent.importEntity()
            }
            ToolbarButton {
                enabledIconSource: "images/export_gltf.png"
                tooltip: qsTr("Export Scene as GLTF (Ctrl + E)") + editorScene.emptyString
                onEnabledButtonClicked: exportDialog.show()
                visible: editorScene.canExportGltf
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/save.png"
                tooltip: qsTr("Save") + editorScene.emptyString
                onEnabledButtonClicked: editorContent.fileSave()
            }
            ToolbarButton {
                enabledIconSource: "images/save_as.png"
                tooltip: qsTr("Save As") + editorScene.emptyString
                onEnabledButtonClicked: editorContent.fileSaveAs()
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/undo.png"
                disabledIconSource: "images/undo_disabled.png"
                tooltip: (editorScene.undoHandler.undoText === "" || !buttonEnabled)
                         ? qsTr ("Undo") + editorScene.emptyString
                         : qsTr ("Undo '%1'").arg(editorScene.undoHandler.undoText)
                           + editorScene.emptyString
                buttonEnabled: editorScene.undoHandler.canUndo
                onEnabledButtonClicked: editorContent.undo()
            }
            ToolbarButton {
                enabledIconSource: "images/redo.png"
                disabledIconSource: "images/redo_disabled.png"
                tooltip: (editorScene.undoHandler.redoText === "" || !buttonEnabled)
                         ? qsTr ("Redo") + editorScene.emptyString
                         : qsTr ("Redo '%1'").arg(editorScene.undoHandler.redoText)
                           + editorScene.emptyString
                buttonEnabled: editorScene.undoHandler.canRedo
                onEnabledButtonClicked: editorContent.redo()
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/helperplane_x_deselected.png"
                disabledIconSource: "images/helperplane_x_selected.png"
                selectedBgColor: editorContent.iconHighlightColor
                tooltip: qsTr("Normal X (Ctrl + 1)") + editorScene.emptyString
                buttonEnabled: editorContent.currentHelperPlane === 0 ? false : true
                onEnabledButtonClicked: editorContent.showNormalXPlane()
            }
            ToolbarButton {
                enabledIconSource: "images/helperplane_y_deselected.png"
                disabledIconSource: "images/helperplane_y_selected.png"
                selectedBgColor: editorContent.iconHighlightColor
                tooltip: qsTr("Normal Y (Ctrl + 2)") + editorScene.emptyString
                buttonEnabled: editorContent.currentHelperPlane === 1 ? false : true
                onEnabledButtonClicked: editorContent.showNormalYPlane()
            }
            ToolbarButton {
                enabledIconSource: "images/helperplane_z_deselected.png"
                disabledIconSource: "images/helperplane_z_selected.png"
                selectedBgColor: editorContent.iconHighlightColor
                tooltip: qsTr("Normal Z (Ctrl + 3)") + editorScene.emptyString
                buttonEnabled: editorContent.currentHelperPlane === 2 ? false : true
                onEnabledButtonClicked: editorContent.showNormalZPlane()
            }
            ToolbarButton {
                enabledIconSource: "images/helperplane_none_deselected.png"
                disabledIconSource: "images/helperplane_none_selected.png"
                selectedBgColor: editorContent.iconHighlightColor
                tooltip: qsTr("Hide helper plane (Ctrl + 4)") + editorScene.emptyString
                buttonEnabled: editorContent.currentHelperPlane === 3 ? false : true
                onEnabledButtonClicked: editorContent.hideHelperPlane()
            }

            ToolbarButton {
                enabledIconSource: "images/helperarrows_local.png"
                disabledIconSource: "images/helperarrows_global.png"
                selectedBgColor: editorContent.iconHighlightColor
                tooltip: editorScene.helperArrowsLocal
                         ? qsTr("Local transformation orientation (6)") + editorScene.emptyString
                         : qsTr("Global transformation orientation (6)") + editorScene.emptyString
                buttonEnabled: editorScene.helperArrowsLocal
                onEnabledButtonClicked: editorScene.helperArrowsLocal = !editorScene.helperArrowsLocal
                hoverAlways: true
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/snap_to.png"
                disabledIconSource: "images/snap_to_disabled.png"
                tooltip: qsTr("Snap to active camera position (Ctrl + 5)") + editorScene.emptyString
                buttonEnabled: freeViewCheckBox.checked
                onEnabledButtonClicked: editorScene.snapFreeViewCameraToActiveSceneCamera()
            }
            ToolbarButton {
                enabledIconSource: "images/change_camera_position.png"
                tooltip: qsTr("Change a camera position") + editorScene.emptyString
                onEnabledButtonClicked: editorContent.changeCameraPosition()
            }
            ToolbarButton {
                enabledIconSource: "images/reset_camera_to_default.png"
                disabledIconSource: "images/reset_camera_to_default.png"
                pressedIconSource: "images/reset_camera_to_default_pressed.png"
                tooltip: qsTr("Reset to default position (Ctrl + 0)") + editorScene.emptyString
                onEnabledButtonClicked: editorContent.resetCameraToDefault()
            }
            StyledCheckBox {
                id: freeViewCheckBox
                rightPadding: 2
                indicatorWidth: 10
                indicatorHeight: 10
                checked: true
                onCheckedChanged: {
                    editorScene.freeView = checked
                }
                Connections {
                    target: editorScene
                    onFreeViewChanged: freeViewCheckBox.checked = editorScene.freeView
                }
            }
            StyledLabel {
                text: qsTr("Free view") + editorScene.emptyString
                rightPadding: 8
                leftPadding: 0
            }

            Item {
                id: cameraComboboxItem
                width: 200
                height: cameraCombobox.height

                QQC2.ComboBox {
                    id: cameraCombobox
                    anchors.right: parent.right
                    anchors.rightMargin: 4
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.bottomMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    implicitHeight: editorContent.qlcControlHeight
                    currentIndex: editorScene.activeSceneCameraIndex

                    model: editorScene.sceneCamerasModel
                    textRole: "display"

                    onActivated: {
                        editorScene.undoHandler.createChangeGenericPropertyCommand(
                                    editorScene, "activeSceneCameraIndex",
                                    currentIndex, editorScene.activeSceneCameraIndex,
                                    qsTr("Change active camera"))
                    }
                }
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/settings.png"
                tooltip: qsTr("Settings (Shift+S)") + editorScene.emptyString
                onEnabledButtonClicked: settingsDialog.show()
            }

            ToolbarSeparator {}

            // Coordinate display label size is variable, so it should be the last item
            StyledLabel {
                id: coordinateDisplayLabel
                rightPadding: 8
                leftPadding: 8
            }
        }
    }

    Shortcut {
        sequence: StandardKey.New
        onActivated: editorContent.fileNew()
    }
    Shortcut {
        sequence: StandardKey.Open
        onActivated: editorContent.fileLoad()
    }
    Shortcut {
        sequence: "Ctrl+I"
        onActivated: editorContent.importEntity()
    }
    Shortcut {
        sequence: StandardKey.Save
        onActivated: editorContent.fileSave()
    }
    Shortcut {
        sequence: StandardKey.SaveAs
        onActivated: editorContent.fileSaveAs()
    }

    Shortcut {
        sequence: StandardKey.Undo
        onActivated: editorContent.undo()
    }
    Shortcut {
        sequence: StandardKey.Redo
        onActivated: editorContent.redo()
    }
    Shortcut {
        sequence: "Ctrl+0"
        onActivated: editorContent.resetCameraToDefault()
    }
    Shortcut {
        sequence: "Ctrl+1"
        onActivated: editorContent.showNormalXPlane()
    }
    Shortcut {
        sequence: "Ctrl+2"
        onActivated: editorContent.showNormalYPlane()
    }
    Shortcut {
        sequence: "Ctrl+3"
        onActivated: editorContent.showNormalZPlane()
    }
    Shortcut {
        sequence: "Ctrl+4"
        onActivated: editorContent.hideHelperPlane()
    }
    Shortcut {
        sequence: "Ctrl+5"
        onActivated: editorScene.snapFreeViewCameraToActiveSceneCamera()
    }
    Shortcut {
        sequence: "Ctrl+6"
        onActivated: editorScene.freeView = !editorScene.freeView
    }
    Shortcut {
        sequence: "6"
        onActivated: editorScene.helperArrowsLocal = !editorScene.helperArrowsLocal
    }
    Shortcut {
        sequence: "7"
        onActivated: editorScene.changeCameraPosition(EditorScene.CameraPositionTop)
    }
    Shortcut {
        sequence: "Ctrl+7"
        onActivated: editorScene.changeCameraPosition(EditorScene.CameraPositionBottom)
    }
    Shortcut {
        sequence: "8"
        onActivated: editorScene.changeCameraPosition(EditorScene.CameraPositionLeft)
    }
    Shortcut {
        sequence: "Ctrl+8"
        onActivated: editorScene.changeCameraPosition(EditorScene.CameraPositionRight)
    }
    Shortcut {
        sequence: "9"
        onActivated: editorScene.changeCameraPosition(EditorScene.CameraPositionFront)
    }
    Shortcut {
        sequence: "Ctrl+9"
        onActivated: editorScene.changeCameraPosition(EditorScene.CameraPositionBack)
    }
    Shortcut {
        sequence: "Shift+S"
        onActivated: settingsDialog.show()
    }
    Shortcut {
        sequence: "Ctrl+E"
        onActivated: {
            if (editorScene.canExportGltf)
                exportDialog.show();
        }
    }
}
