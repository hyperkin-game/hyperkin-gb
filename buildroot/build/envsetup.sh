#!/bin/bash

if [ -z "${BASH_SOURCE}" ];then
	echo Not in bash, switch to it...
	bash -c $0
fi

DEFCONFIG_ARRAY=(
"rockchip_rk3308_release" \
"rockchip_rk3308_32_release" \
"rockchip_rk3308_32_debug" \
"rockchip_rk3308_32_dueros" \
"rockchip_rk3308_64_dueros" \
"rockchip_rk3308_mini_release" \
"rockchip_rk3308_32_mini_release" \
"rockchip_rk3308_robot32" \
"rockchip_rk3308_robot64" \
"rockchip_rk3308_pcba" \
"rockchip_rk3308_recovery" \
"rockchip_rk3326_32" \
"rockchip_rk3326_64" \
"rockchip_rk3326_recovery" \
"rockchip_rk3326_robot32" \
"rockchip_rk3326_robot64" \
"rockchip_rk3399" \
"rockchip_rk3399_recovery" \
"rockchip_rk3288" \
"rockchip_rk3288_recovery" \
"rockchip_px30_32" \
"rockchip_px30_64" \
"rockchip_px30_robot32" \
"rockchip_px30_robot64" \
"rockchip_px30_recovery" \
"rockchip_px3se" \
"rockchip_px3se_recovery" \
"rockchip_rk3128_game" \
"rockchip_rk2818_game" \
"rockchip_rk3328" \
"rockchip_rk3328_recovery" \
"rockchip_rk1808" \
"rockchip_rk1808_recovery" \
"rockchip_rk3399pro_npu")

