/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tests of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick3D 1.14
import QtQuick 2.14

Rectangle {
    id: opacitymap
    width: 800
    height: 480
    color: Qt.rgba(0, 0, 0, 1)

    View3D {
        id: layer
        anchors.left: parent.left
        anchors.leftMargin: parent.width * 0
        width: parent.width * 1
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0
        height: parent.height * 1
        environment: SceneEnvironment {
            clearColor: Qt.rgba(0, 0, 0, 1)
            aoDither: true
            depthPrePassEnabled: true
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, -600)
            rotationOrder: Node.YZX
            clipFar: 5000
        }

        DirectionalLight {
            id: light
            rotationOrder: Node.YZX
            shadowFactor: 10
        }

        Model {
            id: rectangle
            position: Qt.vector3d(15.6206, 1.91976, -21.2664)
            rotation: Qt.vector3d(62.5, 0, 0)
            scale: Qt.vector3d(6.24243, 4.98461, 1)
            rotationOrder: Node.YZX
            source: "#Rectangle"
            
            

            DefaultMaterial {
                id: material
                lighting: DefaultMaterial.FragmentLighting
                diffuseColor: Qt.rgba(1, 0.870588, 0.752941, 1)
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                bumpAmount: 0.5
                translucentFalloff: 1
                displacementAmount: 20
            }
            materials: [material]
        }

        Model {
            id: sphere
            position: Qt.vector3d(3.93619, 42.917, -251.294)
            rotationOrder: Node.YZX
            source: "#Sphere"
            
            

            DefaultMaterial {
                id: material_001
                lighting: DefaultMaterial.FragmentLighting
                diffuseColor: Qt.rgba(0, 0.878431, 0, 1)
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                opacityMap: material_001_opacitymap
                bumpAmount: 0.5
                translucentFalloff: 1
                displacementAmount: 20

                Texture {
                    id: material_001_opacitymap
                    source: "../shared/maps/opacitymap.png"
                }
            }
            materials: [material_001]
        }

        Model {
            id: cube
            position: Qt.vector3d(-259.951, 176.081, -5.02271)
            rotation: Qt.vector3d(-30.5, -34, 0)
            rotationOrder: Node.YZX
            source: "#Cube"
            
            

            DefaultMaterial {
                id: material_002
                lighting: DefaultMaterial.FragmentLighting
                diffuseColor: Qt.rgba(1, 0.658824, 0, 1)
                indexOfRefraction: 1.5
                specularAmount: 0
                specularRoughness: 0
                opacityMap: material_002_opacitymap
                bumpAmount: 0.5
                translucentFalloff: 1
                displacementAmount: 20

                Texture {
                    id: material_002_opacitymap
                    source: "../shared/maps/opacitymap.png"
                }
            }
            materials: [material_002]
        }
    }
}
