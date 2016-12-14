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
#ifndef EDITORSCENESAVER_H
#define EDITORSCENESAVER_H

#include <QtCore/qobject.h>

namespace Qt3DCore {
    class QEntity;
}

class QTemporaryDir;

class EditorSceneSaver : public QObject
{
    Q_OBJECT

public:
    explicit EditorSceneSaver(QObject *parent = 0);
    ~EditorSceneSaver();

    bool saveScene(Qt3DCore::QEntity *sceneEntity,  const QString &activeSceneCamera,
                   const QString &saveFileName, bool autosave = false);

    Qt3DCore::QEntity *loadScene(const QString &fileName, Qt3DCore::QEntity *camera);
    void deleteSavedScene(const QString &saveFileName, bool autosave = false);

private:
    QTemporaryDir *m_loadDir;
};

#endif // EDITORSCENESAVER_H
