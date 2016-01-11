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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.2

PropertyInputField {
    id: colorInput
    width: parent.width
    height: mainLayout.height

    property alias label: colorLabel.text
    property color colorValue: component[propertyName]
    property color oldColor

    onComponentValueChanged: {
        colorValue = component[propertyName]
        if (colorDialog.visible && colorDialog.currentColor !== colorValue) {
            // The colorValue has changed without user input on color dialog, likely meaning
            // user had invoked undo affecting this property. We need to reset the
            // oldColor and the dialog color to reflect the current state.
            colorDialog.color = colorValue
            oldColor = colorValue
        }
    }

    ColorDialog {
        id: colorDialog
        title: colorLabel.text

        onVisibleChanged: {
            if (visible) {
                color = colorValue
                oldColor = colorValue
            }
        }

        onCurrentColorChanged: {
            if (visible && currentColor != colorValue)
                doTemporaryPropertyChange(currentColor)
        }
        onAccepted: {
            handleEditingFinished(color, oldColor)
        }
        onRejected: {
            doTemporaryPropertyChange(oldColor)
        }
    }

    RowLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        Label {
            id: colorLabel
            text: qsTr("Color")
            color: labelTextColor
            Layout.alignment: Qt.AlignLeft
        }

        Button {
            id: colorButton
            Layout.alignment: Qt.AlignRight
            style: ButtonStyle {
                background: Rectangle {
                    implicitWidth: colorInput.width * 0.6
                    implicitHeight: colorLabel.height * 1.1
                    border.color: "#888"
                    radius: 4
                    color: colorValue
                }
            }
            onClicked: colorDialog.open()
        }
    }
}

