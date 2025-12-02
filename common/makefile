CC=gcc
CFLAGS=
BUILDDIR=build
SRCDIR=src
LIBDIR=lib
DEPSFILE=.deps

SRCS=leaderboard.c

.PHONY: leaderboard
leaderboard: leaderboard.a

%.a: %.o
	ar rcs $(LIBDIR)/lib$@ $(BUILDDIR)/$^

$(DEPSFILE): $(SRCS:%.c=$(SRCDIR)/%.c)
	$(CC) -MM $(SRCDIR)/* -o $(DEPSFILE)
include $(DEPSFILE)

%.o: $(SRCS:%.c=$(SRCDIR)/%.c)
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)/$@

clean:
	rm $(BUILDDIR)/*.o
