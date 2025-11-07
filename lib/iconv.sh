#!/bin/sh
export CC="i686-w64-mingw32-gcc"
export CFLAGS="-Os -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables"
export LDFLAGS="-Wl,-s -Wl,-Bsymbolic -Wl,--gc-sections -static-libgcc"
mkdir -p build32 && cd build32 && ../configure --host i686-w64-mingw32 --disable-pthreads --enable-static --disable-shared && make -j$(nproc)
