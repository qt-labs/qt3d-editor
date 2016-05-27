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
import QtQuick.Layouts 1.2

Item {
    id: thisItem
    width: parent.width
    height: columnLayout.y + columnLayout.height + 8

    property int componentType: EditorSceneItemComponentsModel.Light
    property bool initialState: true
    property alias inputLayout: columnLayout

    property string constantAttenuationTooltip: qsTr("Constant attenuation of the light, i.e.\nthe intensity of the light will be\nunaffected by distance.")
                                                + editorScene.emptyString
    property string linearAttenuationTooltip: qsTr("Linear attenuation of the light, i.e.\nthe light intensity will diminish at a\nfixed rate as it travels from its source.")
                                              + editorScene.emptyString
    property string quadraticAttenuationTooltip: qsTr("Quadratic attenuation of the light, i.e.\nthe further the light travels from its source,\nthe more it will be diminished.")
                                                 + editorScene.emptyString

    Component.onCompleted: {
        initialState = false
        if (parent.repeater.colorSet)
            colorField.component[colorField.propertyName] = parent.repeater.lightColor
        else
            parent.setNewColor(colorField.component[colorField.propertyName])
        if (parent.repeater.intensitySet)
            intensityField.component[intensityField.propertyName] = parent.repeater.lightIntensity
        else
            parent.setNewIntensity(intensityField.component[intensityField.propertyName])
    }

    Column {
        id: columnLayout
        spacing: 4
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8

        ColorPropertyInputField {
            id: colorField
            label: qsTr("Color") + editorScene.emptyString
            propertyName: "color"
            component: lightComponentData
            componentType: thisItem.componentType
            onColorValueChanged: {
                if (!thisItem.initialState)
                    thisItem.parent.setNewColor(colorValue)
            }
            tooltip: qsTr("The color of the light.") + editorScene.emptyString
        }

        FloatPropertyInputField {
            id: intensityField
            label: qsTr("Intensity") + editorScene.emptyString
            propertyName: "intensity"
            component: lightComponentData
            componentType: thisItem.componentType
            roundDigits: 1
            step: 1 // = 0.1
            minimum: 0
            onFieldValueChanged: {
                if (!thisItem.initialState)
                    thisItem.parent.setNewIntensity(fieldValue)
            }
            tooltip: qsTr("Light intensity, i.e. the brightness.") + editorScene.emptyString
        }
    }
}
