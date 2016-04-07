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
import Qt.labs.controls 1.0 as QLC

Rectangle {
    property string headerText
    property bool viewVisible: true
    property int minimumHeaderHeight: viewHeaderText.implicitHeight + 12
    property bool viewHeaderInitialized: viewHeaderText.implicitHeight === 0 ? false : true

    signal showViewTitle(bool showView)
    signal viewHeaderClicked()

    height: minimumHeaderHeight
    width: parent.width
    color: "darkGray"

    onViewVisibleChanged: {
        showViewTitle(viewVisible)
    }

    QLC.Label {
        id: viewHeaderText
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        text: headerText
        font.bold: true
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            viewHeaderClicked()
        }
    }
}
