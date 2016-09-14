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
import QtQuick.Controls.Styles 1.4

Item {
    id: enableButton
    height: 16
    width: 16

    property string enabledIconSource
    property string disabledIconSource
    property string pressedIconSource
    property alias tooltip: propertyButton.tooltip
    property bool buttonEnabled
    property color defaultBgColor: "transparent"
    property color hoveredBgColor: "transparent"
    property color selectedBgColor: "transparent"
    property bool hoverAlways: false

    signal enabledButtonClicked()

    // Button style delegate
    Component {
        id: enabledButtonStyle
        ButtonStyle {
            padding.top: 0
            padding.bottom: 0
            padding.right: 0
            padding.left: 0
            background: Rectangle {
                implicitHeight: enableButton.height
                implicitWidth: enableButton.width
                color: {
                    if ((buttonEnabled || hoverAlways) && enabled) {
                        if (propertyButton.hovered)
                            hoveredBgColor
                        else
                            defaultBgColor
                    } else {
                        selectedBgColor
                    }
                    if (propertyButton.pressed)
                        selectedBgColor
                }
            }
        }
    }

    Button {
        id: propertyButton
        anchors.centerIn: parent.Center
        iconSource: {
            if (buttonEnabled && enabled) {
                if (pressed && pressedIconSource !== "")
                    pressedIconSource
                else
                    enabledIconSource
            } else {
                disabledIconSource
            }
        }
        style: enabledButtonStyle
        activeFocusOnTab: false
        onClicked: {
            enabledButtonClicked()
        }
    }
}
