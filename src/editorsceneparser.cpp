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
#include "editorsceneparser.h"
#include "qdummyobjectpicker.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QComponent>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>

#include <Qt3DRender/QTexture>

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QObjectPicker>

#include <Qt3DRender/QMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTorusMesh>

#include <Qt3DRender/QMaterial>
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QDiffuseSpecularMapMaterial>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DExtras/QNormalDiffuseMapMaterial>
#include <Qt3DExtras/QNormalDiffuseMapAlphaMaterial>
#include <Qt3DExtras/QNormalDiffuseSpecularMapMaterial>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QTextureImage>

#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QLight>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QSpotLight>
#include <Qt3DRender/QSceneLoader>

#include <Qt3DInput/QInputSettings>

#include <QtCore/QFile>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QVariant>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>

static const QString generatedQmlFileTag = QStringLiteral("// Qt3D Editor generated file");
static const QString generatorVersionTag = QStringLiteral("// v0.1");
static const QString frameGraphCameraTag = QStringLiteral("@FRAMEGRAPH_CAMERA_TAG@");
static const QString exposedPropertiesTag = QStringLiteral("@EXPOSED_PROPERTIES_TAG@");
static const QString endTag = QStringLiteral("@@@");
static const QString sceneStartTag = QStringLiteral("// --- Scene start ---");
static const QString resourceNameTemplate = QStringLiteral("qrc:///%1/r%2_%3");
static const QString resourceDirTemplate = QStringLiteral("%1_scene_res");
static const QString doubleQuotedStringTemplate = QStringLiteral("\"%1\"");
static const QString tempExportSuffix = QStringLiteral("_temp");
static const QString resourceFileTemplate = QStringLiteral("%1/%2/%3.qrc");
static const QString internalNamePrefix = QStringLiteral("__internal");
static const QString componentsStart = QStringLiteral("components: [");
static const QString componentsEnd = QStringLiteral("]");
static const QString idPropertyStr = QStringLiteral("id:");
static const QString cameraPropertyStr = QStringLiteral("camera:");
static const QString qrcFileStartTag = QStringLiteral("<file>");
static const QString qrcFileEndTag = QStringLiteral("</file>");
static const QString diffuseProperty = QStringLiteral("diffuse");
static const QString specularProperty = QStringLiteral("specular");
static const QString normalProperty = QStringLiteral("normal");
static const QString sourceProperty = QStringLiteral("source");
static const QString enumPropertyTag = QStringLiteral(" // ENUM:");
static const QString autoSavePostfix = QStringLiteral(".autosave");

EditorSceneParser::EditorSceneParser(QObject *parent)
    : QObject(parent)
    , m_indentLevel(0)
    , m_resourceIdCounter(0)

{
    m_spaceArray.fill(QLatin1Char(' '), 0x1000);
    m_typeStrings
            << QStringLiteral("Scene3D")
            << QStringLiteral("Entity")
            << QStringLiteral("Camera")
            << QStringLiteral("Transform")
            << QStringLiteral("RenderSettings")
            << QStringLiteral("InputSettings")
            << QStringLiteral("DiffuseMapMaterial")
            << QStringLiteral("DiffuseSpecularMapMaterial")
            << QStringLiteral("GoochMaterial")
            << QStringLiteral("NormalDiffuseMapMaterial")
            << QStringLiteral("NormalDiffuseMapAlphaMaterial")
            << QStringLiteral("NormalDiffuseSpecularMapMaterial")
            << QStringLiteral("PerVertexColorMaterial")
            << QStringLiteral("PhongAlphaMaterial")
            << QStringLiteral("PhongMaterial")
            << QStringLiteral("CuboidMesh")
            << QStringLiteral("Mesh") // CustomMesh
            << QStringLiteral("CylinderMesh")
            << QStringLiteral("PlaneMesh")
            << QStringLiteral("SphereMesh")
            << QStringLiteral("TorusMesh")
            << QStringLiteral("ObjectPicker")
            << QStringLiteral("Light")
            << QStringLiteral("DirectionalLight")
            << QStringLiteral("PointLight")
            << QStringLiteral("SpotLight")
            << QStringLiteral("SceneLoader")
            << QStringLiteral("Unknown");
    m_stream.setCodec("UTF-8");

    cacheProperties(Entity, new Qt3DCore::QEntity());
    cacheProperties(Camera, new Qt3DRender::QCamera());
    cacheProperties(Transform, new Qt3DCore::QTransform());
    cacheProperties(RenderSettings, new Qt3DRender::QRenderSettings());
    cacheProperties(InputSettings, new Qt3DInput::QInputSettings());
    cacheProperties(DiffuseMapMaterial, new Qt3DExtras::QDiffuseMapMaterial());
    cacheProperties(DiffuseSpecularMapMaterial, new Qt3DExtras::QDiffuseSpecularMapMaterial());
    cacheProperties(GoochMaterial, new Qt3DExtras::QGoochMaterial());
    cacheProperties(NormalDiffuseMapMaterial, new Qt3DExtras::QNormalDiffuseMapMaterial());
    cacheProperties(NormalDiffuseMapAlphaMaterial, new Qt3DExtras::QNormalDiffuseMapAlphaMaterial());
    cacheProperties(NormalDiffuseSpecularMapMaterial, new Qt3DExtras::QNormalDiffuseSpecularMapMaterial());
    cacheProperties(PerVertexColorMaterial, new Qt3DExtras::QPerVertexColorMaterial());
    cacheProperties(PhongAlphaMaterial, new Qt3DExtras::QPhongAlphaMaterial());
    cacheProperties(PhongMaterial, new Qt3DExtras::QPhongMaterial());
    cacheProperties(CuboidMesh, new Qt3DExtras::QCuboidMesh());
    cacheProperties(CustomMesh, new Qt3DRender::QMesh());
    cacheProperties(CylinderMesh, new Qt3DExtras::QCylinderMesh());
    cacheProperties(PlaneMesh, new Qt3DExtras::QPlaneMesh());
    cacheProperties(SphereMesh, new Qt3DExtras::QSphereMesh());
    cacheProperties(TorusMesh, new Qt3DExtras::QTorusMesh());
    cacheProperties(ObjectPicker, new Qt3DRender::QObjectPicker());
    cacheProperties(Light, new Qt3DRender::QLight());
    cacheProperties(DirectionalLight, new Qt3DRender::QDirectionalLight());
    cacheProperties(PointLight, new Qt3DRender::QPointLight());
    cacheProperties(SpotLight, new Qt3DRender::QSpotLight());
    cacheProperties(SceneLoader, new Qt3DRender::QSceneLoader());
}

