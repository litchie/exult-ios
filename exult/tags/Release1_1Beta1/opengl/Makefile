CC = g++
CPPFLAGS = -I../shapes `sdl-config --cflags`
CXXFLAGS = -g -Wall
LIBS  = -L../shapes/.libs -lshapes -lpng -lz -lGL -lGLU `sdl-config --libs`

.cc.o:
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $<

all:	gltest1 gltest2 gltest3

gltest1: gltest1.o
	$(CC) $(CXXFLAGS) -o gltest1 $< $(LIBS)

gltest2: gltest2.o
	$(CC) $(CXXFLAGS) -o gltest2 $< $(LIBS)

gltest3: gltest3.o model3ds.o modelgl.o model.o
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBS) -lSDL_image

clean:
	@echo Cleaning up...
	@rm gltest1
	@rm gltest2
	@echo Done.
