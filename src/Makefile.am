## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS = \
	 -Wall \
	 -O3 \
	 -g

bin_PROGRAMS = bspd

bspd_SOURCES = \
	serialization/json.c \
	protocol/internal.c \
	protocol/normal.c \
	protocol/http.c \
	protocol/websocket.c \
	protocol/protocol.h \
	utils/debug.c \
	utils/utils.h \
	server/main.c \
	server/conf.c \
	server/script.c \
	server/clients.c \
	server/server.h \
	wrapper/standard.c \
	wrapper/misc.c \
	wrapper/wrapper.h \
	bspd.c \
	bspd.h

bspd_LDFLAGS = 

bspd_LDADD = \
	-L../deps/zlib/ -lbspd-zlib \
	-L../deps/lz4/ -lbspd-lz4 \
	-L../deps/snappy/ -lbspd-snappy \
	-L../deps/sqlite/ -lbspd-sqlite \
	-L../deps/mariadb-client/ -lbspd-mariadb \
	-L../deps/hiredis/ -lbspd-hiredis \
	-L../deps/lua/ -lbspd-lua
