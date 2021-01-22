#! /bin/sh

EXT_PROJECT_FILE=audioservice
TOP_DIR=$(pwd)
BUILD_DIR=$(pwd)/build
#EXT_DIR_NAME=$(basename $TOP_DIR)
BUILDROOT_TARGET_PATH=$(pwd)/../../buildroot/output/target/
TARGET_BIN_PATH=$BUILDROOT_TARGET_PATH/usr/bin/
TOOLCHAIN_FILE=$(pwd)/../../buildroot/output/host/usr/share/buildroot/toolchainfile.cmake
PRODUCT_NAME=`ls ../../device/rockchip`
TARGET_EXECUTABLE=""

if [ "$PRODUCT_NAME"x = "px3-se"x ]; then
	STRIP=$(pwd)/../../buildroot/output/host/usr/bin/arm-rockchip-linux-gnueabihf-strip
elif [ "$PRODUCT_NAME"x = "rk3399"x ]; then
	STRIP=$(pwd)/../../buildroot/output/host/usr/bin/aarch64-rockchip-linux-gnu-strip
fi
#define build err exit function
check_err_exit(){
  if [ $1 -ne "0" ]; then
     echo -e $MSG_ERR
     exit 2
  fi
}

#get parameter for "-j2~8 and clean"
result=$(echo "$1" | grep -Eo '*clean')
if [ "$result" = "" ];then
        mulcore_cmd=$(echo "$1" | grep '^-j[0-9]\{1,2\}$')
elif [ "$1" = "clean" ];then
        make clean
elif [ "$1" = "distclean" ];then
        make clean
else
	mulcore_cmd=-j4
fi

# build executable bin.
mkdir build
cd $BUILD_DIR
cmake -D CMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" ..
make $mulcore_cmd
check_err_exit $?

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
		$STRIP $BUILD_DIR/$TARGET_EXECUTABLE
		cp $BUILD_DIR/$TARGET_EXECUTABLE $TARGET_BIN_PATH
		echo "audioservice is ready in /usr/bin/audioservice."
	fi
    fi
done

make clean
cd $TOP_DIR
rm -rf $BUILD_DIR

#call just for buid_all.sh
#if [ "$1" = "cleanthen" ] || [ "$2" = "cleanthen" ];then
        #make clean
#fi

