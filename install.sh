#!/bin/bash

gcc -O3 -Wall -lX11 -lImlib2 -g -o out backman.c argparser.c utils.c;
cwd=$(pwd);
sudo ln "$cwd/out" /bin/backman
echo 'Installation done!'