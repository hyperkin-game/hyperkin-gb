#! /bin/sh

TOP_DIR=$(pwd)
BUILDROOT_TARGET_PATH=$(pwd)/../../buildroot/output/target

aarch64_version=$(aarch64-linux-gcc --version 2>/dev/null)
arm_version=$(arm-linux-gcc --version 2>/dev/null)

if [ ! "$aarch64_version" = "" ] ;then
	gcc=aarch64-linux-gcc
	echo "gcc is aarch64-linux-gcc"
elif [ ! "$arm_version" = "" ] ;then
	gcc=arm-linux-gcc
	echo "gcc is arm-linux-gcc"
fi

$gcc -rdynamic -g -funwind-tables  -O0 -D_GNU_SOURCE $PM_DEFINES -o  power_manager_service power_service.c  power_manager.c  thermal.c  -lpthread -lxml2 -I$(pwd) -I$(pwd)/include -I$(pwd)/include/libxml


cp $TOP_DIR/power_manager_service $BUILDROOT_TARGET_PATH/usr/bin/
cp $TOP_DIR/thermal_sensor_config.xml $BUILDROOT_TARGET_PATH/etc/
cp $TOP_DIR/thermal_throttle_config.xml $BUILDROOT_TARGET_PATH/etc/

rm -rf $TOP_DIR/power_manager_service

echo "power_manager_service is ready on buildroot/output/target/usr/bin/"
