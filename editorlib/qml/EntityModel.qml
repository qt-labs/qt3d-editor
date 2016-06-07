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

ListModel {
    property string language: editorContent.systemLanguage

    function retranslateUi() {
        setProperty(0, "meshString", qsTr("Cube"))
        setProperty(1, "meshString", qsTr("Cylinder"))
        setProperty(2, "meshString", qsTr("Plane"))
        setProperty(3, "meshString", qsTr("Sphere"))
        setProperty(4, "meshString", qsTr("Torus"))
        setProperty(5, "meshString", qsTr("Custom"))
        setProperty(6, "meshString", qsTr("Camera"))
        setProperty(7, "meshString", qsTr("Light"))
        setProperty(8, "meshString", qsTr("Group"))
    }

    Component.onCompleted: {
        retranslateUi()
    }

    onLanguageChanged: {
        retranslateUi()
    }

    ListElement {
        meshType: EditorUtils.CuboidEntity
        meshImage: "images/mesh_cuboid_large.png"
        meshDragImage: "images/mesh_cuboid_large.png"
    }
    ListElement {
        meshType: EditorUtils.CylinderEntity
        meshImage: "images/mesh_cylinder_large.png"
        meshDragImage: "images/mesh_cylinder_large.png"
    }
    ListElement {
        meshType: EditorUtils.PlaneEntity
        meshImage: "images/mesh_plane_large.png"
        meshDragImage: "images/mesh_plane_large.png"
    }
    ListElement {
        meshType: EditorUtils.SphereEntity
        meshImage: "images/mesh_sphere_large.png"
        meshDragImage: "images/mesh_sphere_large.png"
    }
    ListElement {
        meshType: EditorUtils.TorusEntity
        meshImage: "images/mesh_torus_large.png"
        meshDragImage: "images/mesh_torus_large.png"
    }
    ListElement {
        meshType: EditorUtils.CustomEntity
        meshImage: "images/mesh_custom_large.png"
        meshDragImage: "images/mesh_custom_large.png"
    }
    ListElement {
        meshType: EditorUtils.CameraEntity
        meshImage: "images/camera_large.png"
        meshDragImage: "images/camera_large.png"
    }
    ListElement {
        meshType: EditorUtils.LightEntity
        meshImage: "images/light_large.png"
        meshDragImage: "images/light_large.png"
    }
    ListElement {
        meshType: EditorUtils.GroupEntity
        meshImage: "images/group_large.png"
        meshDragImage: "images/group_large.png"
    }
}
