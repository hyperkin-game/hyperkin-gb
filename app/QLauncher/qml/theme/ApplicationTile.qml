import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import com.cai.qlauncher 1.0

Item {
    id: root

    property alias text: label.text
    property alias source: image.source
    property var dragTarget

    property var _originalParent
    property var _newParent

    signal pressAndHold(var model, int x, int y)
    signal clicked

    Drag.active: mouseArea.drag.active
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    //width: 80* QL.DisplayConfig.dp
    //height: 80* QL.DisplayConfig.dp

    Component.onCompleted: {
        _originalParent = parent
        _newParent = _originalParent
    }

    ColumnLayout {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Image {
            id: image

            anchors.horizontalCenter: parent.horizontalCenter

            asynchronous: true
            sourceSize.width: Math.round(72 * DisplayConfig.dp)
            sourceSize.height: Math.round(72 * DisplayConfig.dp)
            //Layout.preferredHeight: Math.round(72 * DisplayConfig.dp)
            //Layout.preferredWidth: Math.round(72 * DisplayConfig.dp)

            fillMode: Image.PreserveAspectFit

            MouseArea {
            id: mouseArea

            property var originalParent

            anchors.fill: parent

            drag.target: root.dragTarget

            onClicked:
            {
            //console.debug("ApplicationTile onClicked");
            root.clicked() //call ApplicationGrid:onClicked

            }

            onPressAndHold: {
            var mappedItem = mapToItem(loaderMainTheme, mouse.x, mouse.y)
            root.pressAndHold(model, mappedItem.x, mappedItem.y)
            }

            onReleased: root._newParent = (root.Drag.target !== null ? root.Drag.target : root._originalParent)

            }
        }

        Label {
            id: label

            anchors.horizontalCenter: parent.horizontalCenter

            Layout.preferredWidth: parent.width * 0.90

            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.NoWrap
            maximumLineCount: 1

            font.pixelSize: 18 * DisplayConfig.dp

            color: "#000000"
        }
    }
}
