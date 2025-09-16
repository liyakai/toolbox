#!/usr/bin/sh

#脚本当前路径
scriptDir=$(cd $(dirname $0); pwd)
echo ${scriptDir};

# 可选编译第三方库libiouring
# git submodule update --init --recursive
# cd $scriptDir/src/thirdparty/liburing/
# ./configure --prefix=../../../../ --libdir=../../../../lib/ --includedir=../../../../include/
# make CFLAGS=-std=gnu99 && make install


# 编译 toolbox
cd $scriptDir
if [ ! -d build ]; then
    mkdir build
else 
    echo build exist
fi

cd $scriptDir/build
#cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_GPERF_TOOLS_CPU=OFF -DCMAKE_TCMALLOC=OFF -DCMAKE_USE_LIBIOURING=OFF  -DCMAKE_C_COMPILER=clang-3.8 -DCMAKE_CXX_COMPILER=clang++-3.8  -DGCC_TOOLCHAIN="/usr/lib/gcc/myfaketoolchain" .. 
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DCMAKE_BUILD_TYPE=Debug    \
    -DCMAKE_GPERF_TOOLS_CPU=OFF     \
    -DCMAKE_GPERF_TOOLS_MEM=OFF  \
    -DCMAKE_TCMALLOC=OFF     \
    -DCMAKE_JEMALLOC=OFF    \
    -DCMAKE_USE_LIBIOURING=OFF  \
    .. 

# clangd tool
cp compile_commands.json ../compile_commands.json

make -j8
