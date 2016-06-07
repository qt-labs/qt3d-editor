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
    id: lightDelegate
    title: qsTr("Light") + editorScene.emptyString

    property int currentLight: 0

    viewTitleVisible: editorContent.lightViewVisible

    onChangeViewVisibity: {
        editorContent.lightViewVisible = viewVisibility
    }

    Component.onCompleted: {
        if (!editorContent.lightViewVisible)
            height = minimumComponentHeight
    }

    Item {
        id: comboboxItem
        width: parent.width
        height: lightCombobox.height + 8

        Component.onCompleted: lightCombobox.currentIndex = lightDelegate.currentLight - 1

        QQC2.ComboBox {
            id: lightCombobox
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottomMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            implicitHeight: editorContent.qlcControlHeight
            property int validIndex: -1

            model: ListModel {
                property string language: editorContent.systemLanguage

                function retranslateUi() {
                    // Repopulate list to change the current text as well
                    clear()
                    append({text: qsTr("Directional Light")})
                    append({text: qsTr("Point Light")})
                    append({text: qsTr("Spot Light")})
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
                    if (currentIndex === EditorSceneItemLightComponentsModel.Directional - 1)
                        componentData.model.setLight(EditorSceneItemLightComponentsModel.Directional)
                    else if (currentIndex === EditorSceneItemLightComponentsModel.Point - 1)
                        componentData.model.setLight(EditorSceneItemLightComponentsModel.Point)
                    else if (currentIndex === EditorSceneItemLightComponentsModel.Spot - 1)
                        componentData.model.setLight(EditorSceneItemLightComponentsModel.Spot)
                } else {
                    currentIndex = validIndex
                }
            }
        }
    }

    Repeater {
        id: lightRepeater
        model: componentData.model

        // Note: Various light properties of non-selected light types are only remembered
        // until selected entity changes. Only the selected type properties are retained
        // between selected entity changes.

        property color lightColor: "#ffffff"
        property real lightIntensity: 1
        property vector3d lightDirection: Qt.vector3d(0, -1, 0)
        property vector3d lightAttenuation: Qt.vector3d(0, 0, 0.002)
        property real lightCutOffAngle: 45

        property bool colorSet: false
        property bool intensitySet: false
        property bool directionSet: false
        property bool attenuationSet: false
        property bool cutOffAngleSet: false

        Loader {
            id: lightLoader
            width: parent.width

            property var repeater: lightRepeater

            function lightTypetoDelegateSource(lightType) {
                lightDelegate.currentLight = lightType
                if (lightType == EditorSceneItemLightComponentsModel.Directional)
                    return "DirectionalLightDelegate.qml";
                if (lightType == EditorSceneItemLightComponentsModel.Point)
                    return "PointLightDelegate.qml";
                if (lightType == EditorSceneItemLightComponentsModel.Spot)
                    return "SpotLightDelegate.qml";

                return "UnknownLightDelegate.qml";
            }

            source: lightTypetoDelegateSource(lightType)

            onLoaded: {
                if (lightDelegate)
                    lightCombobox.currentIndex = lightDelegate.currentLight - 1
            }

            function setNewColor(color) {
                lightRepeater.lightColor = color
                lightRepeater.colorSet = true
            }
            function setNewIntensity(intensity) {
                lightRepeater.lightIntensity = intensity
                lightRepeater.intensitySet = true
            }
            function setNewDirection(direction) {
                lightRepeater.lightDirection = direction
                lightRepeater.directionSet = true
            }
            function setNewAttenuation(attenuation) {
                lightRepeater.lightAttenuation = attenuation
                lightRepeater.attenuationSet = true
            }
            function setNewCutOffAngle(cutOffAngle) {
                lightRepeater.lightCutOffAngle = cutOffAngle
                lightRepeater.cutOffAngleSet = true
            }
        }
    }
}

