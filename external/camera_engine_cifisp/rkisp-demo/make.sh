rm -rf ./rkisp_demo
../../../../toolchains/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++ rkisp_demo.cpp -rdynamic -o rkisp_demo -L./ -ldl
