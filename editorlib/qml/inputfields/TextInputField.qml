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
    id: textInput
    width: parent.width
    height: mainLayout.height

    property alias label: textLabel.text
    property alias tooltip: textLabel.tooltip

    // text is what is shown on the field
    property string displayText: ""

    // This is the text we want to change to, but it is subject to approval by the using code
    property string desiredText: ""

    property alias validator: textInputCtrl.validator

    onDisplayTextChanged: {
        textInputCtrl.text = displayText
    }

    RowLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        StyledLabel {
            id: textLabel
            text: qsTr("Text") + editorScene.emptyString
            Layout.alignment: Qt.AlignLeft
        }

        Rectangle {
            color: editorContent.paneBackgroundColor
            height: textLabel.height
            implicitWidth: textInputCtrl.width + editorContent.controlMargin
            anchors.right: textInputCtrl.right
        }

        StyledTextField {
            id: textInputCtrl
            Layout.alignment: Qt.AlignRight
            anchors.right: parent.right
            implicitWidth: textInput.width * 0.6 + 20 // Lockbutton width
            text: textInput.displayText
            selectByMouse: true

            onEditingFinished: {
                textInput.desiredText = text
                text = textInput.displayText
                textInput.desiredText = ""
            }
        }
    }
}

