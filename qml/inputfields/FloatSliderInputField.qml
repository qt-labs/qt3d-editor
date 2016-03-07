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
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2

Item {
    id: floatSliderInputField
    width: parent.width
    height: sliderLayout.height

    property alias lockProperty: lockButton.lockProperty
    property alias lockComponent: lockButton.lockComponent
    property alias locked: lockButton.locked
    property alias label: title.text
    property alias stepSize: slider.stepSize
    property double minimum: 0.0
    property double maximum: 100.0
    property double value: 0.0
    property int roundDigits: 4 // TODO: Determine nice default rounding
    property int roundMultiplier: Math.pow(10, roundDigits) // Calculated from roundDigits, do not set directly

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

        Label {
            id: title
            text: qsTr("Float Slider") + editorScene.emptyString
            Layout.alignment: Qt.AlignLeft
            color: labelTextColor
        }

        DoubleValidator {
            id: doubleValidator
            locale: "C"
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            Slider {
                id: slider

                property bool blockCommit: false
                property bool pendingValue: false

                minimumValue: minimum
                maximumValue: maximum
                implicitWidth: floatSliderInputField.width * 0.4 - 4 // 4 = column spacing
                enabled: lockButton.buttonEnabled

                onValueChanged: {
                    if (!blockCommit) {
                        // Do not try to commit immediately, if we do not have focus.
                        // Instead, delay the commit until we get the focus. This should ensure
                        // any onEditingFinishes in other fields get executed before slider value
                        // creates its undo command into the stack, thus ensuring the undo stack
                        // is kept in correct order.
                        if (focus) {
                            pendingValue = false
                            tryCommitValue(value)
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

            TextField {
                id: floatInput
                implicitWidth: floatSliderInputField.width * 0.2 - 4 // 4 = column spacing
                validator: doubleValidator
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                enabled: lockButton.buttonEnabled

                onEditingFinished: {
                    tryCommitValue(floatInput.text)
                }
            }

            PropertyLockButton {
                id: lockButton
                Layout.alignment: Qt.AlignVCenter
                Layout.maximumWidth: 16
                label: floatSliderInputField.label
            }
        }
    }
}
