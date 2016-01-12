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
import QtQuick 2.4
import com.theqtcompany.SceneEditor3D 1.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2

ComponentDelegate {
    id: transformDelegate
    title: qsTr("Transform")

    property int currentTransform: 0

    Item {
        width: parent.width
        height: transformCombobox.height + 8

        Component.onCompleted: transformCombobox.currentIndex = transformDelegate.currentTransform - 1

        ComboBox {
            id: transformCombobox
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.bottomMargin: 4
            anchors.verticalCenter: parent.verticalCenter

            model: ListModel {
                ListElement { text: qsTr("Scale, Rotate & Translate") }
                ListElement { text: qsTr("Matrix") }
            }
            onCurrentTextChanged: {
                if (currentText == qsTr("Scale, Rotate & Translate"))
                    componentData.model.setTransform(EditorSceneItemTransformComponentsModel.SRT)
                else if (currentText == qsTr("Matrix"))
                    componentData.model.setTransform(EditorSceneItemTransformComponentsModel.Matrix)
            }
        }
    }

    Repeater {
        model: componentData.model

        Loader {
            width: parent.width

            function transformTypetoDelegateSource(transformType) {
                transformDelegate.currentTransform = transformType
                if (transformType == EditorSceneItemTransformComponentsModel.SRT)
                    return "SRTTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Matrix)
                    return "MatrixTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Translate)
                    return "TranslateTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Rotate)
                    return "RotateTransformDelegate.qml"
                if (transformType == EditorSceneItemTransformComponentsModel.Scale)
                    return "ScaleTransformDelegate.qml"

                return "UnknownTransformDelegate.qml"
            }

            source: transformTypetoDelegateSource(transformType)
        }
    }
}
