#!/bin/bash

# Target arch
export RK_ARCH=arm
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rk2818_simulator_rev0_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk2818_simulator_rev0
# boot image type
export RK_BOOT_IMG=kernel.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/zImage
# parameter for GPT table
export RK_PARAMETER=parameter.txt
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk2818_game
# ramboot config
export RK_CFG_RAMBOOT=
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rk2818
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=squashfs
# rootfs image path
export RK_ROOTFS_IMG=buildroot/output/$RK_CFG_BUILDROOT/images/rootfs.$RK_ROOTFS_TYPE
# Set oem partition type, including ext2 squashfs
export RK_OEM_FS_TYPE=ext2
# Set userdata partition type, including ext2, fat
export RK_USERDATA_FS_TYPE=ext2
# Set flash type. support <emmc, nand, spi_nand, spi_nor>
export RK_STORAGE_TYPE=nand
#OEM config
export RK_OEM_DIR=oem_empty
#userdata config
export RK_USERDATA_DIR=userdata_empty
#misc image
export RK_MISC=wipe_all-misc.img
#choose enable distro module
export RK_DISTRO_MODULE=
