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
import QtQuick.Layouts 1.1
import Qt3D.Core 2.0

ComponentDelegate {
    id: thisDelegate
    componentType: EditorSceneItemComponentsModel.CameraLens
    title: qsTr("Camera Lens")

    // TODO: This component is not supported. Should probably be removed

//    // TODO: Undo support for this field
//    GroupBox {
//        title: "Projection Type"

//        RowLayout {
//            ExclusiveGroup { id: projectionTypeGroup }
//            RadioButton {
//                text: qsTr("Orthographic")
//                checked: componentData.projectionType === CameraLens.OrthographicProjection ? true : false
//                exclusiveGroup: projectionTypeGroup
//                onCheckedChanged: {
//                    if (checked) {
//                        componentData.projectionType = CameraLens.OrthographicProjection;
//                    }
//                }

//            }
//            RadioButton {
//                text: qsTr("Perspective")
//                checked: componentData.projectionType === CameraLens.PerspectiveProjection ? true : false
//                exclusiveGroup: projectionTypeGroup
//                onCheckedChanged: {
//                    if (checked) {
//                        componentData.projectionType = CameraLens.PerspectiveProjection;
//                    }
//                }
//            }
//        }
//    }

//    // TODO: Undo support for this field
//    FloatPropertyInputField {
//        id: nearPlaneField
//        label: qsTr("Near Plane")
//        value: componentData.nearPlane
//        onValueChanged: {
//            componentData.nearPlane = value
//        }
//    }

//    // TODO: Undo support for this field
//    FloatPropertyInputField {
//        id: farPlaneField
//        label: qsTr("Far Plane")
//        value: componentData.farPlane
//        onValueChanged: {
//            componentData.farPlane = value
//        }
//    }

//    // TODO: Undo support for this field
//    FloatSliderInputField {
//        id: fieldOfViewField
//        visible: componentData.projectionType === CameraLens.PerspectiveProjection

//        label: qsTr("Field of View")
//        value: componentData.fieldOfView
//        minimum: 0
//        maximum: 180

//        onValueChanged: {
//            componentData.fieldOfView = value
//        }
//    }

//    // TODO: Undo support for this field
//    FloatPropertyInputField {
//        id: aspectRatioField
//        label: qsTr("Aspect Ratio")
//        value: componentData.aspectRatio
//        onValueChanged: {
//            componentData.aspectRatio = value
//        }
//    }

//    // TODO: Undo support for this field
//    FloatPropertyInputField {
//        id: leftField
//        label: qsTr("Left")
//        visible: componentData.projectionType === CameraLens.OrthographicProjection
//        value: componentData.left
//        onValueChanged: {
//            componentData.left = value
//        }
//    }

//    // TODO: Undo support for this field
//    FloatPropertyInputField {
//        id: rightField
//        label: qsTr("Right")
//        visible: componentData.projectionType === CameraLens.OrthographicProjection
//        value: componentData.right
//        onValueChanged: {
//            componentData.right = value
//        }
//    }

//    // TODO: Undo support for this field
//    FloatPropertyInputField {
//        id: bottomField
//        label: qsTr("Bottom")
//        visible: componentData.projectionType === CameraLens.OrthographicProjection
//        value: componentData.bottom
//        onValueChanged: {
//            componentData.bottom = value
//        }
//    }

//    // TODO: Undo support for this field
//    FloatPropertyInputField {
//        id: topField
//        label: qsTr("Top")
//        visible: componentData.projectionType === CameraLens.OrthographicProjection
//        value: componentData.top
//        onValueChanged: {
//            componentData.top = value
//        }
//    }
}

