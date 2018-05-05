/*  Copyright (C) 2018 The REGoth Team
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.2

Window {
    property string versionName: ""
    property string fileName: ""
    property real progress: 0.0
    property bool isIndeterminate: false

    signal cancelDownload()

    title: qsTr("Downloading %1").arg(versionName)
    modality: Qt.WindowModal
    visible: false
    width: 400;
    height: 80
    flags: Qt.Dialog

    maximumHeight: height
    minimumHeight: height

    maximumWidth: width
    minimumWidth: width

    onClosing: cancelDownload()

    Column {
        anchors.left: parent.left
        anchors.right: parent.right

        ProgressBar {
            indeterminate: isIndeterminate
            value: progress
            anchors.left: parent.left
            anchors.right: parent.right
        }

        Label {
            text: qsTr("Downloading %1").arg(fileName)
            width: parent.width
            elide: Text.ElideMiddle
        }
    }

    Button {
        text: qsTr("Cancel")
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        onClicked: cancelDownload()
    }
}