EditorSceneParser::~EditorSceneParser()
{
    foreach (QObject *obj, m_defaultObjectMap.values())
        delete obj;
}

// Caller retains ownership of sceneEntity and frameGraph
bool EditorSceneParser::exportQmlScene(Qt3DCore::QEntity *sceneEntity, const QUrl &fileUrl,
                                       Qt3DCore::QEntity *activeSceneCamera, bool autosave)
{
    // TODO: Maybe change exporting so that use selects the target .qrc file, and generate
    // TODO: qml in the subdirectory? That way user only needs to add the .qrc to the project.
    resetParser();

    // Figure out the final target qml file and directory
    QString qmlFinalFileAbsoluteFilePath = fileUrl.toLocalFile();
    if (autosave)
        qmlFinalFileAbsoluteFilePath.append(autoSavePostfix);
    qDebug() << "Exporting scene to " << qmlFinalFileAbsoluteFilePath;
    QFile qmlFinalFile(qmlFinalFileAbsoluteFilePath);
    QFileInfo qmlFinalFileInfo(qmlFinalFile);
    QDir finalTargetDir = qmlFinalFileInfo.absoluteDir();
    m_resourceDirName = resourceDirTemplate.arg(qmlFinalFileInfo.baseName());
    if (autosave)
        m_resourceDirName.append(autoSavePostfix);

    QString finalResourceFileName = getAbsoluteResourceFileName(qmlFinalFileInfo,
                                                                m_resourceDirName);

    // Create a unique backup suffix from current time
    QDateTime currentTime = QDateTime::currentDateTime();
    QString uniqueSuffix = currentTime.toString(QStringLiteral("yyyyMMddHHmmsszzz"));
    QString backupResDirName = m_resourceDirName + uniqueSuffix;
    QString backupQmlFileName = qmlFinalFileAbsoluteFilePath + uniqueSuffix;

    // Figure out the temporary qml file and resource directory name
    QString qmlTempFileAbsoluteFilePath = qmlFinalFileAbsoluteFilePath + uniqueSuffix + tempExportSuffix;
    QFile qmlTempFile(qmlTempFileAbsoluteFilePath);
    QFileInfo qmlTempFileInfo(qmlTempFile);
    QString tempResourceDirName = m_resourceDirName + uniqueSuffix + tempExportSuffix;
    QString tempResourceFileName = getAbsoluteResourceFileName(qmlTempFileInfo,
                                                               tempResourceDirName);

    if (!qmlTempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open the qml file for writing:" << qmlTempFileAbsoluteFilePath;
        return false;
    }

    QString outputString;

    m_stream.reset();
    m_stream.setString(&outputString, QIODevice::WriteOnly);

    m_stream << generatedQmlFileTag << endl
             << generatorVersionTag << endl
             << QStringLiteral("// This file was generated by a tool. Do not modify it manually!") << endl
             << QStringLiteral("import QtQuick.Scene3D 2.0") << endl
             << QStringLiteral("import Qt3D.Core 2.0") << endl
             << QStringLiteral("import Qt3D.Render 2.0") << endl
             << QStringLiteral("import Qt3D.Extras 2.0") << endl
             << QStringLiteral("import Qt3D.Input 2.0") << endl
             << endl;

    outStartType(Scene3D);

    m_stream << exposedPropertiesTag << endl;

    m_stream << indent() << QStringLiteral("aspects: [\"render\",\"input\"]") << endl;

    outHelperFunctions();

    // Top level container entity
    outStartType(Entity);

    // Create a dummy framegraph and output it
    Qt3DRender::QRenderSettings *renderSettings = new Qt3DRender::QRenderSettings();
    renderSettings->setObjectName(QStringLiteral("Scene render settings"));
    Qt3DExtras::QForwardRenderer *forwardRenderer = new Qt3DExtras::QForwardRenderer();
    forwardRenderer->setClearColor(Qt::lightGray);
    forwardRenderer->setCamera(activeSceneCamera);
    renderSettings->setActiveFrameGraph(forwardRenderer);
    Qt3DInput::QInputSettings *inputSettings = new Qt3DInput::QInputSettings();
    inputSettings->setObjectName(QStringLiteral("Scene Input settings"));

    m_stream << indent() << componentsStart << outComponent(renderSettings) << QStringLiteral(",")
             << outComponent(inputSettings) << componentsEnd << endl << endl;

    forwardRenderer->setCamera(nullptr);
    delete renderSettings;
    delete inputSettings;

    m_stream << indent() << sceneStartTag << endl;
    outEntity(sceneEntity);

    outEndType(); // Top level container entity
    outEndType(); // Scene3D

    // Replace pending tags
    QString replaceString;
    QTextStream replaceStream(&replaceString);

    // Replace exposed properties tag
    replaceString.clear();
    replaceStream.reset();
    m_indentLevel = 1;
    foreach (const IdPair &idPair, m_idMap.values()) {
        replaceStream << indent() << QStringLiteral("property alias ") << idPair.qmlId
                      << ": " << idPair.qmlId << endl;
    }
    outputString.replace(exposedPropertiesTag, replaceString);

    // Replace framegraph cameras with proper identifiers
    int frameGraphCameraIndex = outputString.indexOf(frameGraphCameraTag);
    while (frameGraphCameraIndex >= 0) {
        int endTagIndex = outputString.indexOf(endTag, frameGraphCameraIndex);
        int valueIndex = frameGraphCameraIndex + frameGraphCameraTag.length();
        quint64 tagValue = outputString.mid(valueIndex, endTagIndex - valueIndex).toULongLong();
        outputString.replace(frameGraphCameraIndex,
                             endTagIndex - frameGraphCameraIndex + endTag.length(),
                             m_idMap.value(tagValue).qmlId);
        frameGraphCameraIndex = outputString.indexOf(frameGraphCameraTag);
    }

    if (qmlTempFile.write(outputString.toUtf8()) < 0) {
        qWarning() << "Failed to write the qml file:" << qmlTempFileAbsoluteFilePath;
        qmlTempFile.remove();
        return false;
    }

    // Copy resources to temporary dir and generate a resource file.
    QDir tempResourceDir = finalTargetDir;
    if (m_exportResourceMap.size()) {
        // Make a temporary directory for exporting
        if (tempResourceDir.cd(tempResourceDirName)) {
            // Clean temporary resource dir if it exists for some reason
            tempResourceDir.removeRecursively();
            tempResourceDir = finalTargetDir;
            QCoreApplication::processEvents();
        }

        tempResourceDir.mkdir(tempResourceDirName);
        if (!tempResourceDir.cd(tempResourceDirName)) {
            qWarning() << "Failed to create a directory for resources:" << tempResourceDirName;
            qmlTempFile.remove();
            return false;
        }

        QFile resFile(tempResourceFileName);
        if (!resFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Failed to create a resource file:" << tempResourceFileName;
            qmlTempFile.remove();
            if (tempResourceDir.exists() && tempResourceDir != finalTargetDir)
                tempResourceDir.removeRecursively();
            return false;
        }

        QTextStream resStream(&resFile);
        resStream.setCodec("UTF-8");

        resStream << QStringLiteral("<RCC>") << endl
                  << QStringLiteral("    <qresource prefix=\"/") << m_resourceDirName
                  << QStringLiteral("/\">") << endl;

        QMapIterator<QUrl, QString> i(m_exportResourceMap);
        while (i.hasNext()) {
            i.next();
            QString source = i.key().toString();
            if (source.startsWith(QStringLiteral("qrc:")))
                source = source.mid(3);
            else
                source = i.key().toLocalFile();

            QString target = tempResourceDir.absoluteFilePath(i.value().mid(7));
            int removeIndex = target.lastIndexOf(m_resourceDirName);
            if (removeIndex >= 0)
                target.remove(removeIndex, m_resourceDirName.length() + 1);

            if (!QFile::copy(source, target)) {
                qWarning() << "Failed to copy a file:" << source << "to:" << target;
                qmlTempFile.remove();
                if (tempResourceDir.exists() && tempResourceDir != finalTargetDir)
                    tempResourceDir.removeRecursively();
                return false;
            }

            resStream << qrcFileStartTag
                      << target.mid(target.lastIndexOf(QLatin1Char('/')) + 1)
                      << qrcFileEndTag << endl;
        }

        resStream << QStringLiteral("    </qresource>") << endl
                  << QStringLiteral("</RCC>") << endl;
    }

    // Rename existing export qml file and resource directory
    if (finalTargetDir.exists(m_resourceDirName)) {
        if (!finalTargetDir.rename(m_resourceDirName, backupResDirName)) {
            qWarning() << "Failed to rename the old resource dir:" << m_resourceDirName;
            qmlTempFile.remove();
            if (tempResourceDir.exists() && tempResourceDir != finalTargetDir)
                tempResourceDir.removeRecursively();
            return false;
        }
    }
    if (qmlFinalFile.exists()) {
        if (!qmlFinalFile.rename(backupQmlFileName)) {
            qWarning() << "Failed to rename the old qml file:" << qmlFinalFileAbsoluteFilePath;
            qmlTempFile.remove();
            if (tempResourceDir.exists() && tempResourceDir != finalTargetDir)
                tempResourceDir.removeRecursively();
            return false;
        }
    }

    // Rename the temporary generated files to final
    if (finalTargetDir.exists(tempResourceDirName)) {
        if (!finalTargetDir.rename(tempResourceDirName, m_resourceDirName)) {
            qWarning() << "Failed to rename the temp resource dir:" << tempResourceDirName;
            qmlTempFile.remove();
            if (tempResourceDir.exists() && tempResourceDir != finalTargetDir)
                tempResourceDir.removeRecursively();
            return false;
        }
    }

    if (!qmlTempFile.rename(qmlFinalFileAbsoluteFilePath)) {
        qWarning() << "Failed to rename the temp qml file:" << (qmlTempFileAbsoluteFilePath);
        qmlTempFile.remove();
        return false;
    }

    // If everything went well, remove the renamed originals.
    QFile::remove(backupQmlFileName);
    tempResourceDir = finalTargetDir;
    if (tempResourceDir.cd(backupResDirName))
        tempResourceDir.removeRecursively();

    return true;
}

