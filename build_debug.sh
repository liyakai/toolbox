#!/usr/bin/sh

#脚本当前路径
scriptDir=$(cd $(dirname $0); pwd)
echo ${scriptDir};

# 可选编译第三方库libiouring
cd $scriptDir/src/thirdparty/liburing/
./configure --prefix=../../../../ --libdir=../../../../lib/ --includedir=../../../../include/
make CFLAGS=-std=gnu99 && make install


# 编译 toolbox
cd $scriptDir
if [ ! -d build ]; then
    mkdir build
else 
    echo build exist
fi

cd $scriptDir/build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_GPERF_TOOLS_CPU=OFF -DCMAKE_GPERF_TOOLS_HEAP=OFF -DCMAKE_USE_LIBIOURING=ON .. 
make -j8
