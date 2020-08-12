/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "qssgvertexpipelineimpl_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercontextcore_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>

QT_BEGIN_NAMESPACE

QSSGMaterialVertexPipeline::QSSGMaterialVertexPipeline(const QSSGRef<QSSGProgramGenerator> &programGen,
                                                       const QSSGShaderDefaultMaterialKeyProperties &materialProperties,
                                                       QSSGShaderMaterialAdapter *materialAdapter,
                                                       QSSGDataView<QMatrix4x4> boneGlobals,
                                                       QSSGDataView<QMatrix3x3> boneNormals)
    : m_programGenerator(programGen)
    , defaultMaterialShaderKeyProperties(materialProperties)
    , materialAdapter(materialAdapter)
    , boneGlobals(boneGlobals)
    , boneNormals(boneNormals)
    , hasCustomShadedMain(false)
    , skipCustomFragmentSnippet(false)
{
    m_hasSkinning = boneGlobals.size() > 0;
}

static inline void insertProcessorArgs(QByteArray &snippet, const char *argKey, const char* (*argListFunc)())
{
    const int argKeyLen = int(strlen(argKey));
    const int argKeyPos = snippet.indexOf(argKey);
    if (argKeyPos >= 0)
        snippet = snippet.left(argKeyPos) + argListFunc() + snippet.mid(argKeyPos + argKeyLen);
}

static inline void insertDirectionalLightProcessorArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_DIRECTIONAL_LIGHT%*/", QSSGMaterialShaderGenerator::directionalLightProcessorArgumentList);
}

static inline void insertPointLightProcessorArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_POINT_LIGHT%*/", QSSGMaterialShaderGenerator::pointLightProcessorArgumentList);
}

static inline void insertAreaLightProcessorArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_AREA_LIGHT%*/", QSSGMaterialShaderGenerator::areaLightProcessorArgumentList);
}

static inline void insertSpotLightProcessorArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_SPOT_LIGHT%*/", QSSGMaterialShaderGenerator::spotLightProcessorArgumentList);
}

static inline void insertAmbientLightProcessorArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_AMBIENT_LIGHT%*/", QSSGMaterialShaderGenerator::ambientLightProcessorArgumentList);
}

static inline void insertSpecularLightProcessorArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_SPECULAR_LIGHT%*/", QSSGMaterialShaderGenerator::specularLightProcessorArgumentList);
}

static inline void insertFragmentMainArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_MAIN%*/", QSSGMaterialShaderGenerator::shadedFragmentMainArgumentList);
}

static inline void insertVertexMainArgs(QByteArray &snippet)
{
    insertProcessorArgs(snippet, "/*%QT_ARGS_MAIN%*/", QSSGMaterialShaderGenerator::vertexMainArgumentList);
}

