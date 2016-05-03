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

PropertyInputField {
    id: floatSliderPropertyInputField
    width: parent.width
    height: sliderInputfield.height

    property alias label: sliderInputfield.label
    property alias stepSize: sliderInputfield.stepSize
    property alias minimum: sliderInputfield.minimum
    property alias maximum: sliderInputfield.maximum
    property alias roundDigits: sliderInputfield.roundDigits
    property alias tooltip: sliderInputfield.tooltip

    onComponentValueChanged: {
        if (component !== null)
            sliderInputfield.value = component[propertyName]
    }

    FloatSliderInputField {
        id: sliderInputfield
        lockProperty: floatSliderPropertyInputField.propertyName + editorScene.lockPropertySuffix
        lockComponent: floatSliderPropertyInputField.component
        value: 0
        onValueChanged: {
            handleEditingFinished(value)
        }
    }
}
