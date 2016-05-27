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
#include "modelrolechangecommand.h"

#include "editorscene.h"
#include "editorsceneitem.h"
#include "editorsceneitemmodel.h"
#include "editorsceneitemmaterialcomponentsmodel.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QComponent>

ModelRoleChangeCommand::ModelRoleChangeCommand(const QString &text,
                                               EditorSceneItemModel *sceneModel,
                                               const QString &entityName,
                                               EditorSceneItemComponentsModel::EditorSceneItemComponentTypes componentType,
                                               int roleIndex,
                                               const QVariant &newValue,
                                               const QVariant &oldValue) :
    m_sceneModel(sceneModel),
    m_entityName(entityName),
    m_componentType(componentType),
    m_modelRoleIndex(roleIndex),
    m_newValue(newValue),
    m_oldValue(oldValue)
{
    if (text.isEmpty())
        setText(QObject::tr("Change model role"));
    else
        setText(text);
}

void ModelRoleChangeCommand::undo()
{
    QAbstractItemModel *model = getTargetModel();
    if (model)
        model->setData(QModelIndex(), m_oldValue, m_modelRoleIndex);
}

void ModelRoleChangeCommand::redo()
{
    QAbstractItemModel *model = getTargetModel();
    if (model)
        model->setData(QModelIndex(), m_newValue, m_modelRoleIndex);
}

QAbstractItemModel *ModelRoleChangeCommand::getTargetModel() const
{
    QModelIndex modelIndex = m_sceneModel->getModelIndexByName(m_entityName);
    EditorSceneItem *sceneItem = m_sceneModel->editorSceneItemFromIndex(modelIndex);

    QAbstractItemModel *model = nullptr;

    // Currently only material models are supported
    if (m_componentType == EditorSceneItemComponentsModel::Material)
        model = sceneItem->componentsModel()->materialProxy()->model();

    return model;
}

