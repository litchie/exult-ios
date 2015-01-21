#!/bin/sh

export XCODE_DEVROOT=/Applications/Xcode.app/Contents/Developer

# Select the desired iPhone SDK
export DEVROOT=$XCODE_DEVROOT/Platforms/iPhoneOS.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneOS6.1.sdk

# Set up relevant environment variables
export CPPFLAGS="-I$DEVROOT/usr/llvm-gcc-4.2/lib/gcc/arm-apple-darwin10/4.2.1 -I$SDKROOT/usr/include/ -miphoneos-version-min=5.1"
export CFLAGS="-O2 $CPPFLAGS -arch armv7 -pipe -no-cpp-precomp -isysroot $SDKROOT"
export CPP="$DEVROOT/usr/bin/cpp $CPPFLAGS"
export CXXFLAGS="$CFLAGS"

export LDFLAGS="-L$SDKROOT/usr/lib/ -Wl,-dylib_install_name,@executable_path/$LIBNAME"

# Static library that will be generated for ARM

# TODO: add custom flags as necessary for package
./configure CXX=$DEVROOT/usr/bin/arm-apple-darwin10-llvm-g++-4.2 CC=$DEVROOT/usr/bin/arm-apple-darwin10-gcc-4.2 LD=$DEVROOT/usr/bin/ld --host=arm-apple-darwin

make -j4

