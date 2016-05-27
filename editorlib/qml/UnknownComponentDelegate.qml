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

ComponentDelegate {
    function componentTypeToString(componentType) {
        if (componentType == EditorSceneItemComponentsModel.Transform)
            return qsTr("Transform") + editorScene.emptyString
        if (componentType == EditorSceneItemComponentsModel.Mesh)
            return qsTr("Mesh") + editorScene.emptyString
        if (componentType == EditorSceneItemComponentsModel.Material)
            return qsTr("Material") + editorScene.emptyString
//        if (componentType == EditorSceneItemComponentsModel.CameraLens)
//            return qsTr("Camera lens") + editorScene.emptyString
//        if (componentType == EditorSceneItemComponentsModel.FrameGraph)
//            return qsTr("Frame graph") + editorScene.emptyString
        if (componentType == EditorSceneItemComponentsModel.Layer)
            return qsTr("Layer") + editorScene.emptyString

        return qsTr("Unknown") + editorScene.emptyString
    }

    title: componentTypeToString(type)

//    Menu {
//        id: addComponentMenu
//        title: qsTr("Add Component") + editorScene.emptyString

//        MenuItem {
//            text: qsTr("Transform") + editorScene.emptyString
//            onTriggered: {
//                componentData.model.appendNewComponent(EditorSceneItemComponentsModel.Transform)
//            }
//        }
//        MenuItem {
//            text: qsTr("Mesh") + editorScene.emptyString
//            onTriggered: {
//                componentData.model.appendNewComponent(EditorSceneItemComponentsModel.Mesh)
//            }
//        }
//        MenuItem {
//            text: qsTr("Material") + editorScene.emptyString
//            onTriggered: {
//                componentData.model.appendNewComponent(EditorSceneItemComponentsModel.Material)
//            }
//        }
//        MenuItem {
//            text: qsTr("Camera Lens") + editorScene.emptyString
//            onTriggered: {
//                componentData.model.appendNewComponent(EditorSceneItemComponentsModel.CameraLens)
//            }
//        }
//        MenuItem {
//            text: qsTr("Frame Graph") + editorScene.emptyString
//            onTriggered: {
//                componentData.model.appendNewComponent(EditorSceneItemComponentsModel.FrameGraph)
//            }
//        }
//        MenuItem {
//            text: qsTr("Layer") + editorScene.emptyString
//            onTriggered: {
//                componentData.model.appendNewComponent(EditorSceneItemComponentsModel.Layer)
//            }
//        }
//    }

//    Button {
//        id: addComponentButton
//        text: qsTr("Add Component") + editorScene.emptyString
//        anchors.horizontalCenter: parent.horizontalCenter
//        onClicked: {
//            addComponentMenu.popup();
//        }
//    }
}