void QSSGMaterialVertexPipeline::beginVertexGeneration(const QSSGShaderDefaultMaterialKey &inKey,
                                                       const ShaderFeatureSetList &inFeatureSet,
                                                       const QSSGRef<QSSGShaderLibraryManager> &shaderLibraryManager)
{
    QSSGShaderGeneratorStageFlags theStages(QSSGProgramGenerator::defaultFlags());
    programGenerator()->beginProgram(theStages);

    QSSGStageGeneratorBase &vertexShader(vertex());

    vertexShader.addIncoming("attr_pos", "vec3");

    if (m_hasSkinning) {
        vertexShader.addIncoming("attr_joints", "uvec4");
        vertexShader.addIncoming("attr_weights", "vec4");
        vertexShader.addUniformArray("qt_boneTransforms", "mat4", boneGlobals.mSize);
        vertexShader.addUniformArray("qt_boneNormalTransforms", "mat3", boneNormals.mSize);

        vertexShader << "mat4 qt_getSkinMatrix()"
                     << "\n"
                     << "{"
                     << "\n";
        // If some formats needs these weights to be normalized
        // it should be applied here
        vertexShader << "    return qt_boneTransforms[attr_joints.x] * attr_weights.x"
                     << "\n"
                     << "       + qt_boneTransforms[attr_joints.y] * attr_weights.y"
                     << "\n"
                     << "       + qt_boneTransforms[attr_joints.z] * attr_weights.z"
                     << "\n"
                     << "       + qt_boneTransforms[attr_joints.w] * attr_weights.w;"
                     << "\n"
                     << "}"
                     << "\n";
        vertexShader << "mat3 qt_getSkinNormalMatrix()"
                     << "\n"
                     << "{"
                     << "\n";
        vertexShader << "    return qt_boneNormalTransforms[attr_joints.x] * attr_weights.x"
                     << "\n"
                     << "       + qt_boneNormalTransforms[attr_joints.y] * attr_weights.y"
                     << "\n"
                     << "       + qt_boneNormalTransforms[attr_joints.z] * attr_weights.z"
                     << "\n"
                     << "       + qt_boneNormalTransforms[attr_joints.w] * attr_weights.w;"
                     << "\n"
                     << "}"
                     << "\n";
    }

    const bool hasCustomVertexShader = materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Vertex);
    hasCustomShadedMain = false;
    if (hasCustomVertexShader) {
        QByteArray snippet = materialAdapter->customShaderSnippet(QSSGShaderCache::ShaderType::Vertex,
                                                                  shaderLibraryManager);
        if (materialAdapter->hasCustomShaderFunction(QSSGShaderCache::ShaderType::Vertex,
                                                     QByteArrayLiteral("qt_customMain"),
                                                     shaderLibraryManager))
        {
            insertVertexMainArgs(snippet);
            if (!materialAdapter->isUnshaded())
                hasCustomShadedMain = true;
        }
        vertexShader << snippet;
    }

    vertexShader << "void main()"
                 << "\n"
                 << "{"
                 << "\n";

    vertexShader.addUniform("qt_modelViewProjection", "mat4");

    const bool meshHasNormals = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Normal, inKey);
    const bool meshHasTexCoord0 = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::TexCoord0, inKey);
    const bool meshHasTexCoord1 = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::TexCoord1, inKey);
    const bool meshHasTangents = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Tangent, inKey);
    const bool meshHasBinormals = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Binormal, inKey);
    const bool meshHasColors = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Color, inKey);

    skipCustomFragmentSnippet = false;
    for (const auto &feature : inFeatureSet) {
        if (feature.name == QSSGShaderDefines::asString(QSSGShaderDefines::DepthPass))
            skipCustomFragmentSnippet = feature.enabled;
    }

    if (hasCustomVertexShader) { // this is both for unshaded and shaded
        vertexShader.addUniform("qt_viewProjectionMatrix", "mat4");
        vertexShader.addUniform("qt_modelMatrix", "mat4");
        vertexShader.addUniform("qt_viewMatrix", "mat4");
        vertexShader.addUniform("qt_normalMatrix", "mat3");
        vertexShader.addUniform("qt_cameraPosition", "vec3");
        vertexShader.addUniform("qt_cameraDirection", "vec3");
        vertexShader.addUniform("qt_cameraProperties", "vec2");

        vertexShader.append("    vec3 qt_customPos = attr_pos;");
        if (meshHasNormals) {
            vertexShader.append("    vec3 qt_customNorm = attr_norm;");
            vertexShader.addIncoming("attr_norm", "vec3");
        } else {
            vertexShader.append("    vec3 qt_customNorm = vec3(0.0);");
        }
        if (meshHasTexCoord0) {
            vertexShader.append("    vec2 qt_customUV0 = attr_uv0;");
            vertexShader.addIncoming("attr_uv0", "vec2");
        } else {
            vertexShader.append("    vec2 qt_customUV0 = vec2(0.0);");
        }
        if (meshHasTexCoord1) {
            vertexShader.append("    vec2 qt_customUV1 = attr_uv1;");
            vertexShader.addIncoming("attr_uv1", "vec2");
        } else {
            vertexShader.append("    vec2 qt_customUV1 = vec2(0.0);");
        }
        if (meshHasTangents) {
            vertexShader.append("    vec3 qt_customTextan = attr_textan;");
            vertexShader.addIncoming("attr_textan", "vec3");
        } else {
            vertexShader.append("    vec3 qt_customTextan = vec3(0.0);");
        }
        if (meshHasBinormals) {
            vertexShader.append("    vec3 qt_customBinormal = attr_binormal;");
            vertexShader.addIncoming("attr_binormal", "vec3");
        } else {
            vertexShader.append("    vec3 qt_customBinormal = vec3(0.0);");
        }
        if (meshHasColors) {
            vertexShader.append("    vec4 qt_customColor = attr_color;");
            vertexShader.addIncoming("attr_color", "vec4");
        } else {
            vertexShader.append("    vec4 qt_customColor = vec4(1.0);"); // must be 1,1,1,1 to not alter when multiplying with it
        }
    }

    if (!materialAdapter->isUnshaded() || !hasCustomVertexShader) {
        vertexShader << "    vec3 qt_uTransform;\n";
        vertexShader << "    vec3 qt_vTransform;\n";
        if (m_hasSkinning) {
            vertexShader.append("    vec4 qt_skinnedPos;");
            vertexShader.append("    if (attr_weights != vec4(0.0))");
            if (hasCustomShadedMain) {
                vertexShader.append("        qt_skinnedPos = qt_getSkinMatrix() * vec4(qt_customPos, 1.0);");
                vertexShader.append("    else");
                vertexShader.append("        qt_skinnedPos = vec4(qt_customPos, 1.0);");
            } else {
                vertexShader.append("        qt_skinnedPos = qt_getSkinMatrix() * vec4(attr_pos, 1.0);");
                vertexShader.append("    else");
                vertexShader.append("        qt_skinnedPos = vec4(attr_pos, 1.0);");
            }
            vertexShader.append("    gl_Position = qt_modelViewProjection * qt_skinnedPos;");
        } else {
            if (hasCustomShadedMain) {
                vertexShader.append("    qt_customMain(qt_customPos, qt_customNorm, qt_customUV0, qt_customUV1, qt_customTextan, qt_customBinormal, qt_customColor);");
            } else {
                vertexShader.append("    gl_Position = qt_modelViewProjection * vec4(attr_pos, 1.0);");
            }
        }
    }
}

