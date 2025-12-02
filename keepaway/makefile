CC=gcc
CFLAGS=
SRCDIR=src
BUILDDIR=build
INCLUDEDIR=include
LIBDIR=../common/lib
LIBINCLUDEDIR=../common/include
DEPSFILE=.deps

ifeq ($(OS), Windows_NT)
	CFLAGS += -DNCURSES_STATIC
endif

SRCS=main.c mainmenu.c keepaway.c

keepaway: $(SRCS:%.c=$(BUILDDIR)/%.o)
	$(CC) $(CFLAGS) $(BUILDDIR)/*.o -o $@ -lncurses -lcurl -ljson-c -L$(LIBDIR) -lleaderboard

debug: $(SRCS:%.c=$(BUILDDIR)/%.odbg)
	$(CC) $(CFLAGS) $(BUILDDIR)/*.odbg -o $@ -lncurses $(CFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -I$(LIBINCLUDEDIR) -c $< -o $@

$(BUILDDIR)/%.odbg: %.c
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -I$(LIBINCLUDEDIR) -DDEBUG -c $< -o $@

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/*.odbg *.exe keepaway debug
