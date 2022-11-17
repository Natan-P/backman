CC := gcc
ARGS := $(CC) -Wall -lX11 -lImlib2 -lm

install: backman.c utils.c argparser.c imgutils.c
	$(ARGS) -g -O0 -o out.o backman.c utils.c argparser.c imgutils.c
optimize: backman.c utils.c argparser.c imgutils.c
	$(ARGS) -O3 -o out.o backman.c utils.c argparser.c imgutils.c
