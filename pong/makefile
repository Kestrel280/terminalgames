all: pong

pong: pong.o
	gcc -Wall -lncurses *.o -o pong

%.o: %.c %.h
	gcc -Wall -c $< -o $@

clean:
	rm pong *.o
