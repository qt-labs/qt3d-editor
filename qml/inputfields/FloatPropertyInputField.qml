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

PropertyInputField {
    id: floatInput
    width: parent.width
    height: mainLayout.height

    property alias label: floatLabel.text
    property real minimum: -99999999999 // TODO: Do we need more sensible default minimum?
    property int roundDigits: 4 // TODO: Determine nice default rounding
    property int roundMultiplier: Math.pow(10, roundDigits) // Calculated from roundDigits, do not set directly

    Component.onCompleted: {
        if (selectedEntity) {
            var propertyLocked = selectedEntity.customProperty(label)
            if (propertyLocked !== 0)
                lockButton.buttonEnabled = propertyLocked
        }
    }

    onComponentValueChanged: {
        if (component !== null)
            valueInput.text = roundNumber(component[propertyName])
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

        Label {
            id: floatLabel
            text: qsTr("Float Value")
            color: labelTextColor
            Layout.alignment: Qt.AlignLeft
        }

        TextField {
            id: valueInput
            Layout.alignment: Qt.AlignRight
            validator: doubleValidator
            implicitWidth: floatInput.width * 0.6
            text: ""
            enabled: lockButton.buttonEnabled

            onEditingFinished: {
                var newValue
                if (text !== "")
                    newValue = roundNumber(text)
                else
                    newValue = roundNumber(component[propertyName])
                newValue = Math.max(newValue, minimum)
                handleEditingFinished(newValue, roundNumber(component[propertyName]))
                if (text != newValue)
                    text = newValue
            }
        }

        EnableButton {
            id: lockButton
            Layout.alignment: Qt.AlignVCenter
            Layout.maximumWidth: 16
            enabledIconSource: "/images/lock_open.png"
            disabledIconSource: "/images/lock_locked.png"
            tooltip: qsTr("Lock '%1' Properties").arg(label)
            buttonEnabled: true
            onEnabledButtonClicked: {
                buttonEnabled = !buttonEnabled
                selectedEntity.setCustomProperty(label, buttonEnabled)
            }
        }
    }
}

