all: keepaway

keepaway: main.o mainmenu.o keepaway.o
	gcc -Wall *.o -o $@ -lncurses

%.o: %.c %.h
	gcc -Wall -c $< -o $@

clean:
	rm *.o keepaway