// Ownership of the returned root entity is passed to caller.
// cameraEntity parameter returns the active camera entity.
Qt3DCore::QEntity *EditorSceneParser::importQmlScene(const QUrl &fileUrl,
                                                     Qt3DCore::QEntity *&cameraEntity)
{
    qDebug() << "Importing scene from " << fileUrl;

    resetParser();

    QString qmlFileAbsoluteFilePath = fileUrl.toLocalFile();
    QFile qmlFile(qmlFileAbsoluteFilePath);
    QFileInfo qmlFileInfo(qmlFile);
    QString resourceDirName = resourceDirTemplate.arg(qmlFileInfo.baseName());
    QString resourceFileName = getAbsoluteResourceFileName(qmlFileInfo, resourceDirName);

    if (!qmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open the qml file for reading:" << qmlFileAbsoluteFilePath;
        return nullptr;
    }

    // If there is a resource file, read its contents
    QFile resourceFile(resourceFileName);
    if (resourceFile.exists()) {
        if (!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open the qrc file for reading:" << resourceFileName;
            return nullptr;
        }
        m_stream.reset();
        m_stream.setDevice(&resourceFile);

        QString targetUrlTemplate = QStringLiteral("file:///%1/%2/%3");
        targetUrlTemplate = targetUrlTemplate.arg(qmlFileInfo.absolutePath()).arg(resourceDirName);
        QString sourceResourceUrlTemplate = QStringLiteral("\"qrc:///%1/%2\"");
        sourceResourceUrlTemplate =  sourceResourceUrlTemplate.arg(resourceDirName);

        while (!m_stream.atEnd()) {
            QString line = m_stream.readLine().trimmed();
            if (line.startsWith(qrcFileStartTag)) {
                QString fileName = line.mid(qrcFileStartTag.size(), line.size()
                                            - qrcFileStartTag.size() - qrcFileEndTag.size());
                m_importResourceMap.insert(sourceResourceUrlTemplate.arg(fileName),
                                           QUrl(targetUrlTemplate.arg(fileName)));
            }
        }
    }

    Qt3DCore::QEntity *sceneEntity = nullptr;

    m_stream.reset();
    m_stream.setDevice(&qmlFile);

    QList<Qt3DCore::QEntity *> entityStack;
    Qt3DCore::QEntity *currentEntity = nullptr;
    Qt3DCore::QComponent *currentComponent = nullptr;
    QMap<QString, Qt3DCore::QComponent *> componentMap;
    QMap<QString, Qt3DCore::QEntity *> entityMap;
    bool startFound = false;
    bool parsingComponentIds = false;
    QString cameraId;
    EditorItemType currentItemType = Unknown;

    // TODO: properly handle errors during parsing so that malformed file won't crash the app
    // TODO: or leak memory

    while (!m_stream.atEnd()) {
        QString line = m_stream.readLine().trimmed();
        if (startFound) {
            if (parsingComponentIds){
                if (line.compare(componentsEnd) == 0) {
                    parsingComponentIds = false;
                } else {
                    if (line.endsWith(QStringLiteral(",")))
                        line.chop(1);
                    currentEntity->addComponent(componentMap.value(line));
                }
            } else if (line.endsWith(QStringLiteral("{"))) {
                currentItemType = m_typeMap.value(line.left(line.size() - 2), Unknown);
                Qt3DCore::QEntity *entity = createEntity(currentItemType);
                if (entity) {
                    if (currentEntity)
                        entity->setParent(currentEntity);
                    currentEntity = entity;
                    if (!sceneEntity)
                        sceneEntity = entity;
                } else {
                    Qt3DCore::QComponent *component = createComponent(currentItemType);
                    if (component)
                        currentComponent = component;
                }
            } else if (line.compare(QStringLiteral("}")) == 0) {
                if (currentComponent) {
                    currentComponent = nullptr;
                } else {
                    if (currentEntity)
                        currentEntity = qobject_cast<Qt3DCore::QEntity *>(currentEntity->parent());
                }
                currentItemType = itemType(currentEntity);
            } else if (line.startsWith(idPropertyStr)) {
                int separatorIndex = line.indexOf(QStringLiteral(" // "));
                QString idString = line.mid(idPropertyStr.size() + 1,
                                            separatorIndex - idPropertyStr.size() - 1);
                if (currentComponent) {
                    componentMap.insert(idString, currentComponent);
                } else {
                    entityMap.insert(idString, currentEntity);
                    currentEntity->setObjectName(line.mid(separatorIndex + 4));
                }
            } else if (line.compare(componentsStart) == 0) {
                parsingComponentIds = true;
            } else {
                int separatorIndex = line.indexOf(QStringLiteral(":"));
                QString propertyName = line.left(separatorIndex);
                QString propertyValue = line.mid(separatorIndex + 2);
                if (currentComponent)
                    parseAndSetProperty(propertyName, propertyValue, currentComponent, currentItemType);
                else
                    parseAndSetProperty(propertyName, propertyValue, currentEntity, currentItemType);
            }
        } else if (line.startsWith(cameraPropertyStr)) {
            cameraId = line.mid(line.indexOf(QStringLiteral(":")) + 2);
        } else if (line.compare(sceneStartTag) == 0) {
            startFound = true;
        }
    }

    cameraEntity = entityMap.value(cameraId);

    return sceneEntity;
}

