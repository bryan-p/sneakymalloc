C = gcc
FLAGS = -Wall -g 

all: malloc 

malloc: sneakymalloc.c
	$(C) $(FLAGS) -fpic -shared -o sneakymalloc.so sneakymalloc.c -ldl 

clean:
	rm sneakymalloc.so