void QSSGMaterialVertexPipeline::beginFragmentGeneration(const QSSGRef<QSSGShaderLibraryManager> &shaderLibraryManager)
{
    fragment().addUniform("qt_material_properties", "vec4");

    if (!skipCustomFragmentSnippet && materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment)) {
        QByteArray snippet = materialAdapter->customShaderSnippet(QSSGShaderCache::ShaderType::Fragment,
                                                                  shaderLibraryManager);
        if (!materialAdapter->isUnshaded()) {
            insertAmbientLightProcessorArgs(snippet);
            insertSpecularLightProcessorArgs(snippet);
            insertAreaLightProcessorArgs(snippet);
            insertSpotLightProcessorArgs(snippet);
            insertPointLightProcessorArgs(snippet);
            insertDirectionalLightProcessorArgs(snippet);
            insertFragmentMainArgs(snippet);
        }
        fragment() << snippet;
    }

    fragment() << "void main()"
               << "\n"
               << "{"
               << "\n";

    if (!materialAdapter->isUnshaded() || !materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment))
        fragment() << "    float qt_objectOpacity = qt_material_properties.a;\n";
}

void QSSGMaterialVertexPipeline::assignOutput(const QByteArray &inVarName, const QByteArray &inVarValue)
{
    vertex() << "    " << inVarName << " = " << inVarValue << ";\n";
}

