AUTOMAKE_OPTIONS=foreign

SUBDIRS=audio conf

OBJECTS = actors.o imagewin.o objs.o exult.o gamewin.o drag.o gumps.o \
	gametxt.o \
	vgafile.o segfile.o utils.o readnpcs.o gamedat.o usecode.o npcnear.o \
	tqueue.o gameclk.o mouse.o items.o lists.o vec.o

HFILES = acconfig.h actors.h autoconfig.h fnames.h gameclk.h gamewin.h gumps.h \
	imagewin.h items.h lists.h mouse.h npcnear.h objs.h segfile.h \
	tqueue.h usecode.h utils.h vec.h vgafile.h

SOURCES = $(HFILES) $(OBJECTS:%.o=%.cc)

EXTRA_DIST = CHANGELOG

SUFFIXES=.cc .h

INCLUDES += -I.
INCLUDES += -Iaudio
INCLUDES += -Iconf
INCLUDES += $(SDL_INCLUDES)
INCLUDES += $(KMID_INCLUDES)

LIBS += $(SDL_LIBS) -Laudio -Lconf 
LIBS += $(KMID_LIBS)

CXXFLAGS += $(PEDANTIC)
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

tar:	exult
	strip exult
	(cd ..; tar cvfz exult$(RELNUM).tgz exult/{*.h,*.cc,makefile,README,README.dos,CHANGELOG,exult,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

w32zip:
	(cd ..; zip exult$(RELNUM)w.zip exult/{*.h,*.cc,makefile,exult.ide,README,readme.w95,CHANGELOG,exultw32.exe,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

srczip:
	(cd ..; zip exult$(RELNUM).zip exult/{*.h,*.cc,makefile,README,README.dos,CHANGELOG,*.ttf,*.scr} exult/npc/{*.h,*.cc,makefile} exult/script/{*.h,*.cc,*.y,*.l,makefile})

clean:
	-rm -f *.o core exult


libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# dependencies

actors.o: actors.cc gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h gameclk.h
drag.o: drag.cc gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h gameclk.h gumps.h mouse.h
exult.o: exult.cc gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h gameclk.h audio/Audio.h autoconfig.h audio/SDL_mapping.h audio/Flex.h audio/common.h audio/Mixer.h audio/Midi.h audio/Table.h conf/Configuration.h conf/XMLEntity.h mouse.h
gameclk.o: gameclk.cc gameclk.h tqueue.h gamewin.h imagewin.h actors.h objs.h vec.h usecode.h vgafile.h fnames.h
gamedat.o: gamedat.cc gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h gameclk.h utils.h
gametxt.o: gametxt.cc gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h gameclk.h
gamewin.o: gamewin.cc gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h gameclk.h items.h utils.h npcnear.h gumps.h audio/Audio.h autoconfig.h audio/SDL_mapping.h audio/Flex.h audio/common.h audio/Mixer.h audio/Midi.h audio/Table.h conf/Configuration.h conf/XMLEntity.h
gumps.o: gumps.cc gumps.h objs.h vec.h usecode.h tqueue.h gamewin.h imagewin.h actors.h vgafile.h fnames.h gameclk.h
imagewin.o: imagewin.cc imagewin.h conf/Configuration.h conf/XMLEntity.h
items.o: items.cc items.h utils.h
lists.o: lists.cc lists.h
mouse.o: mouse.cc mouse.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h imagewin.h gamewin.h actors.h gameclk.h
npcnear.o: npcnear.cc npcnear.h tqueue.h gamewin.h imagewin.h actors.h objs.h vec.h usecode.h vgafile.h fnames.h gameclk.h
objs.o: objs.cc objs.h vec.h usecode.h tqueue.h gamewin.h imagewin.h actors.h vgafile.h fnames.h gameclk.h audio/Audio.h autoconfig.h audio/SDL_mapping.h audio/Flex.h audio/common.h audio/Mixer.h audio/Midi.h audio/Table.h
readnpcs.o: readnpcs.cc gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h gameclk.h utils.h
segfile.o: segfile.cc segfile.h utils.h
tqueue.o: tqueue.cc tqueue.h
usecode.o: usecode.cc utils.h usecode.h gamewin.h imagewin.h actors.h objs.h vec.h tqueue.h vgafile.h fnames.h gameclk.h gumps.h mouse.h audio/Audio.h autoconfig.h audio/SDL_mapping.h audio/Flex.h audio/common.h audio/Mixer.h audio/Midi.h audio/Table.h
utils.o: utils.cc
vec.o: vec.cc vec.h
vgafile.o: vgafile.cc vgafile.h fnames.h utils.h objs.h vec.h usecode.h tqueue.h
