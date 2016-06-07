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
    title: qsTr("Settings") + editorScene.emptyString
    modality: Qt.WindowModal
    width: 400
    height: 320
    color: editorContent.paneBackgroundColor
    minimumHeight: settingsLayout.Layout.minimumHeight + buttonRow.Layout.minimumHeight
    minimumWidth: buttonRow.Layout.minimumWidth
    property bool previousAutoSaveEnabled
    property string previousFolder
    property int previousGridSize
    property string previousLanguage
    property bool autoSaveEnabled: false
    property string currentFolder
    property string currentLanguage
    property int currentGridSize
    property string folderPath

    Settings {
        // Use detailed category name, as plugin saves settings under QtCreator application
        category: "Qt 3D SceneEditor General"
        property alias language: dialog.currentLanguage
        property alias gridSize: dialog.currentGridSize
        property alias autoSave: dialog.autoSaveEnabled
    }

    onCurrentLanguageChanged: {
        editorScene.language = currentLanguage
    }

    onCurrentGridSizeChanged: {
        editorScene.gridSize = currentGridSize
    }

    onAutoSaveEnabledChanged: {
        saveCheckBox.checked = autoSaveEnabled
    }

    ColumnLayout {
        id: settingsLayout
        anchors.top: parent.top
        anchors.bottom: separator.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        spacing: 16
        Layout.minimumHeight: savingLayout.Layout.minimumHeight
                              + languageLayout.Layout.minimumHeight
                              + gridLayout.Layout.minimumHeight
                              + defaultFolderLayout.minimumHeight
                              + 6 * spacing

        ColumnLayout {
            id: savingLayout

            StyledLabel {
                text: qsTr("Automatic Saving") + editorScene.emptyString
                font.weight: Font.Bold
            }
            Row {
                StyledCheckBox {
                    id: saveCheckBox
                }
                StyledLabel {
                    text: qsTr("Save work every 10 minutes") + editorScene.emptyString
                    leftPadding: 0
                    anchors.verticalCenter: saveCheckBox.verticalCenter
                }
            }
        }

        ColumnLayout {
            id: languageLayout

            StyledLabel {
                text: qsTr("Language") + editorScene.emptyString
                font.weight: Font.Bold
            }
            RowLayout {
                StyledRadioButton {
                    id: englishButton
                    checked: (currentLanguage === "en")
                }
                StyledLabel {
                    id: englishLabel
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("English") + editorScene.emptyString
                }
                StyledRadioButton {
                    id: finnishButton
                    checked: (currentLanguage === "fi")
                }
                StyledLabel {
                    id: finnishLabel
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Finnish") + editorScene.emptyString
                }
            }
        }

        ColumnLayout {
            id: gridLayout
            width: parent.width

            StyledLabel {
                text: qsTr("Grid Spacing") + editorScene.emptyString
                font.weight: Font.Bold
            }
            StyledSpinBox {
                id: gridSizeSpinBox
                implicitWidth: 140
                to: 20
                stepSize: 1
                from: 1
                Layout.leftMargin: 8
                contentItem: StyledTextInput {
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                }
            }
        }

        ColumnLayout {
            id: defaultFolderLayout
            width: parent.width
            spacing: 8
            Layout.bottomMargin: 8

            StyledLabel {
                id: defaultFolderLabel
                text: qsTr("Default Folder") + editorScene.emptyString
                font.weight: Font.Bold
            }
            StyledButton {
                implicitWidth: parent.width
                implicitHeight: editorContent.qlcControlHeight
                text: folderPath
                onButtonClicked: fileDialog.open()
            }
        }
    }

    FileDialog {
        id: fileDialog
        folder: editorContent.defaultFolder
        title: defaultFolderLabel.text
        selectFolder: true
        onAccepted: {
            currentFolder = fileUrl // Use fileUrl, as dialog is in select folder mode
            parseFolderString(fileUrl)
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
        Layout.minimumWidth: applyButton.width + cancelButton.width + okButton.width + 4 * spacing

        StyledButton {
            id: applyButton
            text: qsTr("Apply") + editorScene.emptyString
            onButtonClicked: {
                setAutoSave()
                currentGridSize = gridSizeSpinBox.value
                setLanguage()
                setFolder()
            }
        }
        StyledButton {
            id: cancelButton
            text: qsTr("Cancel") + editorScene.emptyString
            onButtonClicked: {
                autoSaveEnabled = previousAutoSaveEnabled
                if (previousGridSize !== currentGridSize) {
                    gridSizeSpinBox.value = previousGridSize
                    currentGridSize = previousGridSize
                }
                if (previousLanguage !== currentLanguage) {
                    currentLanguage = previousLanguage
                    if (currentLanguage === "en") {
                        englishButton.checked = true
                        finnishButton.checked = false
                    } else if (currentLanguage === "fi") {
                        englishButton.checked = false
                        finnishButton.checked = true
                    }
                }
                if (previousFolder.length > 0
                        && currentFolder !== previousFolder) {
                    editorContent.defaultFolder = previousFolder
                    currentFolder = previousFolder
                    parseFolderString(currentFolder)
                }
                if (!autoSaveEnabled) {
                    autoSaveTimer.stop()
                }
                dialog.close()
            }
        }
        StyledButton {
            id: okButton
            text: qsTr("Ok") + editorScene.emptyString
            onButtonClicked: {
                setAutoSave()
                previousAutoSaveEnabled = autoSaveEnabled
                currentGridSize = gridSizeSpinBox.value
                previousGridSize = currentGridSize
                setLanguage()
                previousLanguage = currentLanguage
                setFolder()
                previousFolder = currentFolder
                dialog.close()
            }
        }
    }

    function setAutoSave() {
        autoSaveEnabled = saveCheckBox.checked
        if (autoSaveEnabled !== previousAutoSaveEnabled) {
            if (autoSaveEnabled) {
                if (editorContent.saveFileUrl == "")
                    saveFileDialog.open()
                autoSaveTimer.start()
            } else {
                autoSaveTimer.stop()
            }
        }
    }

    function setLanguage() {
        if (englishButton.checked)
            currentLanguage = "en"
        else if (finnishButton.checked)
            currentLanguage = "fi"
    }

    function setFolder() {
        if (previousFolder !== currentFolder) {
            editorContent.defaultFolder = currentFolder
            // When default folder is changed, reset all saved folders
            editorContent.importFolder = currentFolder
            editorContent.saveFolder = currentFolder
            editorContent.textureFolder = currentFolder
        }
    }

    function parseFolderString(url) {
        folderPath = url.toString()
        folderPath = folderPath.replace(/^(file:\/{2,})/,"");
    }

    Component.onCompleted: {
        currentLanguage = editorScene.language
        previousLanguage = currentLanguage
        currentGridSize = editorScene.gridSize
        previousGridSize = currentGridSize
        gridSizeSpinBox.value = currentGridSize
        currentFolder = editorContent.defaultFolder
        previousFolder = currentFolder
        parseFolderString(currentFolder)
        previousAutoSaveEnabled = autoSaveTimer.running
    }
}
