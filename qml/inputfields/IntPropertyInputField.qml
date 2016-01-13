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
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2

PropertyInputField {
    id: intInput
    width: parent.width
    height: mainLayout.height

    property alias label: intLabel.text
    property int minimum: 0

    onComponentValueChanged: {
        if (component !== null)
            valueInput.text = component[propertyName]
    }

    IntValidator {
        id: intValidator
        locale: "C"
    }

    RowLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        Label {
            id: intLabel
            text: qsTr("Integer Value")
            color: labelTextColor
            Layout.alignment: Qt.AlignLeft
        }

        TextField {
            id: valueInput
            Layout.alignment: Qt.AlignRight
            validator: intValidator
            implicitWidth: intInput.width * 0.6
            text: ""

            onEditingFinished: {
                var newValue = component[propertyName]
                if (text != "")
                    newValue = text
                newValue = Math.max(newValue, minimum)
                handleEditingFinished(newValue)
            }
        }
    }
}

