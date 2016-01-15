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
#include "editorsceneitemmaterialcomponentsmodel.h"
#include "editorsceneitemcomponentsmodel.h"
#include "editorsceneitem.h"
#include "editorscene.h"
#include "undohandler.h"
#include "materialcomponentproxyitem.h"
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QDiffuseMapMaterial>
#include <Qt3DRender/QDiffuseSpecularMapMaterial>
#include <Qt3DRender/QGoochMaterial>
#include <Qt3DRender/QNormalDiffuseMapMaterial>
#include <Qt3DRender/QNormalDiffuseMapAlphaMaterial>
#include <Qt3DRender/QNormalDiffuseSpecularMapMaterial>
#include <Qt3DRender/QPerVertexColorMaterial>
#include <Qt3DRender/QPhongAlphaMaterial>
#include <Qt3DRender/QPhongMaterial>
#include <Qt3DRender/QAbstractTextureProvider>
#include <Qt3DRender/QTextureImage>
#include <Qt3DCore/QEntity>
#include <QtCore/QStack>

EditorSceneItemMaterialComponentsModel::EditorSceneItemMaterialComponentsModel(
        Qt3DRender::QMaterial *materialComponent,
        EditorSceneItemComponentsModel *sceneItemModel,
        QObject *parent)
    : QAbstractListModel(parent)
    , m_materialComponent(materialComponent)
    , m_sceneComponentsModel(sceneItemModel)
    , m_type(Unknown)
{
    m_type = materialType(materialComponent);
}

EditorSceneItemMaterialComponentsModel::~EditorSceneItemMaterialComponentsModel()
{

}

int EditorSceneItemMaterialComponentsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return (m_materialComponent != Q_NULLPTR);
}

QVariant EditorSceneItemMaterialComponentsModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)

    switch (role) {
    case MaterialComponentType:
        return m_type;
    case MaterialComponent: {
        QVariant materialComponentData;
        if (m_type != Unknown)
            materialComponentData = QVariant::fromValue(m_materialComponent);
        return materialComponentData;
    }
    case MaterialDiffuseTextureUrl:
        return QVariant::fromValue(getTextureUrl(getDiffuseTexture()));
    case MaterialSpecularTextureUrl:
        return QVariant::fromValue(getTextureUrl(getSpecularTexture()));
    case MaterialNormalTextureUrl:
        return QVariant::fromValue(getTextureUrl(getNormalTexture()));
    default:
        break;
    }
    return QVariant();
}

bool EditorSceneItemMaterialComponentsModel::setData(const QModelIndex &index,
                                                     const QVariant &value, int role)
{
    Qt3DRender::QAbstractTextureProvider *texture = Q_NULLPTR;
    switch (role) {
    case MaterialComponentType:
        return false;
    case MaterialComponent:
        return false;
    case MaterialDiffuseTextureUrl:
        texture = getDiffuseTexture();
        break;
    case MaterialSpecularTextureUrl:
        texture = getSpecularTexture();
        break;
    case MaterialNormalTextureUrl:
        texture = getNormalTexture();
        break;
    default:
        return false;
    }

    if (texture) {
        QUrl currentUrl = getTextureUrl(texture);
        QUrl newUrl = value.toUrl();

        if (currentUrl == newUrl)
            return false;
        //qDebug() << __FUNCTION__ << currentUrl << newUrl;

        // Update the texture
        if (texture->textureImages().size()) {
            Qt3DRender::QTextureImage *image =
                    qobject_cast<Qt3DRender::QTextureImage *>(texture->textureImages()[0]);
            //qDebug() << image;
#if 1
            if (image)
                image->setSource(newUrl);
#else
            if (image) {
                texture->removeTextureImage(image);
                image->deleteLater();
            }
            image = new Qt3DRender::QTextureImage();
            image->setSource(newUrl);
            texture->addTextureImage(image);
#endif
        }
    } else {
        return false;
    }

    QVector<int> roles(1);
    roles[0] = role;
    emit dataChanged(index, index, roles);
    // Convenience signal for our UI, as QVector is difficult to deal with in QML
    emit roleDataChanged(role);
    return true;
}

Qt::ItemFlags EditorSceneItemMaterialComponentsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

QHash<int, QByteArray> EditorSceneItemMaterialComponentsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MaterialComponentType] = "materialType";
    roles[MaterialComponent] = "materialComponentData";
    roles[MaterialDiffuseTextureUrl] = "materialDiffuseTextureUrl";
    roles[MaterialSpecularTextureUrl] = "materialSpecularTextureUrl";
    roles[MaterialNormalTextureUrl] = "materialNormalTextureUrl";
    return roles;
}

