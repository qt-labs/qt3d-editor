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
import QtQuick.Layouts 1.2

ComponentDelegate {
    id: lightDelegate
    title: qsTr("Light")

    property int currentLight: 0

    viewTitleVisible: lightViewVisible

    onChangeViewVisibity: {
        lightViewVisible = viewVisibility
    }

    Component.onCompleted: {
        if (!lightViewVisible)
            height = minimumComponentHeight
    }

    Item {
        id: comboboxItem
        width: parent.width
        height: lightCombobox.height + 8

        Component.onCompleted: lightCombobox.currentIndex = lightDelegate.currentLight - 1

        ComboBox {
            id: lightCombobox
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottomMargin: 4
            anchors.verticalCenter: parent.verticalCenter

            model: ListModel {
                ListElement { text: qsTr("Directional Light") }
                ListElement { text: qsTr("Point Light") }
                ListElement { text: qsTr("Spot Light") }
                ListElement { text: qsTr("Basic Light") }
            }
            onCurrentIndexChanged: {
                if (currentIndex === EditorSceneItemLightComponentsModel.Directional - 1)
                    componentData.model.setLight(EditorSceneItemLightComponentsModel.Directional)
                else if (currentIndex === EditorSceneItemLightComponentsModel.Point - 1)
                    componentData.model.setLight(EditorSceneItemLightComponentsModel.Point)
                else if (currentIndex === EditorSceneItemLightComponentsModel.Spot - 1)
                    componentData.model.setLight(EditorSceneItemLightComponentsModel.Spot)
                else if (currentIndex === EditorSceneItemLightComponentsModel.Basic - 1)
                    componentData.model.setLight(EditorSceneItemLightComponentsModel.Basic)
            }
        }
    }

    Repeater {
        id: lightRepeater
        model: componentData.model

        Loader {
            id: lightLoader
            width: parent.width

            function lightTypetoDelegateSource(lightType) {
                lightDelegate.currentLight = lightType
                if (lightType == EditorSceneItemLightComponentsModel.Basic)
                    return "BasicLightDelegate.qml";
                if (lightType == EditorSceneItemLightComponentsModel.Directional)
                    return "DirectionalLightDelegate.qml";
                if (lightType == EditorSceneItemLightComponentsModel.Point)
                    return "PointLightDelegate.qml";
                if (lightType == EditorSceneItemLightComponentsModel.Spot)
                    return "SpotLightDelegate.qml";

                return "UnknownLightDelegate.qml";
            }

            source: lightTypetoDelegateSource(lightType)
        }
    }
}

