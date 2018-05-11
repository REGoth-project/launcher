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
import QtQuick.Dialogs 1.2

MessageDialog {
    property string releaseName: ""
    title: qsTr("New release available")
    icon: StandardIcon.Information
    text: qsTr("REGoth %1 is available for download! Do you want to download it?").arg(releaseName)
    standardButtons: StandardButton.Yes | StandardButton.No
    visible: false
    modality: Qt.WindowModal
}
