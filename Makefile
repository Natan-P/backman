CC := gcc
ARGS := $(CC) -Wall -lX11 -lImlib2

install: backman.c utils.c argparser.c
	$(ARGS)	-g -O0 -o out backman.c utils.c argparser.c
