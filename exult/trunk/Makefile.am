INCLUDES = -I. -Iimagewin -Ishapes -Iobjs -Iconf -Ifiles -Igumps -Iaudio -Ipathfinder -Iusecode\
	$(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) \
	$(DEBUG_FLAGS) $(CPPFLAGS) -DEXULT_DATADIR=\""$(datadir)/exult"\"

CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(DEBUG_FLAGS) $(CXXFLAGS) $(LDFLAGS) -o $(EXE_TARGET)

SUBDIRS = audio pathfinder files gumps conf flic tools data docs desktop \
	shapes imagewin usecode objs

bin_PROGRAMS = exult

exult_SOURCES =	\
	actions.cc	\
	actions.h	\
	actorio.cc	\
	actors.cc	\
	actors.h	\
	alloc.cc	\
	args.cc		\
	args.h		\
	autoarray.h	\
	bggame.cc	\
	browser.cc	\
	browser.h	\
	cheat.cc	\
	cheat.h		\
	combat.cc	\
	combat.h	\
	delobjs.cc	\
	delobjs.h	\
	dir.cc		\
	dir.h		\
	drag.cc		\
	effects.cc	\
	effects.h	\
	exult.cc	\
	fnames.h	\
	frameseq.h	\
	game.cc		\
	game.h		\
	gameclk.cc	\
	gameclk.h	\
	gamedat.cc	\
	gamerend.cc	\
	gamewin.cc	\
	gamewin.h	\
	lists.h		\
	menulist.cc	\
	menulist.h	\
	mouse.cc	\
	mouse.h		\
	npcnear.cc	\
	npcnear.h	\
	npctime.cc	\
	npctime.h	\
	palette.cc	\
	palette.h	\
	paths.cc	\
	paths.h		\
	readnpcs.cc	\
	ready.h		\
	rect.h		\
	schedule.cc	\
	schedule.h	\
	segfile.cc	\
	segfile.h	\
	shapeid.h	\
	sigame.cc	\
	tiles.h		\
	tqueue.cc	\
	tqueue.h	\
	txtscroll.cc	\
	txtscroll.h	\
	vec.h

exult_LDADD = 			\
	pathfinder/libpathfinder.la		\
	audio/libaudio.la	\
	audio/midi_drivers/libmididrv.la	\
	flic/libflic.la		\
	conf/libconf.la		\
	files/libu7file.la	\
	imagewin/libimagewin.la	\
	usecode/libusecode.la	\
	shapes/libshapes.la \
	objs/libobjs.la \
	gumps/libgumps.la \
	$(SDL_LIBS) $(KMID_LIBS) $(SYSLIBS) $(x_libraries) $(ICON_FILE)

exult_DEPENDENCIES = $(ICON_FILE)

EXTRA_DIST = 	\
	README.win32 \
	Makefile.win32 \
	Makefile.be \
	exult.spec

exultico.o: win32/exultico.rc win32/exult.ico
	windres win32/exultico.rc exultico.o