void EditorSceneParser::cacheProperties(EditorSceneParser::EditorItemType type,
                                        QObject *defaultObject)
{
    // Store the default object for property comparisons
    m_defaultObjectMap.insert(type, defaultObject);

    // Cache metaproperties of supported types (but not their parent class types)
    const QMetaObject *meta = defaultObject->metaObject();
    QVector<QMetaProperty> properties;
    properties.reserve(meta->propertyCount() - meta->propertyOffset());
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); i++) {
        if (meta->property(i).isWritable())
            properties.append(meta->property(i));
    }

    if (type != Light && qobject_cast<Qt3DRender::QLight *>(defaultObject)) {
        // For specialized lights, add the parent class properties
        properties.append(m_propertyMap.value(Light));
    }

    // TODO: "enabled" property for all entities, if we are going to support that in editor

    m_propertyMap.insert(type, properties);

    m_typeMap.insert(m_typeStrings.at(type), type);
}

void EditorSceneParser::resetParser()
{
    m_indentLevel = 0;
    m_resourceIdCounter = 0;
    m_idMap.clear();
    m_exportResourceMap.clear();
    m_importResourceMap.clear();
    m_resourceDirName.clear();
}

// Returns the generated id string
void EditorSceneParser::outStartType(EditorItemType type)
{
    m_stream << indent() << m_typeStrings.at(type) << QStringLiteral(" {") << endl;
    m_indentLevel++;
}

void EditorSceneParser::outEndType()
{
    m_indentLevel--;
    m_stream << indent() << QStringLiteral("}") << endl;
}

EditorSceneParser::IdPair EditorSceneParser::generateItemId(EditorItemType type,
                                                            QObject *item)
{
    IdPair idPair;
    QString objName = item->objectName();
    if (!objName.isEmpty()) {
        if (!objName.startsWith(internalNamePrefix)) {
            idPair.qmlId = QString(item->objectName().toLatin1().toLower().replace(' ', '_'));
            idPair.origName = item->objectName();
        }
    } else {
        // Generate id from parent object id and item type
        int currentCount = m_currentEntityComponentTypeMap.value(type, 0);
        m_currentEntityComponentTypeMap.insert(type, ++currentCount);
        QString parentId = m_idMap.value(quint64(item->parent())).qmlId;
        QString idTemplate = QStringLiteral("%1_%2");
        idPair.qmlId = idTemplate.arg(parentId).arg(m_typeStrings.at(type).toLower());
        if (currentCount > 1)
            idPair.qmlId.append(QString::number(currentCount));
    }

    return idPair;
}

void EditorSceneParser::outItemId(const IdPair idPair)
{
    if (!idPair.qmlId.isEmpty()) {
        m_stream << indent() << idPropertyStr << QStringLiteral(" ") << idPair.qmlId;
        if (!idPair.origName.isEmpty())
            m_stream << QStringLiteral(" // ") << idPair.origName;
        m_stream << endl;
    }
}

