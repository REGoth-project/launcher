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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt5 COMPONENTS Core Quick Network REQUIRED)
find_package(LibArchive REQUIRED)


if(Qt5_FOUND AND WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
    get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt5_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")

    if(EXISTS ${imported_location})
        add_executable(Qt5::windeployqt IMPORTED)

        set_target_properties(Qt5::windeployqt PROPERTIES
            IMPORTED_LOCATION ${imported_location}
        )
    endif()
endif()

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
    "ArchiveExtractor.cpp"
    "qml.qrc"
    ${WINDOWS_RES_FILE} )

target_include_directories(${PROJECT_NAME} PRIVATE "include" ${LibArchive_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Quick ${LibArchive_LIBRARIES})

if(WIN32)
    set(DESTINATION_DIR "/")
else()
    set(DESTINATION_DIR bin)
endif()

install(TARGETS REGothLauncher
    RUNTIME DESTINATION "${DESTINATION_DIR}")


if(TARGET Qt5::windeployqt)
    # execute windeployqt in a tmp directory after build
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        #COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
        COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
        COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/bin/Release" --qmldir "${CMAKE_SOURCE_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    )

    # copy deployment directory during installation
    install(
        DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/bin/Release/"
        DESTINATION "${DESTINATION_DIR}"
    )
endif()

set(CPACK_PACKAGE_NAME "REGothLauncher")
set(CPACK_PACKAGE_VENDOR "REGoth Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Launcher application for REGoth")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "REGoth")
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.md")

if(WIN32)
    set(CPACK_GENERATOR NSIS)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_PACKAGE_NAME "REGoth Launcher")
    set(CPACK_NSIS_DISPLAY_NAME "REGoth Launcher")
elseif(APPLE)
    set(CPACK_GENERATOR Bundle)
    set(CPACK_BUNDLE_NAME "REGoth Launcher")
    set(CPACK_BUNDLE_ICON "${CMAKE_SOURCE_DIR}/gothic.icns")
    set(CPACK_BUNDLE_PLIST "${CMAKE_SOURCE_DIR}/Info.plist")
elseif(BUILD_DEB)
    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5quick5 (>= 5.9.5), libsndfile1 (>= 1.0.28), libarchive13 (>= 3.2.2)")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "bertolaccinifrancesco@gmail.com")
elseif(BUILD_RPM)
    set(CPACK_GENERATOR RPM)
    set(CPACK_RPM_PACKAGE_REQUIRES "qt5 >= 5.10.0, libsndfile >= 1.0.28, libarchive >= 3.2.2")
endif()

include(CPack)
