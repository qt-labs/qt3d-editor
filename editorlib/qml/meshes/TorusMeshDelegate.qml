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

    IntPropertyInputField {
        label: qsTr("Rings") + editorScene.emptyString
        propertyName: "rings"
        component: meshComponentData
        componentType: thisDelegate.componentType
        minimum: 2
        tooltip: qsTr("Ring count of the torus.\nMinimum is 2.") + editorScene.emptyString
    }

    IntPropertyInputField {
        label: qsTr("Slices") + editorScene.emptyString
        propertyName: "slices"
        component: meshComponentData
        componentType: thisDelegate.componentType
        minimum: 2
        tooltip: qsTr("Slice count of the torus.\nMinimum is 2.") + editorScene.emptyString
    }

    FloatPropertyInputField {
        label: qsTr("Ring Radius") + editorScene.emptyString
        propertyName: "radius"
        component: meshComponentData
        componentType: thisDelegate.componentType
        tooltip: qsTr("Ring radius, i.e. the size of the torus.")
                 + editorScene.emptyString
    }

    FloatPropertyInputField {
        id: innerRadius
        label: qsTr("Tube Radius") + editorScene.emptyString
        propertyName: "minorRadius"
        component: meshComponentData
        componentType: thisDelegate.componentType
        tooltip: qsTr("Tube radius, i.e. the radius of the tube.")
                 + editorScene.emptyString
    }
}
