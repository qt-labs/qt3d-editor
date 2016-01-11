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
import com.theqtcompany.Editor3d 1.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2
import Qt3D.Render 2.0

ComponentDelegate {
    title: qsTr("Light")

    Repeater {
        model: componentData.model

        Loader {
            id: lightLoader
            width: parent.width

            function lightTypetoDelegateSource(lightType) {
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

    Menu {
        id: selectLightMenu
        title: qsTr("Select Light")

        MenuItem {
            text: qsTr("Basic")
            iconSource: "qrc:/images/light_basic.png"
            onTriggered: {
                componentData.model.setLight(EditorSceneItemLightComponentsModel.Basic)
            }
        }
        MenuItem {
            text: qsTr("Directional")
            iconSource: "qrc:/images/light_directional.png"
            onTriggered: {
                componentData.model.setLight(EditorSceneItemLightComponentsModel.Directional)
            }
        }
        MenuItem {
            text: qsTr("Point")
            iconSource: "qrc:/images/light_point.png"
            onTriggered: {
                componentData.model.setLight(EditorSceneItemLightComponentsModel.Point)
            }
        }
        MenuItem {
            text: qsTr("Spot")
            iconSource: "qrc:/images/light_spot.png"
            onTriggered: {
                componentData.model.setLight(EditorSceneItemLightComponentsModel.Spot)
            }
        }
    }

    Button {
        id: changeLightButton
        text: qsTr("Change Light")
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            selectLightMenu.popup()
        }
    }
}

