import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id:selectContactsForm
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0

    background: Rectangle {
        color: Qt.lighter(themeManager.chatBackground, 1.8)
        radius: 6
    }
    width: 300
    height: 400

    property string params: params

    Text{
        id:selectContactsText
        text: "Select contacts"
        color: "White"
        font.pointSize: 15
        font.bold: true
        anchors{
            left: parent.left
            leftMargin: 10
            top: parent.top
            topMargin: 10
        }
    }

    ListView {
        id: contactsListView
        width: parent.width - 20
        height: parent.height - 100
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: selectContactsText.bottom
        anchors.topMargin: 10
        clip: true
        model: contactsModel
        delegate: contactDelegate
    }

    Component {
        id: contactDelegate
        Rectangle {
            width: contactsListView.width
            height: 40
            color: model.selected ? themeManager.incomingColor : "transparent"
            SmartImage {
                id:profileImage
                width: 30
                height: 30
                textImage: model.username
                source: avatarSource + model.id + ".png?" + timestamp
                anchors {
                    left: parent.left
                    leftMargin: 5
                    verticalCenter: parent.verticalCenter
                }

            }
            Text {
                text: model.username
                color: "White"
                font.pointSize: 14
                anchors.left: profileImage.right
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    model.selected = !model.selected;
                    contactsModel.setProperty(index, "selected", model.selected);
                }
            }
        }
    }

    Text{
        id:closeButton
        text: "✕"
        color: "White"
        font.pointSize: 15
        font.bold: true

        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
        MouseArea {
            id: closeButtonMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                selectContactsForm.close()
            }
        }
    }

    Text{
        id:backButton
        text: "Back"
        color: "White"
        font.pointSize: 12
        anchors{
            right: createButton.left
            rightMargin: 10
            bottom: createButton.bottom
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: { selectContactsForm.close() }
        }
    }

    Text{
        id:createButton
        text: params === "create" ? "Create" : "Add"
        color: "White"
        font.pointSize: 12
        anchors{
            right: parent.right
            rightMargin: 10
            bottom: parent.bottom
            bottomMargin: 10
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                var selectedContacts = [];
                for (var i = 0; i < contactsModel.count; i++) {
                    var contact = contactsModel.get(i);
                    if (contact.selected) {
                        selectedContacts.push({ "id": contact.id});
                    }
                }
                if(params === "create") {
                    client.createGroup(createGroupForm.group_name, createGroupForm.sourcePath, selectedContacts);
                    selectContactsForm.close();
                    createGroupForm.close();
                }
                else if (params === "add") {
                    var filteredContacts = selectedContacts.filter(function(contact) {
                        return !groupInfoForm.isMemberExists(contact.id)
                    })
                    client.addGroupMembers(groupInfoForm.group_id, filteredContacts);
                    selectContactsForm.close();
                }

            }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 200 }
    }

    ListModel {
        id: contactsModel
    }

    function onLoadContacts(jsonArray) {
        contactsModel.clear();
        for (var i = 0; i < jsonArray.length; i++) {
            var contact = jsonArray[i];
            contactsModel.append({ "id": contact.id, "username": contact.username, "selected": false });
        }
    }
    function setParams(params) {
        selectContactsForm.params = params;
    }

    onOpened: {
        overlay.opacity = 1
        contactsModel.clear();
        client.showContacts();
        selectContactsForm.opacity = 1
    }

    onClosed: {
        params = "create"
        overlay.opacity = 0
        selectContactsForm.opacity = 0
    }

    Component.onCompleted: {
        loadContacts.connect(onLoadContacts);
    }
}
