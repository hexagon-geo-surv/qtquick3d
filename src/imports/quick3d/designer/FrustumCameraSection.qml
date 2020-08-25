/****************************************************************************
**
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

import QtQuick
import HelperWidgets
import QtQuick.Layouts

Section {
    caption: qsTr("Frustum Camera")

    SectionLayout {
        Label {
            text: qsTr("Top")
            tooltip: qsTr("Sets the top plane of the camera view frustum.")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                realDragRange: 5000
                decimals: 0
                backendValue: backendValues.top
                Layout.fillWidth: true
            }
        }
        Label {
            text: qsTr("Bottom")
            tooltip: qsTr("Sets the bottom plane of the camera view frustum.")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                realDragRange: 5000
                decimals: 0
                backendValue: backendValues.bottom
                Layout.fillWidth: true
            }
        }
        Label {
            text: qsTr("Right")
            tooltip: qsTr("Sets the right plane of the camera view frustum.")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                realDragRange: 5000
                decimals: 0
                backendValue: backendValues.right
                Layout.fillWidth: true
            }
        }
        Label {
            text: qsTr("Left")
            tooltip: qsTr("Sets the left plane of the camera view frustum.")
        }
        SecondColumnLayout {
            SpinBox {
                maximumValue: 9999999
                minimumValue: -9999999
                realDragRange: 5000
                decimals: 0
                backendValue: backendValues.left
                Layout.fillWidth: true
            }
        }
    }
}
