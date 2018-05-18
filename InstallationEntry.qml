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
import Qt.labs.platform 1.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Rectangle {
    property string topText: ""
    property string subText: ""

    signal played()
    signal removed()

    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: 10
    anchors.rightMargin: 10

    height: 100

    color: "#f2f2f2"
    border.width: 1
    border.color: "#c1c1c1"

    Column {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 10
        Text {
            font.pixelSize: 24
            font.family: metamorphous.name
            text: topText
        }
        Text {
            text: subText
            font.pixelSize: 10
            font.weight: Font.Thin
        }
    }

    Button {
        id: removeButton
        anchors.right: playButton.left
        height: parent.height

        text: qsTr("Remove")
        onClicked: removed()
    }

    Button {
        id: playButton
        anchors.right: parent.right
        height: parent.height

        text: qsTr("Play")
        onClicked: played()
    }

    FontLoader {
        id: metamorphous
        source: "qrc:/assets/Metamorphous-Regular.ttf"
    }
}
