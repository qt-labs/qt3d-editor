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
import QtQuick.Layouts 1.2

MeshDelegate {
    id: thisDelegate
    componentType: EditorSceneItemComponentsModel.Mesh

    FloatPropertyInputField {
        label: qsTr("Width") + editorScene.emptyString
        propertyName: "width"
        component: meshComponentData
        componentType: thisDelegate.componentType
        minimum: 0.00001
        tooltip: qsTr("Width of the plane.") + editorScene.emptyString
    }

    FloatPropertyInputField {
        label: qsTr("Height") + editorScene.emptyString
        propertyName: "height"
        component: meshComponentData
        componentType: thisDelegate.componentType
        minimum: 0.00001
        tooltip: qsTr("Height of the plane.") + editorScene.emptyString
    }

    StyledLabel {
        text: qsTr("Resolution") + editorScene.emptyString
        Layout.alignment: Qt.AlignLeft
    }

    SizePropertyInputField {
        label: qsTr("W - H") + editorScene.emptyString
        widthLabel: editorScene.emptyString
        heightLabel: " - "
        propertyName: "meshResolution"
        component: meshComponentData
        componentType: thisDelegate.componentType
        minimum: 2
        tooltip: qsTr("Resolution of the plane, i.e. how\nmany strips the mesh is divided into.")
                 + editorScene.emptyString
    }
}
