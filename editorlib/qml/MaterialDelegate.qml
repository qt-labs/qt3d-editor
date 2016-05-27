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

Item {
    id: materialDelegate
    width: parent.width
    height: columnLayout.y + columnLayout.height + 8

    default property alias _contentChildren: columnLayout.data
    property int componentType: EditorSceneItemComponentsModel.Unknown

    property string ambientColorTooltip: qsTr("Ambient color of the material, i.e.\nthe color seen in the absence of light.")
                                         + editorScene.emptyString
    property string diffuseColorTooltip: qsTr("Diffuse color of the material, i.e.\nthe color in the area of light.")
                                         + editorScene.emptyString
    property string specularColorTooltip: qsTr("Specular color of the material, i.e.\nthe color in the specular highlights.")
                                          + editorScene.emptyString
    property string diffuseMapTooltip: qsTr("Path to the file containing\nthe diffuse texture image.")
                                       + editorScene.emptyString
    property string normalMapTooltip: qsTr("Path to the file containing\nthe normal texture image.")
                                      + editorScene.emptyString
    property string specularMapTooltip: qsTr("Path to the file containing\nthe specular texture image.")
                                        + editorScene.emptyString
    property string textureScaleTooltip: qsTr("Scale of the texture. Scale affects in\ninverse, i.e. setting the value to 2\nmakes the texture half the size.")
                                         + editorScene.emptyString

    Column {
        id: columnLayout
        spacing: 4
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.right: parent.right
        anchors.rightMargin: 8

        FloatPropertyInputField {
            label: qsTr("Shininess") + editorScene.emptyString
            propertyName: "shininess"
            component: materialComponentData
            componentType: thisDelegate.componentType
            tooltip: qsTr("Shininess of the material. The\nsmaller the value, the wider and\nless intense the specular highlight.")
                     + editorScene.emptyString
            visible:  materialDelegate.componentType !== EditorSceneItemMaterialComponentsModel.Unknown
        }
    }
}
