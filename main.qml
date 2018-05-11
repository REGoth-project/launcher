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

Window {
    signal addInstallation(string name)
    signal removeInstallation(int position)
    signal playGame(string url)
    signal checkNewReleases()

    visible: true
    width: 800
    height: 600
    title: qsTr("REGoth Launcher")

    RowLayout {
        id: topRow
        Logo {}
        Button {
            id: settingsButton
            text: qsTr("Settings")
        }

        Button {
            id: updatesButton
            text: qsTr("Check for updates")
            onClicked: checkNewReleases()
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
            spacing: 5
            delegate: InstallationEntry {
                topText: name
                subText: url
                onPlayed: playGame(url)
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
        onAccepted: {
            var path = installationDialog.folder.toString();
            // remove prefixed "file://"
            path= path.replace(/^(file|http|qrc):\/{2}/,"");
            // unescape html codes like '%23' for '#'
            var cleanPath = decodeURIComponent(path);
            addInstallation(cleanPath)
        }
    }

    DownloadErrorDialog {
        objectName: "DownloadErrorDialog"
    }

    MessageDialog {
        property string logFile: ""

        objectName: "CrashDialog"
        title: qsTr("REGoth has crashed")
        text: qsTr("A log of the run has been written to %1").arg(logFile)
        icon: StandardIcon.Critical
    }

    NewReleaseAvailableDialog {
        objectName: "NewReleaseAvailableDialog"
    }

    MessageDialog {
        objectName: "UpToDateDialog"
        title: qsTr("You are up-to-date!")
        icon: StandardIcon.Information
        text: qsTr("You already have the latest REGoth release available")
        visible: false
        modality: Qt.WindowModal
    }

}
