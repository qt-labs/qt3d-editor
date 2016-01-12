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
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.4
import com.theqtcompany.SceneEditor3D 1.0

Item {
    height: 24
    width: 24

    property string showIconSource
    property string hideIconSource
    property alias tooltip: propertyButton.tooltip
    property int propertyComponentType: EditorSceneItemComponentsModel.Unknown

    // Button style delegate
    Component {
        id: propertyButtonStyle
        ButtonStyle {
            padding.top: 0
            padding.bottom: 0
            padding.right: 0
            padding.left: 0
            background: Rectangle {
                border.width: 0
            }
        }
    }

    Button {
        id: propertyButton
        anchors.centerIn: parent.Center
        iconSource: showIconSource //TODO: or hideIconSource if the properties are hidden
        style: propertyButtonStyle
        onClicked: {
            //TODO: show/hide properties view
            console.log("Property button clicked")
        }
    }
}
