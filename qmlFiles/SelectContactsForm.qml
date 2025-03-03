import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    modal: true
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    opacity: 0

    background: Rectangle {
        color: "#1e2a36"
        radius: 6
        border.color: "#626a72"
        border.width: 1/2
    }
    width: 300
    height: 400

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
            color: model.selected ? "#626a72" : "transparent"
            Text {
                text: model.username
                color: "White"
                font.pointSize: 14
                anchors.centerIn: parent
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
        text: "Create"
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
                        selectedContacts.push({ "id": contact.id,  "username": contact.username });
                    }
                }
                client.createGroup(groupName.text,selectedContacts);
                selectContactsForm.close();
                createGroupForm.close();
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
    onOpened: {
        overlay.opacity = 1
        contactsModel.clear();
        client.showContacts();
        selectContactsForm.opacity = 1
    }

    onClosed: {
        overlay.opacity = 0
        selectContactsForm.opacity = 0
    }

    Component.onCompleted: {
        loadContacts.connect(onLoadContacts);
    }
}
