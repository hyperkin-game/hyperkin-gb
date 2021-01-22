#!/bin/sh

# Uncomment below to see more logs
# set -x

resize_e2fs()
{
	DEV=$1
	MOUNT_POINT=$2
	PART_NAME=$3
	LABEL=$(dumpe2fs -h $DEV | grep "volume name:" | grep -o "[^ ]*$")

	[ $? -eq 0 ] || return

	# Not first boot, likely already resized
	[ "$LABEL" == "$PART_NAME" ] && return

	echo Resizing $DEV...

	mountpoint -q $MOUNT_POINT || e2fsck -fy $DEV

	# Force using online resize, see:
	# https://bugs.launchpad.net/ubuntu/+source/e2fsprogs/+bug/1796788.
	TEMP=$(mktemp -d)
	mount $DEV $TEMP
	resize2fs $DEV
	umount $TEMP

	# Use volume name to specify first boot
	tune2fs $DEV -L $PART_NAME
}

resize_fatresize()
{
	DEV=$1
	SIZE=$2
	MAX_SIZE=$3

	fatresize -h >/dev/null 2>&1 || echo "No fatresize" && return

	# Somehow fatresize only works for 256M+ fat
	[ ! $SIZE -gt $((256 * 1024 * 1024)) ] && return

	echo Resizing $DEV...

	MIN_SIZE=$(($MAX_SIZE - 16 * 1024 * 1024))
	[ $MIN_SIZE -lt $SIZE ] && MIN_SIZE=$SIZE
	while [ $MAX_SIZE -gt $MIN_SIZE ];do
		# Somehow fatresize cannot resize to max size
		MAX_SIZE=$(($MAX_SIZE - 512 * 1024))

		# Try to resize with fatresize, not always work
		fatresize -s $MAX_SIZE $DEV && return
	done
}

resize_fat()
{
	DEV=$1
	MAX_SIZE=$4
	FAT_INFO=$(fsck.fat -vy $DEV 2>/dev/null |grep -iE "data|files" |grep -o "[0-9]\+ ")

	[ $? -eq 0 ] || return

	DATA_START=$(echo $FAT_INFO|cut -d ' ' -f 1)
	DATA_SIZE=$(echo $FAT_INFO|cut -d ' ' -f 3)
	SIZE=$(($DATA_START + $DATA_SIZE))
	FILES=$(echo $FAT_INFO|cut -d ' ' -f 4)

	# Avoid resizing when it's large enough (delta less than 4M)
	[ $SIZE -gt $(($MAX_SIZE - 4 * 1024 * 1024)) ] && return

	# Format it when it's empty
	if [ $FILES -eq 0 ];then
		echo Formatting $DEV...
		mkfs.fat $DEV
		return
	fi

	# Try fatresize (might not work though)
	resize_fatresize $DEV $SIZE $MAX_SIZE
}

resize_ntfs()
{
	DEV=$1
	MAX_SIZE=$4
	SIZE=$(ntfsresize -if $DEV 2>/dev/null|grep -o "volume size: [0-9]\+" |grep -o "[0-9]\+")

	[ $? -eq 0 ] || return

	# Always attempt to fix it, since fsck might not work for ntfs
	ntfsfix $DEV

	# Avoid resizing when it's large enough (delta less than 4M)
	[ $SIZE -gt $(($MAX_SIZE - 4 * 1024 * 1024)) ] && return

	echo Resizing $DEV...
	echo y | ntfsresize -f $DEV
}

do_resize()
{
	# Not enough args
	[ $# -lt 3 ] && return

	# Ignore comments
	echo $1 |grep -q "^#" && return

	DEV=$1
	MOUNT_POINT=$2
	FSTYPE=$3

	# Find real dev for root dev
	if [ "$MOUNT_POINT" == '/' ];then
		DEV=$(mountpoint -n /|cut -d ' ' -f 1)
	fi

	# Unknown device
	[ -b "$DEV" ] || return

	DEV=$(realpath $DEV)
	SYS_PATH=/sys/class/block/${DEV##*/}
	MAX_SIZE=$(( $(cat ${SYS_PATH}/size) * 512))
	PART_NAME=$(grep PARTNAME ${SYS_PATH}/uevent | cut -d '=' -f 2)

	case $FSTYPE in
		ext*)
			resize_e2fs $DEV $MOUNT_POINT $PART_NAME $MAX_SIZE
			return
			;;
		vfat)
			resize_fat $DEV $MOUNT_POINT $PART_NAME $MAX_SIZE
			return
			;;
		ntfs)
			resize_ntfs $DEV $MOUNT_POINT $PART_NAME $MAX_SIZE
			return
			;;
	esac

	# Unsupported file system
}

resizeall()
{
	echo "Will now resize all file systems of types $MOUNTALL_USER_FSTYPES"
	while read LINE;do
		do_resize $LINE
	done < /etc/fstab
}

checkall()
{
	grep -wq "force_fsck" /proc/cmdline && FORCE_FSCK="-f"
	grep -wq "no_force_fsck" /proc/cmdline && unset FORCE_FSCK

	# Uncomment below to enable force fsck
	# FORCE_FSCK="-f"

	echo "Will now check all file systems of types $MOUNTALL_USER_FSTYPES"

	SKIP_FSCK="/var/.skip_fsck"
	if [ -f $SKIP_FSCK ];then
		echo "Skipped, remove $SKIP_FSCK to enable it again"
		return
	else
		echo "Create $SKIP_FSCK to skip it"
	fi

	fsck -ARy $FORCE_FSCK -t $MOUNTALL_USER_FSTYPES

	# The fsck might not work for vfat/ntfs...
	# But no worry, we've done that in resize_fat/resize_ntfs ;)
}

mountall()
{
	echo "Will now mount all file systems of types $MOUNTALL_FSTYPES"
	mount -a -t $MOUNTALL_FSTYPES
}

is_recovery()
{
	# Recovery's rootfs is ramfs
	mountpoint -d /|grep -wq 0:1
}

case "$1" in
  start|"")
	RESIZE_LOG=/tmp/resizefs.log
	CHECK_LOG=/tmp/checkfs.log
	MOUNT_LOG=/tmp/mountfs.log

	MOUNTALL_BASE_FSTYPES="proc,devpts,tmpfs,sysfs"
	MOUNTALL_USER_FSTYPES="ext2,ext3,ext4,vfat,ntfs,squashfs"
	MOUNTALL_FSTYPES="${MOUNTALL_USER_FSTYPES},${MOUNTALL_BASE_FSTYPES}"

	# Mount /tmp firstly to save logs
	mountpoint -q /tmp || mount -t tmpfs tmpfs /tmp

	if is_recovery;then
		# Only mount basic file systems for recovery
		MOUNTALL_FSTYPES=$MOUNTALL_BASE_FSTYPES
		mountall 2>&1 | tee $MOUNT_LOG
		echo Log saved to $MOUNT_LOG
	else
		resizeall 2>&1 | tee $RESIZE_LOG
		echo Log saved to $RESIZE_LOG
		checkall 2>&1 | tee $CHECK_LOG
		echo Log saved to $CHECK_LOG
		mountall 2>&1 | tee $MOUNT_LOG
		echo Log saved to $MOUNT_LOG
	fi
	touch /tmp/boot_complete
	echo "\n\n S21mountall.sh done\n\n" > /dev/console
	;;
  restart|reload|force-reload)
	echo "Error: argument '$1' not supported" >&2
	exit 3
	;;
  stop|status)
	# No-op
	;;
  *)
	echo "Usage: [start|stop]" >&2
	exit 3
	;;
esac

:
