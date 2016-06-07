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

Item {
    id: vectorInput
    width: parent.width
    height: mainLayout.height

    property alias lockProperty: lockButton.lockProperty
    property alias lockComponent: lockButton.lockComponent
    property alias locked: lockButton.locked
    property string label: qsTr("Vector3D") + editorScene.emptyString
    property alias xLabel: xLabel.text
    property alias yLabel: yLabel.text
    property alias zLabel: zLabel.text
    property bool blockChange: false
    property int roundDigits: 2 // TODO: Determine nice default rounding
    property int roundMultiplier: Math.pow(10, roundDigits) // Calculated from roundDigits, do not set directly
    property real minimum: -999999999 / roundMultiplier // TODO: Do we need more sensible default minimum?
    property real maximum: 999999999 / roundMultiplier // TODO: Do we need more sensible default maximum?
    property int step: roundMultiplier
    property real inputCellWidth: vectorInput.width * 0.6 > editorContent.maximumControlWidth
                                  ? editorContent.maximumControlWidth
                                  : vectorInput.width * 0.6
    property string tooltip: ""
    property var tooltipArgs: ["", "", ""]
    property vector3d value: Qt.vector3d(0, 0, 0)

    signal valueEdited

    function roundNumber(number) {
        if (roundDigits >= 0)
            return Math.round(number * roundMultiplier) / roundMultiplier
        else
            return number
    }

    onValueChanged: {
        blockChange = true
        xInput.value = roundNumber(value.x) * roundMultiplier
        yInput.value = roundNumber(value.y) * roundMultiplier
        zInput.value = roundNumber(value.z) * roundMultiplier
        blockChange = false
    }

    DoubleValidator {
        id: doubleValidator
        locale: "C"
    }

    GridLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        Layout.rightMargin: 1
        columns: 4
        rowSpacing: 1

        StyledLabel {
            id: xLabel
            Layout.alignment: Qt.AlignLeft
            text: label + " " + qsTr("X") + editorScene.emptyString
            enabled: lockButton.buttonEnabled
            tooltip: vectorInput.tooltip.arg(tooltipArgs[0])
        }

        Rectangle {
            color: editorContent.paneBackgroundColor
            implicitHeight: xInput.height
            implicitWidth: inputCellWidth + editorContent.controlMargin
            anchors.right: xInput.right
        }

        StyledSpinBox {
            id: xInput
            anchors.right: lockButton.left
            anchors.rightMargin: 4
            Layout.alignment: Qt.AlignRight
            implicitWidth: inputCellWidth
            implicitHeight: editorContent.qlcControlHeight
            to: maximum * roundMultiplier
            stepSize: step
            from: minimum * roundMultiplier
            editable: true
            enabled: lockButton.buttonEnabled

            contentItem: StyledTextInput {
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            validator: doubleValidator

            textFromValue: function(value) {
                return value / roundMultiplier
            }

            valueFromText: function(text) {
                return roundNumber(text) * roundMultiplier
            }

            onValueChanged: {
                if (!blockChange) {
                    var oldValue = vectorInput.value.x
                    vectorInput.value.x = value / roundMultiplier
                    if (oldValue !== vectorInput.value.x)
                        valueEdited()
                }
            }

            Component.onCompleted: {
                value = roundNumber(vectorInput.value.x) * roundMultiplier
            }
        }

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 16
            source: (lockButton.locked || !lockButton.enabled)
                    ? "images/property_grouping_line_locked.png"
                    : "images/property_grouping_line_open.png"
            anchors.right: parent.right
            anchors.rightMargin: 3
        }

        StyledLabel {
            id: yLabel
            Layout.alignment: Qt.AlignLeft
            text: label + " " + qsTr("Y") + editorScene.emptyString
            enabled: lockButton.buttonEnabled
            tooltip: vectorInput.tooltip.arg(tooltipArgs[1])
        }

        Rectangle {
            color: editorContent.paneBackgroundColor
            implicitHeight: yInput.height
            implicitWidth: inputCellWidth + editorContent.controlMargin
            anchors.right: yInput.right
        }

        StyledSpinBox {
            id: yInput
            anchors.right: lockButton.left
            anchors.rightMargin: 4
            Layout.alignment: Qt.AlignRight
            implicitWidth: inputCellWidth
            implicitHeight: editorContent.qlcControlHeight
            to: maximum * roundMultiplier
            stepSize: step
            from: minimum * roundMultiplier
            editable: true
            enabled: lockButton.buttonEnabled

            contentItem: StyledTextInput {
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            validator: doubleValidator

            textFromValue: function(value) {
                return value / roundMultiplier
            }

            valueFromText: function(text) {
                return roundNumber(text) * roundMultiplier
            }

            onValueChanged: {
                if (!blockChange) {
                    var oldValue = vectorInput.value.y
                    vectorInput.value.y = value / roundMultiplier
                    if (oldValue !== vectorInput.value.y)
                        valueEdited()
                }
            }

            Component.onCompleted: {
                value = roundNumber(vectorInput.value.y) * roundMultiplier
            }
        }

        PropertyLockButton {
            id: lockButton
            Layout.alignment: Qt.AlignVCenter
            Layout.maximumWidth: 16
            anchors.right: parent.right
            label: vectorInput.label
        }

        StyledLabel {
            id: zLabel
            Layout.alignment: Qt.AlignLeft
            text: label + " " + qsTr("Z") + editorScene.emptyString
            enabled: lockButton.buttonEnabled
            tooltip: vectorInput.tooltip.arg(tooltipArgs[2])
        }

        Rectangle {
            color: editorContent.paneBackgroundColor
            implicitHeight: zInput.height
            implicitWidth: inputCellWidth + editorContent.controlMargin
            anchors.right: zInput.right
        }

        StyledSpinBox {
            id: zInput
            anchors.right: lockButton.left
            anchors.rightMargin: 4
            Layout.alignment: Qt.AlignRight
            implicitWidth: inputCellWidth
            implicitHeight: editorContent.qlcControlHeight
            to: maximum * roundMultiplier
            stepSize: step
            from: minimum * roundMultiplier
            editable: true
            enabled: lockButton.buttonEnabled

            contentItem: StyledTextInput {
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            validator: doubleValidator

            textFromValue: function(value) {
                return value / roundMultiplier
            }

            valueFromText: function(text) {
                return roundNumber(text) * roundMultiplier
            }

            onValueChanged: {
                if (!blockChange) {
                    var oldValue = vectorInput.value.z
                    vectorInput.value.z = value / roundMultiplier
                    if (oldValue !== vectorInput.value.z)
                        valueEdited()
                }
            }

            Component.onCompleted: {
                value = roundNumber(vectorInput.value.z) * roundMultiplier
            }
        }

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 16
            anchors.right: parent.right
            anchors.rightMargin: 3
            source: (lockButton.locked || !lockButton.enabled)
                    ? "images/property_grouping_line_locked.png"
                    : "images/property_grouping_line_open.png"
        }
    }
}
