#!/bin/bash

set -e

COMMON_DIR=$(cd `dirname $0`; pwd)
if [ -h $0 ]
then
        CMD=$(readlink $0)
        COMMON_DIR=$(dirname $CMD)
fi
cd $COMMON_DIR
cd ../../..
TOP_DIR=$(pwd)

source $TOP_DIR/device/rockchip/.BoardConfig.mk
ROCKDEV=$TOP_DIR/rockdev
PARAMETER=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/$RK_PARAMETER
OEM_DIR=$TOP_DIR/device/rockchip/oem/$RK_OEM_DIR
USER_DATA_DIR=$TOP_DIR/device/rockchip/userdata/$RK_USERDATA_DIR
MISC_IMG=$TOP_DIR/device/rockchip/rockimg/$RK_MISC
ROOTFS_IMG=$TOP_DIR/$RK_ROOTFS_IMG
RAMBOOT_IMG=$TOP_DIR/buildroot/output/$RK_CFG_RAMBOOT/images/ramboot.img
RECOVERY_IMG=$TOP_DIR/buildroot/output/$RK_CFG_RECOVERY/images/recovery.img
TRUST_IMG=$TOP_DIR/u-boot/trust.img
UBOOT_IMG=$TOP_DIR/u-boot/uboot.img
BOOT_IMG=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/rockimg/boot.img
KERNEL_IMG=$TOP_DIR/kernel/$RK_BOOT_IMG
LOADER=$TOP_DIR/device/rockchip/$RK_TARGET_PRODUCT/rockimg/RK28xxLoader_ext3_new_6.24.bin
#SPINOR_LOADER=$TOP_DIR/u-boot/*_loader_spinor_v*.bin
MKOEM=$TOP_DIR/device/rockchip/common/mk-oem.sh
MKUSERDATA=$TOP_DIR/device/rockchip/common/mk-userdata.sh
mkdir -p $ROCKDEV

if [ $RK_ROOTFS_IMG ]
then
	if [ -f $ROOTFS_IMG ]
	then
		echo -n "create rootfs.img..."
		ln -s -f $ROOTFS_IMG $ROCKDEV/rootfs.img
		echo "done."
	else
		echo "warning: $ROOTFS_IMG not found!"
	fi
fi

if [ -f $PARAMETER ]
then
	echo -n "create parameter..."
	ln -s -f $PARAMETER $ROCKDEV/parameter.txt
	echo "done."
else
	echo "warning: $PARAMETER not found!"
fi

#if [ $RK_OEM_DIR ]
#then
#	if [ -d $OEM_DIR ]
#	then
#		echo -n "create oem.img..."
#		$MKOEM $OEM_DIR $ROCKDEV/oem.img $RK_OEM_FS_TYPE
#		echo "done."
#	else
#		echo "warning: $OEM_DIR  not found!"
#	fi
#fi

#if [ $RK_USERDATA_DIR ]
#then
#	if [ -d $USER_DATA_DIR ]
#	then
#		echo -n "create userdata.img..."
#		$MKUSERDATA $USER_DATA_DIR $ROCKDEV/userdata.img $RK_USERDATA_FS_TYPE
#		echo "done."
#	else
#		echo "warning: $USER_DATA_DIR not found!"
#	fi
#fi

if [ -f $LOADER ]
then
        echo -n "create loader..."
        ln -s -f $LOADER $ROCKDEV/MiniLoaderAll.bin
        echo "done."
else
	echo -e "\e[31m error: $LOADER not found,or there are multiple loaders! \e[0m"
	rm $LOADER
fi

if [ $RK_BOOT_IMG ]
then
	if [ -f $BOOT_IMG ]
	then
		echo -n "create boot.img..."
		ln -s -f $KERNEL_IMG $ROCKDEV/kernel.img
		ln -s -f $BOOT_IMG $ROCKDEV/boot.img
		echo "done."
	else
		echo "warning: $BOOT_IMG not found!"
	fi
fi
echo -e "\e[36m Image: image in rockdev is ready \e[0m"
