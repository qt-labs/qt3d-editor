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
#include "genericpropertychangecommand.h"
#include "undohandler.h"

GenericPropertyChangeCommand::GenericPropertyChangeCommand(const QString &text, QObject *obj,
                                                           const QString &propertyName,
                                                           const QVariant &newValue,
                                                           const QVariant &oldValue) :
    m_object(obj),
    m_propertyName(propertyName.toLatin1()),
    m_newValue(newValue),
    m_oldValue(oldValue)
{
    if (text.isEmpty())
        setText(QObject::tr("Change property"));
    else
        setText(text);
}

void GenericPropertyChangeCommand::undo()
{
    if (isNonOp())
        return;
    if (m_object)
        m_object->setProperty(m_propertyName, m_oldValue);
}

void GenericPropertyChangeCommand::redo()
{
    if (isNonOp())
        return;
    m_object->setProperty(m_propertyName, m_newValue);
}

bool GenericPropertyChangeCommand::isNonOp() const
{
    return m_newValue == m_oldValue;
}
