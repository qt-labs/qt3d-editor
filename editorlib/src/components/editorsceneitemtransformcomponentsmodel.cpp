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
#include "editorsceneitemtransformcomponentsmodel.h"
#include "editorsceneitemcomponentsmodel.h"

#include <Qt3DCore/QTransform>

#include <QtCore/QStack>

EditorSceneItemTransformComponentsModel::EditorSceneItemTransformComponentsModel(
        EditorSceneItemComponentsModel *itemmodel, Qt3DCore::QTransform *transformComponent,
        QObject *parent)
    : QAbstractListModel(parent)
    , m_sceneComponentsModel(itemmodel)
    , m_transformComponent(transformComponent)
    , m_type(SRT)
{

}

EditorSceneItemTransformComponentsModel::~EditorSceneItemTransformComponentsModel()
{
}

int EditorSceneItemTransformComponentsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    // TODO: return value can be != 1 only if type is (Composed)Matrix
    return (m_transformComponent != nullptr);//m_transformComponent->transforms().count();
}

QVariant EditorSceneItemTransformComponentsModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(index)

    Qt3DCore::QTransform *transform = m_transformComponent;
    QVariant transformComponentData = QVariant::fromValue(transform);

//    //    Translate
//    {
//        Qt3DCore::QTranslateTransform *translateTransform = nullptr;
//        translateTransform = qobject_cast<Qt3DCore::QTranslateTransform *>(transform);
//        if (translateTransform != nullptr) {
//            type = Translate;
//            transformComponentData = QVariant::fromValue(translateTransform);
//            goto results;
//        }
//    }
//    //    Rotate
//    {
//        Qt3DCore::QRotateTransform *rotateTransform = nullptr;
//        rotateTransform = qobject_cast<Qt3DCore::QRotateTransform *>(transform);
//        if (rotateTransform != nullptr) {
//            type = Rotate;
//            transformComponentData = QVariant::fromValue(rotateTransform);
//            goto results;
//        }
//    }
//    //    Scale
//    {
//        Qt3DCore::QScaleTransform *scaleTransform = nullptr;
//        scaleTransform = qobject_cast<Qt3DCore::QScaleTransform *>(transform);
//        if (scaleTransform != nullptr) {
//            type = Scale;
//            transformComponentData = QVariant::fromValue(scaleTransform);
//            goto results;
//        }
//    }
//    //    LookAt
//    {
//        Qt3DCore::QLookAtTransform *lookAtTransform = nullptr;
//        lookAtTransform = qobject_cast<Qt3DCore::QLookAtTransform *>(transform);
//        if (lookAtTransform != nullptr) {
//            type = LookAt;
//            transformComponentData = QVariant::fromValue(lookAtTransform);
//            goto results;
//        }
//    }
//    //    Matrix
//    {
//        Qt3DCore::QMatrixTransform *matrixTransform = nullptr;
//        matrixTransform = qobject_cast<Qt3DCore::QMatrixTransform *>(transform);
//        if (matrixTransform != nullptr) {
//            type = Matrix;
//            transformComponentData = QVariant::fromValue(matrixTransform);
//            goto results;
//        }
//    }

    if (role == TransformComponentType)
        return m_type;
    else if (role == TransformComponent)
        return transformComponentData;
    else
        return QVariant();

}

QHash<int, QByteArray> EditorSceneItemTransformComponentsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TransformComponentType] = "transformType";
    roles[TransformComponent] = "transformComponentData";
    return roles;
}

void EditorSceneItemTransformComponentsModel::setTransform(
        EditorSceneItemTransformComponentsModel::TransformComponentTypes type)
{
    if (type != m_type) {
        beginResetModel();
        m_type = type;
        endResetModel();
    }
}

void EditorSceneItemTransformComponentsModel::appendNewTransform(
        EditorSceneItemTransformComponentsModel::TransformComponentTypes type)
{
    Q_UNUSED(type)
    // TODO: Only has effect if type is (Composed)Matrix

//    int transformCount = m_transformComponent->transforms().count();
//    beginInsertRows(QModelIndex(), transformCount, transformCount);
//    Qt3DCore::QAbstractTransform *transform = nullptr;
//    switch (type) {
//    case Translate:
//        transform = new Qt3DCore::QTranslateTransform();
//        break;
//    case Rotate:
//        transform = new Qt3DCore::QRotateTransform();
//        break;
//    case Scale:
//        transform = new Qt3DCore::QScaleTransform();
//        break;
//    case LookAt:
//        transform = new Qt3DCore::QLookAtTransform();
//        break;
//    case Matrix:
//        transform = new Qt3DCore::QMatrixTransform();
//        break;
//    default:
//        //Unsupported Transform type
//        break;
//    }

//    if (transform == nullptr)
//        return;

//    m_transformComponent->addTransform(transform);
//    endInsertRows();
}

void EditorSceneItemTransformComponentsModel::removeTransform(int index)
{
    Q_UNUSED(index)
//    if (index >= m_transformComponent->transforms().count()
//            || m_transformComponent->transforms().count() == 0) {
//        return;
//    }

    // TODO: Different handling if type is (Composed)Matrix

    m_sceneComponentsModel->removeComponent(m_transformComponent);
    m_transformComponent = nullptr;
}

void EditorSceneItemTransformComponentsModel::moveTransform(int from, int to)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    // TODO: Only has effect if type is (Composed)Matrix

//    //This is needlessly complicated because the Transform component doesn't allow moves
//    //which is a problem because transformation matrices are not communitive.
//    if (from < 0 || to < 0 || (from == to)
//            || (from >= m_transformComponent->transforms().count()
//                && to < m_transformComponent->transforms().count())
//            || (to >= m_transformComponent->transforms().count()
//                && from < m_transformComponent->transforms().count())) {
//        return;
//    }
//    int target = (from > to) ? to : from;
//    int source = (from > to) ? from : to;
//    QModelIndex sourceIndex = this->index(source);
//    QModelIndex targetIndex = this->index(target);

//    // Just reset the entire model since we remove everything temporarily
//    beginResetModel();

//    //Remove the target
//    Qt3DCore::QAbstractTransform *transform = m_transformComponent->transforms().at(source);
//    m_transformComponent->removeTransform(transform);

//    QStack<Qt3DCore::QAbstractTransform*> transformStack;
//    for (int i = m_transformComponent->transforms().count() - 1; i >= target; --i) {
//        Qt3DCore::QAbstractTransform *tempTransform = m_transformComponent->transforms().at(i);
//        m_transformComponent->removeTransform(tempTransform);
//        transformStack.push(tempTransform);
//    }

//    m_transformComponent->addTransform(transform);

//    while (transformStack.count() != 0) {
//        m_transformComponent->addTransform(transformStack.pop());
//    }

//    endResetModel();
}
