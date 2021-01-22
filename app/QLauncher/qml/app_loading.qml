import QtQuick 2.5

Item {
    id: splashScreen
    visible: opacity > 0
    anchors.fill: parent

    Image {
            id: background
            fillMode: Image.PreserveAspectCrop
            smooth: true
            verticalAlignment: Image.AlignTop
            source: "qrc:/images/background"
            width: applicationWindow.width
	    height: applicationWindow.height
            sourceSize.width:applicationWindow.width
            sourceSize.height:applicationWindow.height
        }

    BusyIndicator {
        id: busyIndicator
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: splashLabel.top
        anchors.bottomMargin: height * .2
        width: parent.width * .1
    }

    Text {
        id: splashLabel
        color: "black"
        text: qsTr(" %1").arg(applicationWindow.launchingAppName)
        anchors.margins: parent.height * 0.1;
        anchors.bottomMargin: font.pixelSize
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: font.pixelSize * 2.2
    }


}
