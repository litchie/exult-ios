AUTOMAKE_OPTIONS=foreign

SUBDIRS=audio conf npc script

OBJECTS = actors.o imagewin.o objs.o exult.o gamewin.o drag.o gumps.o \
	gametxt.o \
	vgafile.o segfile.o utils.o readnpcs.o gamedat.o usecode.o npcnear.o \
	tqueue.o gameclk.o mouse.o items.o lists.o vec.o

SUFFIXES=.cc .h

INCLUDES += -I.
INCLUDES += -Iaudio
INCLUDES += -Iconf
INCLUDES += $(SDL_INCLUDES)
INCLUDES += $(KMID_INCLUDES)

LIBS += $(SDL_LIBS) -Laudio -Lconf 
LIBS += $(KMID_LIBS)

CXXFLAGS += -DRELNUM=$(RELNUM)
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< $(INCLUDES)



				# X-windows.
LFLAGS += -g -L /usr/X11R6/lib -L /usr/local/lib
LIBS += -lSDL -lX11 -lXext -lstdc++ -lgcc -lc -lgcc -lpthread -laudio -lconf

all-local: exult

exult: $(OBJECTS) audio/libaudio.a
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
	-rm -f *.o exult



libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
