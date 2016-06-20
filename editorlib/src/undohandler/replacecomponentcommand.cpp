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
#include "replacecomponentcommand.h"

#include "editorscene.h"
#include "editorsceneitem.h"
#include "editorsceneitemmodel.h"
#include "undohandler.h"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QComponent>
#include <QtQml/QQmlEngine>

ReplaceComponentCommand::ReplaceComponentCommand(EditorSceneItemModel *sceneModel,
                                                 const QString &entityName,
                                                 EditorSceneItemComponentsModel::EditorSceneItemComponentTypes componentType,
                                                 Qt3DCore::QComponent *newComponent,
                                                 Qt3DCore::QComponent *oldComponent) :
    m_sceneModel(sceneModel),
    m_entityName(entityName),
    m_componentType(componentType),
    m_component1(newComponent),
    m_component2(oldComponent)
{
    setText(QObject::tr("Change component type"));
}

ReplaceComponentCommand::~ReplaceComponentCommand()
{
    if (m_component1->entities().size() == 0)
        m_component1->deleteLater();
}

void ReplaceComponentCommand::undo()
{
    replaceAndSwap();
}

void ReplaceComponentCommand::redo()
{
    replaceAndSwap();
}

void ReplaceComponentCommand::replaceAndSwap()
{
    QModelIndex modelIndex = m_sceneModel->getModelIndexByName(m_entityName);
    if (modelIndex.isValid()) {
        EditorSceneItem *sceneItem = m_sceneModel->editorSceneItemFromIndex(modelIndex);
        sceneItem->componentsModel()->replaceComponent(m_component2, m_component1);
        m_component2->setParent(static_cast<Qt3DCore::QNode *>(nullptr));
        // Grab explicit ownership of the component, otherwise QML garbage collector may clean it up
        QQmlEngine::setObjectOwnership(m_component2, QQmlEngine::CppOwnership);
        std::swap(m_component1, m_component2);
        m_sceneModel->scene()->queueUpdateGroupSelectionBoxes();
    }
}