DEFCONFIG_ARRAY_LEN=${#DEFCONFIG_ARRAY[@]}

i=0
while [[ $i -lt $DEFCONFIG_ARRAY_LEN ]]
do
	let i++
done

function choose_info()
{
	echo
	echo "You're building on Linux"
	echo "Lunch menu...pick a combo:"
	echo ""
	i=0
	while [[ $i -lt $DEFCONFIG_ARRAY_LEN ]]
	do
		if [ $i -lt 100 ]; then
			echo "$((${i}+1)). ${DEFCONFIG_ARRAY[$i]}"
		else
			echo "$((${i}+1)). ${DEFCONFIG_ARRAY[$i]}_release"
		fi
		let i++
	done
	echo
}

function get_index() {
	if [ $# -eq 0 ]; then
		return 0
	fi

	i=0
	while [[ $i -lt $DEFCONFIG_ARRAY_LEN ]]
	do
		if [ $1 = "${DEFCONFIG_ARRAY[$i]}_release" ]; then
			let i++
			return ${i}
		fi
		let i++
	done
	return 0
}

function get_target_board_type() {
	TARGET=$1
	RESULT="$(echo $TARGET | cut -d '_' -f 2)"
	echo "$RESULT"
}

function get_build_config() {
	TARGET=$1
	RESULT1="$(echo $TARGET | cut -d '_' -f 3)"
	RESULT2="$(echo $TARGET | cut -d '_' -f 4)"
	if [[ $RESULT1 = "debug" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	elif [[ $RESULT1 = "release" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	elif [[ $RESULT2 = "debug" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	elif [[ $RESULT2 = "release" ]]; then
		echo "${DEFCONFIG_ARRAY[$index]}"
	else
		echo "${DEFCONFIG_ARRAY[$index]}"
	fi
}

function get_defconfig_name() {
	echo $TARGET_DIR_NAME
}

function get_target_build_type() {
	TARGET=$1
	TYPE="$(echo $TARGET | cut -d '_' -f 1)"

	LENGTH="$(echo $TARGET | awk -F '_' '{print NF}')"
	if [ $LENGTH -le 2 ]; then
		echo "64"
	else
		RESULT="$(echo $TARGET | cut -d '_' -f 3)"
		if [ $RESULT = "32" ]; then
			echo "32"
		elif [ $RESULT = "64" ]; then
			echo "64"
		else
			echo "64"
		fi
	fi
}

function choose_type()
{
	choose_info
	local DEFAULT_NUM DEFAULT_VALUE
	DEFAULT_NUM=1
	DEFAULT_VALUE="rockchip_rk3308_release"

	export TARGET_BUILD_TYPE=
	local ANSWER
	while [ -z $TARGET_BUILD_TYPE ]
	do
		echo -n "Which would you like? ["$DEFAULT_NUM"] "
		if [ -z "$1" ]; then
			read ANSWER
		else
			echo $1
			ANSWER=$1
		fi

		if [ -z "$ANSWER" ]; then
			ANSWER="$DEFAULT_NUM"
		fi

		if [ -n "`echo $ANSWER | sed -n '/^[0-9][0-9]*$/p'`" ]; then
			if [ $ANSWER -le $DEFCONFIG_ARRAY_LEN ] && [ $ANSWER -gt 0 ]; then
				index=$((${ANSWER}-1))
				TARGET_BUILD_CONFIG=`get_build_config ${DEFCONFIG_ARRAY[$index]}`
				TARGET_DIR_NAME="${DEFCONFIG_ARRAY[$index]}"
				TARGET_BUILD_TYPE=`get_target_build_type ${DEFCONFIG_ARRAY[$index]}`
				TARGET_BOARD_TYPE=`get_target_board_type ${DEFCONFIG_ARRAY[$index]}`
			else
				echo
				echo "number not in range. Please try again."
				echo
			fi
		else
			echo $ANSWER
			TARGET_BUILD_CONFIG="$ANSWER"
			TARGET_DIR_NAME="$ANSWER"
			TARGET_BUILD_TYPE=`get_target_build_type $ANSWER`
			TARGET_BOARD_TYPE=`get_target_board_type $ANSWER`
		fi
		if [ -n "$1" ]; then
			break
		fi
	done
	export TARGET_OUTPUT_DIR="$BUILDROOT_OUTPUT_DIR/$TARGET_DIR_NAME"
}

function lunch()
{
	mkdir -p $TARGET_OUTPUT_DIR
	if [ -z "$TARGET_BUILD_CONFIG" ]; then
		return;
	fi

	echo "==========================================="
	echo
	echo "#TARGET_BOARD=${TARGET_BOARD_TYPE}"
	echo "#BUILD_TYPE=${TARGET_BUILD_TYPE}"
	echo "#OUTPUT_DIR=output/$TARGET_DIR_NAME"
	echo "#CONFIG=${TARGET_BUILD_CONFIG}_defconfig"
	echo
	echo "==========================================="

	make -C ${BUILDROOT_DIR} O="$TARGET_OUTPUT_DIR" \
		"$TARGET_BUILD_CONFIG"_defconfig

	OLD_CONF=${TARGET_OUTPUT_DIR}/.config.old
	CONF=${TARGET_OUTPUT_DIR}/.config
	if diff ${OLD_CONF} ${CONF} 2>/dev/null|grep -qE "is not set$|=y$";then
		read -p "Found old config, override it? (y/n):" YES
		[ "$YES" != y ] && cp ${OLD_CONF} ${CONF}
	fi

	if [ -z "$index" ]; then
		return
	fi

	# first check the board dir
	if [ ! -d ${TOP_DIR}/device/rockchip/$TARGET_BOARD_TYPE ]; then
		# the above shell code seems cut the fixed number 2 3 4 by '_' to get target build info,
		# obey the fixed name rule when create new target build.
		echo "<debug> device/rockchip/${TARGET_BOARD_TYPE} is not exist, check the board dirname obey the rule rockchip_[boardtype]_[64/32][_][debug/release/recovery]"
		return
	fi
	echo relink.....
	TARGET_BOARD_CONFIG_FILE=BoardConfig.mk
	if [ $TARGET_BUILD_TYPE = 32 ]; then
		TARGET_BOARD_CONFIG_FILE=BoardConfig_32bit.mk
	fi
	BOARD_CONFIG=${TOP_DIR}/device/rockchip/.BoardConfig.mk
	ln -snf $TARGET_BOARD_TYPE/$TARGET_BOARD_CONFIG_FILE $BOARD_CONFIG
	sed --follow-symlinks -i "s/\(RK_CFG_BUILDROOT=\)..*/\1$TARGET_BUILD_CONFIG/g" $BOARD_CONFIG
	source $BOARD_CONFIG
}

function function_stuff()
{
	choose_type $@
	lunch
}

if [ "${BASH_SOURCE}" == "$0" ];then
	echo This script is executed directly...
	bash -c "source ${BASH_SOURCE}; bash"
else
	SCRIPT_PATH=$(realpath ${BASH_SOURCE})
	SCRIPT_DIR=$(dirname ${SCRIPT_PATH})
	BUILDROOT_DIR=$(dirname ${SCRIPT_DIR})
	BUILDROOT_OUTPUT_DIR=${BUILDROOT_DIR}/output
	TOP_DIR=$(dirname ${BUILDROOT_DIR})
	source ${TOP_DIR}/device/rockchip/.BoardConfig.mk
	echo Top of tree: ${TOP_DIR}

	# Set croot alias
	alias croot="cd ${TOP_DIR}"

	function_stuff $@
fi
