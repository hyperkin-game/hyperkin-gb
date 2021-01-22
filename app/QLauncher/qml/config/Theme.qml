pragma Singleton
import QtQuick 2.0
import Qt.labs.settings 1.0

Item {
    id: root

    property color colorGridContainer: "#f5f5f5"
    property color colorApplicationWindow: "#00000000"

    property bool portrait: true
    property bool tablet: true

    property int columns: getColumns(portrait)
    property int rows: getRows(portrait)

    function getColumns(portrait) {
        if (portrait) {
            if (tablet) {
                return 5
            } else {
                return 5
            }
        } else {
            if (tablet) {
                return 6
            } else {
                return 4
            }
        }
    }

    function getRows(portrait) {
        if (portrait) {
            return 6
        } else {
            if (tablet) {
                return 5
            } else {
                return 4
            }
        }
    }

    Settings {

    }
}
