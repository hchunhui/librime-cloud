CC = gcc
CC_mingw = i686-w64-mingw32-gcc
CC_mingw64 = x86_64-w64-mingw32-gcc

LUAV ?= 5.4
LUAINC = thirdparty/lua${LUAV}
LUALIB = -Llibrime-lua${LUAV}/dist/lib -lrime
LUALIB64 = -Llibrime-lua${LUAV}-x64/dist/lib -lrime

CURLV ?= 8.11.1
CURLINC = curl-${CURLV}/include
CURLLIB = -Lcurl-${CURLV}/build32/lib/.libs -lcurl -lws2_32 -lcrypt32 -lbcrypt
CURLLIB64 = -Lcurl-${CURLV}/build64/lib/.libs -lcurl -lws2_32 -lcrypt32 -lbcrypt

PLAT ?= linux
LDFLAGS_linux = -Wl,--gc-sections -Wl,-s
LDFLAGS_macos = -Wl,-bundle -Wl,-undefined,dynamic_lookup
LDFLAGS = ${LDFLAGS_${PLAT}}

all: simplehttp.so simplehttp.dll simplehttpx64.dll

librime-lua5.4:
	curl -L -O https://github.com/rime/librime/releases/download/1.7.3/rime-with-plugins-1.7.3-win32.zip
	unzip rime-with-plugins-1.7.3-win32.zip -d librime-lua5.4

librime-lua5.4-x64:
	curl -L -O https://github.com/rime/librime/releases/download/1.11.0/rime-76a0a16-Windows-msvc-x64.7z
	7z x -olibrime-lua5.4-x64 rime-76a0a16-Windows-msvc-x64.7z

thirdparty:
	git clone -b thirdparty http://github.com/hchunhui/librime-lua.git thirdparty

curl-${CURLV}:
	curl -L -O https://curl.se/download/curl-${CURLV}.tar.gz
	tar xvf curl-${CURLV}.tar.gz

prepare: librime-lua5.4 librime-lua5.4-x64 thirdparty curl-${CURLV}

curl-${CURLV}/build32/lib/.libs:
	cd curl-${CURLV} && sh ../tinycurl.sh

curl-${CURLV}/build64/lib/.libs:
	cd curl-${CURLV} && sh ../tinycurlx64.sh

simplehttp.so: main.c thirdparty curl-${CURLV}
	${CC} -I${LUAINC} -I${CURLINC} -Os -fPIC -shared ${LDFLAGS} -o $@ $<

simplehttp.dll: main.c librime-lua${LUAV} thirdparty curl-${CURLV}/build32/lib/.libs
	${CC_mingw} -I${LUAINC} -I${CURLINC} -Os -shared -o $@ $< -Wl,--gc-sections -Wl,-s -static-libgcc ${LUALIB} ${CURLLIB}

simplehttpx64.dll: main.c librime-lua${LUAV}-x64 thirdparty curl-${CURLV}/build64/lib/.libs
	${CC_mingw64} -I${LUAINC} -I${CURLINC} -Os -shared -o $@ $< -Wl,--gc-sections -Wl,-s -static-libgcc ${LUALIB64} ${CURLLIB64}

clean:
	rm -f simplehttp.so simplehttp.dll simplehttpx64.dll
