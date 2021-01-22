import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Window 2.2
import Qt.labs.settings 1.0
import com.cai.qlauncher 1.0 as QL
import config 1.0 as Config
import debug 1.0 as D
import config 1.0 as Config
ApplicationWindow {
    id: applicationWindow
    flags: Qt.FramelessWindowHint | Qt.WindowFullScreen
    property bool isWindowActive: Qt.application.state === Qt.ApplicationActive
    property int dpi: Screen.pixelDensity * 25.4

    property variant desktopData: ([])

    property bool activeScreen: Qt.application.state === Qt.ApplicationActive

    property string launchingAppIcon
    property string launchingAppName

    function updatePortraitMode() {
        if (height >= width)
            Config.Theme.portrait = true
        else
            Config.Theme.portrait = false
        console.debug("updatePortraitMode:"+Config.Theme.portrait)
    }

    color: Config.Theme.colorApplicationWindow

    width:  Screen.width
    height: Screen.height

    visible: true

    onWidthChanged: updatePortraitMode()
    onHeightChanged: updatePortraitMode()

    onActiveScreenChanged: {
        if (activeScreen)
            QL.DisplayConfig.updateDisplayConfig()
    }

    onDesktopDataChanged: {
        listModelDesktop.clear()
        listModelDesktop.append(desktopData)

    }

    Component.onCompleted: {
        Config.Theme.tablet = QL.DisplayConfig.isTablet
        updatePortraitMode()
    }

    Timer {
        interval: 250
        running: true

        onTriggered: {
            QL.Launcher.registerMethods()
            QL.ApplicationManager.registerBroadcast()
        }
    }

    Loader {
        id: loaderMainTheme

        anchors.fill: parent

        focus: true

        source: Config.Theme.portrait?"theme/ThemePortrait.qml":"theme/ThemeLandscape.qml"
    }

    Loader {
        id:loading
        anchors.fill: parent
        focus: false
        visible: false
        source: "app_loading.qml"
    }

    Connections {
        target: QL.ApplicationManager

        onAddedApplicationToGrid: {
            var dks = desktopData
            dks.push({'name': name, 'pkgName': pkgName,'ui_name':ui_name,'argv':argv,'icon':icon,'exitCallback':exitCallback})
            //console.debug("onAddedApplicationToGrid name:"+name+",ui_name:"+ui_name);
            desktopData = dks
        }
        onLauncherApplicationState:{
            console.debug("onLauncherApplicationState state:"+state);
            if(state)
            {
                //applicationWindow.setVisible(false)
                loading.visible=true;
                loaderMainTheme.visible=false;
            }else
            {
                loading.visible=false;
                loaderMainTheme.visible=true;
                //applicationWindow.setVisible(true)
            }
        }
    }

    /*Settings {
        property alias desktop: applicationWindow.desktopData
    }*/

    ListModel {
        id: listModelDesktop
    }

   /* D.Debug {
        debugData: {
            'DisplayConfig.dp': QL.DisplayConfig.dp,
            'height': applicationWindow.height,
                    'width': applicationWindow.width,
                    //'wall_height': background.height,
                    //'wall_width': background.width,
                    'Theme.getColumns':Config.Theme.getColumns(),
                    'Theme.getRows':Config.Theme.getRows(),
                    'Config.Theme.portrait':Config.Theme.portrait,
                    //'dp': QL.DisplayConfig.dp.toFixed(2),
                    //'dpi': QL.DisplayConfig.dpi.toFixed(2),
                    //'density': QL.DisplayConfig.density.toFixed(2),
                    'isTablet': QL.DisplayConfig.isTablet
                    //
        }
    }*/
}
