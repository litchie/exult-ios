
OBJECTS = actors.o imagewin.o objs.o exult.o gamewin.o \
	vgafile.o utils.o readnpcs.o gamedat.o usecode.o \
	imagetxt.o text.o items.o npctest.o npc/libnpc.a script/libscr.a

ifdef DOS
CXX = dos-g++
CXXFLAGS =
CPPFLAGS = -DDOS -I/usr/local/include -Inpc -Iscript
LFLAGS =
LIBS =  -L npc -L script -lscr -lnpc -lttf -lstdcx

else				# X-windows.
CXX = g++
CXXFLAGS = -g
CPPFLAGS = -DXWIN -Inpc -Iscript
ifdef RELEASE
STATIC = -static
else
STATIC =
endif
LFLAGS = -g
LIBS = -L npc -L script -L /usr/X11R6/lib -L /usr/local/lib \
	-lscr -lnpc -lX11 -lXext -lttf
endif

all:
	(cd npc; make libnpc.a)
	(cd script; make libscr.a)
	make exult

exult: $(OBJECTS)
	$(CXX) $(LFLAGS) -o $@ $(OBJECTS) $(STATIC) $(LIBS)
#	$(CXX) $(LFLAGS) -o $@ $(OBJECTS) -static $(LIBS)

exult.exe:
	make DOS=1

release:
	make clean
#	make DOS=1
#	make clean
	make RELEASE=1
	make tar
	make w32zip
#	make doszip

tar:	exult
	strip exult
	(cd ..; tar cvfz exult09.tgz exult/{*.h,*.cc,makefile,README,README.dos,CHANGELOG,exult,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

w32zip:
	(cd ..; zip exult09w..zip exult/{*.h,*.cc,makefile,exult.ide,README,readme.w95,CHANGELOG,exultw32.exe,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

doszip:	exult.exe
	(cd ..; zip exult06.zip exult/{exult.exe,avatar.ttf,*.scr,csdpmi3b.zip,README,CHANGELOG})

srczip:
	(cd ..; zip exult09.zip exult/{*.h,*.cc,makefile,README,README.dos,CHANGELOG,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

clean:
	(cd npc; make clean)
	(cd script; make clean)
	-rm *.o exult

imagewin.o:	imagewin.cc imagewin.h
objs.o:		objs.cc objs.h
exult.o:	exult.cc imagewin.h
gamewin.o:	gamewin.h imagewin.h actors.h objs.h vgafile.h
actors.o:	actors.h objs.h
text.o:		text.h
imagetxt.o:	text.h imagewin.h
vgafile.o:	vgafile.h utils.h objs.h
readnpcs.o:	objs.h gamewin.h

#shmtest: shmtest.c
#	gcc -g -o $@ $< -L /usr/X11R6/lib -lX11 -lXext
#
#dgatest: dgatest.c
#	gcc -g -o $@ $< -L /usr/X11R6/lib -lXxf86dga -lX11 -lXext -lm

