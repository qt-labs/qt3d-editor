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
import QtQml.Models 2.2
import com.theqtcompany.SceneEditor3D 1.0

Menu {
    property bool isPopup: false

    title: qsTr("Add Entity (A)") + editorScene.emptyString

    MenuItem {
        text: qsTr("Add Entity") + editorScene.emptyString
        enabled: false
        visible: isPopup
    }
    MenuSeparator {
        visible: isPopup
    }

    MenuItem {
        text: qsTr("Mesh") + editorScene.emptyString
        iconSource: "images/mesh.png"
        enabled: !entityTree.view.cameraSelected
        onTriggered: {
            entityTree.addNewEntity(EditorUtils.CuboidEntity)
        }
    }
    MenuItem {
        text: qsTr("Camera") + editorScene.emptyString
        iconSource: "images/camera.png"
        enabled: entityTree.view.sceneRootSelected
        onTriggered: {
            entityTree.addNewEntity(EditorUtils.CameraEntity)
        }
    }
    MenuItem {
        text: qsTr("Light") + editorScene.emptyString
        iconSource: "images/light.png"
        enabled: !entityTree.view.cameraSelected
        onTriggered: {
            entityTree.addNewEntity(EditorUtils.LightEntity)
        }
    }
    MenuItem {
        text: qsTr("Group") + editorScene.emptyString
        iconSource: "images/group.png"
        enabled: entityTree.view.groupSelected || entityTree.view.sceneRootSelected
        onTriggered: {
            entityTree.addNewEntity(EditorUtils.GroupEntity)
        }
    }
}

