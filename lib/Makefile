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

ICONV_V ?= 1.18
LUA_ICONV_V ?= 7.1.0
ICONV_INC = libiconv-${ICONV_V}/build32/include
ICONV_INC64 = libiconv-${ICONV_V}/build64/include
ICONV_LIB = -Llibiconv-${ICONV_V}/build32/lib/.libs -liconv
ICONV_LIB64 = -Llibiconv-${ICONV_V}/build64/lib/.libs -liconv

PLAT ?= linux
LDFLAGS_linux = -Wl,--gc-sections -Wl,-s
LDFLAGS_macos = -Wl,-bundle -Wl,-undefined,dynamic_lookup
LDFLAGS = ${LDFLAGS_${PLAT}}

all: simplehttp.so simplehttp.dll simplehttpx64.dll iconv.so iconv.dll iconvx64.dll

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

libiconv-${ICONV_V}:
	curl -L -O https://ftp.gnu.org/pub/gnu/libiconv/libiconv-${ICONV_V}.tar.gz
	tar xvf libiconv-${ICONV_V}.tar.gz

lua-iconv-${LUA_ICONV_V}:
	git clone -b v${LUA_ICONV_V} https://github.com/lunarmodules/lua-iconv.git lua-iconv-${LUA_ICONV_V}

prepare: librime-lua5.4 librime-lua5.4-x64 thirdparty curl-${CURLV} lua-iconv-${LUA_ICONV_V}

curl-${CURLV}/build32/lib/.libs: curl-${CURLV}
	cd curl-${CURLV} && sh ../tinycurl.sh

curl-${CURLV}/build64/lib/.libs: curl-${CURLV}
	cd curl-${CURLV} && sh ../tinycurlx64.sh

libiconv-${ICONV_V}/build32/lib/.libs: libiconv-${ICONV_V}
	cd libiconv-${ICONV_V} && sh ../iconv.sh

libiconv-${ICONV_V}/build64/lib/.libs: libiconv-${ICONV_V}
	cd libiconv-${ICONV_V} && sh ../iconvx64.sh

simplehttp.so: main.c thirdparty curl-${CURLV}
	${CC} -I${LUAINC} -I${CURLINC} -Os -fPIC -shared ${LDFLAGS} -o $@ $<

simplehttp.dll: main.c librime-lua${LUAV} thirdparty curl-${CURLV}/build32/lib/.libs
	${CC_mingw} -I${LUAINC} -I${CURLINC} -Os -shared -o $@ $< -Wl,--gc-sections -Wl,-s -static-libgcc ${LUALIB} ${CURLLIB}

simplehttpx64.dll: main.c librime-lua${LUAV}-x64 thirdparty curl-${CURLV}/build64/lib/.libs
	${CC_mingw64} -I${LUAINC} -I${CURLINC} -Os -shared -o $@ $< -Wl,--gc-sections -Wl,-s -static-libgcc ${LUALIB64} ${CURLLIB64}

lua-iconv-${LUA_ICONV_V}/luaiconv.c: lua-iconv-${LUA_ICONV_V}

iconv.so: lua-iconv-${LUA_ICONV_V}/luaiconv.c
	${CC} -I${LUAINC} -Os -fPIC -shared ${LDFLAGS} -o $@ $<

iconv.dll: lua-iconv-${LUA_ICONV_V}/luaiconv.c librime-lua${LUAV} libiconv-${ICONV_V}/build32/lib/.libs
	${CC_mingw} -I${LUAINC} -I${ICONV_INC} -Os -shared -o $@ $< -Wl,--gc-sections -Wl,-s -static-libgcc ${LUALIB} ${ICONV_LIB}

iconvx64.dll: lua-iconv-${LUA_ICONV_V}/luaiconv.c librime-lua${LUAV}-x64 libiconv-${ICONV_V}/build64/lib/.libs
	${CC_mingw64} -I${LUAINC} -I${ICONV_INC64} -Os -shared -o $@ $< -Wl,--gc-sections -Wl,-s -static-libgcc ${LUALIB64} ${ICONV_LIB64}

clean:
	rm -f simplehttp.so simplehttp.dll simplehttpx64.dll iconv.so iconv.dll iconvx64.dll

purge: clean
	rm -rf lua-iconv-* curl-* libiconv-* librime-* thirdparty
	rm -rf *.zip *.tar.gz *.7z
