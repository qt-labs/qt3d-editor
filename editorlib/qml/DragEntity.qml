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

Image {
    z: 5
    width: 32
    height: 32
    visible: false

    property int xDiff: width / 2
    property int yDiff: height / 2
    property var entityType
    property string entityName
    property string dragKey

    function startDrag(dragSource, image, key, startX, startY, meshType, startOpacity, name) {
        source = image
        Drag.source = dragSource
        Drag.hotSpot.x = -xDiff
        Drag.hotSpot.y = -yDiff
        Drag.keys = [ key ]
        dragKey = key
        x = startX + xDiff
        y = startY + yDiff
        visible = true
        entityType = meshType
        if (name)
            entityName = name
        if (startOpacity)
            opacity = startOpacity
        else
            opacity = 1.0
        Drag.active = true
    }

    function endDrag(drop) {
        var dropResult = Qt.IgnoreAction
        if (drop)
            dropResult = Drag.drop()

        x = 0
        y = 0
        Drag.active = false
        dragKey = ""
        visible = false

        return dropResult
    }

    function setPosition(xPos, yPos) {
        x = xPos + xDiff
        y = yPos + yDiff
    }
}