void QSSGMaterialVertexPipeline::doGenerateUVCoords(quint32 inUVSet, const QSSGShaderDefaultMaterialKey &inKey)
{
    Q_ASSERT(inUVSet == 0 || inUVSet == 1);

    if (inUVSet == 0) {
        const bool meshHasTexCoord0 = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                    QSSGShaderKeyVertexAttribute::TexCoord0, inKey);

        if (meshHasTexCoord0)
            vertex().addIncoming("attr_uv0", "vec2");
        else
            vertex().append("    vec2 attr_uv0 = vec2(0.0);");

        if (hasCustomShadedMain)
            vertex() << "    qt_varTexCoord0 = qt_customUV0;\n";
        else
            vertex() << "    qt_varTexCoord0 = attr_uv0;\n";

    } else if (inUVSet == 1) {
        const bool meshHasTexCoord1 = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                    QSSGShaderKeyVertexAttribute::TexCoord1, inKey);

        if (meshHasTexCoord1)
            vertex().addIncoming("attr_uv1", "vec2");
        else
            vertex().append("    vec2 attr_uv1 = vec2(0.0);");

        if (hasCustomShadedMain)
            vertex() << "    qt_varTexCoord1 = qt_customUV1;\n";
        else
            vertex() << "    qt_varTexCoord1 = attr_uv1;\n";
    }
}

void QSSGMaterialVertexPipeline::doGenerateWorldNormal(const QSSGShaderDefaultMaterialKey &inKey)
{
    const bool meshHasNormals = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Normal, inKey);

    QSSGStageGeneratorBase &vertexGenerator(vertex());
    if (meshHasNormals)
        vertexGenerator.addIncoming("attr_norm", "vec3");
    else
        vertexGenerator.append("    vec3 attr_norm = vec3(0.0);");
    vertexGenerator.addUniform("qt_normalMatrix", "mat3");
    if (!m_hasSkinning) {
        if (hasCustomShadedMain)
            vertexGenerator.append("    vec3 qt_world_normal = normalize(qt_normalMatrix * qt_customNorm).xyz;");
        else
            vertexGenerator.append("    vec3 qt_world_normal = normalize(qt_normalMatrix * attr_norm).xyz;");
    } else {
        if (hasCustomShadedMain) {
            vertexGenerator.append("    vec3 skinned_norm = qt_customNorm;");
            vertexGenerator.append("    if (attr_weights != vec4(0.0))");
            vertexGenerator.append("        skinned_norm = qt_getSkinNormalMatrix() * qt_customNorm;");
        } else {
            vertexGenerator.append("    vec3 skinned_norm = attr_norm;");
            vertexGenerator.append("    if (attr_weights != vec4(0.0))");
            vertexGenerator.append("        skinned_norm = qt_getSkinNormalMatrix() * attr_norm;");
        }
        vertexGenerator.append("    vec3 qt_world_normal = normalize(qt_normalMatrix * skinned_norm).xyz;");
    }
    vertexGenerator.append("    qt_varNormal = qt_world_normal;");
}

void QSSGMaterialVertexPipeline::doGenerateObjectNormal()
{
    addInterpolationParameter("qt_varObjectNormal", "vec3");
    if (hasCustomShadedMain)
        vertex().append("    qt_varObjectNormal = qt_customNorm;");
    else
        vertex().append("    qt_varObjectNormal = attr_norm;");
}

void QSSGMaterialVertexPipeline::doGenerateWorldPosition()
{
    if (!m_hasSkinning) {
        if (hasCustomShadedMain)
            vertex().append("    vec3 qt_local_model_world_position = (qt_modelMatrix * vec4(qt_customPos, 1.0)).xyz;");
        else
            vertex().append("    vec3 qt_local_model_world_position = (qt_modelMatrix * vec4(attr_pos, 1.0)).xyz;");
    } else {
        vertex().append("    vec3 qt_local_model_world_position = (qt_modelMatrix * qt_skinnedPos).xyz;");
    }
}