void EditorSceneItemMaterialComponentsModel::setMaterial(
        EditorSceneItemMaterialComponentsModel::MaterialComponentTypes type)
{
    if (type != m_type) {
        Qt3DRender::QMaterial *material = Q_NULLPTR;
        switch (type) {
        case DiffuseMap: {
            Qt3DRender::QDiffuseMapMaterial *newMat = new Qt3DRender::QDiffuseMapMaterial();
            Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
            diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo.png")));
            newMat->diffuse()->addTextureImage(diffuseTextureImage);
            material = newMat;
            break;
        } case DiffuseSpecularMap: {
            Qt3DRender::QDiffuseSpecularMapMaterial *newMat =
                    new Qt3DRender::QDiffuseSpecularMapMaterial();
            Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
            Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
            diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo.png")));
            specularTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_specular.png")));
            newMat->diffuse()->addTextureImage(diffuseTextureImage);
            newMat->specular()->addTextureImage(specularTextureImage);
            material = newMat;
            break;
        }
        case Gooch:
            material = new Qt3DRender::QGoochMaterial();
            break;
        case NormalDiffuseMap: {
            Qt3DRender::QNormalDiffuseMapMaterial *newMat =
                    new Qt3DRender::QNormalDiffuseMapMaterial();
            Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
            Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
            diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo.png")));
            normalTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_normal.png")));
            newMat->diffuse()->addTextureImage(diffuseTextureImage);
            newMat->normal()->addTextureImage(normalTextureImage);
            material = newMat;
            break;
        }
        case NormalDiffuseMapAlpha: {
            Qt3DRender::QNormalDiffuseMapAlphaMaterial *newMat =
                    new Qt3DRender::QNormalDiffuseMapAlphaMaterial();
            Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
            Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
            diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo.png")));
            normalTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_normal.png")));
            newMat->diffuse()->addTextureImage(diffuseTextureImage);
            newMat->normal()->addTextureImage(normalTextureImage);
            material = newMat;
            break;
        }
        case NormalDiffuseSpecularMap: {
            Qt3DRender::QNormalDiffuseSpecularMapMaterial *newMat =
                    new Qt3DRender::QNormalDiffuseSpecularMapMaterial();
            Qt3DRender::QTextureImage *diffuseTextureImage = new Qt3DRender::QTextureImage();
            Qt3DRender::QTextureImage *specularTextureImage = new Qt3DRender::QTextureImage();
            Qt3DRender::QTextureImage *normalTextureImage = new Qt3DRender::QTextureImage();
            diffuseTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo.png")));
            specularTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_specular.png")));
            normalTextureImage->setSource(QUrl(QStringLiteral("qrc:/images/qtlogo_normal.png")));
            newMat->diffuse()->addTextureImage(diffuseTextureImage);
            newMat->specular()->addTextureImage(specularTextureImage);
            newMat->normal()->addTextureImage(normalTextureImage);
            material = newMat;
            break;
        }
        case PerVertexColor:
            material = new Qt3DRender::QPerVertexColorMaterial();
            break;
        case PhongAlpha:
            material = new Qt3DRender::QPhongAlphaMaterial();
            break;
        case Phong: {
            material = new  Qt3DRender::QPhongMaterial();
            break;
        }
        default:
            //Unsupported material type
            break;
        }

        if (material == Q_NULLPTR)
            return;

        m_sceneComponentsModel->sceneItem()->scene()->undoHandler()->createReplaceComponentCommand(
                    m_sceneComponentsModel->sceneItem()->entity()->objectName(),
                    EditorSceneItemComponentsModel::Material,
                    material, m_materialComponent);

    }
}

void EditorSceneItemMaterialComponentsModel::removeMaterial(int index)
{
    Q_UNUSED(index)

    m_sceneComponentsModel->removeComponent(m_materialComponent);
    m_materialComponent = Q_NULLPTR;
}

void EditorSceneItemMaterialComponentsModel::beginReplace()
{
    beginResetModel();
    m_type = Unknown;
    m_materialComponent = Q_NULLPTR;
    endResetModel();
}

void EditorSceneItemMaterialComponentsModel::endReplace(Qt3DRender::QMaterial *newMaterial)
{
    beginResetModel();
    m_type = materialType(newMaterial);
    m_materialComponent = newMaterial;
    endResetModel();
}

