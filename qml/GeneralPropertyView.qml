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
import com.theqtcompany.SceneEditor3D 1.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Item {
    id: generalEntityItem
    width: parent.width

    property alias propertiesButtonVisible: propertiesLayout.visible
    property alias entityName: textInputField.displayText

    property alias title: componentTitle.headerText
    property alias viewTitleVisible: columnLayout.visible

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
            headerText: qsTr("Properties")

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
                label: qsTr("Entity name")
                displayText: componentData.objectName

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

                Label {
                    id: showLabel
                    visible: propertiesLayout.visible
                    text: qsTr("Show Properties:")
                    Layout.alignment: Qt.AlignLeft
                }

                RowLayout {
                    id: propertiesLayout
                    Layout.alignment: Qt.AlignLeft

                    ShowPropertyButton {
                        id: meshButton
                        showIconSource: "/images/property_mesh_shown.png"
                        hideIconSource: "/images/property_mesh_hidden.png"
                        tooltip: qsTr("Show/Hide Mesh Properties")
                        propertyComponentType: EditorSceneItemComponentsModel.Mesh
                        buttonEnabled: meshViewVisible
                        onPropertyButtonClicked: {
                            meshViewVisible = !meshViewVisible
                        }
                    }
                    ShowPropertyButton {
                        id: transformButton
                        showIconSource: "/images/property_transform_shown.png"
                        hideIconSource: "/images/property_transform_hidden.png"
                        tooltip: qsTr("Show/Hide Transform Properties")
                        propertyComponentType: EditorSceneItemComponentsModel.Transform
                        buttonEnabled: transformViewVisible
                        onPropertyButtonClicked: {
                            transformViewVisible = !transformViewVisible
                        }
                    }
                    ShowPropertyButton {
                        id: materialButton
                        showIconSource: "/images/property_material_shown.png"
                        hideIconSource: "/images/property_material_hidden.png"
                        tooltip: qsTr("Show/Hide Material Properties")
                        propertyComponentType: EditorSceneItemComponentsModel.Material
                        buttonEnabled: materialViewVisible
                        onPropertyButtonClicked: {
                            materialViewVisible = !materialViewVisible
                        }
                    }
                }
            }
        }
    }
}

