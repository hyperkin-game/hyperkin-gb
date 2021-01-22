#!/bin/bash

if [ `cat /sys/class/drm/card0-HDMI-A-1/status` == "disconnected" ]; then
	amixer cset name='Master Playback Volume' 99
else
	amixer cset name='HDMI Playback Volume' 99
fi
