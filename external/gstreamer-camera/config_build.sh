#!/bin/sh
IS_LIB64=TRUE

CUR=`pwd`
[ -d "$CUR/out" ] && rm -rf $CUR/out
mkdir -p $CUR/out/lib

export CFLAG=-DLINUX
export CPPFLAGS=-DLINUX
export LIBDRM_CFLAGS="-I\$(abs_top_srcdir)/ext/rkisp/usr/include -I\$(abs_top_srcdir)/ext/rkisp/usr/include/drm"
if [ "$IS_LIB64" = "TRUE" ]; then
	cp $CUR/ext/rkisp/lib64/* $CUR/out/lib
	export CXX=$CUR/../toolchains/gcc-linaro-5.5.0-2017.10-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++
	export CC=$CUR/../toolchains/gcc-linaro-5.5.0-2017.10-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
	export LIBDRM_LIBS="-L\$(abs_top_srcdir)/ext/rkisp/usr/lib64 -ldrm"
	./autogen.sh --prefix=$CUR/out --host=aarch64-linux --enable-rkiq --enable-gst --enable-rklib64
else
	cp $CUR/ext/rkisp/lib/* $CUR/out/lib
	export CXX=$CUR/../toolchains/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
	export CC=$CUR/../toolchains/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
	export LIBDRM_LIBS="-L\$(abs_top_srcdir)/ext/rkisp/usr/lib -ldrm"
	./autogen.sh --prefix=$CUR/out --host=aarch64-linux --enable-rkiq --enable-gst
fi
