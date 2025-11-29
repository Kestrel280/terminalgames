CFLAGS = 
ifeq ($(OS), Windows_NT)
	CFLAGS += -DNCURSES_STATIC
	#CFLAGS += -Incurses
endif

DEPSFILE = .deps
SRCS = main.c snake.c leaderboard.c menu.c bitpack.c
CFLAGS =

snake: $(SRCS:%.c=%.o) # depends on .o files corresponding to all SRCS
	gcc *.o -o snake -lncurses -lcurl $(CFLAGS)

# generate dependency rules using gcc -MM: code.o depends on code.c and any non-system #includes in code.c
$(DEPSFILE): $(SRCS)
	rm -f $(DEPSFILE)
	gcc -MM $^ $(CFLAGS) > $(DEPSFILE)
include $(DEPSFILE)

# standard obj rules as well: conflicts are just merged
%.o: %.c
	gcc -c $< -o $@


clean:
	rm -f *.o snake *.exe $(DEPSFILE)
