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
#include "qdummyobjectpicker.h"

QT_BEGIN_NAMESPACE

QDummyObjectPicker::QDummyObjectPicker(Qt3DCore::QNode *parent)
    : m_hoverEnabled(false)
{
    Q_UNUSED(parent)
}

void QDummyObjectPicker::setHoverEnabled(bool hoverEnabled)
{
    if (hoverEnabled != m_hoverEnabled) {
        m_hoverEnabled = hoverEnabled;
        emit hoverEnabledChanged();
    }
}

bool QDummyObjectPicker::hoverEnabled() const
{
    return m_hoverEnabled;
}

void QDummyObjectPicker::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change)
{
    Q_UNUSED(change)
}

QT_END_NAMESPACE
