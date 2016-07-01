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
#include "qt3dsceneeditorconstants.h"
#include "qt3dsceneeditorcontext.h"
#include "qt3dsceneeditorwidget.h"
#include "../editorlib/src/qt3dsceneeditor.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/designmode.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/documentmanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/session.h>
#include <proparser/profileevaluator.h>
#include <proparser/prowriter.h>
#include <proparser/qmakevfs.h>

#include <utils/mimetypes/mimedatabase.h>
#include <utils/fileutils.h>

#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QLibraryInfo>

namespace Qt3DSceneEditor {
namespace Internal {

typedef QmakeProjectManager::Internal::ProWriter PW;

class Handler : public QMakeHandler {
public:
    virtual void message(int type, const QString &msg, const QString &fileName, int lineNo)
    {
        Q_UNUSED(lineNo);
        Q_UNUSED(fileName);
        Q_UNUSED(type);
        qDebug() << msg;
    }

    virtual void fileMessage(int type, const QString &msg)
    {
        Q_UNUSED(type)
        qWarning("%s", qPrintable(msg));
    }

    virtual void aboutToEval(ProFile *, ProFile *, EvalFileType) {}
    virtual void doneWithEval(ProFile *) {}
};

static Handler handler;

Qt3DSceneEditorPlugin::Qt3DSceneEditorPlugin() :
    m_sceneEditorWidget(nullptr)
  , m_context(nullptr)
  , m_proFileDirty(false)
{
    // Create your members
}

Qt3DSceneEditorPlugin::~Qt3DSceneEditorPlugin()
{
    // Unregister objects from the plugin manager's object pool
    if (m_sceneEditorWidget)
        Core::DesignMode::instance()->unregisterDesignWidget(m_sceneEditorWidget);
    if (m_context)
        Core::ICore::removeContextObject(m_context);

    delete m_context;
    delete m_sceneEditorWidget;
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

    createSceneEditorWidget();

    return true;
}

void Qt3DSceneEditorPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
    QStringList mimeTypes;
    mimeTypes.append(Qt3DSceneEditor::Constants::C_QT3DSCENEEDITOR_MIMETYPE);

    Core::DesignMode::instance()->registerDesignWidget(m_sceneEditorWidget, mimeTypes,
                                                       m_context->context());
    Core::DesignMode::instance()->setDesignModeIsRequired();
}

ExtensionSystem::IPlugin::ShutdownFlag Qt3DSceneEditorPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)

    // TODO: Trigger save?
    return SynchronousShutdown;
}

static bool isSceneEditorDocument(Core::IEditor *editor) {
    return editor && editor->document() && editor->document()->mimeType()
            == Qt3DSceneEditor::Constants::C_QT3DSCENEEDITOR_MIMETYPE;
}

static bool isDesignerMode(Core::Id mode)
{
    return mode == Core::DesignMode::instance()->id();
}

void Qt3DSceneEditorPlugin::createSceneEditorWidget()
{
    m_sceneEditorWidget = new Qt3DSceneEditorWidget;
    m_context = new Qt3DSceneEditorContext(m_sceneEditorWidget);
    Core::ICore::addContextObject(m_context);

    connect(Core::EditorManager::instance(), &Core::EditorManager::currentEditorChanged,
            [=] (Core::IEditor *editor) {
        if (isSceneEditorDocument(editor) && !isDesignerMode(Core::ModeManager::currentMode())) {
            showSceneEditor();
            QTimer::singleShot(0, this, [] () {
                Core::ModeManager::activateMode(Core::Constants::MODE_DESIGN);
            });
        }
    });

    connect(Core::EditorManager::instance(), &Core::EditorManager::editorsClosed,
            [=] (QList<Core::IEditor *> editors) {
        Q_FOREACH (Core::IEditor *editor, editors) {
            if (isSceneEditorDocument(editor))
                hideSceneEditor();
        }
    });

    connect(Core::ModeManager::instance(), &Core::ModeManager::currentModeChanged,
            [=] (Core::Id newMode, Core::Id oldMode) {
        if (!isDesignerMode(newMode) && isDesignerMode(oldMode)) {
            hideSceneEditor();
        } else if (isSceneEditorDocument(Core::EditorManager::currentEditor())
                   && isDesignerMode(newMode)) {
            showSceneEditor();
        }
    });
}

