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
import Qt.labs.controls 1.0 as QLC
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.2

PropertyInputField {
    id: fileInput
    property alias dialog: fileDialog
    width: parent.width
    height: mainLayout.height

    property alias label: fileLabel.text
    property url url: " "
    property url defaultUrl: ""

    onComponentValueChanged: {
        if (component !== null)
            url = component[propertyName]
        if (fileButton.text === "")
            fileDialog.open()
    }

    FileDialog {
        id: fileDialog
        title: fileLabel.text
        onAccepted: {
            handleEditingFinished(fileUrl)
        }
        onRejected: {
            if (fileButton.text === "")
                handleEditingFinished(defaultUrl)
        }
    }

    RowLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        QLC.Label {
            id: fileLabel
            text: qsTr("File") + editorScene.emptyString
            color: labelTextColor
            Layout.alignment: Qt.AlignLeft
        }

        Image {
            source: "qrc:/images/fader.png"
            anchors.right: fileButton.left
        }

        QLC.Button {
            id: fileButton
            Layout.alignment: Qt.AlignRight
            implicitWidth: fileInput.width * 0.6 + 20 // Lockbutton width
            implicitHeight: qlcControlHeight
            text: url
            onClicked: fileDialog.open()
        }
    }
}