void EditorSceneParser::outEntity(Qt3DCore::QEntity *entity)
{
    // Determine entity type
    EditorItemType type = itemType(entity);
    IdPair entityId = generateItemId(type, entity);

    if (!entityId.qmlId.isEmpty()) {
        outStartType(type);
        m_idMap.insert(quint64(entity), entityId);
        outItemId(entityId);

        outGenericProperties(type, entity);

        // Output components
        Qt3DCore::QComponentVector componentList = entity->components();
        int componentCount = componentList.size();
        bool sceneLoader = false;
        if (componentCount) {
            QStringList componentIds;
            m_currentEntityComponentTypeMap.clear();
            for (int i = 0; i < componentCount; i++) {
                // Skip internal camera lens and transform for cameras
                if (type == Camera) {
                    Qt3DRender::QCamera *camera = qobject_cast<Qt3DRender::QCamera *>(entity);
                    if (camera && (componentList.at(i) == camera->lens()
                                   || componentList.at(i) == camera->transform())) {
                        continue;
                    }
                }
                if (qobject_cast<Qt3DRender::QSceneLoader *>(componentList.at(i)))
                    sceneLoader = true;
                QString componentId = outComponent(componentList.at(i));
                // Empty component id typically means internal component
                if (!componentId.isEmpty())
                    componentIds.append(componentId);
            }

            if (componentIds.size()) {
                m_stream << indent() << componentsStart << endl;
                for (int i = 0; i < componentIds.size(); i++) {
                    m_indentLevel++;
                    m_stream << indent() << componentIds.at(i);
                    if (i < (componentIds.size() - 1))
                        m_stream << QStringLiteral(",");
                    m_stream << endl;
                    m_indentLevel--;
                }
                m_stream << indent() << componentsEnd << endl;
            }
        }

        // Output child entities
        if (!sceneLoader) {
            foreach (QObject *child, entity->children()) {
                Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>(child);
                if (childEntity)
                    outEntity(childEntity);
            }
        }

        outEndType();
    }
}

QString EditorSceneParser::outComponent(Qt3DCore::QComponent *component)
{
    EditorItemType type = itemType(component);

    IdPair componentId = generateItemId(type, component);

    // Do not create new component if there already exists an item for it
    if (!componentId.qmlId.isEmpty() && !m_idMap.contains(quint64(component))) {
        m_idMap.insert(quint64(component), componentId);
        outStartType(type);
        outItemId(componentId);

        // Some types require custom handling of properties
        switch (type) {
        case RenderSettings:
            outRenderSettings(component);
            break;
            // Materials with textures. Intentional fall-through.
        case DiffuseMapMaterial:
        case DiffuseSpecularMapMaterial:
        case NormalDiffuseMapMaterial:
        case NormalDiffuseMapAlphaMaterial:
        case NormalDiffuseSpecularMapMaterial:
            outTexturedMaterial(type, component);
            break;
        case SceneLoader:
            outSceneLoader(component);
            break;
        default:
            // The rest
            outGenericProperties(type, component);
            break;
        }

        // TODO the missing components

        outEndType();
    }

    return componentId.qmlId;
}

void EditorSceneParser::outRenderSettings(Qt3DCore::QComponent *component)
{
    Qt3DRender::QRenderSettings *renderSettings = qobject_cast<Qt3DRender::QRenderSettings *>(component);
    Qt3DRender::QFrameGraphNode *activeFrameGraph = renderSettings->activeFrameGraph();
    Qt3DExtras::QForwardRenderer *forwardRenderer = qobject_cast<Qt3DExtras::QForwardRenderer *>(activeFrameGraph);
    if (forwardRenderer) {
        m_stream << indent() << QStringLiteral("activeFrameGraph: ForwardRenderer {") << endl;
        m_indentLevel++;
        m_stream << indent() << QStringLiteral("clearColor: ")
                 << variantToQMLString(QVariant(forwardRenderer->clearColor())) << endl;
        // Placeholder for camera to be replaced later when all components are parsed
        m_stream << indent() << cameraPropertyStr << QStringLiteral(" ") << frameGraphCameraTag
                 << quint64(forwardRenderer->camera()) << endTag << endl;
        // TODO: do we need to set the viewport?
        m_indentLevel--;
        m_stream << indent() << QStringLiteral("}") << endl;
    } else {
        // TODO other renderers
    }

}

void EditorSceneParser::outTexturedMaterial(EditorSceneParser::EditorItemType type,
                                            Qt3DCore::QComponent *component)
{
    Qt3DRender::QAbstractTexture *diffuse = nullptr;
    Qt3DRender::QAbstractTexture *specular = nullptr;
    Qt3DRender::QAbstractTexture *normal = nullptr;

    int textureTypes = getTextureProviders(type, component, diffuse, specular, normal);

    // First add generic properties
    QVector<QMetaProperty> properties = m_propertyMap.value(type);
    QObject *defaultComponent = m_defaultObjectMap.value(type);
    for (int i = 0; i < properties.size(); i++) {
        // Skip texture properties
        QString propertyName = properties.at(i).name();
        if (propertyName == diffuseProperty && (textureTypes & DiffuseTexture))
            outTextureProperty(propertyName, diffuse);
        else if (propertyName == specularProperty && (textureTypes & SpecularTexture))
            outTextureProperty(propertyName, specular);
        else if (propertyName == normalProperty && (textureTypes & NormalTexture))
            outTextureProperty(propertyName, normal);
        else
            outGenericProperty(component, properties.at(i), defaultComponent);
    }
}

void EditorSceneParser::outTextureProperty(const QString &propertyName,
                                           Qt3DRender::QAbstractTexture *textureProvider)
{
    // Get the url of the texture
    if (textureProvider->textureImages().size()) {
        const Qt3DRender::QTextureImage *textureImage =
                qobject_cast<const Qt3DRender::QTextureImage *>(textureProvider->textureImages().at(0));
        if (textureImage) {
            QString imageName = urlToResourceString(textureImage->source());
            m_stream << indent() << propertyName << QStringLiteral(": \"")
                     << imageName
                     << QStringLiteral("\"") << endl;
        }
    }
}

void EditorSceneParser::outGenericProperties(EditorSceneParser::EditorItemType type,
                                             QObject *obj)
{
    QVector<QMetaProperty> properties = m_propertyMap.value(type);
    QObject *defaultObject = m_defaultObjectMap.value(type);
    for (int i = 0; i < properties.size(); i++)
        outGenericProperty(obj, properties.at(i), defaultObject);
}

void EditorSceneParser::outGenericProperty(QObject *obj, const QMetaProperty &property,
                                           const QObject *defaultComponent)
{
    // Only output property if it is different from default
    QVariant defaultValue = defaultComponent->property(property.name());
    QVariant objectValue = obj->property(property.name());
    QString valueStr;
    if (property.isEnumType()) {
        valueStr = QStringLiteral("%1.%2");
        QString scope = property.enumerator().scope();
        int sepIndex = scope.lastIndexOf(QStringLiteral("::"));
        if (sepIndex)
            scope = scope.mid(sepIndex + 2);
        // Horrible hack, but works for our limited scope
        if (scope.startsWith(QStringLiteral("Q")))
            scope = scope.mid(1);
        valueStr = valueStr.arg(scope).arg(property.enumerator().key(objectValue.toInt()));
        valueStr.append(enumPropertyTag).append(QString::number(objectValue.toInt()));
    } else {
        valueStr = variantToQMLString(objectValue);
    }

    if (defaultValue != objectValue)
        m_stream << indent() << property.name() << QStringLiteral(": ") << valueStr << endl;
}

