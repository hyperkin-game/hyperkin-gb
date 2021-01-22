#ifndef RETRANSLATEMANAGER_H
#define RETRANSLATEMANAGER_H

#include <QObject>

// bluetooth
extern QString str_bluetooth_title;
extern QString str_bluetooth_enable_devices;
extern QString str_bluetooth_connect_with;
extern QString str_bluetooth_disconnect_with;
extern QString str_bluetooth_pair_with;
extern QString str_bluetooth_connect;
extern QString str_bluetooth_disconnect;
extern QString str_bluetooth_pair;
extern QString str_bluetooth_cancel;
extern QString str_bluetooth_name;
extern QString str_bluetooth_address;
extern QString str_bluetooth_confirm;
extern QString str_bluetooth_cancel_save;
extern QString str_bluetooth_item_connected;
extern QString str_bluetooth_item_pairing;
extern QString str_bluetooth_item_paired;
// hotspot
extern QString str_hotspot_title;
extern QString str_hotspot_name;
extern QString str_hotspot_password;
extern QString str_hotspot_warning;
extern QString str_hotspot_format_error;
// wifi
extern QString str_wifi_open;
extern QString str_wifi_close;
extern QString str_wifi_item_saved;
extern QString str_wifi_item_failed;
extern QString str_wifi_item_connecting;
extern QString str_wifi_item_connected;
extern QString str_wifi_signal_excellent;
extern QString str_wifi_signal_good;
extern QString str_wifi_signal_ok;
extern QString str_wifi_signal_weak;
extern QString str_wifi_connect;
extern QString str_wifi_cancel;
extern QString str_wifi_forget;
extern QString str_wifi_complete;
extern QString str_wifi_state;
extern QString str_wifi_signal_strength;
extern QString str_wifi_frequency;
extern QString str_wifi_underline_password;
// keyboard
extern QString str_board_close;
extern QString str_board_lab_upper;
extern QString str_board_lab_lower;
extern QString str_board_upper;
extern QString str_board_lower;

class RetranslateManager : public QObject
{
    Q_OBJECT
public:
    RetranslateManager(QObject *parent = 0);
    static RetranslateManager* getInstance(QObject *parent);

public slots:
    void updateString();

private:
    static RetranslateManager *_instance;
};

#endif // RETRANSLATEMANAGER_H
