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

ComponentDelegate {
    id: transformDelegate
    title: qsTr("Transform") + editorScene.emptyString

    property int currentTransform: 0
    property bool fieldsDisabled: transformFieldsDisabled

    viewTitleVisible: editorContent.transformViewVisible
    componentType: EditorSceneItemComponentsModel.Transform

    onChangeViewVisibity: {
        editorContent.transformViewVisible = viewVisibility
    }

    onFieldsDisabledChanged: {
        editorContent.selectedEntity.setCustomProperty(editorScene.sceneModel.editorSceneItemFromIndex(entityTree.view.selection.currentIndex).entity(),
                                         editorScene.lockTransformPropertyName,
                                         fieldsDisabled)
    }

    Component.onCompleted: {
        if (!editorContent.transformViewVisible)
            height = minimumComponentHeight
    }

    Item {
        width: parent.width
        height: transformCombobox.height + 8

        Component.onCompleted: transformCombobox.currentIndex = transformDelegate.currentTransform - 1

        QQC2.ComboBox {
            id: transformCombobox
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottomMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            implicitHeight: editorContent.qlcControlHeight
            enabled: !fieldsDisabled
            property int validIndex: -1

            model: ListModel {
                property string language: editorContent.systemLanguage

                function retranslateUi() {
                    clear()
                    append({text: qsTr("Scale, Rotate & Translate")})
                    append({text: qsTr("Matrix")})
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
                    if (currentIndex === EditorSceneItemTransformComponentsModel.SRT - 1)
                        componentData.model.setTransform(EditorSceneItemTransformComponentsModel.SRT)
                    else if (currentIndex === EditorSceneItemTransformComponentsModel.Matrix - 1)
                        componentData.model.setTransform(EditorSceneItemTransformComponentsModel.Matrix)
                } else {
                    currentIndex = validIndex
                }
            }
        }
    }

    Repeater {
        model: componentData.model

        Loader {
            width: parent.width

            property bool enabledFields: fieldsDisabled

            function transformTypetoDelegateSource(transformType) {
                transformDelegate.currentTransform = transformType
                if (transformType == EditorSceneItemTransformComponentsModel.SRT)
                    return "SRTTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Matrix)
                    return "MatrixTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Translate)
                    return "TranslateTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Rotate)
                    return "RotateTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Scale)
                    return "ScaleTransformDelegate.qml"

                return "UnknownTransformDelegate.qml"
            }

            onLoaded: {
                if (transformDelegate)
                    transformCombobox.currentIndex = transformDelegate.currentTransform - 1
            }

            source: transformTypetoDelegateSource(transformType)
        }
    }
}
