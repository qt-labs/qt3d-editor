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
    id: componentDelegate
    width: parent.width

    property alias title: componentTitle.headerText
    default property alias _contentChildren: columnLayout.data
    property alias viewTitleVisible: componentTitle.viewVisible
    property int componentType: EditorSceneItemComponentsModel.Unknown
    property int componentHeight: columnLayout.y + columnLayout.height
    property int minimumComponentHeight: componentTitle.minimumHeaderHeight
    property alias transformFieldsDisabled: componentTitle.lockTransformFields

    signal changeViewVisibity(bool viewVisibility)

    height: componentHeight

    Item {
        id: titleHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: componentTitle.height

        ButtonViewHeader {
            id: componentTitle
            anchors.top: parent.top
            headerText: componentTitle
            // Currently only Transform delegate has show/hide button,
            // set transform specific tooltip
            tooltip: qsTr("Show/Hide Transform Properties") + editorScene.emptyString
            showViewButtonShown: componentType == EditorSceneItemComponentsModel.Transform ?
                                     true : false
            lockTransformButtonShown: componentType == EditorSceneItemComponentsModel.Transform ?
                                          true : false
            resetButtonShown: componentType == EditorSceneItemComponentsModel.Transform ?
                                  true : false

            onShowViewTitle: {
                if (viewVisible) {
                    if (componentHeight === 0)
                        componentHeight = columnLayout.y + columnLayout.height
                    componentDelegate.height = componentHeight
                } else {
                    componentHeight = componentDelegate.height
                    componentDelegate.height = minimumHeaderHeight
                }
            }

            onShowViewButtonPressed: {
                changeViewVisibity(true)
            }
            onHideViewButtonPressed: {
                changeViewVisibity(false)
            }
        }

        Column {
            id: columnLayout
            anchors.top: componentTitle.bottom
            anchors.topMargin: 8
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
            visible: componentTitle.viewVisible

            onHeightChanged: {
                if (componentTitle.viewHeaderInitialized)
                    componentHeight = columnLayout.y + columnLayout.height
                else
                    componentHeight = columnLayout.y + columnLayout.height
                            + componentTitle.minimumHeaderHeight
                componentDelegate.height = componentHeight
            }
        }
    }
}