void QSSGMaterialVertexPipeline::doGenerateVarTangentAndBinormal(const QSSGShaderDefaultMaterialKey &inKey)
{
    const bool meshHasTangents = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Tangent, inKey);
    const bool meshHasBinormals = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Binormal, inKey);

    if (meshHasTangents)
        vertex().addIncoming("attr_textan", "vec3");
    else
        vertex() << "    vec3 attr_textan = vec3(0.0);\n";

    if (meshHasBinormals)
        vertex().addIncoming("attr_binormal", "vec3");
    else
        vertex() << "    vec3 attr_binormal = vec3(0.0);\n";

    if (!m_hasSkinning) {
        if (hasCustomShadedMain) {
            vertex() << "    qt_varTangent = (qt_modelMatrix * vec4(qt_customTextan, 0.0)).xyz;"
                     << "\n"
                     << "    qt_varBinormal = (qt_modelMatrix * vec4(qt_customBinormal, 0.0)).xyz;"
                     << "\n";
        } else {
            vertex() << "    qt_varTangent = (qt_modelMatrix * vec4(attr_textan, 0.0)).xyz;"
                     << "\n"
                     << "    qt_varBinormal = (qt_modelMatrix * vec4(attr_binormal, 0.0)).xyz;"
                     << "\n";
        }
    } else {
        if (hasCustomShadedMain) {
            vertex() << "    vec4 skinnedTangent = vec4(attr_textan, 0.0);\n"
                     << "    vec4 skinnedBinorm = vec4(attr_binormal, 0.0);\n";
        } else {
            vertex() << "    vec4 skinnedTangent = vec4(qt_customTextan, 0.0);\n"
                     << "    vec4 skinnedBinorm = vec4(qt_customBinormal, 0.0);\n";
        }
        vertex() << "    if (attr_weights != vec4(0.0)) {\n"
                 << "       skinnedTangent = qt_getSkinMatrix() * skinnedTangent;\n"
                 << "       skinnedBinorm = qt_getSkinMatrix() * skinnedBinorm;\n"
                 << "    }\n"
                 << "    qt_varTangent = (qt_modelMatrix * skinnedTangent).xyz;\n"
                 << "    qt_varBinormal = (qt_modelMatrix * skinnedBinorm).xyz;\n";
    }
}

void QSSGMaterialVertexPipeline::doGenerateVertexColor(const QSSGShaderDefaultMaterialKey &inKey)
{
    const bool meshHasColors = defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(
                QSSGShaderKeyVertexAttribute::Color, inKey);

    if (meshHasColors)
        vertex().addIncoming("attr_color", "vec4");
    else
        vertex().append("    vec4 attr_color = vec4(1.0);"); // must be 1,1,1,1 to not alter when multiplying with it

    if (hasCustomShadedMain)
        vertex().append("    qt_varColor = qt_customColor;");
    else
        vertex().append("    qt_varColor = attr_color;");
}

bool QSSGMaterialVertexPipeline::hasAttributeInKey(QSSGShaderKeyVertexAttribute::VertexAttributeBits inAttr,
                                                   const QSSGShaderDefaultMaterialKey &inKey)
{
    return defaultMaterialShaderKeyProperties.m_vertexAttributes.getBitValue(inAttr, inKey);
}

void QSSGMaterialVertexPipeline::endVertexGeneration()
{
    if (materialAdapter->isUnshaded() && materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Vertex))
        vertex() << "    qt_customMain(qt_customPos, qt_customNorm, qt_customUV0, qt_customUV1, qt_customTextan, qt_customBinormal, qt_customColor);\n";

    vertex().append("}");
}

void QSSGMaterialVertexPipeline::endFragmentGeneration()
{
    if (!skipCustomFragmentSnippet && materialAdapter->isUnshaded() && materialAdapter->hasCustomShaderSnippet(QSSGShaderCache::ShaderType::Fragment))
        fragment() << "    qt_customMain();\n";

    fragment().append("}");
}

void QSSGMaterialVertexPipeline::addInterpolationParameter(const QByteArray &inName, const QByteArray &inType)
{
    m_interpolationParameters.insert(inName, inType);
    vertex().addOutgoing(inName, inType);
    fragment().addIncoming(inName, inType);
}

QSSGStageGeneratorBase &QSSGMaterialVertexPipeline::activeStage()
{
    return vertex();
}

QT_END_NAMESPACE