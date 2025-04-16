cmake_minimum_required(VERSION 3.16)

project(clientDes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS
    Quick Core Network Protobuf Gui
    QuickControls2 Widgets Multimedia
    ShaderTools
)

qt_standard_project_setup()

qt_add_protobuf(protosource
    PROTO_FILES
    ${CMAKE_SOURCE_DIR}/proto/envelope.proto
    ${CMAKE_SOURCE_DIR}/proto/login.proto
    ${CMAKE_SOURCE_DIR}/proto/register.proto
    ${CMAKE_SOURCE_DIR}/proto/search.proto
    ${CMAKE_SOURCE_DIR}/proto/editProfile.proto
    ${CMAKE_SOURCE_DIR}/proto/avatarsUpdate.proto
    ${CMAKE_SOURCE_DIR}/proto/identifiers.proto
    ${CMAKE_SOURCE_DIR}/proto/chatsInfo.proto
    ${CMAKE_SOURCE_DIR}/proto/deleteMember.proto
    ${CMAKE_SOURCE_DIR}/proto/addMembers.proto
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
        qmlFiles/CreateGroupForm.qml
        qmlFiles/SelectContactsForm.qml
        qmlFiles/GroupInfoForm.qml
        qmlFiles/SmartImage.qml
        qmlFiles/FallbackImage.qml
    SOURCES
        Core/client.h Core/client.cpp
        Core/configmanager.h Core/configmanager.cpp
        Utils/logger.h Utils/logger.cpp
        Utils/avatargenerator.h Utils/avatargenerator.cpp
        Network/networkmanager.h Network/networkmanager.cpp
        Network/filenetworkmanager.h Network/filenetworkmanager.cpp
        Network/messagenetworkmanager.h Network/messagenetworkmanager.cpp
        Managers/accountmanager.h Managers/accountmanager.cpp
        Managers/audiomanager.h Managers/audiomanager.cpp
        Managers/filemanager.h Managers/filemanager.cpp
        Managers/responsehandler.h Managers/responsehandler.cpp
        Managers/MessageManagers/messagehandler.h Managers/MessageManagers/messagehandler.cpp
        Managers/MessageManagers/messagestorage.h Managers/MessageManagers/messagestorage.cpp
        Managers/MessageManagers/messagesender.h Managers/MessageManagers/messagesender.cpp
    RESOURCES
        images/logo.png
        images/logo2.png
        images/profile.png
)

qt6_add_shaders(regagram "Shaders"
    GLSL "430"
    HLSL 50
    MSL 12
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    FILES
        shaders/roundImage.frag
        shaders/roundImage.vert
)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(regagram PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER regagram
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
add_dependencies(regagram protosource)

target_link_libraries(regagram
    PRIVATE Qt6::Quick
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Protobuf
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    protosource
)

include(GNUInstallDirs)
install(TARGETS regagram
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# configure_file(config.ini ${CMAKE_CURRENT_BINARY_DIR}/config.ini COPYONLY)
