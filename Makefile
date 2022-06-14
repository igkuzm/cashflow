# File              : Makefile
# Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
# Date              : 06.12.2021
# Last Modified Date: 14.06.2022
# Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>

TARGET=cashflow


all:
	mkdir -p build && cd build && cmake -D${TARGET}_BUILD_TEST="1" .. && make && open ${TARGET}_test.app/Contents/MacOS/${TARGET}_test

clean:
	rm -fr build
