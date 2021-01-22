#!/bin/bash

BUILDROOT=$(pwd)
TARGET=$1
NAME=$(whoami)
HOST=$(hostname)
DATETIME=`date +"%Y-%m-%d %H:%M:%S"`
echo "built by $NAME on $HOST at $DATETIME" > $TARGET/timestamp
exit 0
