/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

import QtQuick3D
import QtQuick

Rectangle {
    width: 800
    height: 600
    visible: true
    View3D {
        anchors.fill: parent
        camera: camera
        id: view3d

        environment: SceneEnvironment {
            clearColor: "steelblue"
            backgroundMode: SceneEnvironment.Color
        }

        Node {
            id: cameraNode
            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 600)
            }
            eulerRotation.x: -45
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            ambientColor: Qt.rgba(0.4, 0.4, 0.4, 1.0)
        }

        Component {
            id: referenceCube
            Node {
            Model {
                source: "#Cube"
                position: Qt.vector3d((index - 4) * 100, 0, 0)
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: DefaultMaterial {
                    diffuseColor: (index == 4) ? "red"
                                : (index % 2) ? "darkgray" : "lightgray"
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, (index + 1) * 100, 0)
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: DefaultMaterial {
                    diffuseColor: "white"
                }
            }
            Model {
                source: "#Cube"
                position: Qt.vector3d(0, 0, (index + 1) * 100)
                scale: Qt.vector3d(0.2, 0.2, 0.2)
                materials: DefaultMaterial {
                    diffuseColor: "black"
                }
            }
            }
        }

        Repeater3D {
            model: 10
            delegate: referenceCube
        }

        InstanceList {
            id: manualInstancing
            instances: [
            InstanceListEntry {
                position: Qt.vector3d(-250, 0, 0)
                color: "lightgreen"
            },
            InstanceListEntry {
                position: Qt.vector3d(-250, 0, 250)
                color: "yellow"
            },
            InstanceListEntry {
                position: Qt.vector3d(0, 0, 0)
                color: "lavender"
            },
            InstanceListEntry {
                position: Qt.vector3d(0, 250, 0)
                color: "cyan"
            },
            InstanceListEntry {
                position: Qt.vector3d(0, 0, 250)
                color: "lightblue"
            },
            InstanceListEntry {
                position: Qt.vector3d(250, 0, 0)
                color: "pink"
            },
            InstanceListEntry {
                position: Qt.vector3d(250, 250, 0)
                color: "salmon"
            }
            ]
        }

        Node {
            id: testroot
            scale: "0.5, 0.5, 0.5"
            x: 50
            Node {
                id: intermediate
                Model {
                    instancing: manualInstancing
                    instanceRoot: testroot
                    source: "#Cube"
                    materials: DefaultMaterial { diffuseColor: "white" }
                    opacity: 0.7
                }
            }
        }
    }
}