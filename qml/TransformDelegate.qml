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
import com.theqtcompany.Editor3d 1.0

Item {
    width: parent.width
    height: columnLayout.y + columnLayout.height + 8

    property string title //TODO: needs to be removed
    default property alias _contentChildren: columnLayout.data
    property bool editable: true
    property int componentType: EditorSceneItemComponentsModel.Unknown
/*
    Rectangle {
        id: titleHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: transformTitle.height + 8
        color: "#d7d6d5"

        Label {
            id: transformTitle
            font.pixelSize: 14
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
        }

        // We probably don't want to support removing transforms from entities
//        ToolButton {
//            id: removeTransformButton
//            anchors.right: parent.right
//            anchors.rightMargin: 8
//            anchors.verticalCenter: parent.verticalCenter
//            iconSource: "images/cross.png"
//            height: titleHeader.height
//            width: height
//            onClicked: {
//                componentData.model.removeTransform(index)
//            }
//        }

        ToolButton {
            id: moveUpTransformButton
            anchors.right: removeTransformButton.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            iconSource: "images/up.png"
            visible: editable
            height: titleHeader.height
            width: height
            onClicked: {
                componentData.model.moveTransform(index, index - 1)
            }
        }

        ToolButton {
            id: moveDownTransformButton
            anchors.right: moveUpTransformButton.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            iconSource: "images/down.png"
            visible: editable
            height: titleHeader.height
            width: height
            onClicked: {
                componentData.model.moveTransform(index, index + 1)
            }
        }
    }
*/
    Column {
        id: columnLayout
        spacing: 4
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8
    }
}
