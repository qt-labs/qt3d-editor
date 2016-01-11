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
import QtQuick.Controls 1.4

Menu {
    title: qsTr("Add Component")

    MenuItem {
        text: selectedEntityName
        enabled: false
    }

    MenuSeparator {}

    // TODO: disable types that cannot be added to certain entity?

    // Custom camera entities via CameraLens not supported, only QCamera entities are
//    MenuItem {
//        text: qsTr("Camera Lens")
//        iconSource: "qrc:/images/camera.png"
//        onTriggered: {
//            componentPropertiesView.model.appendNewComponent(sceneModel.CameraLens)
//        }
//    }
    // Adding frame graphs is not supported
//    MenuItem {
//        text: qsTr("Frame Graph")
//        iconSource: "qrc:/images/cross.png" // TODO: Missing its own icon
//        onTriggered: {
//            componentPropertiesView.model.appendNewComponent(sceneModel.FrameGraph)
//        }
//    }
    MenuItem {
        text: qsTr("Keyboard Input")
        iconSource: "qrc:/images/keyboard_input.png"
        onTriggered: {
            componentPropertiesView.model.appendNewComponent(sceneModel.KeyboardInput)
        }
    }
    MenuItem {
        text: qsTr("Layer")
        iconSource: "qrc:/images/layer.png"
        onTriggered: {
            componentPropertiesView.model.appendNewComponent(sceneModel.Layer)
        }
    }
    // Adding material, mesh, transform, or light to an existing entity is not supported
//    MenuItem {
//        text: qsTr("Light")
//        iconSource: "qrc:/images/light.png"
//        onTriggered: {
//            componentPropertiesView.model.appendNewComponent(sceneModel.Light)
//        }
//    }
    MenuItem {
        text: qsTr("Logic")
        iconSource: "qrc:/images/logic.png"
        onTriggered: {
            componentPropertiesView.model.appendNewComponent(sceneModel.Logic)
        }
    }
//    MenuItem {
//        text: qsTr("Material")
//        iconSource: "qrc:/images/material.png"
//        onTriggered: {
//            componentPropertiesView.model.appendNewComponent(sceneModel.Material)
//        }
//    }
//    MenuItem {
//        text: qsTr("Mesh")
//        iconSource: "qrc:/images/mesh.png"
//        onTriggered: {
//            componentPropertiesView.model.appendNewComponent(sceneModel.Mesh)
//        }
//    }
    MenuItem {
        text: qsTr("Mouse Input")
        iconSource: "qrc:/images/cross.png" // TODO: Missing its own icon
        onTriggered: {
            // TODO: Crashes, investigate
            //componentPropertiesView.model.appendNewComponent(sceneModel.MouseInput)
        }
    }
    MenuItem {
        text: qsTr("Object Picker")
        iconSource: "qrc:/images/picker.png"
        onTriggered: {
            componentPropertiesView.model.appendNewComponent(sceneModel.ObjectPicker)
        }
    }
//    MenuItem {
//        text: qsTr("Transform")
//        iconSource: "qrc:/images/transform.png"
//        onTriggered: {
//            componentPropertiesView.model.appendNewComponent(sceneModel.Transform)
//        }
//    }
}
