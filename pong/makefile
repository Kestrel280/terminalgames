default:
	@echo "usage: make normal | make interp"

normal: pong.c pong.h
	gcc -Wall -lncurses pong.c -o pong

interp: pong.c pong.h
	gcc -Wall -DINTERPOLATE -lncurses pong.c -o pong

clean:
	rm pong *.o
