#!/bin/sh
export XDG_RUNTIME_DIR=/tmp/.xdg
gst-launch-1.0 -vvv v4l2src device=/dev/video0 ! videoconvert ! waylandsink
