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

Item {
    id: dragHandle
    z: 5
    width: 16
    height: 16
    visible: false

    // handleRect is the visible part by default, but dragging can be initiated by clicking
    // on the invisible margins, too.
    Rectangle {
        id: handleRect
        anchors.fill: parent
        anchors.margins: 4
        color: "#f4be04"
    }
    property int handleType: EditorScene.DragNone
    property point offset: Qt.point(width / 2, height / 2)
    property int handleIndex: 0
    property alias color: handleRect.color

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onPositionChanged: {
            var scenePos = editorViewport.mapFromItem(parent, mouseX, mouseY)
            editorScene.dragHandleMove(scenePos,
                                       mouse.modifiers & Qt.ShiftModifier,
                                       mouse.modifiers & Qt.ControlModifier,
                                       mouse.modifiers & Qt.AltModifier)
        }
        onPressed: {
            var scenePos = editorViewport.mapFromItem(parent, mouseX, mouseY)
            editorScene.dragHandlePress(handleType, scenePos, handleIndex)
        }
        onReleased: {
            editorScene.dragHandleRelease()
        }
    }

    Connections {
        target: editorScene
        onRepositionDragHandle: {
            if (dragMode == dragHandle.handleType && handleIndex == dragHandle.handleIndex) {
                x = pos.x - offset.x
                y = pos.y - offset.y
                dragHandle.visible = visible
            }
        }
    }
}
