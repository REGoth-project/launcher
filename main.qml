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

Window {
    signal addInstallation(url name)
    signal removeInstallation(int position)

    visible: true
    width: 800
    height: 600
    title: qsTr("REGoth Launcher")

    Row {
        id: topRow
        Button {
            text: qsTr("Settings")
        }

        Button {
            text: qsTr("Check for updates")
        }
    }

    ScrollView {
        anchors.top: topRow.bottom
        anchors.bottom: addInstallationButton.top
        width: parent.width
        clip: true

        ListView {
            id: installationsView
            model: installations
            delegate: Row {
                Text {
                    text: name
                }
                Button {
                    text: qsTr("Play")
                }
            }
        }
    }

    Button {
        id: addInstallationButton
        anchors.bottom: parent.bottom
        text: qsTr("Add Gothic installation")
        onClicked: installationDialog.open()
    }

    // Dialogs
    FirstStartupDialog {
        objectName: "FirstStartupDialog"
    }

    VersionDownloadDialog {
        objectName: "VersionDownloadDialog"
    }

    FolderDialog {
        id: installationDialog
        onAccepted: addInstallation(installationDialog.folder)
    }
}
