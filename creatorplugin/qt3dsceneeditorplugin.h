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

#include "qt3dsceneeditor_global.h"
#include <extensionsystem/iplugin.h>
#include <utils/fileutils.h>

namespace Qt3DSceneEditor {
namespace Internal {

class Qt3DSceneEditorWidget;
class Qt3DSceneEditorContext;

class Qt3DSceneEditorPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Qt3DSceneEditor.json")

public:
    Qt3DSceneEditorPlugin();
    ~Qt3DSceneEditorPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

    Qt3DSceneEditorWidget *sceneEditorWidget() const { return m_sceneEditorWidget; }

    void showSceneEditor();
    void hideSceneEditor();

private:
    void createSceneEditorWidget();
    void fixProFile(const Utils::FileName &filePath);

    Qt3DSceneEditorWidget *m_sceneEditorWidget;
    Qt3DSceneEditorContext *m_context;
    Utils::FileName m_previouslyLoadedScene;
    bool m_proFileDirty;
};

} // namespace Internal
} // namespace Qt3DSceneEditor
