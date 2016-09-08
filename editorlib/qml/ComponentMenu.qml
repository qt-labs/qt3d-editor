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
import com.theqtcompany.SceneEditor3D 1.0

Menu {
    title: qsTr("Add Component") + editorScene.emptyString

    MenuItem {
        text: editorContent.selectedEntityName
        enabled: false
    }

    MenuSeparator {}

    MenuItem {
        enabled: editorScene.freeView
        text: qsTr("Add scene camera here") + editorScene.emptyString
        onTriggered: {
            entityTree.selectSceneRoot()
            editorScene.undoHandler.beginMacro(text)
            entityTree.addNewEntity(EditorUtils.CameraEntity)
            // When a new camera is added, it is automatically selected
            editorScene.undoHandler.createCopyCameraPropertiesCommand(
                        editorContent.selectedEntityName);
            editorScene.undoHandler.endMacro()
        }
    }
    MenuItem {
        enabled: editorScene.freeView
        text: qsTr("Move active camera here") + editorScene.emptyString
        onTriggered: {
            editorScene.undoHandler.createCopyCameraPropertiesCommand(
                        editorScene.cameraName(editorScene.activeSceneCameraIndex),
                        "", text);
        }
    }

    MenuSeparator {}

    MenuItem {
        text: qsTr("Object Picker") + editorScene.emptyString
        iconSource: "images/picker.png"
        enabled: !editorScene.multiSelection
        onTriggered: {
            componentPropertiesView.model.appendNewComponent(EditorSceneItemComponentsModel.ObjectPicker)
        }
    }

    MenuSeparator {}

    EntityMenu {
        iconSource: "images/plus.png"
        enabled: !editorScene.multiSelection && !entityTreeView.cameraSelected
    }
    MenuItem {
        text: qsTr("Remove") + editorScene.emptyString
        enabled: !entityTreeView.sceneRootSelected
        iconSource: "images/minus.png"
        onTriggered: {
            if (editorScene.multiSelection) {
                // Handle multiselection removal
                editorScene.undoHandler.beginMacro(text)
                var removeList = editorScene.sceneModel.parentList(editorScene.multiSelectionList)
                for (var i = 0; i < removeList.length; ++i)
                    editorScene.undoHandler.createRemoveEntityCommand(removeList[i])
                editorScene.undoHandler.endMacro()
                entityTree.selectSceneRoot()
            } else {
                // Doublecheck that we don't try to remove the scene root
                if (entityTreeView.selection.currentIndex !== editorScene.sceneModel.sceneEntityIndex())
                    editorScene.undoHandler.createRemoveEntityCommand(editorContent.selectedEntityName)
            }
        }
    }
    MenuItem {
        text: qsTr("Duplicate") + editorScene.emptyString
        enabled: !entityTreeView.sceneRootSelected
        iconSource: "images/duplicate.png"
        onTriggered: {
            if (editorScene.multiSelection) {
                // Handle multiselection duplication
                editorScene.undoHandler.beginMacro(text)
                var duplicateList = editorScene.sceneModel.parentList(editorScene.multiSelectionList)
                for (var i = 0; i < duplicateList.length; ++i)
                    editorScene.undoHandler.createDuplicateEntityCommand(duplicateList[i])
                editorScene.undoHandler.endMacro()
                editorScene.restoreMultiSelection(editorScene.multiSelectionList)
            } else {
                var currentSelection = editorContent.selectedEntity.entity()
                editorScene.undoHandler.createDuplicateEntityCommand(editorContent.selectedEntityName)
                editorScene.restoreSelection(currentSelection)
            }
        }
    }
    MenuItem {
        text: qsTr("Copy (Ctrl + c)") + editorScene.emptyString
        enabled: !editorScene.multiSelection && !entityTreeView.sceneRootSelected
        iconSource: "images/copy.png"
        onTriggered: {
            editorContent.copyEntity(editorContent.selectedEntityName)
        }
    }
    MenuItem {
        text: qsTr("Cut (Ctrl + x)") + editorScene.emptyString
        enabled: !editorScene.multiSelection && !entityTreeView.sceneRootSelected
        iconSource: "images/cut.png"
        onTriggered: {
            editorContent.cutEntity(editorContent.selectedEntityName, editorContent.selectedEntity)
        }
    }
    MenuItem {
        text: qsTr("Paste (Ctrl + v)") + editorScene.emptyString
        enabled: editorScene.clipboardOperation !== EditorScene.ClipboardNone
                 && !editorScene.multiSelection
                 && (!entityTree.treeviewPasting || (entityTree.treeviewPasting
                                                     && editorScene.sceneModel.canReparent(
                         editorScene.sceneModel.editorSceneItemFromIndex(
                             editorScene.sceneModel.getModelIndexByName(
                                 editorContent.selectedEntityName)),
                         editorScene.sceneModel.editorSceneItemFromIndex(
                             editorScene.sceneModel.getModelIndexByName(
                                 editorScene.clipboardContent)), true)))
        iconSource: "images/paste.png"
        onTriggered: {
            editorContent.pasteEntity()
        }
    }
    MenuItem {
        text: qsTr("Reset") + editorScene.emptyString
        iconSource: "images/reset_all.png"
        enabled: !editorScene.multiSelection && !entityTreeView.sceneRootSelected
        onTriggered: {
            editorScene.undoHandler.createResetEntityCommand(editorContent.selectedEntityName)
        }
    }
    MenuItem {
        text: qsTr("Reset Transform") + editorScene.emptyString
        iconSource: "images/reset.png"
        enabled: !editorScene.multiSelection && !entityTreeView.sceneRootSelected
                 && !entityTreeView.cameraSelected
        onTriggered: {
            editorScene.undoHandler.createResetTransformCommand(editorContent.selectedEntityName)
        }
    }
    MenuItem {
        text: qsTr("Group Selected") + editorScene.emptyString
        enabled: !entityTreeView.sceneRootSelected
        iconSource: "images/group.png"
        onTriggered: {
            editorScene.undoHandler.beginMacro(text)
            // Copy list, as the original is emptied on insertEntity
            var reparentList = []
            var groupCenter
            if (editorScene.multiSelection) {
                reparentList = editorScene.sceneModel.parentList(editorScene.multiSelectionList)
                groupCenter = editorScene.getMultiSelectionCenter()
            } else {
                reparentList[0] = editorContent.selectedEntityName
                groupCenter = editorContent.selectedEntity.selectionBoxCenter()
            }

            // TODO: Allow creating groups under other entities?

            // Add new group
            editorScene.undoHandler.createInsertEntityCommand(1, editorScene.sceneRootName(),
                                                              groupCenter)
            var index = editorScene.sceneModel.lastInsertedIndex()
            var groupName = editorScene.sceneModel.entityName(index)
            // Move selected entities under the added group
            for (var i = 0; i < reparentList.length; ++i)
                editorScene.undoHandler.createReparentEntityCommand(groupName, reparentList[i])
            editorScene.undoHandler.endMacro()

            // Single-select the added group. Need to fetch group index again as reparenting
            // resets the model.
            editorScene.clearMultiSelection()
            editorScene.selectIndex(editorScene.sceneModel.getModelIndexByName(groupName))
        }
    }
}
