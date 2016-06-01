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
    title: qsTr("Add Component") + editorScene.emptyString

    MenuItem {
        text: selectedEntityName
        enabled: false
    }

    MenuSeparator {}

    MenuItem {
        text: qsTr("Object Picker") + editorScene.emptyString
        iconSource: "images/picker.png"
        enabled: !multiSelect
        onTriggered: {
            componentPropertiesView.model.appendNewComponent(sceneModel.ObjectPicker)
        }
    }

    MenuSeparator {}

    EntityMenu {
        iconSource: "images/plus.png"
        enabled: !multiSelect && !entityTreeView.cameraSelected
    }
    MenuItem {
        text: qsTr("Remove") + editorScene.emptyString
        enabled: !entityTreeView.sceneRootSelected
        iconSource: "images/minus.png"
        onTriggered: {
            entityTreeView.editing = false
            if (multiSelect) {
                // Handle multiselection removal
                editorScene.undoHandler.beginMacro(text)
                for (var i = 0; i < selectionList.length; ++i)
                    editorScene.undoHandler.createRemoveEntityCommand(selectionList[i])
                editorScene.undoHandler.endMacro()
            } else {
                // Doublecheck that we don't try to remove the scene root
                if (entityTreeView.selection.currentIndex !== editorScene.sceneModel.sceneEntityIndex())
                    editorScene.undoHandler.createRemoveEntityCommand(selectedEntityName)
            }
        }
    }
    MenuItem {
        text: qsTr("Duplicate") + editorScene.emptyString
        enabled: !entityTreeView.sceneRootSelected
        iconSource: "images/duplicate.png"
        onTriggered: {
            if (multiSelect) {
                var duplicateList = selectionList // Copy list, as the original is emptied on insertEntity
                // Handle multiselection duplication
                editorScene.undoHandler.beginMacro(text)
                for (var i = 0; i < duplicateList.length; ++i)
                    editorScene.undoHandler.createDuplicateEntityCommand(duplicateList[i])
                editorScene.undoHandler.endMacro()
            } else {
                var currentSelection = selectedEntity.entity()
                editorScene.undoHandler.createDuplicateEntityCommand(selectedEntityName)
                editorScene.restoreSelection(currentSelection)
            }
        }
    }
    MenuItem {
        text: qsTr("Reset") + editorScene.emptyString
        iconSource: "images/reset_all.png"
        enabled: !multiSelect && !entityTreeView.sceneRootSelected
        onTriggered: {
            editorScene.undoHandler.createResetEntityCommand(selectedEntityName)
        }
    }
    MenuItem {
        text: qsTr("Reset Transform") + editorScene.emptyString
        iconSource: "images/reset.png"
        enabled: !multiSelect && !entityTreeView.sceneRootSelected
                 && !entityTreeView.cameraSelected
        onTriggered: {
            editorScene.undoHandler.createResetTransformCommand(selectedEntityName)
        }
    }
    MenuItem {
        text: qsTr("Group Selected") + editorScene.emptyString
        enabled: multiSelect && !multiSelectedCamera
        iconSource: "images/group_large.png" // TODO: add a new icon, or a small version of group icon
        onTriggered: {
            editorScene.undoHandler.beginMacro(text)
            var reparentList = selectionList // Copy list, as the original is emptied on insertEntity
            // Add new group
            // TODO: Allow creating groups under other entities?
            editorScene.undoHandler.createInsertEntityCommand(1, editorScene.sceneRootName(),
                                                              Qt.vector3d(0, 0, 0))
            var index = editorScene.sceneModel.lastInsertedIndex()
            var groupName = editorScene.sceneModel.entityName(index)
            // Move selected entities under the added group
            for (var i = 0; i < reparentList.length; ++i)
                editorScene.undoHandler.createReparentEntityCommand(groupName, reparentList[i])
            editorScene.undoHandler.endMacro()
            // Clear selection
            entityTree.multiSelect = false
            editorScene.multiSelection = []
            // Select the added group
            editorScene.selectIndex(index)
        }
    }
}
