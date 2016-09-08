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
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQml.Models 2.2

Item {
    id: entityLibrary

    property int gridMargin: 10
    property int buttonSize: 80
    property int optimalWidth: 2 * (buttonSize + gridMargin) + gridMargin
    property int splitHeight: entityViewHeader.height + 260

    signal createNewEntity(int entityType, int xPos, int yPos)

    Layout.minimumHeight: entityViewHeader.height
    height: splitHeight
    // Adjust width automatically for scrollbar, unless width has been adjusted manually
    // TODO: This causes settings save problems. Do we need it, or want to save width instead?
//    width: ((gridRoot.cellHeight * (gridRoot.count / 2)) > entityView.height)
//           ? optimalWidth + 21 : optimalWidth

    ButtonViewHeader {
        id: entityViewHeader
        headerText: qsTr("Shapes") + editorScene.emptyString
        tooltip: qsTr("Show/Hide Shapes View") + editorScene.emptyString
    }

    Rectangle {
        id: entityView
        anchors.top: entityViewHeader.bottom
        height: entityLibrary.height - entityViewHeader.height
        width: parent.width
        color: editorContent.paneBackgroundColor
        border.color: editorContent.viewBorderColor
        visible: entityViewHeader.viewVisible
        ScrollView {
            anchors.fill: parent
            anchors.leftMargin: gridMargin
            GridView {
                id: gridRoot
                clip: true
                topMargin: gridMargin
                cellHeight: buttonSize + gridMargin
                cellWidth: buttonSize + gridMargin
                model: EntityModel { id: entityModel }
                delegate: Button {
                    id: entityButton
                    width: buttonSize
                    height: buttonSize
                    style: ButtonStyle {
                        background: Rectangle {
                            border.width: 0
                            color: editorContent.itemBackgroundColor
                        }
                    }
                    tooltip: qsTr("You can click here or drag'n'drop to add a new <i>%1</i> to the scene.").arg(
                                 meshString.toLowerCase()) + editorScene.emptyString
                    Column {
                        anchors.centerIn: parent
                        Image {
                            source: meshImage
                            width: 50
                            height: 50
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: meshString
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: editorContent.textColor
                            font.family: editorContent.labelFontFamily
                            font.weight: editorContent.labelFontWeight
                            font.pixelSize: editorContent.labelFontPixelSize
                        }
                    }
                    MouseArea {
                        id: delegateRoot
                        width: buttonSize
                        height: buttonSize

                        property int dragPositionX
                        property int dragPositionY

                        drag.target: dragEntityItem

                        onPressed: {
                            entityTree.focusTree()
                            var globalPos = mapToItem(applicationArea, mouseX, mouseY)
                            dragPositionX = globalPos.x
                            dragPositionY = globalPos.y
                            dragEntityItem.startDrag(delegateRoot, meshDragImage,
                                                     "insertEntity", dragPositionX, dragPositionY,
                                                     meshType)
                            editorScene.showPlaceholderEntity("dragInsert", meshType)
                        }

                        onPositionChanged: {
                            var globalPos = mapToItem(applicationArea, mouseX, mouseY)
                            dragPositionX = globalPos.x
                            dragPositionY = globalPos.y
                            dragEntityItem.setPosition(dragPositionX, dragPositionY)
                            var scenePos = editorViewport.mapFromItem(applicationArea,
                                                                      dragPositionX,
                                                                      dragPositionY)
                            editorScene.movePlaceholderEntity("dragInsert",
                                                              editorScene.getWorldPosition(scenePos.x, scenePos.y))
                            editorScene.updateWorldPositionLabel(scenePos.x, scenePos.y)
                        }

                        onReleased: {
                            var dropResult = dragEntityItem.endDrag(true)
                            editorScene.hidePlaceholderEntity("dragInsert")
                            // If no DropArea handled the drop, create new entity
                            if (dropResult === Qt.IgnoreAction) {
                                var scenePos = editorViewport.mapFromItem(applicationArea,
                                                                          dragPositionX,
                                                                          dragPositionY)
                                createNewEntity(meshType, scenePos.x, scenePos.y)
                                editorScene.updateWorldPositionLabel(scenePos.x, scenePos.y)
                            }
                        }

                        onCanceled: {
                            dragEntityItem.endDrag(false)
                            editorScene.hidePlaceholderEntity("dragInsert")
                            editorScene.updateWorldPositionLabel(-1, -1)
                        }

                    }
                }
            }
        }
    }
    Component.onCompleted: gridRoot.forceLayout()
}