void Qt3DSceneEditorPlugin::fixProFile(const Utils::FileName &filePath)
{
    const ProjectExplorer::Project *project =
            ProjectExplorer::SessionManager::projectForFile(filePath);

    if (!project) {
        // If the actual project could not be determined, check current project, as it is usually
        // where things are added.
        project = ProjectExplorer::ProjectTree::instance()->currentProject();
        if (!project)
            return;
    }

    QString checkedFile = filePath.fileName(0);
    QString proFileName = project->projectFilePath().toString();
    QMakeVfs vfs;
    QMakeParser parser(0, &vfs, &handler);
    ProFileGlobals option;
    QString qmake = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QLatin1String("/qmake");
    option.qmake_abslocation = QDir::cleanPath(qmake);

    ProFileEvaluator visitor(&option, &parser, &vfs, &handler);

#ifdef PROEVALUATOR_CUMULATIVE
    visitor.setCumulative(true);
#endif
    visitor.initialize();

    QStringList lines;
    {
        QString contents;
        {
            Utils::FileReader reader;
            if (!reader.fetch(proFileName, QIODevice::Text)) {
                return;
            }
            contents = QString::fromLocal8Bit(reader.data());
            lines = contents.split(QLatin1Char('\n'));
        }
    }

    ProFile *pro;

    if (!(pro = parser.parsedProFile(proFileName))) {
        if (!QFile::exists(proFileName)) {
            qCritical("Input file %s does not exist.", qPrintable(proFileName));
            return;
        }
        return;
    }

    if (!visitor.accept(pro, QMakeEvaluator::LoadProOnly)) {
        pro->deref();
        return;
    }

    QStringList resources = visitor.values(QStringLiteral("RESOURCES"));
    if (resources.contains(checkedFile)) {
        pro->deref();
        return;
    }

    QStringList distFiles = visitor.values(QStringLiteral("DISTFILES"));
    if (!distFiles.contains(checkedFile)) {
        pro->deref();
        return;
    }
    m_proFileDirty = false;

    QString scope = QLatin1String("");

    QStringList checkedFileList;
    checkedFileList << checkedFile;

    Core::DocumentManager::expectFileChange(proFileName);
    PW::removeVarValues(pro, &lines, checkedFileList,
                        QStringList() << QStringLiteral("DISTFILES"));
    PW::putVarValues(pro, &lines, checkedFileList, QStringLiteral("RESOURCES"),
                     PW::PutFlags(PW::AppendValues | PW::MultiLine), scope);

    Utils::FileSaver saver(proFileName, QIODevice::Text);
    saver.write(lines.join(QLatin1Char('\n')).toLocal8Bit());
    saver.finalize();

    pro->deref();
}

void Qt3DSceneEditorPlugin::showSceneEditor()
{
    Utils::FileName filePath = Core::EditorManager::currentDocument()->filePath();

    if (filePath != m_previouslyLoadedScene) {
        m_previouslyLoadedScene = filePath;
        m_proFileDirty = true;
        m_sceneEditorWidget->initialize(QUrl::fromLocalFile(filePath.toString()));
        fixProFile(filePath);
    }
}

void Qt3DSceneEditorPlugin::hideSceneEditor()
{
    // Reattempt fixing at editor hiding if fixing failed when showing editor, which can happen
    // when adding scene to non-current project.
    if (m_proFileDirty)
        fixProFile(m_previouslyLoadedScene);

    // TODO: should save settings on qml side?
}

} // namespace Internal
} // namespace Qt3DSceneEditor
