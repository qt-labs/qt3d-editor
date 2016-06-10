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

#include "ontopeffect.h"

#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QRenderPass>
#include <QtCore/QUrl>

OnTopEffect::OnTopEffect(Qt3DCore::QNode *parent)
    : Qt3DRender::QEffect(parent)
{
    Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique();
    technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::NoProfile);
    technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
    technique->graphicsApiFilter()->setMajorVersion(2);
    technique->graphicsApiFilter()->setMinorVersion(1);

    Qt3DRender::QTechnique *techniqueES2 = new Qt3DRender::QTechnique();
    techniqueES2->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGLES);
    techniqueES2->graphicsApiFilter()->setMajorVersion(2);
    techniqueES2->graphicsApiFilter()->setMinorVersion(0);
    techniqueES2->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::NoProfile);

    Qt3DRender::QFilterKey *filterkey = new Qt3DRender::QFilterKey(this);
    filterkey->setName(QStringLiteral("renderingStyle"));
    filterkey->setValue(QStringLiteral("forward"));

    technique->addFilterKey(filterkey);
    techniqueES2->addFilterKey(filterkey);

    Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram();
    shader->setVertexShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/shaders/ontopmaterial.vert"))));
    shader->setFragmentShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/qt3deditorlib/shaders/ontopmaterial.frag"))));

    Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass();
    renderPass->setShaderProgram(shader);
    technique->addRenderPass(renderPass);
    techniqueES2->addRenderPass(renderPass);

    addTechnique(technique);
    addTechnique(techniqueES2);
}
