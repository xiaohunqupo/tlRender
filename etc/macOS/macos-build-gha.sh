#!/bin/bash

set -x

BUILD_TYPE=$1

mkdir build
cd build
cmake ../etc/SuperBuild \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$PWD/install \
    -DCMAKE_PREFIX_PATH=$PWD/install \
    -DCMAKE_CXX_STANDARD=$CMAKE_CXX_STANDARD \
    -Ddtk_API=$dtk_API \
    -DTLRENDER_NET=$TLRENDER_NET \
    -DTLRENDER_OCIO=$TLRENDER_OCIO \
    -DTLRENDER_AUDIO=$TLRENDER_AUDIO \
    -DTLRENDER_JPEG=$TLRENDER_JPEG \
    -DTLRENDER_TIFF=$TLRENDER_TIFF \
    -DTLRENDER_STB=$TLRENDER_STB \
    -DTLRENDER_PNG=$TLRENDER_PNG \
    -DTLRENDER_EXR=$TLRENDER_EXR \
    -DTLRENDER_FFMPEG=$TLRENDER_FFMPEG \
    -DTLRENDER_USD=$TLRENDER_USD \
    -DTLRENDER_QT5=$TLRENDER_QT5 \
    -DTLRENDER_PROGRAMS=$TLRENDER_PROGRAMS \
    -DTLRENDER_EXAMPLES=$TLRENDER_EXAMPLES \
    -DTLRENDER_TESTS=$TLRENDER_TESTS \
    -DTLRENDER_GCOV=$TLRENDER_GCOV \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} \
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
cmake --build . -j 1 --config $BUILD_TYPE
