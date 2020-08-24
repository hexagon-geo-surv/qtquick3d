/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include <QCryptographicHash>
#include <QDir>

#include "genshaders.h"

#include <QtGui/private/qrhinull_p_p.h>

#include <QtQuick3D/private/qquick3dsceneenvironment_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3D/private/qquick3dscenerenderer_p.h>

// Lights
#include <QtQuick3D/private/qquick3dspotlight_p.h>
#include <QtQuick3D/private/qquick3darealight_p.h>
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>
#include <QtQuick3D/private/qquick3dpointlight_p.h>

#include <private/qssgrenderer_p.h>
#include <private/qssgrendererimpllayerrenderdata_p.h>

#include <QtShaderTools/private/qshaderbaker_p.h>

static bool writeFile(const QByteArray &data, const QByteArray &name)
{
    QFile file(name);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;
    file.write(data);
    file.close();
    return true;
}

static void initBaker(QShaderBaker *baker, QRhi::Implementation target)
{
    Q_UNUSED(target);
    QVector<QShaderBaker::GeneratedShader> outputs;
    // TODO: For simplicity we're just going to add all off these for now.
    outputs.append({ QShader::HlslShader, QShaderVersion(50) }); // Shader Model 5.0
    outputs.append({ QShader::MslShader, QShaderVersion(20) }); // Metal 2.0 (required for array of textures (custom materials))
    outputs.append({ QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs) }); // GLES 3.0+

    baker->setGeneratedShaders(outputs);
    baker->setGeneratedShaderVariants({ QShader::StandardShader });
}

GenShaders::GenShaders(const QString &workingDir)
{
    rhi = QRhi::create(QRhi::Null, nullptr);
    QRhiCommandBuffer *cb;
    rhi->beginOffscreenFrame(&cb);

    const auto rhiContext = QSSGRef<QSSGRhiContext>(new QSSGRhiContext);
    rhiContext->initialize(rhi);
    rhiContext->setCommandBuffer(cb);

    auto inputStreamFactory = new QSSGInputStreamFactory;
    renderContext = QSSGRef<QSSGRenderContextInterface>(new QSSGRenderContextInterface(rhiContext,
                                                                                       inputStreamFactory,
                                                                                       new QSSGBufferManager(rhiContext, inputStreamFactory),
                                                                                       new QSSGResourceManager(rhiContext),
                                                                                       new QSSGRenderer,
                                                                                       new QSSGShaderLibraryManager(inputStreamFactory),
                                                                                       new QSSGShaderCache(rhiContext, inputStreamFactory, &initBaker),
                                                                                       QSSGAbstractThreadPool::createThreadPool(1),
                                                                                       new QSSGCustomMaterialSystem,
                                                                                       new QSSGProgramGenerator,
                                                                                       workingDir));
}

GenShaders::~GenShaders() = default;

bool GenShaders::process(const MaterialParser::SceneData &sceneData,
                         QVector<QByteArray> &shaderFiles,
                         QSet<QByteArray> &keyFiles,
                         bool generateMultipleLights)
{
    Q_UNUSED(generateMultipleLights);

    const QByteArray resouceFolder = QSSGShaderCache::resourceFolder().mid(2);
    QDir dir;
    if (!dir.exists(resouceFolder))
        if (!dir.mkpath(resouceFolder))
            return false;

    QSSGRenderLayer layer;
    renderContext->setViewport(QRect(QPoint(), QSize(888,666)));
    const auto &renderer = renderContext->renderer();
    QSSGLayerRenderData layerData(layer, renderer);

    const auto &shaderLibraryManager = renderContext->shaderLibraryManager();
    const auto &shaderCache = renderContext->shaderCache();
    const auto &shaderProgramGenerator = renderContext->shaderProgramGenerator();

    bool aaIsDirty = false;
    bool temporalIsDirty = false;
    float ssaaMultiplier = 1.5f;

    QQuick3DViewport *view3D = sceneData.viewport;
    Q_ASSERT(view3D);

    // Dummy models (one for each of the properties that changes the shader output)
    QSSGRenderModel model;
    model.meshPath = QSSGRenderPath("#Cube");
    layer.addChild(model);
    // Needs handling: dummyModel.castsShadows; dummyModel.receivesShadows;

    // Realize resources
    // Materials
    const auto &textures = sceneData.textures;
    QVector<QSSGRenderGraphObject *> nodes;
    for (const auto &tex : textures) {
        auto node = QQuick3DObjectPrivate::updateSpatialNode(tex, nullptr);
        auto obj = QQuick3DObjectPrivate::get(tex);
        obj->spatialNode = node;
        nodes.append(node);
    }

    // Lights
    const auto &lights = sceneData.directionalLights;
    for (const auto &light : lights) {
        auto node = QQuick3DObjectPrivate::updateSpatialNode(light, nullptr);
        nodes.append(node);
        layer.addChild(static_cast<QSSGRenderNode &>(*node));
    }

    QQuick3DRenderLayerHelpers::updateLayerNodeHelper(*view3D, layer, aaIsDirty, temporalIsDirty, ssaaMultiplier);

    const auto &materials = sceneData.materials;
    QByteArray shaderString;
    for (const auto &mat : materials) {
        auto node = QQuick3DObjectPrivate::updateSpatialNode(mat, nullptr);
        nodes.append(node);
        model.materials.clear();
        model.materials.append(node);
        layerData.resetForFrame();
        layerData.prepareForRender(QSize(888, 666));

        const auto &features = layerData.features;

        auto &materialPropertis = layerData.renderer->defaultMaterialShaderKeyProperties();

        QSSGSubsetRenderable *renderable = nullptr;
        if (!layerData.opaqueObjects.isEmpty())
            renderable = static_cast<QSSGSubsetRenderable *>(layerData.opaqueObjects.first().obj);
        else if (!layerData.transparentObjects.isEmpty())
            renderable = static_cast<QSSGSubsetRenderable *>(layerData.transparentObjects.first().obj);
        else
            continue;

        auto stages = QSSGRenderer::generateRhiShaderStagesImpl(*renderable, shaderLibraryManager, shaderCache, shaderProgramGenerator, materialPropertis, features, shaderString);
        if (!stages.isNull()) {
            const auto key = QSSGShaderCacheKey::hashString(shaderString, features);
            const QByteArray baseName = resouceFolder + key;
            const QByteArray keyFile = baseName + QByteArrayLiteral(".key");

            if (!keyFiles.contains(keyFile)) {
                keyFiles.insert(keyFile);
                writeFile(renderable->shaderDescription.toByteArray(), keyFile);
                writeFile(shaderString, keyFile + QByteArrayLiteral(".txt"));

                auto vertexStage = stages->vertexStage();
                auto fragmentStage = stages->fragmentStage();
                auto vertexShader = vertexStage->shader();
                auto fragmentShader = fragmentStage->shader();
                const QByteArray vsData = vertexShader.serialized();
                const QByteArray fsData = fragmentShader.serialized();
                const QByteArray vsName = baseName + QByteArrayLiteral(".vert.qsb");
                const QByteArray fsName = baseName + QByteArrayLiteral(".frag.qsb");
                if (writeFile(vsData, vsName))
                    shaderFiles.push_back(vsName);
                if (writeFile(fsData, fsName))
                    shaderFiles.push_back(fsName);
            }
        }
    }

    for (auto c = layer.firstChild, d = c; c != nullptr;) {
        layer.removeChild(*c);
        d = c;
        c = c->nextSibling;
        if (d != &model)
            delete d;
    }

    qDeleteAll(nodes);

    return true;
}