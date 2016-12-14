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
#include "editorscenesaver.h"
#include "editorutils.h"

#include <Qt3DCore/qentity.h>
#include <private/qsceneexportfactory_p.h>
#include <private/qsceneexporter_p.h>
#include <private/qsceneimportfactory_p.h>
#include <private/qsceneimporter_p.h>

#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qtemporarydir.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qdatetime.h>

namespace {

const QString autoSavePostfix = QStringLiteral("_autosave");
const QString saveSuffix = QStringLiteral(".qt3dscene");
const QString exportNameTemplate = QStringLiteral("%1_scene_res");
const QString editorDataFile = QStringLiteral("qt3dscene_editor_data.json");
const qint32 saveFileVersion = 1;
const QByteArray saveFileId = QByteArrayLiteral("Qt3DSceneEditor_SaveFile");
const QString activeCameraKey = QStringLiteral("activeCamera");
const QString rootEntityNameKey = QStringLiteral("rootEntityName");

} // namespace

EditorSceneSaver::EditorSceneSaver(QObject *parent)
    : QObject(parent)
    , m_loadDir(nullptr)
{
}

EditorSceneSaver::~EditorSceneSaver()
{
    delete m_loadDir;
}

bool EditorSceneSaver::saveScene(Qt3DCore::QEntity *sceneEntity,
                                 const QString &activeSceneCamera,
                                 const QString &saveFileName,
                                 bool autosave)
{
    // Save consists of a single JSON file with .qt3dscene extension and the exported QGLTF scene
    // in a separate subfolder in the same folder as the main save file.

    // The save is first created in a temp directory to ensure saving over existing save doesn't
    // partially overwrite the old save in case of error.
    QString finalFullSaveFilePathName = saveFileName;
    if (!finalFullSaveFilePathName.endsWith(saveSuffix))
        finalFullSaveFilePathName.append(saveSuffix);
    QFileInfo saveFileInfo(finalFullSaveFilePathName);
    QString finalSavePath = saveFileInfo.path();
    QString finalFileName = saveFileInfo.fileName();
    QString saveExportName = exportNameTemplate.arg(saveFileInfo.completeBaseName());
    QDir finalSaveDir = saveFileInfo.absoluteDir();
    if (autosave) {
        finalFullSaveFilePathName.append(autoSavePostfix);
        finalFileName.append(autoSavePostfix);
        saveExportName.append(autoSavePostfix);
    }
    QString finalScenePath = finalSavePath + QStringLiteral("/") + saveExportName;

    // Save scene to a temp folder using GLTF export plugin
    QTemporaryDir exportDir(finalScenePath + QStringLiteral("_temp_save_XXXXXX"));

    const QStringList keys = Qt3DRender::QSceneExportFactory::keys();
    for (const QString &key : keys) {
        Qt3DRender::QSceneExporter *exporter =
                Qt3DRender::QSceneExportFactory::create(key, QStringList());
        if (exporter != nullptr && key == QStringLiteral("gltfexport")) {
            QVariantHash options;
            if (!exporter->exportScene(sceneEntity, exportDir.path(), saveExportName, options)) {
                qWarning() << "Failed to export GLTF when saving the scene";
                return false;
            }
            break;
        }
    }

    // Create new save file in the temp folder
    QJsonDocument jsonDoc;
    QJsonObject editorData;
    editorData[activeCameraKey] = activeSceneCamera;
    editorData[rootEntityNameKey] = sceneEntity->objectName();
    jsonDoc.setObject(editorData);
    const QString tempScenePath = exportDir.path() + QStringLiteral("/") + saveExportName;
    const QString saveFilePath = exportDir.path() + QStringLiteral("/") + finalFileName;
    QFile saveFile(saveFilePath);
    if (saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QByteArray json = jsonDoc.toJson();
        saveFile.write(json);
        saveFile.close();
    } else {
        qWarning() << "Failed to create editor data file when saving the scene";
        return false;
    }

    // Create a unique backup suffix from current time
    QDateTime currentTime = QDateTime::currentDateTime();
    QString uniqueSuffix = currentTime.toString(QStringLiteral("yyyyMMddHHmmsszzz"));
    QString backupExportName = saveExportName + uniqueSuffix;
    QString backupSaveFileName = finalFullSaveFilePathName + uniqueSuffix;

    // Rename the old save file and exported resources and savefile
    if (finalSaveDir.exists(saveExportName)) {
        if (!finalSaveDir.rename(saveExportName, backupExportName)) {
            qWarning() << "Failed to rename the old resource dir:" << saveExportName;
            return false;
        }
    }
    QFile oldSaveFile(finalFullSaveFilePathName);
    if (oldSaveFile.exists()) {
        if (!oldSaveFile.rename(backupSaveFileName)) {
            qWarning() << "Failed to rename the old save file:" << finalFullSaveFilePathName;
            finalSaveDir.rename(backupExportName, saveExportName);
            return false;
        }
    }

    // Rename the temporary files as finals
    if (!QFile::rename(tempScenePath, finalScenePath)) {
        qWarning() << "Failed to rename the temp scene:" << tempScenePath << "->" << finalScenePath;
        return false;
    }
    if (!saveFile.rename(finalFullSaveFilePathName)) {
        qWarning() << "Failed to rename the temp save file:" << saveFilePath <<
                      "->" << finalFullSaveFilePathName;
        return false;
    }

    // If everything went well, remove the renamed originals.
    QFile::remove(backupSaveFileName);
    if (finalSaveDir.cd(backupExportName))
        finalSaveDir.removeRecursively();

    return true;
}

