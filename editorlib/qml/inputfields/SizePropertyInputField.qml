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
import QtQuick.Layouts 1.2

PropertyInputField {
    id: sizeInput
    width: parent.width
    height: mainLayout.height

    property alias label: sizeLabel.text
    property alias widthLabel: widthLabel.text
    property alias heightLabel: heightLabel.text
    property int minimum: 0
    property size newValue: Qt.size(0, 0)
    property alias tooltip: sizeLabel.tooltip

    onComponentValueChanged: {
        if (component !== null)
            newValue = component[propertyName]
        widthInput.text = newValue.width
        heightInput.text = newValue.height
    }

    IntValidator {
        id: intValidator
        locale: "C"
    }

    RowLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        StyledLabel {
            id: sizeLabel
            text: qsTr("Size") + editorScene.emptyString
            enabled: lockButton.buttonEnabled
            Layout.alignment: Qt.AlignLeft
        }

        RowLayout {
            id: row
            anchors.right: parent.right
            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: sizeInput.width * 0.6 > editorContent.maximumControlWidth
                                   ? editorContent.maximumControlWidth
                                   : sizeInput.width * 0.6
            property real columnSpacing: widthLabel.contentWidth != 0 ? 6 : 4
            property real cellwidth: (Layout.preferredWidth
                                      - widthLabel.contentWidth
                                      - heightLabel.contentWidth) / 2 - columnSpacing


            Rectangle {
                color: editorContent.paneBackgroundColor
                height: sizeInput.height
                width: sizeInput.width * 0.6 > editorContent.maximumControlWidth
                       ? editorContent.maximumControlWidth + editorContent.controlMargin
                       : sizeInput.width * 0.62
                anchors.right: heightInput.right
            }

            StyledLabel {
                id: widthLabel
                text: qsTr("X") + editorScene.emptyString
                enabled: lockButton.buttonEnabled
                anchors.right: widthInput.left
                anchors.rightMargin: 4
            }

            StyledTextField {
                id: widthInput
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                implicitWidth: parent.cellwidth
                validator: intValidator
                enabled: lockButton.buttonEnabled
                anchors.right: heightLabel.left
                anchors.rightMargin: 4
                selectByMouse: true
                horizontalAlignment: TextInput.AlignHCenter

                onEditingFinished: {
                    newValue.height = component[propertyName].height
                    if (text !== "")
                        newValue.width = text
                    else
                        newValue.width = component[propertyName].width
                    newValue.width = Math.max(newValue.width, minimum)
                    handleEditingFinished(newValue)
                }
            }

            StyledLabel {
                id: heightLabel
                text: qsTr("Y") + editorScene.emptyString
                enabled: lockButton.buttonEnabled
                anchors.right: heightInput.left
                anchors.rightMargin: 4
            }

            StyledTextField {
                id: heightInput
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                anchors.right: lockButton.left
                anchors.rightMargin: 4
                validator: intValidator
                implicitWidth: parent.cellwidth
                enabled: lockButton.buttonEnabled
                selectByMouse: true
                horizontalAlignment: TextInput.AlignHCenter

                onEditingFinished: {
                    newValue.width = component[propertyName].width
                    if (text !== "")
                        newValue.height = text
                    else
                        newValue.height = component[propertyName].height
                    newValue.height = Math.max(newValue.height, minimum)
                    handleEditingFinished(newValue)
                }
            }

            PropertyLockButton {
                id: lockButton
                Layout.alignment: Qt.AlignVCenter
                Layout.maximumWidth: 16
                anchors.right: parent.right
                lockProperty: sizeInput.propertyName + editorScene.lockPropertySuffix
                lockComponent: sizeInput.component
                label: sizeInput.label
            }
        }
    }
}
