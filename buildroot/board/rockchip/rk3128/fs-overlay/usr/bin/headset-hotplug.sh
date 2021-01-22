#!/bin/sh -x

case $SWITCH_STATE in
    2)
        echo "New headset"
        # when hdmi connect do not use headset
        if [ `cat /sys/class/drm/card0-HDMI-A-1/status` == "connected" ]; then
          echo "headset plugin, but hdmi connected"
          amixer cset name='Playback Path' 'OFF'
        else
          amixer cset name='Playback Path' 'HP'
        fi
        ;;
    0)
        echo "No headset"
        if [ `cat /sys/class/drm/card0-HDMI-A-1/status` == "connected" ]; then
          echo "headset plugout, but hdmi connected"
          amixer cset name='Playback Path' 'OFF'
        else
          amixer cset name='Playback Path' 'OFF'
          amixer cset name='Playback Path' 'SPK'
        fi
        ;;
    *)
        echo "Unexpected state: $SWITCH_STATE"
esac

exit 0
