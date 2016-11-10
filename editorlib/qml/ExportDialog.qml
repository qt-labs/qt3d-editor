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
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

Window {
    id: dialog
    title: qsTr("Export GLTF scene") + editorScene.emptyString
    modality: Qt.WindowModal
    width: 400
    height: 300
    color: editorContent.paneBackgroundColor
    minimumHeight: exportLayout.Layout.minimumHeight + buttonRow.Layout.minimumHeight
    minimumWidth: buttonRow.Layout.minimumWidth
    property bool previousExportSceneRoot
    property bool previousExportBinaryJson
    property bool previousExportCompactJson
    property string previousExportSceneName
    property url previousExportFolder

    property bool exportSceneRoot
    property bool exportBinaryJson: true
    property bool exportCompactJson: false
    property string exportSceneName: ""
    property url exportFolder: "file:///"
    property url currentFolder: exportFolder
    property string folderPath: parseFolderString(currentFolder)

    property bool accepted: false

    onVisibleChanged: {
        if (visible == true) {
            accepted = false;
            previousExportSceneRoot = exportSceneRoot;
            previousExportBinaryJson = exportBinaryJson;
            previousExportCompactJson = exportCompactJson;
            previousExportSceneName = exportSceneName;
            previousExportFolder = exportFolder;
            selectedEntityButton.checked = true;
            sceneRootButton.checked = exportSceneRoot;
            humanReadableJsonButton.checked = true;
            binaryJsonButton.checked = exportBinaryJson;
            compactJsonButton.checked = exportCompactJson;
            sceneNameField.text = exportSceneName;
            currentFolder = exportFolder;
        } else {
            if (accepted) {
                exportSceneRoot = sceneRootButton.checked;
                exportBinaryJson = binaryJsonButton.checked;
                exportCompactJson = compactJsonButton.checked;
                exportSceneName = sceneNameField.text;
                exportFolder = currentFolder;
                notification.title = qsTr("GLTF Export Success");
                notification.text = qsTr("Scene exported successfully.");
                notification.open();
            } else {
                exportSceneRoot = previousExportSceneRoot;
                exportBinaryJson = previousExportBinaryJson;
                exportCompactJson = previousExportCompactJson;
                exportSceneName = previousExportSceneName;
                exportFolder = previousExportFolder;
            }
        }
    }

    Settings {
        id: settings
        // Use detailed category name, as plugin saves settings under QtCreator application
        category: "Qt 3D SceneEditor GLTF Export"
        property alias binaryJson: dialog.exportBinaryJson
        property alias compactJson: dialog.exportCompactJson
        property alias sceneName: dialog.exportSceneName
        property alias folder: dialog.exportFolder
    }

    ColumnLayout {
        id: exportLayout
        anchors.top: parent.top
        anchors.bottom: separator.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        spacing: 8
        Layout.minimumHeight: gltfOptionsLayout.Layout.minimumHeight
                              + sceneFilesLayout.Layout.minimumHeight
                              + 6 * spacing

        ColumnLayout {
            id: gltfOptionsLayout
            StyledLabel {
                text: qsTr("Exported entity") + editorScene.emptyString
                font.weight: Font.Bold
            }
            RowLayout {
                StyledRadioButton {
                    id: sceneRootButton
                }
                StyledLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Scene Root") + editorScene.emptyString
                }
                StyledRadioButton {
                    id: selectedEntityButton
                }
                StyledLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Selected Entity: %1").arg(selectedEntityName) + editorScene.emptyString
                }
            }
            StyledLabel {
                text: qsTr("JSON format") + editorScene.emptyString
                font.weight: Font.Bold
            }
            RowLayout {
                StyledRadioButton {
                    id: humanReadableJsonButton
                }
                StyledLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Human Readable") + editorScene.emptyString
                }
                StyledRadioButton {
                    id: compactJsonButton
                }
                StyledLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Compact") + editorScene.emptyString
                }
                StyledRadioButton {
                    id: binaryJsonButton
                }
                StyledLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Binary") + editorScene.emptyString
                }
            }
        }

        ColumnLayout {
            id: sceneFilesLayout
            width: parent.width
            spacing: 8
            Layout.bottomMargin: 8

            StyledLabel {
                text: qsTr("Export Folder") + editorScene.emptyString
                font.weight: Font.Bold
            }
            StyledButton {
                implicitWidth: parent.width
                implicitHeight: editorContent.qlcControlHeight
                text: folderPath
                margins: 0
                onButtonClicked: fileDialog.open()
            }
            StyledLabel {
                text: qsTr("Exported Scene Name") + editorScene.emptyString
                font.weight: Font.Bold
            }
            StyledTextField {
                id: sceneNameField
                anchors.margins: 4
                inputMethodHints: Qt.ImhUrlCharactersOnly
                implicitWidth: parent.width
                selectByMouse: true
            }

        }
    }

    FileDialog {
        id: fileDialog
        selectMultiple: false
        selectExisting: false
        selectFolder: true
        folder: dialog.currentFolder
        title: qsTr("Export GLTF Scene to a folder") + editorScene.emptyString
        onAccepted: {
            dialog.currentFolder = fileUrl;
        }
    }

    Rectangle {
        id: separator
        width: parent.width
        height: 1
        color: editorContent.listHighlightColor
        anchors.bottom: buttonRow.top
        anchors.bottomMargin: 8
        anchors.topMargin: 8
    }

    RowLayout {
        id: buttonRow
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 4
        anchors.rightMargin: 4
        spacing: 4
        Layout.minimumHeight: applyButton.height + 2 * spacing
        Layout.minimumWidth: applyButton.width + cancelButton.width + 4 * spacing

        StyledButton {
            id: applyButton
            text: qsTr("Export Scene") + editorScene.emptyString
            onButtonClicked: {
                var options = {binaryJson: binaryJsonButton.checked,
                    compactJson: compactJsonButton.checked};
                if (editorScene.exportGltfScene(currentFolder, sceneNameField.text,
                                                selectedEntityButton.checked, options) === true) {
                    dialog.accepted = true;
                    dialog.close();
                }
            }
        }
        StyledButton {
            id: cancelButton
            text: qsTr("Cancel") + editorScene.emptyString
            onButtonClicked: {
                dialog.accepted = false;
                dialog.close();
            }
        }
    }

    function parseFolderString(url) {
        return url.toString().replace(/^(file:\/{2,})/,"");
    }
}
