CC := gcc
ARGS := $(CC) -Wall -lX11 -lImlib2 -lm -lbsd

default: backman.c utils.c argparser.c imgutils.c
	$(ARGS) -g -O0 -o out backman.c utils.c argparser.c imgutils.c
install:
	install -m755 out ~/.local/bin/backman
make-install: default install
clean:
	rm ~/.local/bin/backman
	rm out
