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
import com.theqtcompany.SceneEditor3D 1.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

Item {
    id: generalEntityItem
    width: parent.width

    property alias propertiesButtonVisible: propertiesLayout.visible
    property alias entityName: textInputField.displayText

    property alias title: componentTitle.headerText
    property alias viewTitleVisible: columnLayout.visible

    property int entityType: 0
    property int componentHeight: componentTitle.minimumHeaderHeight

    Layout.minimumHeight: componentHeight
    Layout.maximumHeight: Layout.minimumHeight

    visible: !editorScene.multiSelection

    Component.onCompleted: {
        componentHeight = columnLayout.y + columnLayout.height
    }

    Rectangle {
        id: titleHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: componentTitle.height

        ButtonViewHeader {
            id: componentTitle
            anchors.top: parent.top
            visibleEntityButtonShown: true
            headerText: qsTr("Properties") + editorScene.emptyString
            tooltip: qsTr("Show/Hide Properties View") + editorScene.emptyString

            function showView() {
                columnLayout.visible = true
                if (propertiesButtonVisible)
                    generalEntityItem.Layout.minimumHeight = minimumHeaderHeight + componentHeight
                else
                    generalEntityItem.Layout.minimumHeight = minimumHeaderHeight + textInputField.height + 8
            }
            function hideView() {
                columnLayout.visible = false
                generalEntityItem.Layout.minimumHeight = minimumHeaderHeight
            }

            onShowViewButtonPressed: {
                showView()
            }
            onHideViewButtonPressed: {
                hideView()
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

            TextInputField {
                id: textInputField
                label: qsTr("Entity name") + editorScene.emptyString
                tooltip: qsTr("Name of the entity. Name may not be a duplicate to an\nalready existing one, or it will be automatically renamed.")
                         + editorScene.emptyString
                displayText: componentData.objectName
                validator: RegExpValidator {
                    regExp: /^[A-Za-z_][A-Za-z0-9_ ]*$/
                }

                onDesiredTextChanged: {
                    if (desiredText !== "") {
                        editorScene.undoHandler.createRenameEntityCommand(editorContent.selectedEntityName,
                                                                          desiredText)
                        editorContent.selectedEntityName = editorScene.sceneModel.entityName(entityTree.view.selection.currentIndex)
                    }
                }

                onDisplayTextChanged: {
                    if (componentTitle.viewVisible)
                        componentTitle.showView()
                    else
                        componentTitle.hideView()
                }
            }

            ColumnLayout {
                Item {
                    height: 8
                }

                StyledLabel {
                    id: showLabel
                    visible: propertiesLayout.visible
                    text: qsTr("Show Properties:") + editorScene.emptyString
                    Layout.alignment: Qt.AlignLeft
                    tooltip: qsTr("Buttons to show or hide the properties of the selected entity.")
                             + editorScene.emptyString
                }

                RowLayout {
                    id: propertiesLayout
                    Layout.alignment: Qt.AlignLeft

                    EnableButton {
                        id: lightButton
                        width: 24
                        height: 24
                        enabledIconSource: "images/property_light_shown.png"
                        disabledIconSource: "images/property_light_hidden.png"
                        tooltip: qsTr("Show/Hide Light Properties") + editorScene.emptyString
                        buttonEnabled: editorContent.lightViewVisible
                        onEnabledButtonClicked: {
                            editorContent.lightViewVisible = !editorContent.lightViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Light) ? true : false
                    }
                    EnableButton {
                        id: meshButton
                        width: 24
                        height: 24
                        enabledIconSource: "images/property_mesh_shown.png"
                        disabledIconSource: "images/property_mesh_hidden.png"
                        tooltip: qsTr("Show/Hide Mesh Properties") + editorScene.emptyString
                        buttonEnabled: editorContent.meshViewVisible
                        onEnabledButtonClicked: {
                            editorContent.meshViewVisible = !editorContent.meshViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Mesh) ? true : false
                    }
                    EnableButton {
                        id: transformButton
                        width: 24
                        height: 24
                        enabledIconSource: "images/property_transform_shown.png"
                        disabledIconSource: "images/property_transform_hidden.png"
                        tooltip: qsTr("Show/Hide Transform Properties") + editorScene.emptyString
                        buttonEnabled: editorContent.transformViewVisible
                        onEnabledButtonClicked: {
                            editorContent.transformViewVisible = !editorContent.transformViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Mesh
                                  || entityType === EditorSceneItem.Light
                                  || entityType === EditorSceneItem.Group) ? true : false
                    }
                    EnableButton {
                        id: materialButton
                        width: 24
                        height: 24
                        enabledIconSource: "images/property_material_shown.png"
                        disabledIconSource: "images/property_material_hidden.png"
                        tooltip: qsTr("Show/Hide Material Properties") + editorScene.emptyString
                        buttonEnabled: editorContent.materialViewVisible
                        onEnabledButtonClicked: {
                            editorContent.materialViewVisible = !editorContent.materialViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Mesh) ? true : false
                    }
                    EnableButton {
                        id: cameraButton
                        width: 24
                        height: 24
                        enabledIconSource: "images/property_camera_shown.png"
                        disabledIconSource: "images/property_camera_hidden.png"
                        tooltip: qsTr("Show/Hide Camera Properties") + editorScene.emptyString
                        buttonEnabled: editorContent.cameraViewVisible
                        onEnabledButtonClicked: {
                            editorContent.cameraViewVisible = !editorContent.cameraViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Camera) ? true : false
                    }
                    Item {
                        width: 1
                        height: lightButton.height
                    }
                }
            }
        }
    }
}