Qt3DCore::QEntity *EditorSceneSaver::loadScene(const QString &fileName, Qt3DCore::QEntity *camera)
{
    Qt3DCore::QEntity *loadedScene = nullptr;
    camera = nullptr;

    // Read the scene data
    QFile jsonFile(fileName);
    QString activeCameraId;
    QString sceneName;
    if (jsonFile.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = jsonFile.readAll();
        jsonFile.close();
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
        QJsonObject editorData = jsonDocument.object();
        activeCameraId = editorData.value(activeCameraKey).toString();
        sceneName = editorData.value(rootEntityNameKey).toString();
    } else {
        qWarning() << "Failed to open scene save file:" << fileName;
        return nullptr;
    }

    // Import scene from temp folder
    const QStringList keys = Qt3DRender::QSceneImportFactory::keys();
    for (const QString &key : keys) {
        Qt3DRender::QSceneImporter *importer =
                Qt3DRender::QSceneImportFactory::create(key, QStringList());
        if (importer != nullptr && key == QStringLiteral("gltf")) {
            QFileInfo saveFileInfo(fileName);
            const QString exportName = exportNameTemplate.arg(saveFileInfo.completeBaseName());
            QString sceneSource = saveFileInfo.absolutePath();
            sceneSource += QStringLiteral("/");
            sceneSource += exportName;
            sceneSource += QStringLiteral("/");
            sceneSource += exportName;
            sceneSource += QStringLiteral(".qgltf");
            importer->setSource(QUrl::fromLocalFile(sceneSource));
            loadedScene = importer->scene();
            break;
        }
    }

    if (!loadedScene) {
        qWarning() << "Failed to load the saved scene";
        return nullptr;
    }

    loadedScene->setObjectName(sceneName);

    // Find the active camera
    camera = EditorUtils::findEntityByName(loadedScene, activeCameraId);
    if (!EditorUtils::entityCameraLens(camera))
        camera = nullptr;

    return loadedScene;
}

void EditorSceneSaver::deleteSavedScene(const QString &saveFileName, bool autosave)
{
    QString fullSaveFilePathName = saveFileName;
    if (!fullSaveFilePathName.endsWith(saveSuffix))
        fullSaveFilePathName.append(saveSuffix);
    QFileInfo saveFileInfo(fullSaveFilePathName);
    QString savePath = saveFileInfo.path();
    QString saveExportName = exportNameTemplate.arg(saveFileInfo.completeBaseName());
    if (autosave) {
        fullSaveFilePathName.append(autoSavePostfix);
        saveExportName.append(autoSavePostfix);
    }
    QDir sceneResourcesDir(savePath + QStringLiteral("/") + saveExportName);
    sceneResourcesDir.removeRecursively();
    QFile::remove(fullSaveFilePathName);
}
