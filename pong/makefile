CFLAGS = 
ifeq ($(OS), Windows_NT)
	CFLAGS += -DNCURSES_STATIC
endif

default:
	@echo "usage: make normal | make interp"

normal: pong.c pong.h
	gcc -Wall pong.c -o pong -lncurses $(CFLAGS)

interp: pong.c pong.h
	gcc -Wall -DINTERPOLATE pong.c -o pong -lncurses $(CFLAGS)

clean:
	rm -f pong *.o *.exe
