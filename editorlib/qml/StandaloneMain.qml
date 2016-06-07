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
import QtQuick.Controls 2.0 as QQC2
import Qt.labs.settings 1.0
import com.theqtcompany.SceneEditor3D 1.0

QQC2.ApplicationWindow {
    id: standaloneWindow
    title: qsTr("Qt 3D Scene Editor") + editorContent.editorScene.emptyString + editorContent.saveFileTitleAddition
    width: 1280
    height: 800
    visible: false
    color: editorContent.paneBackgroundColor
    minimumHeight: 400
    minimumWidth: 640

    EditorContent {
        id: editorContent
        visible: false
    }

    onClosing: {
        close.accepted = editorContent.checkUnsavedChanges()
    }

    Component.onCompleted: {
        Qt.application.organization = "The Qt Company"
        Qt.application.domain = "qt.io"
        Qt.application.name = "Qt 3D Scene Editor"
        // Redraw everything to get rid of artifacts
        showMaximized()
        show()
        editorContent.visible = true
    }

    Settings {
        // Save window placement, size
        category: "Qt 3D SceneEditor Window"
        property alias x: standaloneWindow.x
        property alias y: standaloneWindow.y
        property alias width: standaloneWindow.width
        property alias height: standaloneWindow.height
    }
}
