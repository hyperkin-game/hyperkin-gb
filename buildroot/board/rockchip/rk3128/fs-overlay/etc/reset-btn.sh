#!/bin/sh
source /etc/check_dumper.sh
echo "reset.sh start" > /dev/console
$(ResetROM)
echo "reset.sh done" > /dev/console
