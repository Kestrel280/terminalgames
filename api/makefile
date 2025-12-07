CC=gcc
BUILDDIR=build
SRCDIR=src
INCLUDEDIR=include
LIBINCLUDEDIR=../common/include
LIBDIR=../common/lib
CFLAGS=-Wall
DEPSFILE=.deps
SRCS=main.c leaderboardapi.c
TARGETNAME=leaderboardapi

$(TARGETNAME): $(SRCS:%.c=$(BUILDDIR)/%.o)
	$(CC) $(CFLAGS) $(BUILDDIR)/*.o -o $(TARGETNAME) -L$(LIBDIR) -lsdserver -lmicrohttpd -lsqlite3 -ljson-c -lsystemd

$(DEPSFILE): $(SRCDIR)/*.c
	rm -f $(DEPSFILE)
	gcc -MM $^ $(CFLAGS) -I$(INCLUDEDIR) -I$(LIBINCLUDEDIR) > $(DEPSFILE)
include $(DEPSFILE)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	gcc $(CFLAGS) -I$(INCLUDEDIR) -I$(LIBINCLUDEDIR) -c $< -o $@

clean:
	rm -f $(DEPSFILE) $(BUILDDIR)/*.o $(TARGETNAME)
