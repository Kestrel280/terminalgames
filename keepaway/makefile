all: keepaway

%.o: %.c
	gcc -Wall -c $< -o $@

%.odbg: %.c
	gcc -Wall -DDEBUG -c $< -o $@

debug: main.odbg mainmenu.odbg keepaway.odbg
	gcc -Wall *.odbg -o $@ -lncurses

keepaway: main.o mainmenu.o keepaway.o
	gcc -Wall *.o -o $@ -lncurses

clean:
	rm -f *.o *.odbg keepaway debug
