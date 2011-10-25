# Exult makefile for use in BeOS
# It may require a little tweaking. (paths)

# Base of the exult source
SRC=.

### Modify these paths
SDL_CFLAGS=-I/boot/develop/tools/gnupro/include/SDL
SDL_LIBS=-L/boot/develop/tools/gnupro/lib -lSDLmain -lSDL

CPPFLAGS=-DVERSION=\"$(VERSION)\" -DBEOS -DDEBUG -DEXULT_DATADIR=\"data\" \
	-DHAVE_ZIP_SUPPORT -I$(SRC)/files/zip \
	-I$(SRC)/imagewin -I$(SRC)/shapes -I$(SRC)/data -I$(SRC)/tools \
	-I$(SRC)/files -I$(SRC) -I$(SRC)/audio -I$(SRC)/audio/midi_drivers -I$(SRC)/conf \
	-I$(SRC)/gumps -I$(SRC)/objs -I$(SRC)/pathfinder -I$(SRC)/usecode
	-I$(SRC)/headers -DHAVE_SNPRINTF $(SDL_CFLAGS)
CXXFLAGS=-O2 -Wall

LFLAGS=
LIBS=$(SDL_LIBS) -lmidi -lbe -lz

EXEC=exult

include Makefile.common

beos/exult.rsrc : beos/exult.rsrc
	xres -o $(EXEC) beos/exult.rsrc

finishedbinary: $(EXEC)
	strip $(EXEC)
	xres -o $(EXEC) beos/exult.rsrc

run: $(EXEC)
	./$(EXEC)

bindist: finishedbinary
	rm -rf exult-$(VERSION)
	mkdir exult-$(VERSION)
	mkdir exult-$(VERSION)/data
	mkdir exult-$(VERSION)/doc
	mkdir exult-$(VERSION)/lib
	cp $(EXEC) exult-$(VERSION)
	cp README FAQ README.BeOS COPYING NEWS AUTHORS ChangeLog exult-$(VERSION)/doc
	cp data/exult.flx data/exult_bg.flx data/exult_si.flx exult-$(VERSION)/data
	cp data/bodies.txt data/bodies.txt exult-$(VERSION)/data
	cp $(HOME)/lib/libSDL.so $(HOME)/lib/README-SDL.txt exult-$(VERSION)/lib
	rm -f exult-$(VERSION).x86.beos.zip
	zip -r exult-$(VERSION).x86.beos.zip exult-$(VERSION)
	rm -rf exult-$(VERSION) 
