CC=gcc
CFLAGS=
BUILDDIR=build
SRCDIR=src
LIBDIR=lib
INCLUDEDIR=include
DEPSFILE=.deps

SRCS=leaderboard.c bitpack.c
LIBS=leaderboard.a bitpack.a


all: $(LIBS:%.a=$(LIBDIR)/lib%.a)

#.PHONY: leaderboard bitpack
#leaderboard: leaderboard.a
#bitpack: bitpack.a

$(LIBDIR)/lib%.a: $(BUILDDIR)/%.o
	ar rcs $@ $^

# leaderboard.a: $(BUILDDIR)/leaderboard.o
# 	ar rcs $(LIBDIR)/lib$@ $^
# 
# bitpack.a: $(BUILDDIR)/bitpack.o
# 	ar rcs $(LIBDIR)/lib$@ $^

$(DEPSFILE): $(SRCDIR)/*.c
	rm -f $(DEPSFILE)
	$(CC) -MM -I$(INCLUDEDIR) $(SRCDIR)/* -o $(DEPSFILE)
include $(DEPSFILE)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDEDIR) -c $< -o $@

clean:
	rm -f $(LIBDIR)/*.a $(BUILDDIR)/*.o $(DEPSFILE)
