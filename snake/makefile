BUILDDIR=build
SRCDIR=src
INCLUDEDIR=include
CC=gcc
CFLAGS=
ifeq ($(OS), Windows_NT)
	CC = x86_64-w64-mingw32-gcc.exe
	CYGWIN_INCLUDE="C:/cygwin64/usr/include"
	CYGWIN_LIB="C:/cygwin64/lib"
	MINGW_INCLUDE="C:/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/include"
	MINGW_LIB="C:/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/lib"
	CURL_INCLUDE="C:/curl-8.17.0_4-win64-mingw/include"
	CURL_LIB="C:/curl-8.17.0_4-win64-mingw/lib"
	CYGWIN_W32_LIB="C:/cygwin64/lib/w32api"
	CFLAGS += -DNCURSES_STATIC -DCURL_STATICLIB
endif

DEPSFILE = .deps
SRCS = main.c snake.c menu.c bitpack.c

snake: $(SRCS:%.c=$(BUILDDIR)/%.o) # depends on .o files corresponding to all SRCS
	$(CC) $(CFLAGS) $(BUILDDIR)/*.o -o snake -lncurses -lcurl -ljson-c -L../common/lib -lleaderboard

# generate dependency rules using gcc -MM: code.o depends on code.c and any non-system #includes in code.c
$(DEPSFILE): $(SRCDIR)/*.c
	rm -f $(DEPSFILE)
	$(CC) -MM -I$(INCLUDEDIR) $^ $(CFLAGS) > $(DEPSFILE)
include $(DEPSFILE)

# standard obj rules: requirements from DEPSFILE are merged
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -c $< -o $@

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/*.ostat snake *.exe $(DEPSFILE)

# windows static compilation nonsense
$(BUILDDIR)/%.ostat: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(CURL_INCLUDE) -c $< -o $@

winstatic: $(SRCS:%.c=$(BUILDDIR)/%.ostat)
	#$(CC) $(CFLAGS) -static -o snake.exe *.ostat -L$(CYGWIN_LIB) -ljson-c -lcurl -lncurses
	$(CC) $(CFLAGS) -I$(CURL_INCLUDE) -static -o snake.exe $(BUILDDIR)/*.ostat -L$(CURL_LIB) -lcurl -lpsl -lbrotlidec -lbrotlicommon -lnghttp2 -lnghttp3 -lngtcp2 -lngtcp2_crypto_libressl -lssh2 -lssl -lcrypto -lz -lzstd -L$(MINGW_LIB) -ljson-c -lws2_32 -lwldap32 -L$(CYGWIN_LIB) -lncurses -L$(CYGWIN_W32_LIB) -lsecur32 -lcrypt32 -lbcrypt -liphlpapi

