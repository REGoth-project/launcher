# Copyright (C) 2018 The REGoth Team
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.1)

project(REGothLauncher LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)
find_package(curl)

if(WIN32)
    set(WINDOWS_RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/resources.obj)
    if (MSVC)
        add_custom_command(OUTPUT ${WINDOWS_RES_FILE}
            COMMAND rc.exe /fo ${WINDOWS_RES_FILE} regoth_launcher.rc
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        add_custom_command(OUTPUT ${WINDOWS_RES_FILE}
            COMMAND windres.exe regoth_launcher.rc ${WINDOWS_RES_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
endif()



add_executable(${PROJECT_NAME}
    "main.cpp"
    "ReleaseFetcher.cpp"
    "LauncherConfig.cpp"
    "CurlDownloader.cpp"
    "qml.qrc"
    ${WINDOWS_RES_FILE})

target_include_directories(${PROJECT_NAME} PRIVATE "include" ${CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Quick ${CURL_LIBRARIES})
