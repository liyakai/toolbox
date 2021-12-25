#!/usr/bin/sh

if [ ! -d build ]; then
    mkdir build
else 
    echo build exist
fi

cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_GPERF_TOOLS_CPU=OFF -DCMAKE_GPERF_TOOLS_HEAP=OFF .. 
make -j8
