AUTOMAKE_OPTIONS=foreign

SUBDIRS=$(TOP_LEVEL_SUBDIRS)

OBJECTS = args.o actors.o imagewin.o objs.o exult.o gamewin.o drag.o gumps.o \
	actorio.o actions.o gametxt.o \
	vgafile.o segfile.o utils.o readnpcs.o gamedat.o usecode.o npcnear.o \
	tqueue.o gameclk.o mouse.o items.o lists.o vec.o

HFILES = args.h acconfig.h actions.h actors.h autoconfig.h fnames.h gameclk.h \
	gamewin.h gumps.h ready.h tiles.h \
	imagewin.h items.h lists.h mouse.h npcnear.h objs.h segfile.h \
	tqueue.h usecode.h utils.h vec.h vgafile.h

SOURCES = $(HFILES) $(OBJECTS:%.o=%.cc)

EXTRA_DIST = 

SUFFIXES=.cc .h

INCLUDES += $(AUDIO_INCLUDES)
INCLUDES += -Iconf
INCLUDES += -Ipathfinder
INCLUDES += -I.
INCLUDES += $(SDL_INCLUDES)
INCLUDES += $(KMID_INCLUDES)

LIBS += $(SDL_LIBS) -Lconf -Lpathfinder
LIBS += $(KMID_LIBS)
LIBS += $(AUDIO_LIBRARY)

CXXFLAGS += $(PEDANTIC)
CXXFLAGS += -DRELNUM=$(RELNUM)
CXXFLAGS += $(WINDOWING_SYSTEM)
CXXFLAGS += $(DEBUG_FLAGS)

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< $(INCLUDES)



LFLAGS += -g 
LIBS += $(SYSLIBS)
LIBS += $(AUDIO_LIBRARY)

all-local: $(EXE_TARGET)

exult: $(OBJECTS)
	$(CXX) $(LFLAGS) -o $@ $(OBJECTS) $(STATIC) $(LIBS)

exult.exe: $(OBJECTS)
	$(CXX) $(LFLAGS) -o $@ $(OBJECTS) $(STATIC) $(LIBS)

snapshot: dist
	mv exult*.tar.gz exultsnap.tar.gz

snapshotw: exult.exe
	zip exultsnapw.zip exult.exe README.win32

tar:	exult
	strip exult
	(cd ..; tar cvfz exult$(RELNUM).tgz exult/{*.h,*.cc,Makefile,README,README.dos,CHANGELOG,exult,*.scr} exult/npc/{*.h,*.cc,Makefile} exult/script/{*.h,*.cc,*.y,*.l,Makefile})

w32zip:
	zip exult$(RELNUM)w.zip README.win32 exult.exe

srczip:
	(cd ..; zip exult$(RELNUM).zip exult/{*.h,*.cc,Makefile,README,README.dos,CHANGELOG,*.ttf,*.scr} exult/npc/{*.h,*.cc,Makefile} exult/script/{*.h,*.cc,*.y,*.l,Makefile})

clean:
	-rm -f *.o core exult


libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# dependencies generated with:
# makedepend -f Makefile.am -Y -I. -Iconf -Ipathfinder -Iaudio -- *.cc
# DO NOT DELETE

actions.o: actions.h actors.h objs.h vec.h usecode.h tqueue.h utils.h
actors.o: gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h
actors.o: utils.h vgafile.h fnames.h gameclk.h actions.h ready.h
args.o: args.h
drag.o: gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h utils.h
drag.o: vgafile.h fnames.h gameclk.h gumps.h mouse.h
exult.o: gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h
exult.o: utils.h vgafile.h fnames.h gameclk.h audio/Audio.h autoconfig.h
exult.o: audio/SDL_mapping.h audio/Mixer.h audio/Midi.h conf/Configuration.h
exult.o: conf/XMLEntity.h mouse.h gumps.h args.h
gameclk.o: gameclk.h tqueue.h gamewin.h imagewin.h actors.h objs.h vec.h
gameclk.o: usecode.h utils.h vgafile.h fnames.h
gamedat.o: gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h
gamedat.o: utils.h vgafile.h fnames.h gameclk.h
gametxt.o: gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h
gametxt.o: utils.h vgafile.h fnames.h gameclk.h
gamewin.o: gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h
gamewin.o: utils.h vgafile.h fnames.h gameclk.h items.h npcnear.h gumps.h
gamewin.o: segfile.h audio/Audio.h autoconfig.h audio/SDL_mapping.h
gamewin.o: audio/Mixer.h audio/Midi.h
gumps.o: gumps.h objs.h vec.h usecode.h tqueue.h gamewin.h imagewin.h
gumps.o: actors.h utils.h vgafile.h fnames.h gameclk.h
imagetxt.o: imagewin.h text.h
imagewin.o: imagewin.h conf/Configuration.h conf/XMLEntity.h
items.o: items.h utils.h
lists.o: lists.h
mouse.o: mouse.h objs.h vec.h usecode.h tqueue.h vgafile.h fnames.h
mouse.o: imagewin.h gamewin.h actors.h utils.h gameclk.h
npcnear.o: npcnear.h tqueue.h gamewin.h imagewin.h actors.h objs.h vec.h
npcnear.o: usecode.h utils.h vgafile.h fnames.h gameclk.h
npctest.o: actors.h objs.h vec.h usecode.h tqueue.h utils.h
objs.o: objs.h vec.h usecode.h tqueue.h gamewin.h imagewin.h actors.h utils.h
objs.o: vgafile.h fnames.h gameclk.h audio/Audio.h autoconfig.h
objs.o: audio/SDL_mapping.h audio/Mixer.h audio/Midi.h
readnpcs.o: gamewin.h imagewin.h actors.h objs.h vec.h usecode.h tqueue.h
readnpcs.o: utils.h vgafile.h fnames.h gameclk.h
segfile.o: segfile.h utils.h
text.o: text.h
tqueue.o: tqueue.h
usecode.o: utils.h usecode.h gamewin.h imagewin.h actors.h objs.h vec.h
usecode.o: tqueue.h vgafile.h fnames.h gameclk.h gumps.h mouse.h
usecode.o: pathfinder/Astar.h pathfinder/PathFinder.h audio/Audio.h
usecode.o: autoconfig.h audio/SDL_mapping.h audio/Mixer.h audio/Midi.h
vec.o: vec.h
vgafile.o: vgafile.h fnames.h utils.h objs.h vec.h usecode.h tqueue.h
vgafile.o: imagewin.h
