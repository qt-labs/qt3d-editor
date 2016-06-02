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

Item {
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
                color: mainwindow.itemBackgroundColor
            }
        }

        RowLayout {
            spacing: 0

            ToolbarButton {
                id: newButton
                enabledIconSource: "images/new.png"
                tooltip: qsTr("New") + editorScene.emptyString
                onEnabledButtonClicked: mainwindow.fileNew()
            }
            ToolbarButton {
                enabledIconSource: "images/load.png"
                tooltip: qsTr("Load (Ctrl + O)") + editorScene.emptyString
                onEnabledButtonClicked: mainwindow.fileLoad()
            }
            ToolbarButton {
                enabledIconSource: "images/import.png"
                disabledIconSource: "images/import_disabled.png"
                tooltip: qsTr("Import Entity") + editorScene.emptyString
                buttonEnabled: !editorScene.sceneModel.importEntityInProgress
                onEnabledButtonClicked: importEntityDialog.open()
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/save.png"
                tooltip: qsTr("Save") + editorScene.emptyString
                onEnabledButtonClicked: mainwindow.fileSave()
            }
            ToolbarButton {
                enabledIconSource: "images/save_as.png"
                tooltip: qsTr("Save As") + editorScene.emptyString
                onEnabledButtonClicked: mainwindow.fileSaveAs()
            }
            StyledCheckBox {
                rightPadding: 2
                indicatorWidth: 10
                indicatorHeight: 10
                onCheckedChanged: {
                    if (checked) {
                        if (saveFileUrl == "")
                            saveFileDialog.open()
                        autoSaveTimer.start()
                    } else {
                        autoSaveTimer.stop()
                    }
                }
            }
            StyledLabel {
                text: qsTr("Auto save") + editorScene.emptyString
                rightPadding: 8
                leftPadding: 0
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
                onEnabledButtonClicked: mainwindow.undo()
            }
            ToolbarButton {
                enabledIconSource: "images/redo.png"
                disabledIconSource: "images/redo_disabled.png"
                tooltip: (editorScene.undoHandler.undoText === "" || !buttonEnabled)
                         ? qsTr ("Redo") + editorScene.emptyString
                         : qsTr ("Redo '%1'").arg(editorScene.undoHandler.redoText)
                           + editorScene.emptyString
                buttonEnabled: editorScene.undoHandler.canRedo
                onEnabledButtonClicked: mainwindow.redo()
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/helperplane_x_deselected.png"
                disabledIconSource: "images/helperplane_x_selected.png"
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Normal X (Ctrl + 1)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 0 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalXPlane()
            }
            ToolbarButton {
                enabledIconSource: "images/helperplane_y_deselected.png"
                disabledIconSource: "images/helperplane_y_selected.png"
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Normal Y (Ctrl + 2)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 1 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalYPlane()
            }
            ToolbarButton {
                enabledIconSource: "images/helperplane_z_deselected.png"
                disabledIconSource: "images/helperplane_z_selected.png"
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Normal Z (Ctrl + 3)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 2 ? false : true
                onEnabledButtonClicked: mainwindow.showNormalZPlane()
            }
            ToolbarButton {
                enabledIconSource: "images/helperplane_none_deselected.png"
                disabledIconSource: "images/helperplane_none_selected.png"
                selectedBgColor: mainwindow.iconHighlightColor
                tooltip: qsTr("Hide helper plane (Ctrl + 4)") + editorScene.emptyString
                buttonEnabled: currentHelperPlane === 3 ? false : true
                onEnabledButtonClicked: mainwindow.hideHelperPlane()
            }

            ToolbarSeparator {}

            ToolbarButton {
                enabledIconSource: "images/reset_camera_to_default.png"
                disabledIconSource: "images/reset_camera_to_default.png"
                pressedIconSource: "images/reset_camera_to_default_pressed.png"
                tooltip: qsTr("Reset to Default (Ctrl + R)") + editorScene.emptyString
                onEnabledButtonClicked: mainwindow.resetCameraToDefault()
            }

            ToolbarSeparator {}
        }
    }

    Shortcut {
        id: fileNewShortcut
        sequence: StandardKey.New
        onActivated: mainwindow.fileNew()
    }
    Shortcut {
        id: fileLoadShortcut
        sequence: StandardKey.Open
        onActivated: mainwindow.fileLoad()
    }
    Shortcut {
        id: fileSaveShortcut
        sequence: StandardKey.Save
        onActivated: mainwindow.fileSave()
    }
    Shortcut {
        id: fileSaveAsShortcut
        sequence: StandardKey.SaveAs
        onActivated: mainwindow.fileSaveAs()
    }

    Shortcut {
        id: undoShortcut
        sequence: StandardKey.Undo
        onActivated: mainwindow.undo()
    }
    Shortcut {
        id: redoShortcut
        sequence: StandardKey.Redo
        onActivated: mainwindow.redo()
    }

    Shortcut {
        id: normalXShortcut
        sequence: "Ctrl+1"
        onActivated: mainwindow.showNormalXPlane()
    }
    Shortcut {
        id: normalYShortcut
        sequence: "Ctrl+2"
        onActivated: mainwindow.showNormalYPlane()
    }
    Shortcut {
        id: normalZShortcut
        sequence: "Ctrl+3"
        onActivated: mainwindow.showNormalZPlane()
    }
    Shortcut {
        id: hideHelperPlaneShortcut
        sequence: "Ctrl+4"
        onActivated: mainwindow.hideHelperPlane()
    }
    Shortcut {
        id: resetCameraShortcut
        sequence: "Ctrl+R"
        onActivated: mainwindow.resetCameraToDefault()
    }
}
