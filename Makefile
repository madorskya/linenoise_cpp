CC=g++

all: first

first: linenoise.h linenoise.cpp

first: linenoise.cpp first.cpp
	$(CC) -Wall -W  -ggdb -o first linenoise.cpp first.cpp -lboost_regex-mt

clean:
	rm -f first
