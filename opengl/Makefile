CC = g++ -Wall

all:	gltest1

gltest1: gltest1.cc
	$(CC) gltest1.cc -o gltest1 -lGL -lGLU `sdl-config --cflags --libs`

clean:
	@echo Cleaning up...
	@rm gltest1
	@echo Done.
