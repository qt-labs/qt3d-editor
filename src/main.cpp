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
#include <QApplication>
#include <QQmlApplicationEngine>

#include <QtQuick>

#include "undohandler.h"
#include "editorscene.h"
#include "editorviewportitem.h"
#include "editorsceneitemmodel.h"
#include "editorsceneitem.h"
#include "editorsceneitemcomponentsmodel.h"
#include "editorsceneitemtransformcomponentsmodel.h"
#include "editorsceneitemmaterialcomponentsmodel.h"
#include "editorsceneitemmeshcomponentsmodel.h"
#include "editorsceneitemlightcomponentsmodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator appTranslator;
    appTranslator.load(":/qt3dsceneeditor_" + QLocale::system().name());
    app.installTranslator(&appTranslator);

    qmlRegisterType<EditorScene>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorScene");
    qmlRegisterType<EditorViewportItem>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorViewport");
    qmlRegisterUncreatableType<EditorSceneItemModel>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorSceneItemModel", "Created by EditorScene");
    qmlRegisterUncreatableType<EditorSceneItem>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorSceneItem", "Created by EditorScene");
    qmlRegisterUncreatableType<EditorSceneItemComponentsModel>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorSceneItemComponentsModel", "Created by EditorSceneItem");
    qmlRegisterUncreatableType<EditorSceneItemTransformComponentsModel>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorSceneItemTransformComponentsModel", "Created by EditorSceneItemComponentsModel");
    qmlRegisterUncreatableType<EditorSceneItemMaterialComponentsModel>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorSceneItemMaterialComponentsModel", "Created by EditorSceneItemComponentsModel");
    qmlRegisterUncreatableType<EditorSceneItemMeshComponentsModel>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorSceneItemMeshComponentsModel", "Created by EditorSceneItemComponentsModel");
    qmlRegisterUncreatableType<EditorSceneItemLightComponentsModel>("com.theqtcompany.SceneEditor3D", 1, 0, "EditorSceneItemLightComponentsModel", "Created by EditorSceneItemComponentsModel");
    qmlRegisterUncreatableType<UndoHandler>("com.theqtcompany.SceneEditor3D", 1, 0, "UndoHandler", "Created by EditorScene");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
