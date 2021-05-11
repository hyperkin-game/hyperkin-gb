DUMPER_PATH="/media/usb0/"
ROM_PATH="/userdata/rom/"
RETROARCH_BIN="/usr/bin/retroarch"
RETROARCH_CON="/userdata/retroarch/retroarch.cfg"
GBA_PLUGIN="/usr/lib/libretro/gpsp_libretro.so"
GB_PLUGIN="/usr/lib/libretro/vbam_libretro.so"
MGBA_PLUGIN="/usr/lib/libretro/mgba_libretro.so"
HOME="/userdata"

if [ ! -f "/tmp/rom.txt" ]; then
	PREROM=""
else
	source "/tmp/rom.txt"
fi

function CheckROM(){
	cd ${DUMPER_PATH}
	for file in *.gb*
	do
		if [ -f "${file}" ]; then
			echo ${file};
			break;
		fi
	done
}


function KillGame(){
	killall retroarch
	/bin/sync
}

function ResetROM(){
	if pidof "retroarch" > /dev/null; then
		echo "kill retroarch" > /dev/console
		/usr/bin/killall -9 retroarch
	fi
	result=$(CheckROM)
	if [[ "x$result" == "x" ]]; then
		echo "without any rom file in usb" > /dev/console
		if [[ "x${PREROM}" != "x" ]]; then
			echo "remove ${PREROM}" > /dev/console
			/bin/rm -rf ${PREROM}
			/bin/sync
			echo 3 > /proc/sys/vm/drop_caches
		fi
	else
		if  pidof "hyperkin-loading" > /dev/null; then
			echo "Now executing the hyperkin-loading" > /dev/console
			echo "Disabled reset function until hyperkin-loading finished." > /dev/console
		else
			echo "remove ${PREROM}" > /dev/console
			/bin/rm -rf ${ROM_PATH}${result}
			echo 3 > /proc/sys/vm/drop_caches
			echo "loading:${ROM_PATH}${result}" > /dev/console
			/usr/bin/hyperkin-loading > /dev/console
			/bin/sync
			result=$(CheckROM)
			if [[ "x$result" != "x" ]]; then
				echo "PREROM=\"${ROM_PATH}${result}\"" > /tmp/rom.txt
				is_gba=$(echo $result | grep ".gba")
				if [[ "x$is_gba" != "x" ]]; then
					if [ `grep -c  $result /userdata/gb.txt` == 1 ]; then
						echo "loading:${ROM_PATH}${result} and Execute in Mgba." > /dev/console
						HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $MGBA_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &
					else
						HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $GBA_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &
					fi
				else
					if [ `grep -c  $result /userdata/gb.txt` == 1 ]; then
						echo "loading:${ROM_PATH}${result} and Execute in Mgba." > /dev/console
						HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $MGBA_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &
					else
						HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $GB_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &
					fi
				fi
			else
				echo "No Cartridge" > /dev/console
			fi
		fi
	fi
}

function RunGame(){
	#echo "RunGame" > /dev/console
if pidof "retroarch" > /dev/null; then
        echo "retroarch already running" > /dev/console
else
	echo "retroarch not running" > /dev/console
	result=$(CheckROM)
	if [[ "x$result" == "x" ]]; then
		echo "without any rom file" > /dev/console
	elif [ ! -f ${ROM_PATH}${result} ]; then
		#file not found, start copy files
		echo "loading:${ROM_PATH}${result}" > /dev/console
		/usr/bin/hyperkin-loading > /dev/console
	else
		/usr/bin/hyperkin-crc ${ROM_PATH}${result} > /dev/console
		retval=$?
		echo "retval=$retval" > /dev/console
		if [ $retval -ne 0 ]; then #if retval not 0, then do copy again
			echo "rom crc check fail, do copy again" > /dev/console
			/usr/bin/hyperkin-loading > /dev/console
		else
			echo "rom crc check passed" > /dev/console
		fi
	fi
	/bin/sync
	result=$(CheckROM)
	
	if [[ "x$result" != "x" ]]; then
		echo "PREROM=\"${ROM_PATH}${result}\"" > /tmp/rom.txt
		is_gba=$(echo $result | grep ".gba")
		if [[ "x$is_gba" != "x" ]]; then
			if [ `grep -c  $result /userdata/gb.txt` == 1 ]; then
				echo "loading:${ROM_PATH}${result} and Execute in Mgba." > /dev/console
				HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $MGBA_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &
			else
				cd /userdata/rom/
				result=$(CheckROM)
				new=${result//gba/srm}
				echo $new	
				new1=${result//gba}
				echo $new1
				ln -s $new $new1"sav"
				/bin/sync 	
				HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $GBA_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &
			fi
		else
			if [ `grep -c  $result /userdata/gb.txt` == 1 ]; then
				echo "loading:${ROM_PATH}${result} and Execute in Mgba." > /dev/console
				HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $MGBA_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &				
			else
				HOME=$HOME $RETROARCH_BIN -v -c $RETROARCH_CON -L $GB_PLUGIN "${ROM_PATH}${result}" > /dev/console 2>&1 &
			fi
		fi
	fi
fi
}