void EditorSceneParser::outHelperFunctions()
{
    m_stream << indent() << QStringLiteral("function addComponentToEntity(entity, component) {") << endl;
    m_indentLevel++;
    m_stream << indent() << QStringLiteral("var entityComponents = []") << endl;
    m_stream << indent() << QStringLiteral("for (var i = 0; i < entity.components.length; i++)") << endl;
    m_indentLevel++;
    m_stream << indent() << QStringLiteral("entityComponents.push(entity.components[i])") << endl;
    m_indentLevel--;
    m_stream << indent() << QStringLiteral("entityComponents.push(component)") << endl;
    m_stream << indent() << QStringLiteral("entity.components = entityComponents") << endl;
    m_indentLevel--;
    m_stream << indent() << QStringLiteral("}") << endl;
    m_stream << indent() << QStringLiteral("function removeComponentFromEntity(entity, component) {") << endl;
    m_indentLevel++;
    m_stream << indent() << QStringLiteral("var entityComponents = []") << endl;
    m_stream << indent() << QStringLiteral("for (var i = 0; i < entity.components.length; i++) {") << endl;
    m_indentLevel++;
    m_stream << indent() << QStringLiteral("if (entity.components[i] !== component)") << endl;
    m_indentLevel++;
    m_stream << indent() << QStringLiteral("entityComponents.push(entity.components[i])") << endl;
    m_indentLevel--;
    m_indentLevel--;
    m_stream << indent() << QStringLiteral("}") << endl;
    m_stream << indent() << QStringLiteral("entity.components = entityComponents") << endl;
    m_indentLevel--;
    m_stream << indent() << QStringLiteral("}") << endl;
}

void EditorSceneParser::outSceneLoader(Qt3DCore::QComponent *component)
{
    outGenericProperties(SceneLoader, component);
}

QString EditorSceneParser::indent() const
{
    return QString(m_spaceArray.constData(), m_indentLevel * 4);
}

EditorSceneParser::EditorItemType EditorSceneParser::itemType(QObject *item) const
{
    if (item) {
        if (qobject_cast<Qt3DCore::QComponent *>(item)) {
            if (qobject_cast<Qt3DRender::QGeometryRenderer *>(item)) {
                if (qobject_cast<Qt3DRender::QMesh *>(item))
                    return CustomMesh;
                else if (qobject_cast<Qt3DExtras::QCuboidMesh *>(item))
                    return CuboidMesh;
                else if (qobject_cast<Qt3DExtras::QCylinderMesh *>(item))
                    return CylinderMesh;
                else if (qobject_cast<Qt3DExtras::QPlaneMesh *>(item))
                    return PlaneMesh;
                else if (qobject_cast<Qt3DExtras::QSphereMesh *>(item))
                    return SphereMesh;
                else if (qobject_cast<Qt3DExtras::QTorusMesh *>(item))
                    return TorusMesh;
            } else if (qobject_cast<Qt3DRender::QRenderSettings *>(item)) {
                return RenderSettings;
            } else if (qobject_cast<Qt3DInput::QInputSettings *>(item)) {
                return InputSettings;
            } else if (qobject_cast<QDummyObjectPicker *>(item)) {
                return ObjectPicker;
            } else if (qobject_cast<Qt3DRender::QDirectionalLight *>(item)) {
                return DirectionalLight;
            } else if (qobject_cast<Qt3DRender::QPointLight *>(item)) {
                return PointLight;
            } else if (qobject_cast<Qt3DRender::QSpotLight *>(item)) {
                return SpotLight;
            } else if (qobject_cast<Qt3DRender::QLight *>(item)) {
                return Light; // Must be checked last, as all the other lights inherit this one
            } else if (qobject_cast<Qt3DCore::QTransform *>(item)) {
                return Transform;
            } else if (qobject_cast<Qt3DRender::QMaterial *>(item)) {
                if (qobject_cast<Qt3DExtras::QDiffuseMapMaterial *>(item))
                    return DiffuseMapMaterial;
                else if (qobject_cast<Qt3DExtras::QDiffuseSpecularMapMaterial *>(item))
                    return DiffuseSpecularMapMaterial;
                else if (qobject_cast<Qt3DExtras::QGoochMaterial *>(item))
                    return GoochMaterial;
                else if (qobject_cast<Qt3DExtras::QNormalDiffuseMapMaterial *>(item))
                    return NormalDiffuseMapMaterial;
                else if (qobject_cast<Qt3DExtras::QNormalDiffuseMapAlphaMaterial *>(item))
                    return NormalDiffuseMapAlphaMaterial;
                else if (qobject_cast<Qt3DExtras::QNormalDiffuseSpecularMapMaterial *>(item))
                    return NormalDiffuseSpecularMapMaterial;
                else if (qobject_cast<Qt3DExtras::QPerVertexColorMaterial *>(item))
                    return PerVertexColorMaterial;
                else if (qobject_cast<Qt3DExtras::QPhongAlphaMaterial *>(item))
                    return PhongAlphaMaterial;
                else if (qobject_cast<Qt3DExtras::QPhongMaterial *>(item))
                    return PhongMaterial;
            } else if (qobject_cast<Qt3DRender::QSceneLoader *>(item)) {
                return SceneLoader;
            }
        } else if (qobject_cast<Qt3DCore::QEntity *>(item)) {
            if (qobject_cast<Qt3DRender::QCamera *>(item))
                return Camera;
            else
                return Entity;
        }

        if (!item->objectName().startsWith(internalNamePrefix))
            qWarning() << __FUNCTION__ << "Unknown item type for:" << item->objectName();
    }

    return Unknown;
}

