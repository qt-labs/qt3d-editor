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

#include <coreplugin/idocument.h>


namespace Qt3DSceneEditor {
namespace Internal {

class Qt3DSceneEditorPlugin;
class Qt3DSceneEditorW;

class Qt3DSceneEditorDocument
  : public Core::IDocument
{
    Q_OBJECT
public:
    Qt3DSceneEditorDocument(QObject *parent = 0);
    ~Qt3DSceneEditorDocument() {}

    //IDocument
    OpenResult open(QString *errorString, const QString &fileName,
                    const QString &realFileName) override;
    bool save(QString *errorString, const QString &fileName, bool autoSave) override;
    QByteArray contents() const override;
    bool setContents(const QByteArray &contents) override;
    bool shouldAutoSave() const override;
    bool isModified() const override;
    bool isSaveAsAllowed() const override;
    bool reload(QString *errorString, ReloadFlag flag, ChangeType type) override;
    void setFilePath(const Utils::FileName &newName) override;
    void setBlockDirtyChanged(bool value);
    void setShouldAutoSave(bool save);

signals:
    void loaded(bool success);

private:
    void dirtyChanged(bool);

    bool m_blockDirtyChanged = false;
    bool m_shouldAutoSave = false;
};

} // namespace Internal
} // namespace Qt3DSceneEditor
