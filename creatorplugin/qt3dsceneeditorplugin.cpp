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
#include "qt3dsceneeditorplugin.h"
#include "qt3dsceneeditorfactory.h"
#include "qt3dsceneeditorconstants.h"
#include "../editorlib/src/qt3dsceneeditor.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/designmode.h>

#include <utils/mimetypes/mimedatabase.h>

namespace Qt3DSceneEditor {
namespace Internal {

Qt3DSceneEditorPlugin::Qt3DSceneEditorPlugin() :
    m_qmlEngine(nullptr)
{
    // Create your members
}

Qt3DSceneEditorPlugin::~Qt3DSceneEditorPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    delete m_qmlEngine;
}

bool Qt3DSceneEditorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    Utils::MimeDatabase::addMimeTypes(QLatin1String(":/qt3deditorplugin/mimetypes.xml"));

    Qt3DSceneEditorFactory *editor = new Qt3DSceneEditorFactory(this);
    addAutoReleasedObject(editor);

    return true;
}

void Qt3DSceneEditorPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.

    // TODO: How to enable design mode properly, as this doesn't seem to work
    //Core::DesignMode::instance()->setDesignModeIsRequired();
}

ExtensionSystem::IPlugin::ShutdownFlag Qt3DSceneEditorPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)

    // TODO: Trigger save?
    return SynchronousShutdown;
}

} // namespace Internal
} // namespace Qt3DSceneEditor
