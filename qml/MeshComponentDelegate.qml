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
    id: meshDelegate
    title: qsTr("Mesh")

    property int currentMesh: 0

    Item {
        width: parent.width
        height: meshCombobox.height + 8

        Component.onCompleted: meshCombobox.currentIndex = meshDelegate.currentMesh - 1

        ComboBox {
            id: meshCombobox
            anchors.right: parent.right
            anchors.rightMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottomMargin: 4
            anchors.verticalCenter: parent.verticalCenter

            model: ListModel {
                ListElement { text: qsTr("Cube") }
                ListElement { text: qsTr("Custom") }
                ListElement { text: qsTr("Cylinder") }
                ListElement { text: qsTr("Plane") }
                ListElement { text: qsTr("Sphere") }
                ListElement { text: qsTr("Torus") }
            }
            onCurrentIndexChanged: {
                if (currentIndex === EditorSceneItemMeshComponentsModel.Cuboid - 1)
                    componentData.model.setMesh(EditorSceneItemMeshComponentsModel.Cuboid)
                else if (currentIndex === EditorSceneItemMeshComponentsModel.Custom - 1)
                    componentData.model.setMesh(EditorSceneItemMeshComponentsModel.Custom)
                else if (currentIndex === EditorSceneItemMeshComponentsModel.Cylinder - 1)
                    componentData.model.setMesh(EditorSceneItemMeshComponentsModel.Cylinder)
                else if (currentIndex === EditorSceneItemMeshComponentsModel.Plane - 1)
                    componentData.model.setMesh(EditorSceneItemMeshComponentsModel.Plane)
                else if (currentIndex === EditorSceneItemMeshComponentsModel.Sphere - 1)
                    componentData.model.setMesh(EditorSceneItemMeshComponentsModel.Sphere)
                else if (currentIndex === EditorSceneItemMeshComponentsModel.Torus - 1)
                    componentData.model.setMesh(EditorSceneItemMeshComponentsModel.Torus)
            }
        }
    }

    Repeater {
        model: componentData.model

        Loader {
            id: meshLoader
            width: parent.width

            function meshTypetoDelegateSource(meshType) {
                meshDelegate.currentMesh = meshType
                if (meshType == EditorSceneItemMeshComponentsModel.Cuboid)
                    return "CuboidMeshDelegate.qml";
                if (meshType == EditorSceneItemMeshComponentsModel.Custom)
                    return "CustomMeshDelegate.qml";
                if (meshType == EditorSceneItemMeshComponentsModel.Cylinder)
                    return "CylinderMeshDelegate.qml";
                if (meshType == EditorSceneItemMeshComponentsModel.Plane)
                    return "PlaneMeshDelegate.qml";
                if (meshType == EditorSceneItemMeshComponentsModel.Sphere)
                    return "SphereMeshDelegate.qml";
                if (meshType == EditorSceneItemMeshComponentsModel.Torus)
                    return "TorusMeshDelegate.qml";

                return "UnknownMeshDelegate.qml";
            }

            source: meshTypetoDelegateSource(meshType)
        }
    }
}

