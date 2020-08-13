CC=g++

all: linenoise_example first

linenoise_example: linenoise.h linenoise.c
first: linenoise.h linenoise.c

linenoise_example: linenoise.c example.c
	$(CC) -Wall -W -Os -g -o linenoise_example linenoise.c example.c

first: linenoise.c first.c
	$(CC) -Wall -W -Os -g -o first linenoise.c first.c

clean:
	rm -f linenoise_example first
