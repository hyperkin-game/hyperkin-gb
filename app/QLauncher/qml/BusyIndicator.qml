import QtQuick 2.4

Image {
    height: width
    source: applicationWindow.launchingAppIcon
    sourceSize.width: Math.min(127, width)
    sourceSize.height: Math.min(127, height)

//    RotationAnimator on rotation {
//        duration: 800
//        loops: Animation.Infinite
//        from: 0
//        to: 360
//        running: visible
//    }
}
