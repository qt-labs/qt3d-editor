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
    width: 16
    height: 16
    visible: false
    opacity: dragHandleMouseArea.containsMouse || dragging ? 1.0 : 0.4

    // handleRect is the visible part by default, but dragging can be initiated by clicking
    // on the invisible margins, too.
    Rectangle {
        id: handleRect
        anchors.fill: parent
        anchors.margins: 4
        color: editorContent.selectionColor
    }

    property int handleType: EditorScene.DragNone
    property point offset: Qt.point(width / 2, height / 2)
    property int handleIndex: 0
    property alias color: handleRect.color
    property alias radius: handleRect.radius
    property bool dragging: false
    property real baseZ: 2

    MouseArea {
        id: dragHandleMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        onPositionChanged: {
            var scenePos = editorViewport.mapFromItem(parent, mouseX, mouseY)
            if (mouse.buttons & Qt.LeftButton) {
                editorScene.dragHandleMove(scenePos,
                                           mouse.modifiers & Qt.ShiftModifier,
                                           mouse.modifiers & Qt.ControlModifier,
                                           mouse.modifiers & Qt.AltModifier)
            }
            editorScene.updateWorldPositionLabelToDragHandle(handleType, handleIndex)
        }
        onPressed: {
            entityTree.focusTree()
            var scenePos = editorViewport.mapFromItem(parent, mouseX, mouseY)
            editorScene.dragHandlePress(handleType, scenePos, handleIndex)
            dragHandle.dragging = true
            editorScene.updateWorldPositionLabelToDragHandle(handleType, handleIndex)
        }
        onReleased: {
            editorScene.updateWorldPositionLabelToDragHandle(handleType, handleIndex)
            editorScene.dragHandleRelease()
            dragHandle.dragging = false
            var scenePos = editorViewport.mapFromItem(parent, mouseX, mouseY)
            editorScene.updateWorldPositionLabel(scenePos.x, scenePos.y)
        }
        onCanceled: {
            editorScene.dragHandleRelease()
            dragHandle.dragging = false
            editorScene.updateWorldPositionLabel(-1, -1)
        }
    }

    Connections {
        target: editorScene
        onRepositionDragHandle: {
            if (dragMode == dragHandle.handleType && handleIndex == dragHandle.handleIndex) {
                x = pos.x - offset.x
                y = pos.y - offset.y
                dragHandle.visible = visible
                z = baseZ + handleZ
            }
        }
        onBeginDragHandlesRepositioning: {
            // Repositioning multiple handles can cause two or more handles to overlap and get
            // "containsMouse" state simultaneously, if they have hover enabled, so disable it
            // for the duration of the handle repositioning.
            dragHandleMouseArea.hoverEnabled = false
        }
        onEndDragHandlesRepositioning: {
            dragHandleMouseArea.hoverEnabled = true
        }
    }
}
