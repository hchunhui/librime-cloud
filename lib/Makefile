CC = gcc
CC_mingw = i686-w64-mingw32-gcc

LUAV ?= 5.4
LUAINC = thirdparty/lua${LUAV}
LUALIB = -Llibrime-lua${LUAV}/dist/lib -lrime

CURLV ?= 8.3.0
CURLINC = curl-${CURLV}/include
CURLLIB = -Lcurl-${CURLV}/lib/.libs -lcurl -lws2_32 -lcrypt32 -lbcrypt

PLAT ?= linux
LDFLAGS_linux = -Wl,--gc-sections -Wl,-s
LDFLAGS_macos = -Wl,-bundle -Wl,-undefined,dynamic_lookup
LDFLAGS = ${LDFLAGS_${PLAT}}

all: simplehttp.so simplehttp.dll

librime-lua5.4:
	curl -L -O https://github.com/rime/librime/releases/download/1.7.3/rime-with-plugins-1.7.3-win32.zip
	unzip rime-with-plugins-1.7.3-win32.zip -d librime-lua5.4

thirdparty:
	git clone -b thirdparty http://github.com/hchunhui/librime-lua.git thirdparty

curl-${CURLV}:
	curl -L -O https://curl.se/download/curl-${CURLV}.tar.gz
	tar xvf curl-${CURLV}.tar.gz

prepare: librime-lua5.4 thirdparty curl-${CURLV}

curl-${CURLV}/lib/.libs:
	cd curl-${CURLV} && sh ../tinycurl.sh

simplehttp.so: main.c thirdparty curl-${CURLV}
	${CC} -I${LUAINC} -I${CURLINC} -Os -fPIC -shared ${LDFLAGS} -o $@ $<

simplehttp.dll: main.c librime-lua${LUAV} thirdparty curl-${CURLV}/lib/.libs
	${CC_mingw} -I${LUAINC} -I${CURLINC} -Os -shared -o $@ $< -Wl,--gc-sections -Wl,-s -static-libgcc ${LUALIB} ${CURLLIB}

clean:
	rm -f simplehttp.so simplehttp.dll
