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
import QtQuick.Dialogs 1.2

PropertyInputField {
    id: colorInput
    width: parent.width
    height: mainLayout.height

    property alias label: colorLabel.text
    property color colorValue: component[propertyName]
    property color oldColor
    property alias tooltip: colorLabel.tooltip

    onComponentValueChanged: {
        if (component !== null)
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

        StyledLabel {
            id: colorLabel
            text: qsTr("Color") + editorScene.emptyString
            enabled: lockButton.buttonEnabled
            Layout.alignment: Qt.AlignLeft
        }

        Rectangle {
            color: editorContent.paneBackgroundColor
            implicitHeight: colorInput.height
            implicitWidth: colorButton.width + editorContent.controlMargin
            anchors.right: colorButton.right
        }

        QQC2.Button {
            id: colorButton
            Layout.alignment: Qt.AlignRight
            anchors.right: lockButton.left
            anchors.rightMargin: 4
            implicitWidth: colorInput.width * 0.6 > editorContent.maximumControlWidth
                           ? editorContent.maximumControlWidth
                           : colorInput.width * 0.6
            implicitHeight: editorContent.qlcControlHeight
            enabled: lockButton.buttonEnabled
            background: Rectangle {
                border.width: colorButton.activeFocus ? 2 : 1
                border.color: enabled ? editorContent.listHighlightColor
                                      : editorContent.itemBackgroundColor
                color: enabled ? colorValue : Qt.rgba(colorValue.r, colorValue.g, colorValue.b,
                                                      0.5)
            }
            onClicked: colorDialog.open()
        }

        PropertyLockButton {
            id: lockButton
            Layout.alignment: Qt.AlignVCenter
            Layout.maximumWidth: 16
            anchors.right: parent.right
            lockProperty: colorInput.propertyName + editorScene.lockPropertySuffix
            lockComponent: colorInput.component
            label: colorInput.label
        }
    }
}

