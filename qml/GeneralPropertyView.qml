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
import Qt.labs.controls 1.0 as QLC
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
                displayText: componentData.objectName
                validator: RegExpValidator {
                    regExp: /^[A-Za-z_][A-Za-z0-9_ ]*$/
                }

                onDesiredTextChanged: {
                    if (desiredText !== "") {
                        editorScene.undoHandler.createRenameEntityCommand(selectedEntityName,
                                                                          desiredText)
                        selectedEntityName = editorScene.sceneModel.entityName(entityTree.view.selection.currentIndex)
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

                QLC.Label {
                    id: showLabel
                    visible: propertiesLayout.visible
                    text: qsTr("Show Properties:") + editorScene.emptyString
                    Layout.alignment: Qt.AlignLeft
                }

                RowLayout {
                    id: propertiesLayout
                    Layout.alignment: Qt.AlignLeft

                    EnableButton {
                        id: lightButton
                        enabledIconSource: "/images/property_light_shown.png"
                        disabledIconSource: "/images/property_light_hidden.png"
                        tooltip: qsTr("Show/Hide Light Properties") + editorScene.emptyString
                        buttonEnabled: lightViewVisible
                        onEnabledButtonClicked: {
                            lightViewVisible = !lightViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Light) ? true : false
                    }
                    EnableButton {
                        id: meshButton
                        enabledIconSource: "/images/property_mesh_shown.png"
                        disabledIconSource: "/images/property_mesh_hidden.png"
                        tooltip: qsTr("Show/Hide Mesh Properties") + editorScene.emptyString
                        buttonEnabled: meshViewVisible
                        onEnabledButtonClicked: {
                            meshViewVisible = !meshViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Mesh) ? true : false
                    }
                    EnableButton {
                        id: transformButton
                        enabledIconSource: "/images/property_transform_shown.png"
                        disabledIconSource: "/images/property_transform_hidden.png"
                        tooltip: qsTr("Show/Hide Transform Properties") + editorScene.emptyString
                        buttonEnabled: transformViewVisible
                        onEnabledButtonClicked: {
                            transformViewVisible = !transformViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Mesh
                                  || entityType === EditorSceneItem.Light
                                  || entityType === EditorSceneItem.Group) ? true : false
                    }
                    EnableButton {
                        id: materialButton
                        enabledIconSource: "/images/property_material_shown.png"
                        disabledIconSource: "/images/property_material_hidden.png"
                        tooltip: qsTr("Show/Hide Material Properties") + editorScene.emptyString
                        buttonEnabled: materialViewVisible
                        onEnabledButtonClicked: {
                            materialViewVisible = !materialViewVisible
                        }
                        visible: (entityType === EditorSceneItem.Mesh) ? true : false
                    }
                    EnableButton {
                        id: cameraButton
                        enabledIconSource: "/images/property_camera_shown.png"
                        disabledIconSource: "/images/property_camera_hidden.png"
                        tooltip: qsTr("Show/Hide Camera Properties") + editorScene.emptyString
                        buttonEnabled: cameraViewVisible
                        onEnabledButtonClicked: {
                            cameraViewVisible = !cameraViewVisible
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

