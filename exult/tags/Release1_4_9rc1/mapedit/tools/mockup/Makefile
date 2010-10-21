CC=gcc

all: mockup

mockup: main.c main.h defs.h
	${CC} main.c -o mockup `sdl-config --libs --cflags` -lSDL_image -Wall
