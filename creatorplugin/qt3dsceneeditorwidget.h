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

#pragma once

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE
class QQuickWidget;
class QQuickItem;
QT_END_NAMESPACE

namespace Qt3DSceneEditor {
namespace Internal {

class Qt3DSceneEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Qt3DSceneEditorWidget(QWidget *parent = 0);

    ~Qt3DSceneEditorWidget();

    void initialize(const QUrl &fileName);

private: // functions
    enum InitializeStatus { NotInitialized, Initializing, Initialized };
    void setup();

private: // variables
    InitializeStatus m_initStatus = NotInitialized;
    QQuickWidget *m_sceneEditor;
    QQuickItem *m_rootItem;
};

} // namespace Internal
} // namespace Qt3DSceneEditor
