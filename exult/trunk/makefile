
OBJECTS = actors.o imagewin.o objs.o exult.o gamewin.o drag.o \
	gumps.o \
	vgafile.o segfile.o utils.o readnpcs.o gamedat.o usecode.o npcnear.o \
	tqueue.o gameclk.o imagetxt.o text.o items.o lists.o vec.o

RELNUM = 12

				# X-windows.
CXX = g++
CXXFLAGS = -g
CPPFLAGS = -DXWIN -D_REENTRANT -DRELNUM=$(RELNUM) -Iaudio -I/usr/local/include/SDL
ifdef RELEASE
# STATIC = -static
STATIC =
else
STATIC =
endif
LFLAGS = -g -L /usr/X11R6/lib -L /usr/local/lib -L audio
LIBS = -laudio -lttf -lSDL -lX11 -lXext -lstdc++ -lgcc -lc -lgcc -lpthread

all:
	make exult

exult: $(OBJECTS)
	$(CXX) $(LFLAGS) -o $@ $(OBJECTS) $(STATIC) $(LIBS)

exult.exe:
	make DOS=1

release:
	make clean
#	make DOS=1
#	make clean
	make RELEASE=1
	make tar
#	make w32zip
#	make doszip

tar:	exult
	strip exult
	(cd ..; tar cvfz exult$(RELNUM).tgz exult/{*.h,*.cc,makefile,README,README.dos,CHANGELOG,exult,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

w32zip:
	(cd ..; zip exult$(RELNUM)w.zip exult/{*.h,*.cc,makefile,exult.ide,README,readme.w95,CHANGELOG,exultw32.exe,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

doszip:	exult.exe
	(cd ..; zip exult06.zip exult/{exult.exe,avatar.ttf,*.scr,csdpmi3b.zip,README,CHANGELOG})

srczip:
	(cd ..; zip exult$(RELNUM).zip exult/{*.h,*.cc,makefile,README,README.dos,CHANGELOG,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

clean:
	(cd npc; make clean)
	(cd script; make clean)
	-rm *.o exult

imagewin.o:	imagewin.cc imagewin.h
objs.o:		objs.cc objs.h
exult.o:	exult.cc imagewin.h gamewin.h
gamewin.o:	gamewin.h imagewin.h actors.h objs.h vgafile.h
actors.o:	actors.h objs.h
text.o:		text.h
imagetxt.o:	text.h imagewin.h
vgafile.o:	vgafile.h utils.h objs.h
readnpcs.o:	objs.h gamewin.h
drag.o:		objs.h gamewin.h

#shmtest: shmtest.c
#	gcc -g -o $@ $< -L /usr/X11R6/lib -lX11 -lXext
#
#dgatest: dgatest.c
#	gcc -g -o $@ $< -L /usr/X11R6/lib -lXxf86dga -lX11 -lXext -lm

