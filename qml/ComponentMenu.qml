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
        iconSource: "qrc:/images/picker.png"
        onTriggered: {
            componentPropertiesView.model.appendNewComponent(sceneModel.ObjectPicker)
        }
    }

    MenuSeparator {}

    EntityMenu {
        iconSource: "qrc:/images/plus.png"
        enabled: !entityTreeView.cameraSelected
    }
    MenuItem {
        text: qsTr("Remove") + editorScene.emptyString
        enabled: !entityTreeView.sceneRootSelected
        iconSource: "qrc:/images/minus.png"
        onTriggered: {
            entityTreeView.editing = false
            // Doublecheck that we don't try to remove the scene root
            if (entityTreeView.selection.currentIndex !== editorScene.sceneModel.sceneEntityIndex())
                editorScene.undoHandler.createRemoveEntityCommand(selectedEntityName)
        }
    }
    MenuItem {
        text: qsTr("Duplicate") + editorScene.emptyString
        enabled: !entityTreeView.sceneRootSelected
        iconSource: "qrc:/images/duplicate.png"
        onTriggered: {
            var currentSelection = selectedEntity.entity()
            editorScene.undoHandler.createDuplicateEntityCommand(selectedEntityName)
            editorScene.restoreSelection(currentSelection)
        }
    }
}
