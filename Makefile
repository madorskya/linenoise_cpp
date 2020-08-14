CC=g++

all: first

linenoise_example: linenoise.h linenoise.c
first: linenoise.h linenoise.c

#linenoise_example: linenoise.c example.c
#	$(CC) -Wall -W  -ggdb -o linenoise_example linenoise.c example.c -lboost_regex-mt

first: linenoise.c first.c
	$(CC) -Wall -W  -ggdb -o first linenoise.c first.c -lboost_regex-mt

clean:
	rm -f linenoise_example first
