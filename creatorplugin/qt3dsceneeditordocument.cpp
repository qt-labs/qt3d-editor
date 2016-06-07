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

#include "qt3dsceneeditordocument.h"
#include "qt3dsceneeditorplugin.h"
#include "qt3dsceneeditorconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/editormanager/editormanager.h>

using namespace Utils;

namespace Qt3DSceneEditor {
namespace Internal {

enum { debugQt3DSceneEditorDocument = 0 };

Qt3DSceneEditorDocument::Qt3DSceneEditorDocument(QObject *parent) :
    IDocument(parent)
{
    setId(Qt3DSceneEditor::Constants::QT3DSCENEEDITOR_ID);
    setMimeType(QLatin1String(Qt3DSceneEditor::Constants::C_QT3DSCENEEDITOR_MIMETYPE));

    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__;
}

Core::IDocument::OpenResult Qt3DSceneEditorDocument::open(QString *errorString,
                                                          const QString &fileName,
                                                          const QString &realFileName)
{
    Q_UNUSED(errorString)

    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__ << fileName << realFileName;

    setBlockDirtyChanged(true);

    // TODO: How to pass the actual file to open to launched editor?

    setFilePath(FileName::fromString(fileName));
    setBlockDirtyChanged(false);
    m_shouldAutoSave = false;

    emit loaded(true);
    return OpenResult::Success;
}

bool Qt3DSceneEditorDocument::save(QString *errorString, const QString &name, bool autoSave)
{
    Q_UNUSED(errorString)

    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__ << name << autoSave;

    const FileName oldFileName = filePath();
    const FileName actualName = name.isEmpty() ? oldFileName : FileName::fromString(name);
    if (actualName.isEmpty())
        return false;

    m_blockDirtyChanged = true;

    // TODO: How to pass the save command to editor

    m_shouldAutoSave = false;
    if (autoSave) {
        m_blockDirtyChanged = false;
        return true;
    }

    setFilePath(actualName);
    m_blockDirtyChanged = false;

    emit changed();
    return true;
}

QByteArray Qt3DSceneEditorDocument::contents() const
{
    // TODO: is this function actually needed?
    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__;
    return QByteArray();
}

bool Qt3DSceneEditorDocument::setContents(const QByteArray &contents)
{
    Q_UNUSED(contents)
    // TODO: Do we need this?
    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__;
    return true;
}

void Qt3DSceneEditorDocument::setFilePath(const FileName &newName)
{
    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__;
    IDocument::setFilePath(newName);
}

void Qt3DSceneEditorDocument::setBlockDirtyChanged(bool value)
{
    m_blockDirtyChanged = value;
}

void Qt3DSceneEditorDocument::setShouldAutoSave(bool save)
{
    m_shouldAutoSave = save;
}

bool Qt3DSceneEditorDocument::shouldAutoSave() const
{
    return m_shouldAutoSave;
}

bool Qt3DSceneEditorDocument::isModified() const
{
    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__;
    return false;
}

bool Qt3DSceneEditorDocument::isSaveAsAllowed() const
{
    return true;
}

bool Qt3DSceneEditorDocument::reload(QString *errorString, ReloadFlag flag, ChangeType type)
{
    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__;
    if (flag == FlagIgnore)
        return true;
    if (type == TypePermissions) {
        emit changed();
    } else {
        emit aboutToReload();
        QString fn = filePath().toString();
        const bool success = (open(errorString, fn, fn) == OpenResult::Success);
        emit reloadFinished(success);
        return success;
    }
    return true;
}

void Qt3DSceneEditorDocument::dirtyChanged(bool dirty)
{
    if (m_blockDirtyChanged)
        return; // We emit changed() afterwards, unless it was an autosave

    if (debugQt3DSceneEditorDocument)
        qDebug() << __FUNCTION__ << dirty;
    emit changed();
}

} // namespace Internal
} // namespace Qt3DSceneEditor
