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
import Qt3D.Core 2.0
import Qt3D.Render 2.0

ComponentDelegate {
    id: thisDelegate
    componentType: EditorSceneItemComponentsModel.CameraEntity
    title: qsTr("Camera") + editorScene.emptyString

    viewTitleVisible: editorContent.cameraViewVisible

    onChangeViewVisibity: {
        editorContent.cameraViewVisible = viewVisibility
    }

    Component.onCompleted: {
        if (!editorContent.cameraViewVisible)
            height = minimumComponentHeight
    }

    // Need this separate connection as any checked property binding will not persist
    // over user clicking on the radio buttons.
    Connections {
        target: componentData
        onProjectionTypeChanged: {
            if (componentData.projectionType === CameraLens.OrthographicProjection)
                orthoButton.checked = true
            else
                perspectiveButton.checked = true
        }
    }

    function changeProjectionType(newValue) {
        var oldValue = componentData.projectionType;
        if (oldValue !== newValue) {
            editorScene.undoHandler.createChangePropertyCommand(
                        editorContent.selectedEntityName, componentType,
                        "projectionType", newValue, oldValue, true);
        }
    }

    Column {
        spacing: 4
        width: parent.width

        QQC2.GroupBox {
            id: groupBoxControl
            title: qsTr("Projection Type") + editorScene.emptyString

            label: Text {
                width: groupBoxControl.availableWidth
                text: groupBoxControl.title
                color: enabled ? editorContent.textColor : editorContent.disabledTextColor
                font.family: editorContent.labelFontFamily
                font.weight: editorContent.labelFontWeight
                font.pixelSize: editorContent.labelFontPixelSize
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Row {
                StyledRadioButton {
                    id: orthoButton
                    checked: componentData.projectionType === CameraLens.OrthographicProjection
                    onCheckedChanged: {
                        if (checked)
                            changeProjectionType(CameraLens.OrthographicProjection)
                    }
                }
                Rectangle {
                    color: "transparent"
                    width: orthoLabel.width
                    height: orthoButton.height
                    StyledLabel {
                        id: orthoLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Orthographic") + editorScene.emptyString
                        tooltip: qsTr("Use orthographic projection, i.e. camera\nwith no perspective correction.")
                                 + editorScene.emptyString
                    }
                }
                StyledRadioButton {
                    id: perspectiveButton
                    checked: componentData.projectionType === CameraLens.PerspectiveProjection
                    onCheckedChanged: {
                        if (checked)
                            changeProjectionType(CameraLens.PerspectiveProjection)
                    }
                }
                Rectangle {
                    color: "transparent"
                    width: perspectiveLabel.width
                    height: perspectiveButton.height
                    StyledLabel {
                        id: perspectiveLabel
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Perspective") + editorScene.emptyString
                        tooltip: qsTr("Use perspective projection, i.e. camera with\nperspective correction based on field-of-view.")
                                 + editorScene.emptyString
                    }
                }
            }
        }

        FloatPropertyInputField {
            id: nearPlaneField
            label: qsTr("Near Plane") + editorScene.emptyString
            propertyName: "nearPlane"
            minimum: 0.01
            roundDigits: 2
            step: 1 // = 0.01 (because roundDigits is 2)
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Near plane of the camera, i.e. objects closer\nto the camera than this will be cropped.")
                     + editorScene.emptyString
        }

        FloatPropertyInputField {
            id: farPlaneField
            label: qsTr("Far Plane") + editorScene.emptyString
            propertyName: "farPlane"
            minimum: nearPlaneField.fieldValue
            roundDigits: 2
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Far plane of the camera, i.e. objects further\naway from the camera than this will be cropped.")
                     + editorScene.emptyString
        }

        FloatSliderPropertyInputField {
            id: fieldOfViewField
            visible: componentData.projectionType === CameraLens.PerspectiveProjection
            label: qsTr("Field of View") + editorScene.emptyString
            propertyName: "fieldOfView"
            component: componentData
            componentType: thisDelegate.componentType
            minimum: 0
            maximum: 180
            stepSize: 1
            tooltip: qsTr("Field of view (FOV) of the camera.") + editorScene.emptyString
        }

        FloatPropertyInputField {
            id: aspectRatioField
            visible: componentData.projectionType === CameraLens.PerspectiveProjection
            label: qsTr("Aspect Ratio") + editorScene.emptyString
            propertyName: "aspectRatio"
            minimum: 0.1
            roundDigits: 3
            step: 10 // = 0.01 (because roundDigits is 3)
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Aspect ratio, i.e. width to height\nratio of the intended view.")
                     + editorScene.emptyString
        }

        FloatPropertyInputField {
            id: leftField
            label: qsTr("Left") + editorScene.emptyString
            visible: componentData.projectionType === CameraLens.OrthographicProjection
            propertyName: "left"
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Left limit of the view.") + editorScene.emptyString
        }

        FloatPropertyInputField {
            id: rightField
            label: qsTr("Right") + editorScene.emptyString
            visible: componentData.projectionType === CameraLens.OrthographicProjection
            propertyName: "right"
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Right limit of the view.") + editorScene.emptyString
        }

        FloatPropertyInputField {
            id: bottomField
            label: qsTr("Bottom") + editorScene.emptyString
            visible: componentData.projectionType === CameraLens.OrthographicProjection
            propertyName: "bottom"
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Bottom limit of the view.") + editorScene.emptyString
        }

        FloatPropertyInputField {
            id: topField
            label: qsTr("Top") + editorScene.emptyString
            visible: componentData.projectionType === CameraLens.OrthographicProjection
            propertyName: "top"
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Top limit of the view.") + editorScene.emptyString
        }

        Vector3DPropertyInputField {
            id: positionVectorField
            label: qsTr("Position") + editorScene.emptyString
            propertyName: "position"
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Position of the camera\non the %1 axis.") // Needs arg, as it will be replaced in the vector3dinpufield
            tooltipArgs: ["X", "Y", "Z"]
        }

        Vector3DPropertyInputField {
            id: upVectorField
            label: qsTr("Up") + editorScene.emptyString
            propertyName: "upVector"
            component: componentData
            componentType: thisDelegate.componentType
            // TODO: Need to block (0,0,0) value for upvector somehow, it crashes the camera
            tooltip: qsTr("Which way is up for the camera.%1") // Needs arg, as it will be replaced in the vector3dinpufield
        }

        Vector3DPropertyInputField {
            id: viewCenterVectorField
            label: qsTr("View Center") + editorScene.emptyString
            propertyName: "viewCenter"
            component: componentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("The point the camera is\nfacing on the %1 axis.") // Needs arg, as it will be replaced in the vector3dinpufield
            tooltipArgs: ["X", "Y", "Z"]
        }
    }
}

