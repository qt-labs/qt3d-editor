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

MeshDelegate {
    id: thisDelegate
    componentType: EditorSceneItemComponentsModel.Mesh

    FilePropertyInputField {
        label: qsTr("Source") + editorScene.emptyString
        propertyName: "source"
        component: meshComponentData
        componentType: thisDelegate.componentType
        dialog.nameFilters: [qsTr("Wavefront files (*.obj)") + editorScene.emptyString]
        defaultUrl: "qrc:/qt3deditorlib/meshes/defaultmesh.obj"
        tooltip: qsTr("The path to the object\nfile to be loaded.") + editorScene.emptyString
    }
}

