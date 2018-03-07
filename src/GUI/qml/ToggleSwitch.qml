import QtQuick 2.0
import "."

Item {
    property bool checked: false
    property alias label: label.text

    implicitWidth: track.width + label.width + 16
    implicitHeight: label.height

    Rectangle {
        id: track
        width: height * 2
        height: radius * 2
        radius: Style.roundRadius

        color: checked ? Style.frameColor : Style.minorFrameColor
        anchors.left: parent.left
        anchors.leftMargin: Style.margin
        anchors.verticalCenter: parent.verticalCenter

        Rectangle {
            id: thumb
            width: radius * 2
            height: radius * 2
            radius: Style.roundRadius * 1.5

            anchors.verticalCenter: parent.verticalCenter
            color: checked ? Style.themeColor : "white"
            border.width: 1
            border.color: Style.inactiveThemeColor

            x: checked ? parent.width - (track.radius + radius) : (track.radius - radius)

            Behavior on x {
                NumberAnimation {
                    duration: 250
                }
            }
        }
    }

    Text {
        id: label
        anchors.left: track.right
        anchors.leftMargin: Style.margin
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: Style.baseFontPixelSize
    }

    MouseArea {
        anchors.fill: parent
        id: mouseArea
        hoverEnabled: true
        onClicked: {
            checked = !checked
        }
    }
}