EditorSceneItemMaterialComponentsModel::MaterialComponentTypes
EditorSceneItemMaterialComponentsModel::materialType(Qt3DRender::QMaterial *material) const
{
    if (qobject_cast<Qt3DRender::QDiffuseMapMaterial *>(material))
        return DiffuseMap;
    else if (qobject_cast<Qt3DRender::QDiffuseSpecularMapMaterial *>(material))
        return DiffuseSpecularMap;
    else if (qobject_cast<Qt3DRender::QGoochMaterial *>(material))
        return Gooch;
    // QNormalDiffuseMapAlphaMaterial inherits QNormalDiffuseMapMaterial, so it must be first
    else if (qobject_cast<Qt3DRender::QNormalDiffuseMapAlphaMaterial *>(material))
        return NormalDiffuseMapAlpha;
    else if (qobject_cast<Qt3DRender::QNormalDiffuseMapMaterial *>(material))
        return NormalDiffuseMap;
    else if (qobject_cast<Qt3DRender::QNormalDiffuseSpecularMapMaterial *>(material))
        return NormalDiffuseSpecularMap;
    else if (qobject_cast<Qt3DRender::QPerVertexColorMaterial *>(material))
        return PerVertexColor;
    else if (qobject_cast<Qt3DRender::QPhongAlphaMaterial *>(material))
        return PhongAlpha;
    else if (qobject_cast<Qt3DRender::QPhongMaterial *>(material))
        return Phong;
    else
        return Unknown;
}

Qt3DRender::QAbstractTextureProvider *EditorSceneItemMaterialComponentsModel::getDiffuseTexture() const
{
    switch (m_type) {
    case DiffuseMap: {
        Qt3DRender::QDiffuseMapMaterial *mat =
                qobject_cast<Qt3DRender::QDiffuseMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->diffuse();
        break;
    }
    case DiffuseSpecularMap: {
        Qt3DRender::QDiffuseSpecularMapMaterial *mat =
                qobject_cast<Qt3DRender::QDiffuseSpecularMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->diffuse();
        break;
    }
    case NormalDiffuseMap: {
        Qt3DRender::QNormalDiffuseMapMaterial *mat =
                qobject_cast<Qt3DRender::QNormalDiffuseMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->diffuse();
        break;
    }
    case NormalDiffuseMapAlpha: {
        Qt3DRender::QNormalDiffuseMapAlphaMaterial *mat =
                qobject_cast<Qt3DRender::QNormalDiffuseMapAlphaMaterial *>(m_materialComponent);
        if (mat)
            return mat->diffuse();
        break;
    }
    case NormalDiffuseSpecularMap: {
        Qt3DRender::QNormalDiffuseSpecularMapMaterial *mat =
                qobject_cast<Qt3DRender::QNormalDiffuseSpecularMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->diffuse();
        break;
    }
    default:
        break;
    }
    return Q_NULLPTR;
}

Qt3DRender::QAbstractTextureProvider *EditorSceneItemMaterialComponentsModel::getSpecularTexture() const
{
    switch (m_type) {
    case DiffuseSpecularMap: {
        Qt3DRender::QDiffuseSpecularMapMaterial *mat =
                qobject_cast<Qt3DRender::QDiffuseSpecularMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->specular();
        break;
    }
    case NormalDiffuseSpecularMap: {
        Qt3DRender::QNormalDiffuseSpecularMapMaterial *mat =
                qobject_cast<Qt3DRender::QNormalDiffuseSpecularMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->specular();
        break;
    }
    default:
        break;
    }
    return Q_NULLPTR;
}

Qt3DRender::QAbstractTextureProvider *EditorSceneItemMaterialComponentsModel::getNormalTexture() const
{
    switch (m_type) {
    case NormalDiffuseMap: {
        Qt3DRender::QNormalDiffuseMapMaterial *mat =
                qobject_cast<Qt3DRender::QNormalDiffuseMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->normal();
        break;
    }
    case NormalDiffuseMapAlpha: {
        Qt3DRender::QNormalDiffuseMapAlphaMaterial *mat =
                qobject_cast<Qt3DRender::QNormalDiffuseMapAlphaMaterial *>(m_materialComponent);
        if (mat)
            return mat->normal();
        break;
    }
    case NormalDiffuseSpecularMap: {
        Qt3DRender::QNormalDiffuseSpecularMapMaterial *mat =
                qobject_cast<Qt3DRender::QNormalDiffuseSpecularMapMaterial *>(m_materialComponent);
        if (mat)
            return mat->normal();
        break;
    }
    default:
        break;
    }
    return Q_NULLPTR;
}

QUrl EditorSceneItemMaterialComponentsModel::getTextureUrl(Qt3DRender::QAbstractTextureProvider *texture) const
{
    if (texture) {
        // We are going to assume each provider only has maximum of one texture, and it is of type
        // QTextureImage.
        if (texture->textureImages().size()) {
            const Qt3DRender::QTextureImage *image =
                    qobject_cast<const Qt3DRender::QTextureImage *>(texture->textureImages().at(0));
            if (image)
                return image->source();
        }
    }
    return QUrl();
}
