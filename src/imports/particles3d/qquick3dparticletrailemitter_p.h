/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICK3DPARTICLETRAILEMITTER_H
#define QQUICK3DPARTICLETRAILEMITTER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qquick3dparticleemitter_p.h"
#include "qquick3dparticlemodelparticle_p.h"
#include "qquick3dparticledata_p.h"
#include <QQmlEngine>

QT_BEGIN_NAMESPACE

class QQuick3DParticleTrailEmitter : public QQuick3DParticleEmitter
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DParticleModelParticle *follow READ follow WRITE setFollow NOTIFY followChanged)
    QML_NAMED_ELEMENT(TrailEmitter3D)

public:
    QQuick3DParticleTrailEmitter(QQuick3DNode *parent = nullptr);

    QQuick3DParticleModelParticle * follow() const;

    // Emit count amount of particles immediately
    Q_INVOKABLE void burst(int count) override;

public Q_SLOTS:
    void setFollow(QQuick3DParticleModelParticle *follow);

protected:
    friend class QQuick3DParticleSystem;
    void emitTrailParticles(QQuick3DParticleDataCurrent *d, int emitAmount);

Q_SIGNALS:
    void followChanged();

private:
    QQuick3DParticleModelParticle *m_follow = nullptr;

};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLETRAILEMITTER_H