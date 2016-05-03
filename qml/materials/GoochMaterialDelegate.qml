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

MaterialDelegate {
    id: thisDelegate
    componentType: EditorSceneItemComponentsModel.Material

    ColorPropertyInputField {
        label: qsTr("Diffuse Color") + editorScene.emptyString
        propertyName: "diffuse"
        component: materialComponentData
        componentType: thisDelegate.componentType
        tooltip: diffuseColorTooltip
    }

    ColorPropertyInputField {
        label: qsTr("Specular Color") + editorScene.emptyString
        propertyName: "specular"
        component: materialComponentData
        componentType: thisDelegate.componentType
        tooltip: specularColorTooltip
    }

    ColorPropertyInputField {
        label: qsTr("Cool Color") + editorScene.emptyString
        propertyName: "cool"
        component: materialComponentData
        componentType: thisDelegate.componentType
        tooltip: qsTr("Cool color of the material, i.e. the color\nin the areas away from the light source.")
                 + editorScene.emptyString
    }

    ColorPropertyInputField {
        label: qsTr("Warm Color") + editorScene.emptyString
        propertyName: "warm"
        component: materialComponentData
        componentType: thisDelegate.componentType
        tooltip: qsTr("Warm color of the material, i.e. the color\nin the areas facing the light source.")
                 + editorScene.emptyString
    }

    FloatPropertyInputField {
        label: qsTr("Alpha") + editorScene.emptyString
        propertyName: "alpha"
        component: materialComponentData
        componentType: thisDelegate.componentType
        tooltip: qsTr("Alpha of the material, i.e. the scale\nfactor that controls the combination\nof cool color and base color.")
                 + editorScene.emptyString
    }

    FloatPropertyInputField {
        label: qsTr("Beta") + editorScene.emptyString
        propertyName: "beta"
        component: materialComponentData
        componentType: thisDelegate.componentType
        tooltip: qsTr("Beta of the material, i.e. the scale\nfactor that controls the combination\nof warm color and base color.")
                 + editorScene.emptyString
    }
}

