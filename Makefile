CC_mingw=i686-w64-mingw32-gcc
LUAINC=${PWD}/thirdparty/lua5.3
LUALIB="-L${PWD}/librime/dist/lib -lrime"
INSTALL_mingw_DIR=${PWD}/out-mingw/lua
INSTALL_linux_DIR=${PWD}/out-linux/lua

all: linux win32

librime:
	wget https://github.com/rime/librime/releases/download/1.5.3/rime-with-plugins-1.5.3-win32.zip
	unzip rime-with-plugins-1.5.3-win32.zip -d librime

luasocket:
	git clone https://github.com/diegonehab/luasocket.git

thirdparty:
	git clone -b thirdparty http://github.com/hchunhui/librime-lua.git thirdparty

out-mingw: luasocket thirdparty librime
	make CC_mingw=${CC_mingw} LD_mingw=${CC_mingw} LUAINC_mingw=${LUAINC} LUALIB_mingw=${LUALIB} -C luasocket clean mingw
	make SO=dll INSTALL_TOP_CDIR=${INSTALL_mingw_DIR} INSTALL_TOP_LDIR=${INSTALL_mingw_DIR} -C luasocket/src install

out-linux: luasocket thirdparty
	make LUAINC_linux=${LUAINC} -C luasocket clean linux
	make INSTALL_TOP_CDIR=${INSTALL_linux_DIR} INSTALL_TOP_LDIR=${INSTALL_linux_DIR} -C luasocket/src install

linux: out-linux
	tar czvf linux-`uname -m`.tar.gz out-linux scripts

win32: out-mingw
	zip -r win32.zip out-mingw scripts

clean:
	rm -rf out-mingw out-linux linux-*.tar.gz win32.zip
