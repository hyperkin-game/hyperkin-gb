#!/bin/bash

# Version info
source device/rockchip/rk3128/Version.mk

# Target arch
export RK_ARCH=arm
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=brk01-rk3128
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rk3128_brk01_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk3128-brk01
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/zImage
# parameter for GPT table
export RK_PARAMETER=parameter-buildroot.txt
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk3128_brk01
# Recovery config
export RK_CFG_RECOVERY=rockchip_rk3128_recovery
# Pcba config
export RK_CFG_PCBA=rockchip_rk3128_pcba
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rk3128
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=squashfs
# rootfs image path
export RK_ROOTFS_IMG=buildroot/output/${RK_CFG_BUILDROOT}/images/rootfs.${RK_ROOTFS_TYPE}
# Set oem partition type, including ext2 squashfs
export RK_OEM_FS_TYPE=squashfs
# Set userdata partition type, including ext2, fat
export RK_USERDATA_FS_TYPE=ext2
# Set flash type. support <emmc, nand, spi_nand, spi_nor>
export RK_STORAGE_TYPE=nand
#OEM config: /oem/dueros/aispeech/iflytekSDK/CaeDemo_VAD/smart_voice
export RK_OEM_DIR=oem_normal
#userdata config
export RK_USERDATA_DIR=userdata_brk01
#misc image
export RK_MISC=wipe_all-misc.img
