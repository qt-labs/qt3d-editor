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
    id: floatInput
    width: parent.width
    height: mainLayout.height

    property alias label: floatLabel.text
    property int roundDigits: 4 // TODO: Determine nice default rounding
    property int roundMultiplier: Math.pow(10, roundDigits) // Calculated from roundDigits, do not set directly
    property real minimum: -999999999 / roundMultiplier // TODO: Do we need more sensible default minimum?
    property real maximum: 999999999 / roundMultiplier // TODO: Do we need more sensible default maximum?
    property int step: roundMultiplier
    property double fieldValue: component[propertyName]
    property bool blockChange: false
    property alias tooltip: floatLabel.tooltip

    onComponentValueChanged: {
        blockChange = true
        if (component !== null)
            valueInput.value = roundNumber(component[propertyName]) * roundMultiplier
        blockChange = false
    }

    function roundNumber(number) {
        if (roundDigits >= 0)
            return Math.round(number * roundMultiplier) / roundMultiplier
        else
            return number
    }

    DoubleValidator {
        id: doubleValidator
        locale: "C"
    }

    RowLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        StyledLabel {
            id: floatLabel
            text: qsTr("Float Value") + editorScene.emptyString
            enabled: lockButton.buttonEnabled
            Layout.alignment: Qt.AlignLeft
        }

        Rectangle {
            color: editorContent.paneBackgroundColor
            height: floatInput.height
            width: floatInput.width * 0.6 > editorContent.maximumControlWidth
                   ? editorContent.maximumControlWidth + editorContent.controlMargin
                   : floatInput.width * 0.62
            anchors.right: valueInput.right
        }

        StyledSpinBox {
            id: valueInput
            anchors.right: lockButton.left
            anchors.rightMargin: 4
            Layout.alignment: Qt.AlignRight
            implicitWidth: floatInput.width * 0.6 > editorContent.maximumControlWidth
                           ? editorContent.maximumControlWidth
                           : floatInput.width * 0.6
            implicitHeight: editorContent.qlcControlHeight
            to: maximum * roundMultiplier
            stepSize: step
            from: minimum * roundMultiplier
            editable: true
            enabled: lockButton.buttonEnabled

            contentItem: StyledTextInput {
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            validator: DoubleValidator {
                locale: "C"
            }

            textFromValue: function(value) {
                return value / roundMultiplier
            }

            valueFromText: function(text) {
                return roundNumber(text) * roundMultiplier
            }

            onValueChanged: {
                if (!blockChange) {
                    var newValue = value
                    newValue = Math.max(value, minimum) / roundMultiplier
                    handleEditingFinished(newValue)
                }
            }
        }

        PropertyLockButton {
            id: lockButton
            Layout.alignment: Qt.AlignVCenter
            anchors.right: parent.right
            Layout.maximumWidth: 16
            lockProperty: floatInput.propertyName + editorScene.lockPropertySuffix
            lockComponent: floatInput.component
            label: floatInput.label
        }
    }
}

