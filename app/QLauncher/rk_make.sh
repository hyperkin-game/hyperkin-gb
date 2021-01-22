#! /bin/sh
QT_PROJECT_FILE=QLauncher.pro
TOP_DIR=$(pwd)
APP_DIR_NAME=$(basename $TOP_DIR)
BUILDROOT_TARGET_PATH=$(pwd)/../../buildroot/output/target/
TARGET_APP_PATH=$BUILDROOT_TARGET_PATH/usr/local/$APP_DIR_NAME/
QMAKE=$(pwd)/../../buildroot/output/host/bin/qmake
STRIP=$(pwd)/../../buildroot/output/host/usr/bin/arm-linux-strip
PRODUCT_NAME=`ls ../../device/rockchip/`
package_config=$(pwd)/../../device/rockchip/$PRODUCT_NAME/package_config.sh
TARGET_EXECUTABLE=""

if [ "$PRODUCT_NAME"x = "px3-se"x ];then
sed -i '/DEVICE_EVB/s/^/#&/' $QT_PROJECT_FILE
fi

if [ -f $package_config ];then
     source $package_config
     if [[ "$PLATFORM_WAYLAND"x == "no"x ]];then
	 echo "build Qluancher with EGLFS support"
	 sed -i '/PLATFORM_WAYLAND/s/^/#&/' $QT_PROJECT_FILE
    fi
fi

#get parameter for "-j2~8 and clean"
result=$(echo "$1" | grep -Eo '*clean')
if [ "$result" = "" ];then
        mulcore_cmd=$(echo "$1" | grep '^-j[0-9]\{1,2\}$')
elif [ "$1" = "clean" ];then
        make clean
elif [ "$1" = "distclean" ];then
        make clean
else
	mulcore_cmd=-j16
fi

#qmake and build target
$QMAKE
make $mulcore_cmd
rm -rf $TARGET_APP_PATH
if [ ! -d "$TARGET_APP_PATH" ]; then
	mkdir -p "$TARGET_APP_PATH"  
fi  
for file in ./*
do
    if test -x $file
    then
	elf=$(file -e elf $file)
	result=$(echo $elf | grep "LSB executable")
	if [ -n "$result" ]
	then
		TARGET_EXECUTABLE=$(basename "$file")
		echo "found executable file $TARGET_EXECUTABLE"
		$STRIP $TOP_DIR/$TARGET_EXECUTABLE
		cp $TOP_DIR/$TARGET_EXECUTABLE $TARGET_APP_PATH
		#cp -r $TOP_DIR/resources $TARGET_APP_PATH
		echo "$TARGET_EXECUTABLE app is ready."
	fi
    fi
done

make clean


#call just for buid_all.sh
if [ "$1" = "cleanthen" ] || [ "$2" = "cleanthen" ];then
        make clean
fi

#we should restore the modifcation which is made on this script above.
if [ "$PRODUCT_NAME"x = "px3-se"x ];then
sed -i '/DEVICE_EVB/s/^.//' $QT_PROJECT_FILE
fi

if [ -f $package_config ];then
     if [[ "$PLATFORM_WAYLAND"x == "no"x ]];then
         sed -i '/PLATFORM_WAYLAND/s/^.//' $QT_PROJECT_FILE
    fi
fi
