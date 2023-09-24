all: linux win32

linux:
	cd lib && make clean simplehttp.so
	rm -rf out-linux && mkdir out-linux && cp lib/simplehttp.so out-linux
	tar czvf linux-`uname -m`-lua5.4.tar.gz out-linux scripts README.md

win32:
	cd lib && make clean simplehttp.dll
	rm -rf out-mingw && mkdir out-mingw && cp lib/simplehttp.dll out-mingw
	zip -r win32-lua5.4.zip out-mingw scripts README.md

macos:
	cd lib && make PLAT=macos clean simplehttp.so
	rm -rf out-macos && mkdir out-macos && cp lib/simplehttp.so out-macos
	tar czvf macos-`uname -m`-lua5.4.tar.gz out-macos scripts README.md

clean:
	rm -rf out-mingw out-linux out-macos linux-*.tar.gz win32-*.zipa macos-*.tar.gz
