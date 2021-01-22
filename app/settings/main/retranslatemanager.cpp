#include "retranslatemanager.h"

RetranslateManager* RetranslateManager::_instance = 0;

// bluetooth
QString str_bluetooth_title = "";
QString str_bluetooth_enable_devices = "";
QString str_bluetooth_connect_with = "";
QString str_bluetooth_disconnect_with = "";
QString str_bluetooth_pair_with = "";
QString str_bluetooth_connect = "";
QString str_bluetooth_disconnect = "";
QString str_bluetooth_pair = "";
QString str_bluetooth_cancel = "";
QString str_bluetooth_name = "";
QString str_bluetooth_address = "";
QString str_bluetooth_confirm = "";
QString str_bluetooth_cancel_save = "";
QString str_bluetooth_item_connected = "";
QString str_bluetooth_item_pairing = "";
QString str_bluetooth_item_paired = "";
// hotspot
QString str_hotspot_title = "";
QString str_hotspot_name = "";
QString str_hotspot_password = "";
QString str_hotspot_warning = "";
QString str_hotspot_format_error = "";
//wifi
QString str_wifi_open = "";
QString str_wifi_close = "";
QString str_wifi_item_saved = "";
QString str_wifi_item_failed = "";
QString str_wifi_item_connecting = "";
QString str_wifi_item_connected = "";
QString str_wifi_signal_excellent = "";
QString str_wifi_signal_good = "";
QString str_wifi_signal_ok = "";
QString str_wifi_signal_weak = "";
QString str_wifi_connect = "";
QString str_wifi_cancel = "";
QString str_wifi_forget = "";
QString str_wifi_complete = "";
QString str_wifi_state = "";
QString str_wifi_signal_strength = "";
QString str_wifi_frequency = "";
QString str_wifi_underline_password = "";
// keyboard
QString str_board_close = "";
QString str_board_lab_upper = "";
QString str_board_lab_lower = "";
QString str_board_upper = "";
QString str_board_lower = "";

RetranslateManager* RetranslateManager::getInstance(QObject *parent = 0)
{
    if (_instance == NULL)
        _instance = new RetranslateManager(parent);

    return _instance;
}

RetranslateManager::RetranslateManager(QObject *parent) : QObject(parent)
{
}

void RetranslateManager::updateString()
{
    // bluetooth
    str_bluetooth_title = tr("Bluetooth(%1)");
    str_bluetooth_enable_devices = tr("Available Devices");
    str_bluetooth_connect_with = tr("Connect with ");
    str_bluetooth_disconnect_with = tr("Disconnect with ");
    str_bluetooth_pair_with = tr("Pair with ");
    str_bluetooth_connect = tr("Connect");
    str_bluetooth_disconnect = tr("Disconnect");
    str_bluetooth_pair = tr("Pair");
    str_bluetooth_cancel = tr("Cancel");
    str_bluetooth_name = tr("name:");
    str_bluetooth_address = tr("address:");
    str_bluetooth_confirm = tr("Confirm");
    str_bluetooth_cancel_save = tr("Cancel Save");
    str_bluetooth_item_connected = tr("Connected");
    str_bluetooth_item_pairing = tr("Pairing");
    str_bluetooth_item_paired = tr("Paired");
    // hotspot
    str_hotspot_title = tr("Hotspot");
    str_hotspot_name = tr("name");
    str_hotspot_password = tr("password");
    str_hotspot_warning = tr("warning");
    str_hotspot_format_error = tr("name or password is not available.");
    // wifi
    str_wifi_open = tr("OPEN");
    str_wifi_close = tr("CLOSE");
    str_wifi_item_saved = tr("Saved");
    str_wifi_item_failed = tr("Auth Failed");
    str_wifi_item_connecting = tr("Connecting");
    str_wifi_item_connected = tr("Connected");
    str_wifi_signal_excellent = tr("Excellent");
    str_wifi_signal_good = tr("Good");
    str_wifi_signal_ok = tr("Ok");
    str_wifi_signal_weak = tr("Weak");
    str_wifi_connect = tr("Connect");
    str_wifi_cancel = tr("Cancel");
    str_wifi_forget = tr("Forget");
    str_wifi_complete = tr("Complete");
    str_wifi_state = tr("State");
    str_wifi_signal_strength = tr("Signal Strength");
    str_wifi_frequency = tr("Frequency");
    str_wifi_underline_password = tr("<u>password</u>");
    // keyboard
    str_board_close = tr("Close");
    str_board_lab_upper = tr("Input Method —— Upper-Case");
    str_board_lab_lower = tr("Input Method —— Lower-Case");
    str_board_upper = tr("Upper");
    str_board_lower = tr("Lower");
}
