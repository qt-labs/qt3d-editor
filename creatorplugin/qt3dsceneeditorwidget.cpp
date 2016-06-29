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

#include "qt3dsceneeditorwidget.h"
#include "../editorlib/src/qt3dsceneeditor.h"

#include <QtWidgets/QBoxLayout>
#include <QtQuickWidgets/QQuickWidget>
#include <QtQuick/QQuickItem>

using namespace Qt3DSceneEditor;

namespace Qt3DSceneEditor {
namespace Internal {

Qt3DSceneEditorWidget::Qt3DSceneEditorWidget(QWidget *parent)
    : QWidget(parent)
    , m_sceneEditor(nullptr)
    , m_rootItem(nullptr)
{
}

Qt3DSceneEditorWidget::~Qt3DSceneEditorWidget()
{
}

void Qt3DSceneEditorWidget::setup()
{
    Qt3DSceneEditorLib::register3DSceneEditorQML();
    m_sceneEditor = new QQuickWidget(this);
    m_sceneEditor->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_sceneEditor->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/PluginMain.qml")));
    m_rootItem = m_sceneEditor->rootObject();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_sceneEditor);
    setLayout(layout);

    show();
}

void Qt3DSceneEditorWidget::initialize(const QUrl &fileName)
{
    if (m_initStatus == NotInitialized) {
        m_initStatus = Initializing;
        setup();
    }
    m_initStatus = Initialized;

    QMetaObject::invokeMethod(m_rootItem, "loadScene", Q_ARG(QVariant, fileName));
}

} // namespace Internal
} // namespace Qt3DSceneEditor
