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
    id: vectorInput
    width: parent.width
    height: mainLayout.height

    property string label: qsTr("Vector3D")
    property alias xLabel: xLabel.text
    property alias yLabel: yLabel.text
    property alias zLabel: zLabel.text
    property bool typing: false
    property int roundDigits: 2 // TODO: Determine nice default rounding
    property int roundMultiplier: Math.pow(10, roundDigits) // Calculated from roundDigits, do not set directly
    property real inputCellWidth: vectorInput.width * 0.6

    property vector3d value: Qt.vector3d(0, 0, 0)

    signal valueEdited

    function roundNumber(number) {
        if (roundDigits >= 0)
            return Math.round(number * roundMultiplier) / roundMultiplier
        else
            return number
    }

    onValueChanged: {
        xInput.text = roundNumber(value.x)
        yInput.text = roundNumber(value.y)
        zInput.text = roundNumber(value.z)
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
        columns: 2

            Label {
                id: xLabel
                Layout.alignment: Qt.AlignLeft
                text: label + " " + qsTr("X")
                color: labelTextColor
            }

            TextField {
                id: xInput
                Layout.alignment: Qt.AlignRight
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                implicitWidth: inputCellWidth
                validator: doubleValidator

                onEditingFinished: {
                    if (text !== "") {
                        var oldValue = vectorInput.value.x
                        vectorInput.value.x = text
                        if (oldValue !== vectorInput.value.x)
                            valueEdited()
                    }
                }

                Component.onCompleted: text = roundNumber(vectorInput.value.x)
            }

            Label {
                id: yLabel
                Layout.alignment: Qt.AlignLeft
                text: label + " " + qsTr("Y")
                color: labelTextColor
            }

            TextField {
                id: yInput
                Layout.alignment: Qt.AlignRight
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                validator: doubleValidator
                implicitWidth: inputCellWidth

                onEditingFinished: {
                    if (text !== "") {
                        var oldValue = vectorInput.value.y
                        vectorInput.value.y = text
                        if (oldValue !== vectorInput.value.y)
                            valueEdited()
                    }
                }

                Component.onCompleted: text = roundNumber(vectorInput.value.y)
            }

            Label {
                id: zLabel
                Layout.alignment: Qt.AlignLeft
                text: label + " " + qsTr("Z")
                color: labelTextColor
            }

            TextField {
                id: zInput
                Layout.alignment: Qt.AlignRight
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                validator: doubleValidator
                implicitWidth: inputCellWidth

                onEditingFinished: {
                    if (text !== "") {
                        var oldValue = vectorInput.value.z
                        vectorInput.value.z = text
                        if (oldValue !== vectorInput.value.z)
                            valueEdited()
                    }
                }

                Component.onCompleted: text = roundNumber(vectorInput.value.z)
            }
    }
}
