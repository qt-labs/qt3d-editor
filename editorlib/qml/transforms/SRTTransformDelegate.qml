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

TransformDelegate {
    id: thisDelegate
    componentType: EditorSceneItemComponentsModel.Transform
    editable: false
    enabledField: parent.enabledFields

    Vector3DPropertyInputField {
        label: qsTr("Scale") + editorScene.emptyString
        propertyName: "scale3D"
        component: transformComponentData
        componentType: thisDelegate.componentType
        lockedField: enabledField
        visible: editorContent.selectedEntityType !== EditorSceneItem.Light
        tooltip: qsTr("The scale of the object\non the %1 axis.")
                 + editorScene.emptyString
        tooltipArgs: ["X", "Y", "Z"]
    }

    RotationPropertyInputField {
        propertyName: "rotation"
        component: transformComponentData
        componentType: thisDelegate.componentType
        stepSize: 1
        minimum: 0
        maximum: 359
        lockedField: enabledField
        visible: editorContent.selectedEntityType !== EditorSceneItem.Light
        tooltip: qsTr("Angle of the rotation. Is applied\nto axes that have non-zero value.")
                 + editorScene.emptyString
    }

    Vector3DPropertyInputField {
        label: qsTr("Translate") + editorScene.emptyString
        propertyName: "translation"
        component: transformComponentData
        componentType: thisDelegate.componentType
        lockedField: enabledField
        tooltip: qsTr("The position of the object\non the %1 axis.")
                 + editorScene.emptyString
        tooltipArgs: ["X", "Y", "Z"]
    }
}
