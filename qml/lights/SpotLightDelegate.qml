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

BasicLightDelegate {
    id: thisItem

    property vector3d attenuation: Qt.vector3d(0.0, 0.0, 0.002)

    Component.onCompleted: {
        initialState = false
        if (parent.repeater.directionSet)
            directionField.component[directionField.propertyName] = parent.repeater.lightDirection
        else
            parent.setNewDirection(directionField.component[directionField.propertyName])
        if (parent.repeater.cutOffAngleSet)
            cutOffAngleField.component[cutOffAngleField.propertyName] = parent.repeater.lightCutOffAngle
        else
            parent.setNewCutOffAngle(cutOffAngleField.component[cutOffAngleField.propertyName])
        if (parent.repeater.attenuationSet) {
            attenuation = parent.repeater.lightAttenuation
            constantAttenuationField.component[constantAttenuationField.propertyName]
                    = attenuation.x
            linearAttenuationField.component[linearAttenuationField.propertyName]
                    = attenuation.y
            quadraticAttenuationField.component[quadraticAttenuationField.propertyName]
                    = attenuation.z
        } else {
            parent.setNewAttenuation(
                        Qt.vector3d(
                            constantAttenuationField.component[constantAttenuationField.propertyName],
                            linearAttenuationField.component[linearAttenuationField.propertyName],
                            quadraticAttenuationField.component[quadraticAttenuationField.propertyName]))
        }
    }

    Vector3DPropertyInputField {
        id: directionField
        parent: inputLayout
        label: qsTr("Direction") + editorScene.emptyString
        propertyName: "localDirection"
        component: lightComponentData
        componentType: thisItem.componentType
        onFieldValueChanged: {
            if (!thisItem.initialState)
                thisItem.parent.setNewDirection(fieldValue)
        }
        tooltip: qsTr("The point the light is\nfacing at on the %1 axis.")
                 + editorScene.emptyString
        tooltipArgs: ["X", "Y", "Z"]
    }

    FloatPropertyInputField {
        id: cutOffAngleField
        parent: inputLayout
        label: qsTr("Cut-Off Angle") + editorScene.emptyString
        propertyName: "cutOffAngle"
        component: lightComponentData
        componentType: thisItem.componentType
        minimum: 1
        onFieldValueChanged: {
            if (!thisItem.initialState)
                thisItem.parent.setNewCutOffAngle(fieldValue)
        }
        tooltip: qsTr("Angle of the spotlight cone.") + editorScene.emptyString
    }

    FloatPropertyInputField {
        id: quadraticAttenuationField
        parent: inputLayout
        label: qsTr("Quadratic Attenuation") + editorScene.emptyString
        propertyName: "quadraticAttenuation"
        component: lightComponentData
        componentType: thisItem.componentType
        roundDigits: 4
        step: 10 // = 0.001
        minimum: 0
        onFieldValueChanged: {
            attenuation.z = fieldValue
            if (!thisItem.initialState)
                thisItem.parent.setNewAttenuation(attenuation)
        }
        tooltip: quadraticAttenuationTooltip
    }

    FloatPropertyInputField {
        id: linearAttenuationField
        parent: inputLayout
        label: qsTr("Linear Attenuation") + editorScene.emptyString
        propertyName: "linearAttenuation"
        component: lightComponentData
        componentType: thisItem.componentType
        roundDigits: 4
        step: 10 // = 0.001
        minimum: 0
        onFieldValueChanged: {
            attenuation.y = fieldValue
            if (!thisItem.initialState)
                thisItem.parent.setNewAttenuation(attenuation)
        }
        tooltip: linearAttenuationTooltip
    }

    FloatPropertyInputField {
        id: constantAttenuationField
        parent: inputLayout
        label: qsTr("Constant Attenuation") + editorScene.emptyString
        propertyName: "constantAttenuation"
        component: lightComponentData
        componentType: thisItem.componentType
        roundDigits: 4
        step: 10 // = 0.001
        minimum: 0
        onFieldValueChanged: {
            attenuation.x = fieldValue
            if (!thisItem.initialState)
                thisItem.parent.setNewAttenuation(attenuation)
        }
        tooltip: constantAttenuationTooltip
    }
}

