CFLAGS?=-O2 -g -Wall -Werror
OBJS:=main.o

all: chip8

.PHONY: all clean

chip8: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm *.o
	rm chip8