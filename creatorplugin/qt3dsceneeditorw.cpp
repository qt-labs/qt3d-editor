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

#include "qt3dsceneeditorw.h"
#include "qt3dsceneeditorplugin.h"
#include "qt3dsceneeditordocument.h"
#include "qt3dsceneeditorconstants.h"
#include "../editorlib/src/qt3dsceneeditor.h"

#include <QQuickWidget>

using namespace Utils;

namespace Qt3DSceneEditor {
namespace Internal {

enum { debugQt3DSceneEditorW = 0 };

Qt3DSceneEditorW::Qt3DSceneEditorW(const Core::Context &context,
                                   Qt3DSceneEditorPlugin *plugin,
                                   QWidget *parent)
    : m_document(new Qt3DSceneEditorDocument(this)),
      m_plugin(plugin),
      m_sceneEditor(nullptr)
{
    Qt3DSceneEditorLib::register3DSceneEditorQML();
    m_sceneEditor = new QQuickWidget(parent);
    m_sceneEditor->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_sceneEditor->setSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/PluginMain.qml")));

    setContext(context);
    setWidget(m_sceneEditor);

    if (debugQt3DSceneEditorW)
        qDebug() <<  __FUNCTION__;
}

Qt3DSceneEditorW::~Qt3DSceneEditorW()
{
    if (m_sceneEditor)
        m_sceneEditor->deleteLater();
}

QWidget *Qt3DSceneEditorW::toolBar()
{
    return nullptr;
}

} // namespace Internal
} // namespace Qt3DSceneEditor
