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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle {
    id: header

    property string headerText
    property bool viewVisible: true
    property bool visibleEntityButtonShown: false
    property bool showViewButtonShown: true
    property bool lockTransformButtonShown: false
    property bool lockTransformFields: lockTransformButton.locked
    property int minimumHeaderHeight: viewHeaderText.implicitHeight + 12
    property alias tooltip: showViewButton.tooltip

    signal showViewButtonPressed()
    signal hideViewButtonPressed()
    signal showViewTitle(bool showView)

    height: minimumHeaderHeight
    width: parent.width
    color: "darkGray"

    onViewVisibleChanged: {
        showViewTitle(viewVisible)
    }

    QLC.Label {
        id: viewHeaderText
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        text: headerText
        font.bold: true
    }

    PropertyLockButton {
        id: lockTransformButton
        anchors.right: showViewButton.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        visible: lockTransformButtonShown
        label: qsTr("Transform") + editorScene.emptyString
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

    Button {
        id: showViewButton
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        visible: showViewButtonShown
        style: ButtonStyle {
            padding.top: 0
            padding.bottom: 0
            padding.right: 0
            padding.left: 0
            background: Rectangle {
                border.color: "transparent"
                border.width: 0
                color: "transparent"
            }
        }
        activeFocusOnTab: false
        height: 10
        width: 20
        tooltip: qsTr("Show/Hide View") + editorScene.emptyString // Default tooltip

        Image {
            id: collapseArrowImage
            source: "images/arrow.png"
        }
    }

    MouseArea {
        anchors.fill: showViewButtonShown ? showViewButton : header
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
