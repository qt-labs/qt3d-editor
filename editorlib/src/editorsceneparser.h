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
#ifndef EDITORSCENEPARSER_H
#define EDITORSCENEPARSER_H

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QMap>
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <QtCore/QFileInfo>

namespace Qt3DCore {
    class QEntity;
    class QComponent;
}
namespace Qt3DRender {
    class QAbstractTexture;
    class QGeometry;
}

class EditorSceneParser : public QObject
{
    Q_OBJECT
    enum EditorItemType {
        Scene3D = 0,
        // Entities
        Entity,
        Camera,
        // Components
        Transform,
        RenderSettings,
        InputSettings,
        DiffuseMapMaterial,
        DiffuseSpecularMapMaterial,
        GoochMaterial,
        NormalDiffuseMapMaterial,
        NormalDiffuseMapAlphaMaterial,
        NormalDiffuseSpecularMapMaterial,
        PerVertexColorMaterial,
        PhongAlphaMaterial,
        PhongMaterial,
        GenericMaterial,
        CuboidMesh,
        CustomMesh,
        CylinderMesh,
        PlaneMesh,
        SphereMesh,
        TorusMesh,
        GenericMesh,
        Attribute,
        Buffer,
        Geometry,
        ObjectPicker,
        DirectionalLight,
        PointLight,
        SpotLight,
        SceneLoader,
        Unknown
    };

public:
    explicit EditorSceneParser(QObject *parent = 0);
    ~EditorSceneParser();

    bool exportQmlScene(Qt3DCore::QEntity *sceneEntity, const QUrl &fileUrl,
                        Qt3DCore::QEntity *activeSceneCamera, bool autosave = false);
    Qt3DCore::QEntity *importQmlScene(const QUrl &fileUrl, Qt3DCore::QEntity *&cameraEntity);

private:
    enum TextureTypes {
        DiffuseTexture = 1,
        SpecularTexture = 2,
        NormalTexture = 4
    };

    struct IdPair {
        QString qmlId;
        QString origName;
    };

    void cacheProperties(EditorItemType type, QObject *defaultObject);
    void resetParser();
    void outStartType(EditorItemType type);
    void outEndType();
    IdPair generateItemId(EditorItemType type, QObject *item);
    void outItemId(const IdPair IdPair);
    void outEntity(Qt3DCore::QEntity *entity);
    QString outComponent(Qt3DCore::QComponent *component);
    void outRenderSettings(Qt3DCore::QComponent *component);
    void outTexturedMaterial(EditorItemType type, Qt3DCore::QComponent *component);
    void outTextureProperty(const QString &propertyName,
                            Qt3DRender::QAbstractTexture *textureProvider);
    void outGenericMesh(Qt3DCore::QComponent *component, const QString &componentId);
    void outGeometry(Qt3DRender::QGeometry *geometry, const QString &componentId);
    template <typename T> void outBufferData(T *dataPtr, int count, const QString arrayType);
    void outGenericProperties(EditorItemType type, QObject *obj);
    void outGenericProperty(QObject *obj,
                            const QMetaProperty &property, const QObject *defaultComponent);
    void outHelperFunctions();
    void outSceneLoader(Qt3DCore::QComponent *component);
    void outCamera(Qt3DCore::QEntity *entity);

    QString indent() const;
    EditorItemType itemType(QObject *item) const;
    QString variantToQMLString(const QVariant &var);
    QVariant QMLStringToVariant(QVariant::Type type, const QString &qmlStr);
    Qt3DCore::QEntity *createEntity(EditorItemType type);
    QObject *createObject(EditorItemType type);
    void parseAndSetProperty(const QString &propertyName, const QString &propertyValue,
                             QObject *obj, EditorItemType type,
                             const QMap<QString, QObject *> &objectMap);
    void parseAndSetGenericProperty(const QString &propertyName, const QString &propertyValue,
                                    QObject *obj);
    void parseAndSetTextureProperty(const QString &propertyName, const QString &propertyValue,
                                    QObject *obj, EditorItemType type);
    int getTextureProviders(EditorItemType type, QObject *component,
                            Qt3DRender::QAbstractTexture *&diffuse,
                            Qt3DRender::QAbstractTexture *&specular,
                            Qt3DRender::QAbstractTexture *&normal);
    QString getAbsoluteQmlFileName(const QFileInfo &qrcFileInfo, const QString &resourceDirName);
    QString urlToResourceString(const QUrl &url);
    QByteArray importBuffer();

    int m_indentLevel;
    int m_resourceIdCounter;

    QTextStream m_stream;
    QString m_spaceArray;

    QStringList m_typeStrings;
    QMap<QString, EditorItemType> m_typeMap;
    QMap<EditorItemType, QObject *> m_defaultObjectMap;
    QMap<EditorItemType, QVector<QMetaProperty> > m_propertyMap;
    QMap<quint64, IdPair> m_idMap;
    QMap<QUrl, QString> m_exportResourceMap; // <texture url source, qml target>
    QMap<QString, QUrl> m_importResourceMap; // <qml source, texture url target>
    QString m_resourceDirName;
    QMap<EditorItemType, int> m_currentEntityComponentTypeMap;

};

#endif // EDITORSCENEPARSER_H
