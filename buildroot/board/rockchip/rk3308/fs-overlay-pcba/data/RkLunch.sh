#! /bin/sh

echo_pcbatest_server &
echo_auto_test echo_wlan_test &
echo_auto_test echo_bt_test &
echo_auto_test echo_ddr_test &

#echo_auto_test echo_emmc_test &
#echo_auto_test echo_rtc_test &