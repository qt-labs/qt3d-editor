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

Window {
    id: dialog
    title: qsTr("Settings") + editorScene.emptyString
    modality: Qt.WindowModal
    width: 400
    height: 320
    color: mainwindow.paneBackgroundColor
    minimumHeight: settingsLayout.Layout.minimumHeight + buttonRow.Layout.minimumHeight
    minimumWidth: buttonRow.Layout.minimumWidth
    property bool previousAutoSaveEnabled: autoSaveTimer.running

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
                              + 4 * spacing

        ColumnLayout {
            id: savingLayout

            StyledLabel  {
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

            StyledLabel  {
                text: qsTr("Language") + editorScene.emptyString
                font.weight: Font.Bold
            }
            RowLayout {
                StyledRadioButton {
                    id: englishButton
                    checked: (editorScene.language === "en")
                }
                StyledLabel {
                    id: englishLabel
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("English") + editorScene.emptyString
                }
                StyledRadioButton {
                    id: finnishButton
                    checked: (editorScene.language === "fi")
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
            spacing: 8
            Layout.bottomMargin: 8

            StyledLabel  {
                text: qsTr("Grid Spacing") + editorScene.emptyString
                font.weight: Font.Bold
            }
            StyledSpinBox {
                id: gridSizeSpinBox
                implicitWidth: 140
                to: 20
                stepSize: 1
                from: 1
                value: editorScene.gridSize
                Layout.leftMargin: 8
                contentItem: StyledTextInput {
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                }
            }
        }
    }

    Rectangle {
        id: separator
        width: parent.width
        height: 1
        color: mainwindow.listHighlightColor
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
                editorScene.gridSize = gridSizeSpinBox.value
                setLanguage()
            }

        }
        StyledButton {
            id: cancelButton
            text: qsTr("Cancel") + editorScene.emptyString
            onButtonClicked: {
                saveCheckBox.checked = previousAutoSaveEnabled
                if (gridSizeSpinBox.value !== editorScene.gridSize)
                    gridSizeSpinBox.value = editorScene.gridSize
                if (editorScene.language === "en") {
                    englishButton.checked = true
                    finnishButton.checked = false
                } else if (editorScene.language === "fi") {
                    englishButton.checked = false
                    finnishButton.checked = true
                }
                dialog.close()
            }
        }
        StyledButton {
            id: okButton
            text: qsTr("Ok") + editorScene.emptyString
            onButtonClicked: {
                setAutoSave()
                editorScene.gridSize = gridSizeSpinBox.value
                setLanguage()
                dialog.close()
            }
        }
    }

    function setAutoSave() {
        if (saveCheckBox.checked !== previousAutoSaveEnabled) {
            if (saveCheckBox.checked) {
                if (saveFileUrl == "")
                    saveFileDialog.open()
                autoSaveTimer.start()
            } else {
                autoSaveTimer.stop()
            }
        }
    }

    function setLanguage() {
        if (englishButton.checked)
            editorScene.language = "en"
        else if (finnishButton.checked)
            editorScene.language = "fi"
    }
}
