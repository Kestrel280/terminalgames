CFLAGS = 
ifeq ($(OS), Windows_NT)
	CFLAGS += -DNCURSES_STATIC
endif

all: keepaway

%.o: %.c
	gcc -Wall -c $< -o $@

%.odbg: %.c
	gcc -Wall -DDEBUG -c $< -o $@

debug: main.odbg mainmenu.odbg keepaway.odbg
	gcc -Wall *.odbg -o $@ -lncurses $(CFLAGS)

keepaway: main.o mainmenu.o keepaway.o
	gcc -Wall *.o -o $@ -lncurses $(CFLAGS)

clean:
	rm -f *.o *.odbg *.exe keepaway debug
