C = gcc
FLAGS = -Wall -g 

all: malloc 

malloc: sneakymalloc.c
	$(C) $(FLAGS) -fpic -shared -o sneakymalloc.so sneakymalloc.c -ldl 
	$(C) $(FLAGS) -o attackmalloc attackmalloc.c

clean:
	rm sneakymalloc.so
	rm attackmalloc
