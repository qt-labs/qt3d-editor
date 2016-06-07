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
import QtQuick.Controls 2.0 as QQC2
import Qt3D.Extras 2.0

ComponentDelegate {
    id: materialComponentDelegate
    title: qsTr("Material") + editorScene.emptyString

    property int currentMaterial: 0

    viewTitleVisible: editorContent.materialViewVisible

    onChangeViewVisibity: {
        editorContent.materialViewVisible = viewVisibility
    }

    Component.onCompleted: {
        if (!editorContent.materialViewVisible)
            height = minimumComponentHeight
    }

    Item {
        width: parent.width
        height: materialCombobox.height + 8

        Component.onCompleted: materialCombobox.currentIndex = materialComponentDelegate.currentMaterial - 1

        StyledLabel {
            id: importedLabel
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottomMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Imported custom material") + editorScene.emptyString
            visible: materialComponentDelegate.currentMaterial === EditorSceneItemMaterialComponentsModel.Unknown
        }

        QQC2.ComboBox {
            id: materialCombobox
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottomMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            property int validIndex: -1
            implicitHeight: editorContent.qlcControlHeight
            visible:  materialComponentDelegate.currentMaterial !== EditorSceneItemMaterialComponentsModel.Unknown

            model: ListModel {
                property string language: editorContent.systemLanguage

                function retranslateUi() {
                    clear()
                    append({text: qsTr("Diffuse Map")})
                    append({text: qsTr("Diffuse & Specular Map")})
                    append({text: qsTr("Gooch")})
                    append({text: qsTr("Normal & Diffuse Map")})
                    append({text: qsTr("Normal & Diffuse Map Alpha")})
                    append({text: qsTr("Normal, Diffuse & Specular Map")})
                    append({text: qsTr("Phong Alpha")})
                    append({text: qsTr("Phong")})
                }

                Component.onCompleted: {
                    retranslateUi()
                }

                onLanguageChanged: {
                    retranslateUi()
                }
            }
            onCurrentIndexChanged: {
                if (activeFocus || validIndex === -1) {
                    validIndex = currentIndex
                    if (currentIndex === EditorSceneItemMaterialComponentsModel.DiffuseMap - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.DiffuseMap)
                    else if (currentIndex === EditorSceneItemMaterialComponentsModel.DiffuseSpecularMap - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.DiffuseSpecularMap)
                    else if (currentIndex === EditorSceneItemMaterialComponentsModel.Gooch - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.Gooch)
                    else if (currentIndex === EditorSceneItemMaterialComponentsModel.NormalDiffuseMap - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.NormalDiffuseMap)
                    else if (currentIndex === EditorSceneItemMaterialComponentsModel.NormalDiffuseMapAlpha - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.NormalDiffuseMapAlpha)
                    else if (currentIndex === EditorSceneItemMaterialComponentsModel.NormalDiffuseSpecularMap - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.NormalDiffuseSpecularMap)
                    else if (currentIndex === EditorSceneItemMaterialComponentsModel.PhongAlpha - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.PhongAlpha)
                    else if (currentIndex === EditorSceneItemMaterialComponentsModel.Phong - 1)
                        componentData.model.setMaterial(EditorSceneItemMaterialComponentsModel.Phong)
                } else {
                    currentIndex = validIndex
                }
            }
        }
    }

    Repeater {
        id: materialRepeater
        model: componentData.model

        Loader {
            id: materialLoader
            width: parent.width

            function materialTypetoDelegateSource(materialType) {
                materialComponentDelegate.currentMaterial = materialType
                if (materialType == EditorSceneItemMaterialComponentsModel.DiffuseMap)
                    return "DiffuseMapMaterialDelegate.qml";
                if (materialType == EditorSceneItemMaterialComponentsModel.DiffuseSpecularMap)
                    return "DiffuseSpecularMapMaterialDelegate.qml";
                if (materialType == EditorSceneItemMaterialComponentsModel.Gooch)
                    return "GoochMaterialDelegate.qml";
                if (materialType == EditorSceneItemMaterialComponentsModel.NormalDiffuseMap)
                    return "NormalDiffuseMapMaterialDelegate.qml";
                if (materialType == EditorSceneItemMaterialComponentsModel.NormalDiffuseMapAlpha)
                    return "NormalDiffuseMapAlphaMaterialDelegate.qml";
                if (materialType == EditorSceneItemMaterialComponentsModel.NormalDiffuseSpecularMap)
                    return "NormalDiffuseSpecularMapMaterialDelegate.qml";
                if (materialType == EditorSceneItemMaterialComponentsModel.PhongAlpha)
                    return "PhongAlphaMaterialDelegate.qml";
                if (materialType == EditorSceneItemMaterialComponentsModel.Phong)
                    return "PhongMaterialDelegate.qml";

                return "UnknownMaterialDelegate.qml";
            }

            onLoaded: {
                if (materialComponentDelegate)
                    materialCombobox.currentIndex = materialComponentDelegate.currentMaterial - 1
            }

            source: materialTypetoDelegateSource(materialType)
        }
    }
}

