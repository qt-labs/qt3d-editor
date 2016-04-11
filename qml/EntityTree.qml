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
import com.theqtcompany.SceneEditor3D 1.0

Item {
    id: treeViewSplit

    property bool entityTreeViewEditing: entityTreeView.editing

    property alias view: entityTreeView
    property alias menu: addComponentMenu

    Keys.onDeletePressed: {
        // Doublecheck that we don't try to remove the scene root
        if (entityTreeView.selection.currentIndex !== editorScene.sceneModel.sceneEntityIndex())
            editorScene.undoHandler.createRemoveEntityCommand(selectedEntityName)
    }

    function focusTree() {
        entityTreeView.forceActiveFocus(Qt.MouseFocusReason)
    }

    function selectSceneRoot() {
        entityTreeView.selection.setCurrentIndex(
                    editorScene.sceneModel.sceneEntityIndex(),
                    ItemSelectionModel.SelectCurrent)
        entityTreeView.expand(entityTreeView.selection.currentIndex)
    }

    function addNewEntity(entityType, xPos, yPos) {
        var x = xPos ? xPos : -1
        var y = yPos ? yPos : -1
        entityTreeView.editing = false

        // Never allow inserting to root
        if (entityTreeView.selection.currentIndex.row === -1)
            selectSceneRoot()
        editorScene.undoHandler.createInsertEntityCommand(entityType, selectedEntityName,
                                                          editorScene.getWorldPosition(x, y))
        var newItemIndex = editorScene.sceneModel.lastInsertedIndex()

        entityTreeView.expand(entityTreeView.selection.currentIndex)
        entityTreeView.selection.setCurrentIndex(newItemIndex,
                                                 ItemSelectionModel.SelectCurrent)
        // Remove focus so activating editing will always force it on
        entityTreeView.focus = true
    }

    Component.onCompleted: selectSceneRoot()

    property int splitHeight: treeViewHeader.height + 130

    Layout.minimumHeight: treeViewHeader.height
    height: splitHeight

    ButtonViewHeader {
        id: treeViewHeader
        anchors.top: treeViewSplit.top
        headerText: qsTr("Scene") + editorScene.emptyString

        onShowViewButtonPressed: {
            treeViewSplit.height = splitHeight
        }
        onHideViewButtonPressed: {
            splitHeight = treeViewSplit.height
            treeViewSplit.height = minimumHeaderHeight
        }
    }

    TreeView {
        id: entityTreeView
        anchors.top: treeViewHeader.bottom
        anchors.bottom: treeViewSplit.bottom
        width: parent.width
        visible: treeViewHeader.viewVisible
        model: editorScene.sceneModel
        selection: ItemSelectionModel {
            model: editorScene.sceneModel
        }
        headerVisible: false
        alternatingRowColors: false
        backgroundVisible: false

        property bool editing: false
        property bool sceneRootSelected: true

        itemDelegate: FocusScope {
            Text {
                id: valueField
                anchors.verticalCenter: parent.verticalCenter
                color: styleData.textColor
                elide: styleData.elideMode
                text: styleData.value
                visible: !entityTreeView.editing || !styleData.selected
                anchors.fill: parent
                clip: true
            }
            Rectangle {
                id: renameField
                anchors.fill: parent
                color: "white"
                border.color: "lightGray"
                visible: !valueField.visible
                TextInput {
                    id: renameTextiInput
                    anchors.fill: parent
                    clip: true
                    visible: !valueField.visible
                    selectByMouse: true
                    focus: true
                    validator: RegExpValidator {
                        regExp: /^[A-Za-z_][A-Za-z0-9_ ]*$/
                    }

                    onVisibleChanged: {
                        if (visible) {
                            text = styleData.value
                            selectAll()
                            forceActiveFocus(Qt.MouseFocusReason)
                        }
                    }

                    onCursorVisibleChanged: {
                        if (!cursorVisible)
                            entityTreeView.editing = false
                    }

                    Keys.onReturnPressed: {
                        entityTreeView.editing = false
                        if (text !== model.name) {
                            editorScene.undoHandler.createRenameEntityCommand(selectedEntityName,
                                                                              text)
                        }
                        selectedEntityName = editorScene.sceneModel.entityName(entityTreeView.selection.currentIndex)
                    }
                }
            }
        }
        onDoubleClicked: {
            entityTreeView.editing = true
        }

        onCurrentIndexChanged: {
            entityTreeView.editing = false
        }

        TableViewColumn {
            title: qsTr("Entities") + editorScene.emptyString
            role: "name"
            width: parent.width - 50
        }
        TableViewColumn {
            title: qsTr("Visibility") + editorScene.emptyString
            role: "visibility"
            width: 18
            delegate: VisiblePropertyInputField {
                id: visibleProperty
                component: editorScene.sceneModel.editorSceneItemFromIndex(styleData.index).entity()
                propertyName: "enabled"
                visibleOnImage: "/images/visible_on.png"
                visibleOffImage: "/images/visible_off.png"
                // The component is not shown for root item
                visible: (styleData.index !== editorScene.sceneModel.sceneEntityIndex()) ? true
                                                                                         : false

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        entityName = editorScene.sceneModel.entityName(styleData.index)
                        if (entityEnabled)
                            entityEnabled = false
                        else
                            entityEnabled = true
                        visibleProperty.visibleImageClicked()
                    }
                }
            }
        }

        Connections {
            target: entityTreeView.selection
            onCurrentIndexChanged: {
                // If there is no current item selected for some reason, fall back to scene root
                if (entityTreeView.selection.currentIndex.row === -1) {
                    selectedEntityName = ""
                    selectSceneRoot()
                } else {
                    entityTreeView.sceneRootSelected =
                            (editorScene.sceneModel.sceneEntityIndex() === entityTreeView.selection.currentIndex)
                    selectedEntity = editorScene.sceneModel.editorSceneItemFromIndex(entityTreeView.selection.currentIndex)
                    if (selectedEntity) {
                        componentPropertiesView.model = selectedEntity.componentsModel
                        selectedEntityName = editorScene.sceneModel.entityName(entityTreeView.selection.currentIndex)
                        editorScene.clearSelectionBoxes()
                        selectedEntity.showSelectionBox = true
                    } else {
                        // Should never get here
                        selectedEntityName = ""
                        editorScene.clearSelectionBoxes()
                    }
                    editorScene.selection = selectedEntity.entity()
                }
            }
        }

        ComponentMenu {
            id: addComponentMenu
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                // Prevent menu popup if no entity is selected
                if (componentPropertiesView.model !== undefined)
                    addComponentMenu.popup()
            }
        }
    }
}
