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
import QtQuick.Layouts 1.1
import QtQml.Models 2.2

Item {
    id: entityLibrary

    property int gridMargin: 10
    property int buttonSize: 80
    property int splitHeight: entityViewHeader.height + 260

    signal createNewEntity(int entityType, int xPos, int yPos)

    Layout.minimumHeight: entityViewHeader.height
    height: splitHeight
    width: buttonSize * 3 - gridMargin

    ButtonViewHeader {
        id: entityViewHeader
        headerText: qsTr("Shapes") + editorScene.emptyString

        onShowViewButtonPressed: {
            entityLibrary.height = splitHeight
        }
        onHideViewButtonPressed: {
            splitHeight = entityLibrary.height
            entityLibrary.height = minimumHeaderHeight
        }
    }

    Rectangle {
        id: entityView
        anchors.top: entityViewHeader.bottom
        height: entityLibrary.height - entityViewHeader.height
        width: parent.width
        color: "lightGray"
        border.color: "darkGray"
        visible: entityViewHeader.viewVisible
        ScrollView {
            anchors.fill: parent
            anchors.leftMargin: gridMargin
            GridView {
                id: gridRoot
                clip: true
                topMargin: gridMargin
                model: EntityModel { id: entityModel }
                delegate: MouseArea {
                    id: delegateRoot
                    width: buttonSize
                    height: buttonSize

                    property variant dragIconObj
                    property int dragPositionX
                    property int dragPositionY

                    onPressed: {
                        var globalPos = mapToItem(applicationMouseArea, mouseX, mouseY)
                        dragPositionX = globalPos.x
                        dragPositionY = globalPos.y
                        var component = Qt.createComponent("DragEntity.qml")
                        dragIconObj = component.createObject(mainwindow)
                        dragIconObj.source = meshDragImage
                        dragIconObj.x = globalPos.x - dragIconObj.width / 2
                        dragIconObj.y = globalPos.y - dragIconObj.height / 2
                        editorScene.showPlaceholderEntity("dragInsert", meshType)
                    }

                    onPositionChanged: {
                        if (dragIconObj) {
                            var globalPos = mapToItem(applicationMouseArea, mouseX, mouseY)
                            dragIconObj.x += globalPos.x - dragPositionX
                            dragIconObj.y += globalPos.y - dragPositionY
                            dragPositionX = globalPos.x
                            dragPositionY = globalPos.y
                            var scenePos = editorViewport.mapFromItem(applicationMouseArea,
                                                                      dragPositionX,
                                                                      dragPositionY)
                            editorScene.movePlaceholderEntity("dragInsert",
                                        editorScene.getWorldPosition(scenePos.x, scenePos.y))
                        }
                    }

                    onReleased: {
                        dragIconObj.destroy()
                        var scenePos = editorViewport.mapFromItem(applicationMouseArea,
                                                                  dragPositionX,
                                                                  dragPositionY)
                        editorScene.hidePlaceholderEntity("dragInsert")
                        createNewEntity(meshType, scenePos.x, scenePos.y)
                    }

                    onCanceled: {
                        dragIconObj.destroy()
                        editorScene.hidePlaceholderEntity("dragInsert")
                    }

                    Rectangle {
                        id: entityButton
                        width: buttonSize
                        height: buttonSize
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
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
                            }
                        }
                    }
                }
            }
        }
    }
}
