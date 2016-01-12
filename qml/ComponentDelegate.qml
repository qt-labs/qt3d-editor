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
import com.theqtcompany.SceneEditor3D 1.0

Item {
    id: componentDelegate
    width: parent.width

    property alias title: componentTitle.headerText
    default property alias _contentChildren: columnLayout.data
    property alias viewTitleVisible: componentTitle.viewVisible
    property int componentType: EditorSceneItemComponentsModel.Unknown
    property int componentHeight: columnLayout.y + columnLayout.height

    height: componentHeight

    Rectangle {
        id: titleHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: componentTitle.height

        ViewHeader {
            id: componentTitle
            anchors.top: parent.top
            headerText: componentTitle
            viewVisible: columnLayout.visible

            onShowViewTitle: {
                columnLayout.visible = true
                componentDelegate.height = componentHeight
            }
            onHideViewTitle: {
                columnLayout.visible = false
                componentHeight = componentDelegate.height
                componentDelegate.height = minimumHeaderHeight
            }
        }

        Column {
            id: columnLayout
            anchors.top: componentTitle.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
        }
    }
}

