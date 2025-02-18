cmake_minimum_required(VERSION 3.16)

project(clientDes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS
    Quick Core Network Gui
    QuickControls2 Widgets Multimedia
)

qt_standard_project_setup()


qt_add_executable(regagram
    main.cpp
)

qt_policy(SET QTP0001 NEW)

qt_add_qml_module(regagram
    URI resources
    VERSION 1.0
    QML_FILES
        qmlFiles/Main.qml
        qmlFiles/MessageLine.qml
        qmlFiles/LoginPage.qml
        qmlFiles/RegPage.qml
        qmlFiles/pageSwitch.qml
        qmlFiles/ProfilePanel.qml
        qmlFiles/ChatsList.qml
        qmlFiles/MyProfile.qml
        qmlFiles/EditProfileDialog.qml
        qmlFiles/EditFiledInformation.qml
        qmlFiles/UsersSearchList.qml
        qmlFiles/PersonalChatsList.qml
        qmlFiles/ChatBubble.qml

    SOURCES
        client.h client.cpp
        networkmanager.h networkmanager.cpp
        messagemanager.h messagemanager.cpp
        accountmanager.h accountmanager.cpp
        filemanager.h filemanager.cpp
        audiomanager.h audiomanager.cpp
        Logger/logger.cpp Logger/logger.h
    RESOURCES
        images/logo.png
        images/logo2.png
        images/profile.png
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(regagram PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER qmlqtdesign
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(regagram
    PRIVATE Qt6::Quick
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
)

include(GNUInstallDirs)
install(TARGETS regagram
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# configure_file(config.ini ${CMAKE_CURRENT_BINARY_DIR}/config.ini COPYONLY)
