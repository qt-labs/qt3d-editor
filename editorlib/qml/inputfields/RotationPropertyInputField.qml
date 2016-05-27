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

PropertyInputField {
    id: rotationPropertyInputField
    width: parent.width
    height: sliderInputfield.height + vectorInputField.height + vectorInputField.anchors.topMargin

    property alias floatLabel: sliderInputfield.label
    property alias stepSize: sliderInputfield.stepSize
    property alias minimum: sliderInputfield.minimum
    property alias maximum: sliderInputfield.maximum
    property alias vectorLabel: vectorInputField.label
    property double angle: 0
    property vector3d axis: Qt.vector3d(0, 0, 0)
    property bool calculatingAxisAndAngle: false
    property bool blockValueChanges: false
    property bool lockedField: false
    property alias tooltip: sliderInputfield.tooltip

    onCalculatingAxisAndAngleChanged: {
        if (!calculatingAxisAndAngle) {
            blockValueChanges = true
            sliderInputfield.value = angle
            vectorInputField.value = axis
            blockValueChanges = false
        }
    }

    onComponentValueChanged: {
        if (!blockValueChanges && component !== null)
            toAxisAndAngle(component[propertyName])
    }

    function handleRotationChangeFinished() {
        blockValueChanges = true
        handleEditingFinished(component.fromAxisAndAngle(axis, angle))
        blockValueChanges = false
    }

    Vector3DInputField {
        id: vectorInputField
        lockProperty: rotationPropertyInputField.propertyName + editorScene.lockPropertySuffix
        lockComponent: rotationPropertyInputField.component
        label: qsTr("Rotate") + editorScene.emptyString
        onValueEdited: {
            axis = value
            handleRotationChangeFinished()
        }
        onLockedChanged: sliderInputfield.locked = locked
        enabled: !lockedField
        tooltip: qsTr("The rotation factor of the\nobject on the %1 axis.")
                 + editorScene.emptyString
        tooltipArgs: ["X", "Y", "Z"]
    }

    FloatSliderInputField {
        id: sliderInputfield
        lockProperty: rotationPropertyInputField.propertyName + editorScene.lockPropertySuffix
        lockComponent: rotationPropertyInputField.component
        value: 0
        label: qsTr("Rotation Angle") + editorScene.emptyString
        anchors.top: vectorInputField.bottom
        anchors.topMargin: 4
        roundDigits: 2
        onValueChanged: {
            if (!blockValueChanges) {
                angle = value
                handleRotationChangeFinished()
            }
        }
        onLockedChanged: vectorInputField.locked = locked
        enabled: !lockedField
    }

    function toAxisAndAngle(quat) {
        // TODO: Undo/redo restores axis to a state where the longest dimension is exactly 1.
        // TODO: E.g. axis (2,0,1) becomes (1,0,0.5)
        // TODO: We need some extra magic if we want to restore the actual axis in undo/redo
        calculatingAxisAndAngle = true
        angle = Math.acos(quat.scalar) * (360 / Math.PI)
        if (angle !== 0) {
            // Let's not update axis if angle is zero, as it will be 0,0,0
            var s = Math.sqrt(1.0 - quat.scalar * quat.scalar)
            if (s < 0.001) {
                // if s is close to zero then direction of axis is not important
                axis = Qt.vector3d(quat.x, quat.y, quat.z)
            } else {
                axis = Qt.vector3d(quat.x / s, quat.y / s, quat.z / s)
            }
            // quaternion was normalized in Qt3D, so axis may not be the same as it was when set.
            // -> normalize axis
            var divider = Math.max(Math.abs(axis.x), Math.max(Math.abs(axis.y), Math.abs(axis.z)))
            if (divider !== 0) {
                axis.x /= divider
                axis.y /= divider
                axis.z /= divider
            }
        }

        calculatingAxisAndAngle = false
    }
}
