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
import QtQuick.Layouts 1.2

Item {
    id: matrix4x4Input
    width: parent.width
    height: mainLayout.height
    property real cellwidth: matrix4x4Input.width / 4 - 4 // 4 = column spacing
    property alias label: matrixLabel.text
    property int roundDigits: 2 // TODO: Determine nice default rounding
    property int roundMultiplier: Math.pow(10, roundDigits) // Calculated from roundDigits, do not set directly
    property matrix4x4 value: Qt.matrix4x4(0.0,0.0,0.0,0.0,
                                           0.0,0.0,0.0,0.0,
                                           0.0,0.0,0.0,0.0,
                                           0.0,0.0,0.0,0.0)
    property bool affine: true
    property alias tooltip: matrixLabel.tooltip

    signal valueEdited

    function roundNumber(number) {
        if (roundDigits >= 0)
            return Math.round(number * roundMultiplier) / roundMultiplier
        else
            return number
    }

    onValueChanged: {
        m11Field.text = roundNumber(value.m11)
        m12Field.text = roundNumber(value.m12)
        m13Field.text = roundNumber(value.m13)
        m14Field.text = roundNumber(value.m14)

        m21Field.text = roundNumber(value.m21)
        m22Field.text = roundNumber(value.m22)
        m23Field.text = roundNumber(value.m23)
        m24Field.text = roundNumber(value.m24)

        m31Field.text = roundNumber(value.m31)
        m32Field.text = roundNumber(value.m32)
        m33Field.text = roundNumber(value.m33)
        m34Field.text = roundNumber(value.m34)

        m41Field.text = roundNumber(value.m41)
        m42Field.text = roundNumber(value.m42)
        m43Field.text = roundNumber(value.m43)
        m44Field.text = roundNumber(value.m44)
    }

    DoubleValidator {
        id: doubleValidator
        locale: "C"
    }

    ColumnLayout {
        id: mainLayout
        anchors.right: parent.right
        anchors.left: parent.left

        StyledLabel {
            id: matrixLabel
            text: qsTr("Matrix4x4") + editorScene.emptyString
            Layout.alignment: Qt.AlignLeft
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignLeft
            RowLayout {
                StyledTextField {
                    id: m11Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m11
                            matrix4x4Input.value.m11 = text
                            if (oldValue !== matrix4x4Input.value.m11)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m11)
                }
                StyledTextField {
                    id: m12Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m12
                            matrix4x4Input.value.m12 = text
                            if (oldValue !== matrix4x4Input.value.m12)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m12)
                }
                StyledTextField {
                    id: m13Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m13
                            matrix4x4Input.value.m13 = text
                            if (oldValue !== matrix4x4Input.value.m13)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m13)
                }
                StyledTextField {
                    id: m14Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m14
                            matrix4x4Input.value.m14 = text
                            if (oldValue !== matrix4x4Input.value.m14)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m14)
                }
            }

            RowLayout {
                StyledTextField {
                    id: m21Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m21
                            matrix4x4Input.value.m21 = text
                            if (oldValue !== matrix4x4Input.value.m21)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m21)
                }
                StyledTextField {
                    id: m22Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m22
                            matrix4x4Input.value.m22 = text
                            if (oldValue !== matrix4x4Input.value.m22)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m22)
                }
                StyledTextField {
                    id: m23Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m23
                            matrix4x4Input.value.m23 = text
                            if (oldValue !== matrix4x4Input.value.m23)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m23)
                }
                StyledTextField {
                    id: m24Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m24
                            matrix4x4Input.value.m24 = text
                            if (oldValue !== matrix4x4Input.value.m24)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m24)
                }
            }
            RowLayout {
                StyledTextField {
                    id: m31Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m31
                            matrix4x4Input.value.m31 = text
                            if (oldValue !== matrix4x4Input.value.m31)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m31)
                }
                StyledTextField {
                    id: m32Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m32
                            matrix4x4Input.value.m32 = text
                            if (oldValue !== matrix4x4Input.value.m32)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m32)
                }
                StyledTextField {
                    id: m33Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m33
                            matrix4x4Input.value.m33 = text
                            if (oldValue !== matrix4x4Input.value.m33)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m33)
                }
                StyledTextField {
                    id: m34Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m34
                            matrix4x4Input.value.m34 = text
                            if (oldValue !== matrix4x4Input.value.m34)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m34)
                }
            }
            RowLayout {
                StyledTextField {
                    id: m41Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    enabled: !affine
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m41
                            matrix4x4Input.value.m41 = text
                            if (oldValue !== matrix4x4Input.value.m41)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m41)
                }
                StyledTextField {
                    id: m42Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    enabled: !affine
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m42
                            matrix4x4Input.value.m42 = text
                            if (oldValue !== matrix4x4Input.value.m42)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m42)
                }
                StyledTextField {
                    id: m43Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    enabled: !affine
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m43
                            matrix4x4Input.value.m43 = text
                            if (oldValue !== matrix4x4Input.value.m43)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m43)
                }
                StyledTextField {
                    id: m44Field
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    validator: doubleValidator
                    implicitWidth: cellwidth
                    enabled: !affine
                    selectByMouse: true

                    onEditingFinished: {
                        if (text !== "") {
                            var oldValue = matrix4x4Input.value.m44
                            matrix4x4Input.value.m44 = text
                            if (oldValue !== matrix4x4Input.value.m44)
                                valueEdited()
                        }
                    }

                    Component.onCompleted: text = roundNumber(matrix4x4Input.value.m44)
                }
            }
        }
    }
}
