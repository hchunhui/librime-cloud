#!/bin/sh
export CC="x86_64-w64-mingw32-gcc"
export CFLAGS="-Os -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables"
export LDFLAGS="-Wl,-s -Wl,-Bsymbolic -Wl,--gc-sections -static-libgcc"
mkdir -p build64 && cd build64 && ../configure --host x86_64-w64-mingw32 \
--disable-pthreads \
--enable-static \
--disable-shared \
--disable-cookies \
--disable-dict \
--disable-file \
--disable-ftp \
--disable-gopher \
--disable-imap \
--disable-ldap \
--disable-mqtt \
--disable-pop3 \
--disable-proxy \
--disable-rtsp \
--disable-smtp \
--disable-telnet \
--disable-tftp \
--disable-unix-sockets \
--disable-verbose \
--disable-versioned-symbols \
--disable-http-auth \
--disable-doh \
--disable-mime \
--disable-dateparse \
--disable-netrc \
--disable-dnsshuffle \
--disable-progress-meter \
--enable-maintainer-mode \
--enable-werror \
--without-brotli \
--without-gssapi \
--without-libidn2 \
--without-libpsl \
--without-librtmp \
--without-libssh2 \
--without-nghttp2 \
--without-zlib \
--without-zstd \
--with-schannel \
&& make
