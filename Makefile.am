INCLUDES = -I$(srcdir) -I$(srcdir)/imagewin -I$(srcdir)/shapes \
	-I$(srcdir)/objs -I$(srcdir)/conf -I$(srcdir)/files -I$(srcdir)/gumps \
	-I$(srcdir)/audio -I$(srcdir)/pathfinder -I$(srcdir)/usecode \
	$(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) \
	$(DEBUG_FLAGS) $(CPPFLAGS) -DEXULT_DATADIR=\""$(datadir)/exult"\"

CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(DEBUG_FLAGS) $(CXXFLAGS) $(LDFLAGS) -o $(EXE_TARGET)

SUBDIRS = audio pathfinder files gumps conf flic tools data docs desktop \
	shapes imagewin usecode objs mapedit

bin_PROGRAMS = exult

exult_SOURCES =	\
	actions.cc	\
	actions.h	\
	actorio.cc	\
	actors.cc	\
	actors.h	\
	alloc.cc	\
	alpha_kludges.h	\
	args.cc		\
	args.h		\
	autoarray.h	\
	bggame.cc	\
	browser.cc	\
	browser.h	\
	cheat.cc	\
	cheat.h		\
	cheat_screen.cc	\
	cheat_screen.h	\
	combat.cc	\
	combat.h	\
	delobjs.cc	\
	delobjs.h	\
	dir.cc		\
	dir.h		\
	drag.cc		\
	effects.cc	\
	effects.h	\
	exceptions.h	\
	exult.cc	\
	exultmenu.cc	\
	exultmenu.h	\
	exult_types.h	\
	exult_constants.h	\
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
	gamma.h		\
	hash_utils.h	\
	keys.cc		\
	keys.h		\
	keyactions.cc	\
	keyactions.h	\
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
	vec.h 		\
	xdrag.cc	\
	xdrag.h

EXULTLIBS = \
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
	gumps/libgumps.la

exult_LDADD = 			\
	$(EXULTLIBS) \
	$(SDL_LIBS) $(KMID_LIBS) $(SYSLIBS) $(x_libraries) $(ICON_FILE)

exult_DEPENDENCIES = $(ICON_FILE) $(EXULTLIBS)

EXTRA_DIST = 	\
	FAQ \
	README.win32 \
	README.Alpha \
	Makefile.win32 \
	Makefile.be \
	exult.spec

exultico.o: win32/exultico.rc win32/exult.ico
	windres $(srcdir)/win32/exultico.rc $(srcdir)/exultico.o

CLEANFILES = *~
