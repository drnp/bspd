# LUA 5.2 ports for BS.Play

lib_LTLIBRARIES = libbspd-lua.la

libbspd_lua_la_SOURCES = \
	llimits.h \
	lua.h \
	luaconf.h \
	lualib.h \
	lapi.h \
	lapi.c \
	lauxlib.h \
	lauxlib.c \
	lbaselib.c \
	lbitlib.c \
	lcode.h \
	lcode.c \
	lcorolib.c \
	lctype.h \
	lctype.c \
	ldblib.c \
	ldebug.h \
	ldebug.c \
	ldo.h \
	ldo.c \
	ldump.c \
	lfunc.h \
	lfunc.c \
	lgc.h \
	lgc.c \
	linit.c \
	liolib.c \
	llex.h \
	llex.c \
	lmathlib.c \
	lmem.h \
	lmem.c \
	loadlib.c \
	lobject.h \
	lobject.c \
	lopcodes.h \
	lopcodes.c \
	loslib.c \
	lparser.h \
	lparser.c \
	lstate.h \
	lstate.c \
	lstring.h \
	lstring.c \
	lstrlib.c \
	ltable.h \
	ltable.c \
	ltablib.c \
	ltm.h \
	ltm.c \
	lundump.h \
	lundump.c \
	lvm.h \
	lvm.c \
	lzio.h \
	lzio.c

libbspd_lua_la_CFLAGS = -DLUA_COMPAT_ALL -DLUA_USE_LINUX
libbspd_lua_la_LDFLAGS = $(AM_LDFLAGS) -avoid-version

