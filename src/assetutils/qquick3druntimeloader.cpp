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

#include "qquick3druntimeloader_p.h"

#include <QtQuick3DAssetUtils/private/qssgscenedesc_p.h>
#include <QtQuick3DAssetUtils/private/qssgqmlutilities_p.h>
#include <QtQuick3DAssetUtils/private/qssgrtutilities_p.h>
#include <QtQuick3DAssetImport/private/qssgassetimportmanager_p.h>

/*!
    \qmltype RuntimeLoader
    \inherits Node
    \inqmlmodule QtQuick3D.AssetUtils
    \since 6.2
    \brief Imports a 3D asset at runtime.

    The RuntimeLoader type provides a way to load a 3D asset directly from source at runtime,
    without converting it to QtQuick3D's internal format first.

    Qt 6.2 supports the loading of glTF version 2.0 files in both in text (.gltf) and binary (.glb) formats.
*/

/*!
    \qmlproperty url QtQuick3D::RuntimeLoader::source

    This property holds the location of the source file containing the 3D asset.
    Changing this property will unload the current asset and attempt to load an asset from
    the given URL.

    The success or failure of the load operation is indicated by \l status.
*/

/*!
    \qmlproperty enumeration QtQuick3D::RuntimeLoader::status

    This property holds the status of the latest load operation.

    \value RuntimeLoader.Empty
        No URL was specified.
    \value RuntimeLoader.Success
        The load operation was successful.
    \value RuntimeLoader.Error
        The load operation failed. A human-readable error message is provided by \l errorString.

    \readonly
*/

/*!
    \qmlproperty string QtQuick3D::RuntimeLoader::errorString

    This property holds a human-readable string indicating the status of the latest load operation.

    \readonly
*/

/*!
    \qmlproperty Bounds QtQuick3D::RuntimeLoader::bounds

    This property describes the extents of the bounding volume around the imported model.

    \note The value may not be available before the first render

    \readonly
*/

/*!
    \qmlproperty QtQuick3D::Instancing QtQuick3D::RuntimeLoader::instancing

    If this property is set, the imported model will not be rendered normally. Instead, a number of
    instances will be rendered, as defined by the instance table.

    See the \l{Instanced Rendering} overview documentation for more information.
*/

QQuick3DRuntimeLoader::QQuick3DRuntimeLoader()
{
}

QUrl QQuick3DRuntimeLoader::source() const
{
    return m_source;
}

void QQuick3DRuntimeLoader::setSource(const QUrl &newSource)
{
    if (m_source == newSource)
        return;
    m_source = newSource;
    emit sourceChanged();

    if (isComponentComplete())
        loadSource();
}

void QQuick3DRuntimeLoader::componentComplete()
{
    QQuick3DNode::componentComplete();
    loadSource();
}

static void boxBoundsRecursive(const QQuick3DNode *baseNode, const QQuick3DNode *node, QQuick3DBounds3 &accBounds)
{
    if (!node)
        return;

    if (auto *model = qobject_cast<const QQuick3DModel *>(node)) {
        auto b = model->bounds();
        QSSGBounds2BoxPoints corners;
        b.bounds.expand(corners);
        for (const auto &point : corners) {
            auto p = model->mapPositionToNode(const_cast<QQuick3DNode *>(baseNode), point);
            if (Q_UNLIKELY(accBounds.bounds.isEmpty()))
                accBounds.bounds = { p, p };
            else
                accBounds.bounds.include(p);
        }
    }
    for (auto *child : node->childItems())
        boxBoundsRecursive(baseNode, qobject_cast<const QQuick3DNode *>(child), accBounds);
}

template<typename Func>
static void applyToModels(QQuick3DObject *obj, Func &&lambda)
{
    if (!obj)
        return;
    for (auto *child : obj->childItems()) {
        if (auto *model = qobject_cast<QQuick3DModel *>(child))
            lambda(model);
        applyToModels(child, lambda);
    }
}

void QQuick3DRuntimeLoader::loadSource()
{
    delete m_imported;
    m_imported.clear();

    m_status = Status::Empty;
    m_errorString = QStringLiteral("No file selected");
    if (!m_source.isValid()) {
        emit statusChanged();
        emit errorStringChanged();
        return;
    }

    QSSGAssetImportManager importManager;
    QSSGSceneDesc::Scene scene;
    QString error(QStringLiteral("Unknown error"));
    auto result = importManager.importFile(m_source, scene, &error);

    switch (result) {
    case QSSGAssetImportManager::ImportState::Success:
        m_errorString = QStringLiteral("Success!");
        m_status = Status::Success;
        break;
    case QSSGAssetImportManager::ImportState::IoError:
        m_errorString = QStringLiteral("IO Error: ") + error;
        m_status = Status::Error;
        break;
    case QSSGAssetImportManager::ImportState::Unsupported:
        m_errorString = QStringLiteral("Unsupported: ") + error;
        m_status = Status::Error;
        break;
    }

    emit statusChanged();
    emit errorStringChanged();

    if (m_status != Status::Success) {
        m_source.clear();
        emit sourceChanged();
        return;
    }

    m_imported = QSSGRuntimeUtils::createScene(*this, scene);
    m_boundsDirty = true;
    m_instancingChanged = m_instancing != nullptr;
    updateModels();
}

void QQuick3DRuntimeLoader::updateModels()
{
    if (m_instancingChanged) {
        applyToModels(m_imported, [this](QQuick3DModel *model) {
            model->setInstancing(m_instancing);
            model->setInstanceRoot(m_imported);
        });
        m_instancingChanged = false;
    }
}

QQuick3DRuntimeLoader::Status QQuick3DRuntimeLoader::status() const
{
    return m_status;
}

QString QQuick3DRuntimeLoader::errorString() const
{
    return m_errorString;
}

QSSGRenderGraphObject *QQuick3DRuntimeLoader::updateSpatialNode(QSSGRenderGraphObject *node)
{
    auto *result = QQuick3DNode::updateSpatialNode(node);
    if (m_boundsDirty)
        QMetaObject::invokeMethod(this, &QQuick3DRuntimeLoader::boundsChanged, Qt::QueuedConnection);
    return result;
}

void QQuick3DRuntimeLoader::calculateBounds()
{
    if (!m_imported || !m_boundsDirty)
        return;

    m_bounds.bounds.setEmpty();
    boxBoundsRecursive(m_imported, m_imported, m_bounds);
    m_boundsDirty = false;
}

const QQuick3DBounds3 &QQuick3DRuntimeLoader::bounds() const
{
    if (m_boundsDirty) {
        auto *that = const_cast<QQuick3DRuntimeLoader *>(this);
        that->calculateBounds();
        return that->m_bounds;
    }

    return m_bounds;
}

QQuick3DInstancing *QQuick3DRuntimeLoader::instancing() const
{
    return m_instancing;
}

void QQuick3DRuntimeLoader::setInstancing(QQuick3DInstancing *newInstancing)
{
    if (m_instancing == newInstancing)
        return;
    m_instancing = newInstancing;
    m_instancingChanged = true;
    updateModels();
    emit instancingChanged();
}