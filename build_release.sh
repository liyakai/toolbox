#!/usr/bin/sh

if [ ! -d build ]; then
    mkdir build
else 
    echo build exist
fi

cd build

cmake ..  
make -j8
