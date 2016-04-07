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

Rectangle {
    property string headerText
    property bool viewVisible: true
    property bool visibleEntityButtonShown: false
    property int minimumHeaderHeight: viewHeaderText.implicitHeight + 12

    signal showViewButtonPressed()
    signal hideViewButtonPressed()

    height: minimumHeaderHeight
    width: parent.width
    color: "darkGray"

    QLC.Label {
        id: viewHeaderText
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        text: headerText
        font.bold: true
    }

    VisiblePropertyInputField {
        id: visibleEntityButton
        anchors.right: showViewButton.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        height: minimumHeaderHeight
        width: 20
        component: editorScene.sceneModel.editorSceneItemFromIndex(entityTree.view.selection.currentIndex).entity()
        propertyName: "enabled"
        visibleOnImage: "/images/visible_on.png"
        visibleOffImage: "/images/visible_off.png"
        // The component is not shown for root item
        visible: (visibleEntityButtonShown && entityTree.view.selection.currentIndex !== editorScene.sceneModel.sceneEntityIndex())
                 ? true : false
        MouseArea {
            anchors.fill: parent
            onClicked: {
                selectedEntityName = editorScene.sceneModel.entityName(entityTree.view.selection.currentIndex)
                visibleEntityButton.entityEnabled = !visibleEntityButton.entityEnabled
                visibleEntityButton.visibleImageClicked()
            }
        }
    }

    Item {
        id: showViewButton
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        height: 10
        width: 20

        Image {
            id: collapseArrowImage
            source: "images/arrow.png"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (viewVisible) {
                    viewVisible = false
                    hideViewButtonPressed()
                    collapseArrowImage.rotation = 180
                } else {
                    viewVisible = true
                    showViewButtonPressed()
                    collapseArrowImage.rotation = 0
                }
            }
        }
    }
}
