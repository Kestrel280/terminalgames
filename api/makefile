CFLAGS = -Wall
DEPSFILE = .deps
SRCS = main.c server.c

all: $(SRCS:%.c=%.o)
	gcc *.o -o api -L/usr/local/lib/ -lmicrohttpd -lsqlite3 -ljson-c $(CFLAGS)

$(DEPSFILE): $(SRCS)
	rm -f $(DEPSFILE)
	gcc -MM $^ $(CFLAGS) > $(DEPSFILE)
include $(DEPSFILE)

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(DEPSFILE) *.o api
