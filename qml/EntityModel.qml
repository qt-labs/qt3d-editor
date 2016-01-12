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
import com.theqtcompany.Editor3d 1.0

ListModel {
    ListElement {
        meshType: EditorSceneItemModel.CuboidEntity
        meshString: qsTr("Cube")
        meshImage: "images/mesh_cuboid-large.png"
        meshDragImage: "images/mesh_cuboid-large.png"
    }
    ListElement {
        meshType: EditorSceneItemModel.CylinderEntity
        meshString: qsTr("Cylinder")
        meshImage: "images/mesh_cylinder-large.png"
        meshDragImage: "images/mesh_cylinder-large.png"
    }
    ListElement {
        meshType: EditorSceneItemModel.PlaneEntity
        meshString: qsTr("Plane")
        meshImage: "images/mesh_plane-large.png"
        meshDragImage: "images/mesh_plane-large.png"
    }
    ListElement {
        meshType: EditorSceneItemModel.SphereEntity
        meshString: qsTr("Sphere")
        meshImage: "images/mesh_sphere-large.png"
        meshDragImage: "images/mesh_sphere-large.png"
    }
    ListElement {
        meshType: EditorSceneItemModel.TorusEntity
        meshString: qsTr("Torus")
        meshImage: "images/mesh_torus-large.png"
        meshDragImage: "images/mesh_torus-large.png"
    }
    ListElement {
        meshType: EditorSceneItemModel.CustomEntity
        meshString: qsTr("Custom")
        meshImage: "images/mesh_custom-large.png"
        meshDragImage: "images/mesh_custom-large.png"
    }
    ListElement {
        meshType: EditorSceneItemModel.CameraEntity
        meshString: qsTr("Camera")
        meshImage: "images/camera-large.png"
        meshDragImage: "images/camera-large.png"
    }
    ListElement {
        meshType: EditorSceneItemModel.LightEntity
        meshString: qsTr("Light")
        meshImage: "images/light-large.png"
        meshDragImage: "images/light-large.png"
    }
}
