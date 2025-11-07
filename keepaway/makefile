all: keepaway

keepaway: keepaway.o
	gcc -Wall *.o -o $@

%.o: %.c %.h
	gcc -Wall -c $< -o $@

clean:
	rm *.o keepaway
