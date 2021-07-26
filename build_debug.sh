#!/usr/bin/sh

if [ ! -d build ]; then
    mkdir build
else 
    echo build exist
fi

cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_GPERF_TOOLS=True .. 
make -j8
