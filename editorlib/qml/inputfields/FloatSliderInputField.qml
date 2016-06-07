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
import QtQuick.Controls 2.0 as QQC2
import QtQuick.Layouts 1.2

Item {
    id: floatSliderInputField
    width: parent.width
    height: sliderLayout.height

    property alias lockProperty: lockButton.lockProperty
    property alias lockComponent: lockButton.lockComponent
    property alias locked: lockButton.locked
    property alias label: sliderLabel.text
    property alias stepSize: slider.stepSize
    property double minimum: 0.0
    property double maximum: 100.0
    property double value: 0.0
    property int roundDigits: 1 // TODO: Determine nice default rounding
    property int roundMultiplier: Math.pow(10, roundDigits) // Calculated from roundDigits, do not set directly
    property alias tooltip: sliderLabel.tooltip

    function roundNumber(number) {
        if (roundDigits >= 0)
            return Math.round(number * roundMultiplier) / roundMultiplier
        else
            return number
    }

    function tryCommitValue(desiredValue) {
        var newValue
        if (desiredValue !== "")
            newValue = roundNumber(desiredValue)
        else
            newValue = roundNumber(component[propertyName])
        newValue = Math.max(newValue, minimum)
        newValue = Math.min(newValue, maximum)
        value = newValue
        if (floatInput.text != newValue)
            floatInput.text = newValue
    }

    onValueChanged: {
        var roundedValue = roundNumber(value)
        floatInput.text = roundedValue
        slider.blockCommit = true
        slider.value = roundedValue
        slider.blockCommit = false
    }

    RowLayout {
        id: sliderLayout
        width: parent.width

        StyledLabel {
            id: sliderLabel
            text: qsTr("Float Slider") + editorScene.emptyString
            enabled: lockButton.buttonEnabled
            Layout.alignment: Qt.AlignLeft
        }

        DoubleValidator {
            id: doubleValidator
            locale: "C"
        }

        Rectangle {
            color: editorContent.paneBackgroundColor
            height: floatSliderInputField.height
            width: floatSliderInputField.width * 0.6 > editorContent.maximumControlWidth
                   ? editorContent.maximumControlWidth + editorContent.controlMargin
                   : floatSliderInputField.width * 0.62
            anchors.right: floatInput.right
        }

        QQC2.Slider {
            id: slider

            property bool blockCommit: false
            property bool pendingValue: false

            from: minimum
            to: maximum
            implicitWidth: floatSliderInputField.width * 0.6 > editorContent.maximumControlWidth
                           ? editorContent.maximumControlWidth * 0.65
                           : floatSliderInputField.width * 0.4 - 4
            enabled: lockButton.buttonEnabled
            anchors.right: floatInput.left
            anchors.rightMargin: 4
            handle: Rectangle {
                x: slider.leftPadding + (horizontal ? slider.visualPosition
                                                      * (slider.availableWidth - width)
                                                    : (slider.availableWidth - width) / 2)
                y: slider.topPadding + (horizontal ? (slider.availableHeight - height) / 2
                                                   : slider.visualPosition
                                                     * (slider.availableHeight - height))
                implicitWidth: 20
                implicitHeight: 20
                radius: width / 2
                border.color: enabled ? editorContent.listHighlightColor
                                      : editorContent.itemBackgroundColor
                color: enabled ? (slider.pressed ? editorContent.selectionColor
                                                 : editorContent.listHighlightColor)
                               : editorContent.itemBackgroundColor

                readonly property bool horizontal: slider.orientation === Qt.Horizontal
            }
            background: Rectangle {
                x: slider.leftPadding + (horizontal ? 0 : (slider.availableWidth - width) / 2)
                y: slider.topPadding + (horizontal ? (slider.availableHeight - height) / 2 : 0)
                implicitWidth: horizontal ? 200 : 6
                implicitHeight: horizontal ? 6 : 200
                width: horizontal ? slider.availableWidth : implicitWidth
                height: horizontal ? implicitHeight : slider.availableHeight
                radius: 3
                border.color: enabled ? editorContent.listHighlightColor
                                      : editorContent.itemBackgroundColor
                color: enabled ? editorContent.listHighlightColor : editorContent.itemBackgroundColor
                scale: horizontal && slider.mirrored ? -1 : 1

                readonly property bool horizontal: slider.orientation === Qt.Horizontal
            }

            onPositionChanged:  {
                var newValue = roundNumber((position * (to - from)) + from)
                floatInput.text = newValue
                if (!blockCommit) {
                    // Do not try to commit immediately, if we do not have focus.
                    // Instead, delay the commit until we get the focus. This should ensure
                    // any onEditingFinishes in other fields get executed before slider value
                    // creates its undo command into the stack, thus ensuring the undo stack
                    // is kept in correct order.
                    if (focus) {
                        pendingValue = false
                        tryCommitValue(newValue)
                    } else {
                        pendingValue = true
                    }
                }
            }

            onPressedChanged: {
                // Grab focus if user presses the slider with mouse.
                // Note that if this is changed, pendingValue logic will likely break.
                if (pressed)
                    forceActiveFocus(Qt.MouseFocusReason)
            }

            onFocusChanged: {
                if (focus && pendingValue) {
                    tryCommitValue(value)
                    pendingValue = false
                }
            }
        }

        StyledTextField {
            id: floatInput
            implicitWidth: floatSliderInputField.width * 0.6 > editorContent.maximumControlWidth
                           ? editorContent.maximumControlWidth * 0.34
                           : floatSliderInputField.width * 0.2
            anchors.right: lockButton.left
            anchors.rightMargin: 4
            validator: doubleValidator
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            enabled: lockButton.buttonEnabled
            selectByMouse: true

            onEditingFinished: {
                tryCommitValue(floatInput.text)
            }

            Component.onCompleted: {
                text = roundNumber(slider.value)
            }
        }

        PropertyLockButton {
            id: lockButton
            Layout.alignment: Qt.AlignVCenter
            anchors.right: parent.right
            Layout.maximumWidth: 16
            label: floatSliderInputField.label
        }
    }
}