QString EditorSceneParser::variantToQMLString(const QVariant &var)
{
    switch (var.type()) {
    case QVariant::Matrix4x4: {
        QString retVal = QStringLiteral("Qt.matrix4x4(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16)");
        QMatrix4x4 matrix = qvariant_cast<QMatrix4x4>(var);
        QVector4D v1 = matrix.row(0);
        QVector4D v2 = matrix.row(1);
        QVector4D v3 = matrix.row(2);
        QVector4D v4 = matrix.row(3);

        return retVal.arg(v1.x()).arg(v1.y()).arg(v1.z()).arg(v1.w())
                .arg(v2.x()).arg(v2.y()).arg(v2.z()).arg(v2.w())
                .arg(v3.x()).arg(v3.y()).arg(v3.z()).arg(v3.w())
                .arg(v4.x()).arg(v4.y()).arg(v4.z()).arg(v4.w());
    }
    case QVariant::Quaternion: {
        QString retVal = QStringLiteral("Qt.quaternion(%1,%2,%3,%4)");
        QQuaternion quat = qvariant_cast<QQuaternion>(var);
        return retVal.arg(quat.scalar()).arg(quat.x()).arg(quat.y()).arg(quat.z());
    }
    case QVariant::Vector3D: {
        QString retVal = QStringLiteral("Qt.vector3d(%1,%2,%3)");
        QVector3D vec = qvariant_cast<QVector3D>(var);
        return retVal.arg(vec.x()).arg(vec.y()).arg(vec.z());
    }
    case QVariant::Color:
    case QVariant::String: {
        return doubleQuotedStringTemplate.arg(var.toString());
    }
    case QVariant::Url: {
        return doubleQuotedStringTemplate.arg(urlToResourceString(var.toUrl()));
    }
    default:
        break;
    }
    return var.toString();
}

QVariant EditorSceneParser::QMLStringToVariant(QVariant::Type type, const QString &qmlStr)
{
    switch (type) {
    case QVariant::Matrix4x4:
        // Intentional fall-through
    case QVariant::Quaternion:
        // Intentional fall-through
    case QVariant::Vector3D: {
        QVector<float> reals;
        int startIndex = qmlStr.indexOf(QStringLiteral("(")) + 1;
        if (startIndex > 0) {
            QString cleanStr = qmlStr.mid(startIndex, qmlStr.size() - startIndex - 1);
            QStringList stringValues = cleanStr.split(QLatin1Char(','));
            reals.resize(stringValues.size());
            for (int i = 0; i < stringValues.size(); i++)
                reals[i] = stringValues.at(i).toFloat();
        }
        switch (type) {
        case QVariant::Matrix4x4: {
            if (reals.size() == 16)
                return QVariant::fromValue(QMatrix4x4(reals.constData()));
            else
                return QVariant::fromValue(QMatrix4x4());
        }
        case QVariant::Quaternion: {
            if (reals.size() == 4)
                return QVariant::fromValue(QQuaternion(reals.at(0), reals.at(1), reals.at(2), reals.at(3)));
            else
                return QVariant::fromValue(QQuaternion());
        }
        case QVariant::Vector3D: {
            if (reals.size() == 3)
                return QVariant::fromValue(QVector3D(reals.at(0), reals.at(1), reals.at(2)));
            else
                return QVariant::fromValue(QVector3D());
        }
        default:
            break;
        }
    }
    case QVariant::Color: {
        if (qmlStr.size() < 3)
            return QVariant::fromValue(QColor());
        else
            return QVariant::fromValue(QColor(qmlStr.mid(1, qmlStr.size() - 2)));
    }
    case QVariant::Url: {
        if (qmlStr.size() < 3) {
            return QVariant::fromValue(QUrl());
        } else {
            QUrl resourceUrl = m_importResourceMap.value(qmlStr);
            if (resourceUrl.isEmpty())
                return QVariant::fromValue(QUrl(qmlStr.mid(1, qmlStr.size() - 2)));
            else
                return QVariant::fromValue(resourceUrl);
        }
    }
    case QVariant::String: {
        if (qmlStr.size() < 3)
            return QVariant::fromValue(QString());
        else
            return QVariant::fromValue(QString(qmlStr.mid(1, qmlStr.size() - 2)));
    }
    case QVariant::Int: {
        int enumTagIndex = qmlStr.lastIndexOf(enumPropertyTag);
        if (enumTagIndex >= 0) {
            int enumValue = qmlStr.mid(enumTagIndex + enumPropertyTag.size()).toInt();
            return QVariant::fromValue(enumValue);
        }
    }
    default:
        break;
    }

    QVariant retval = QVariant::fromValue(qmlStr);
    retval.convert(type);

    return retval;
}

Qt3DCore::QEntity *EditorSceneParser::createEntity(EditorSceneParser::EditorItemType type)
{
    switch (type) {
    case Entity:
        return new Qt3DCore::QEntity();
    case Camera:
        return new Qt3DRender::QCamera();
    default:
        break;
    }

    return nullptr;
}

Qt3DCore::QComponent *EditorSceneParser::createComponent(EditorSceneParser::EditorItemType type)
{
    switch (type) {
    case Transform:
        return new Qt3DCore::QTransform();
    case RenderSettings:
        return new Qt3DRender::QRenderSettings();
    case InputSettings:
        return new Qt3DInput::QInputSettings();
    case DiffuseMapMaterial:
        return new Qt3DExtras::QDiffuseMapMaterial();
    case DiffuseSpecularMapMaterial:
        return new Qt3DExtras::QDiffuseSpecularMapMaterial();
    case GoochMaterial:
        return new Qt3DExtras::QGoochMaterial();
    case NormalDiffuseMapMaterial:
        return new Qt3DExtras::QNormalDiffuseMapMaterial();
    case NormalDiffuseMapAlphaMaterial:
        return new Qt3DExtras::QNormalDiffuseMapAlphaMaterial();
    case NormalDiffuseSpecularMapMaterial:
        return new Qt3DExtras::QNormalDiffuseSpecularMapMaterial();
    case PerVertexColorMaterial:
        return new Qt3DExtras::QPerVertexColorMaterial();
    case PhongAlphaMaterial:
        return new Qt3DExtras::QPhongAlphaMaterial();
    case PhongMaterial:
        return new Qt3DExtras::QPhongMaterial();
    case CuboidMesh:
        return new Qt3DExtras::QCuboidMesh();
    case CustomMesh:
        return new Qt3DRender::QMesh();
    case CylinderMesh:
        return new Qt3DExtras::QCylinderMesh();
    case PlaneMesh:
        return new Qt3DExtras::QPlaneMesh();
    case SphereMesh:
        return new Qt3DExtras::QSphereMesh();
    case TorusMesh:
        return new Qt3DExtras::QTorusMesh();
    case ObjectPicker:
        return new QDummyObjectPicker();
    case DirectionalLight:
        return new Qt3DRender::QDirectionalLight();
    case Light:
        return new Qt3DRender::QLight();
    case PointLight:
        return new Qt3DRender::QPointLight();
    case SpotLight:
        return new Qt3DRender::QSpotLight();
    case SceneLoader:
        return new Qt3DRender::QSceneLoader();
    default:
        break;
    }
    return nullptr;
}

void EditorSceneParser::parseAndSetProperty(const QString &propertyName,
                                            const QString &propertyValue,
                                            QObject *obj, EditorItemType type)
{
    switch (type) {
    case RenderSettings:
    case InputSettings:
        // We ignore settings for now
        break;
        // Materials with textures. Intentional fall-through.
    case DiffuseMapMaterial:
    case DiffuseSpecularMapMaterial:
    case NormalDiffuseMapMaterial:
    case NormalDiffuseMapAlphaMaterial:
    case NormalDiffuseSpecularMapMaterial:
        parseAndSetTextureProperty(propertyName, propertyValue, obj, type);
        break;
    case Transform:
        // We are only interested in the matrix property of the transform, as the rest are derived
        // from the matrix.
        if (propertyName == QStringLiteral("matrix"))
            parseAndSetGenericProperty(propertyName, propertyValue, obj);
        break;
    default:
        // The rest
        parseAndSetGenericProperty(propertyName, propertyValue, obj);
        break;
    }
}

void EditorSceneParser::parseAndSetGenericProperty(const QString &propertyName,
                                                   const QString &propertyValue,
                                                   QObject *obj)
{
    QVariant::Type propertyType = obj->property(propertyName.toLatin1()).type();
    obj->setProperty(propertyName.toLatin1(), QMLStringToVariant(propertyType, propertyValue));
}

void EditorSceneParser::parseAndSetTextureProperty(const QString &propertyName,
                                                   const QString &propertyValue,
                                                   QObject *obj,
                                                   EditorSceneParser::EditorItemType type)
{
    Qt3DRender::QAbstractTexture *diffuse = nullptr;
    Qt3DRender::QAbstractTexture *specular = nullptr;
    Qt3DRender::QAbstractTexture *normal = nullptr;
    Qt3DRender::QAbstractTexture *targetProvider = nullptr;

    int textureTypes = getTextureProviders(type, obj, diffuse, specular, normal);

    if (propertyName == diffuseProperty && (textureTypes & DiffuseTexture))
        targetProvider = diffuse;
    else if (propertyName == specularProperty && (textureTypes & SpecularTexture))
        targetProvider = specular;
    else if (propertyName == normalProperty && (textureTypes & NormalTexture))
        targetProvider = normal;

    if (targetProvider) {
        Qt3DRender::QTextureImage *textureImage = new Qt3DRender::QTextureImage();
        targetProvider->addTextureImage(textureImage);
        textureImage->setSource(m_importResourceMap.value(propertyValue));
    } else {
        parseAndSetGenericProperty(propertyName, propertyValue, obj);
    }
}

int EditorSceneParser::getTextureProviders(EditorSceneParser::EditorItemType type,
                                           QObject *component,
                                           Qt3DRender::QAbstractTexture *&diffuse,
                                           Qt3DRender::QAbstractTexture *&specular,
                                           Qt3DRender::QAbstractTexture *&normal)
{
    int textureTypes = 0;

    switch (type) {
    case DiffuseMapMaterial: {
        Qt3DExtras::QDiffuseMapMaterial *mat =
                qobject_cast<Qt3DExtras::QDiffuseMapMaterial *>(component);
        if (mat) {
            diffuse = mat->diffuse();
            specular = nullptr;
            normal = nullptr;
        }
        textureTypes = DiffuseTexture;
        break;
    }
    case DiffuseSpecularMapMaterial: {
        Qt3DExtras::QDiffuseSpecularMapMaterial *mat =
                qobject_cast<Qt3DExtras::QDiffuseSpecularMapMaterial *>(component);
        if (mat) {
            diffuse = mat->diffuse();
            specular = mat->specular();
            normal = nullptr;
        }
        textureTypes = DiffuseTexture | SpecularTexture;
        break;
    }
    case NormalDiffuseMapMaterial: {
        Qt3DExtras::QNormalDiffuseMapMaterial *mat =
                qobject_cast<Qt3DExtras::QNormalDiffuseMapMaterial *>(component);
        if (mat) {
            diffuse = mat->diffuse();
            specular = nullptr;
            normal = mat->normal();
        }
        textureTypes = DiffuseTexture | NormalTexture;
        break;
    }
    case NormalDiffuseMapAlphaMaterial: {
        Qt3DExtras::QNormalDiffuseMapAlphaMaterial *mat =
                qobject_cast<Qt3DExtras::QNormalDiffuseMapAlphaMaterial *>(component);
        if (mat) {
            diffuse = mat->diffuse();
            specular = nullptr;
            normal = mat->normal();
        }
        textureTypes = DiffuseTexture | NormalTexture;
        break;
    }
    case NormalDiffuseSpecularMapMaterial: {
        Qt3DExtras::QNormalDiffuseSpecularMapMaterial *mat =
                qobject_cast<Qt3DExtras::QNormalDiffuseSpecularMapMaterial *>(component);
        if (mat) {
            diffuse = mat->diffuse();
            specular = mat->specular();
            normal = mat->normal();
        }
        textureTypes = DiffuseTexture | SpecularTexture | NormalTexture;
        break;
    }
    default:
        break;
    }

    return textureTypes;
}

QString EditorSceneParser::getAbsoluteResourceFileName(const QFileInfo &qmlFileInfo,
                                                       const QString &resourceDirName)
{
    return resourceFileTemplate.arg(qmlFileInfo.absolutePath())
            .arg(resourceDirName).arg(qmlFileInfo.baseName());
}

QString EditorSceneParser::urlToResourceString(const QUrl &url)
{
    QString urlString = m_exportResourceMap.value(url);

    if (urlString.isEmpty()) {
        QString fileName = url.toString();
        // If we are sourcing generated files, strip the "r<number>_" prefix from them
        // to avoid consecutive load/save cycles gradually changing the names.
        QString generatedExp = QStringLiteral("%1/r*\\d_");
        generatedExp = generatedExp.arg(m_resourceDirName);
        QRegularExpression re(generatedExp);
        fileName.replace(re, QString());
        fileName = fileName.mid(fileName.lastIndexOf(QLatin1Char('/')) + 1);
        urlString = resourceNameTemplate.arg(m_resourceDirName)
                .arg(m_resourceIdCounter++).arg(fileName);
        m_exportResourceMap.insert(url, urlString);
    }

    return urlString;
}
