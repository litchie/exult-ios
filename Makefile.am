INCLUDES = -I. -Iconf -Ifiles -Iaudio -Ipathfinder \
	$(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) \
	$(DEBUG_FLAGS) $(CPPFLAGS) -DEXULT_DATADIR=\""$(datadir)/exult"\"

CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(DEBUG_FLAGS) $(CXXFLAGS) $(LDFLAGS) -o $(EXE_TARGET)

SUBDIRS = audio pathfinder files conf flic tools data docs

bin_PROGRAMS = exult

exult_SOURCES =	\
	actions.cc	\
	actions.h	\
	actorio.cc	\
	actors.cc	\
	actors.h	\
	alloc.cc	\
	animate.cc	\
	animate.h	\
	args.cc		\
	args.h		\
	autoarray.h	\
	barge.cc	\
	barge.h		\
	bggame.cc	\
	bodies.cc	\
	bodies.h	\
	browser.cc	\
	browser.h	\
	cheat.cc	\
	cheat.h		\
	citerate.h	\
	combat.cc	\
	combat.h	\
	delobjs.cc	\
	delobjs.h	\
	dir.cc		\
	dir.h		\
	drag.cc		\
	effects.cc	\
	effects.h	\
	egg.cc		\
	egg.h		\
	exult.cc	\
	fnames.h	\
	font.cc		\
	font.h		\
	game.cc		\
	game.h		\
	gameclk.cc	\
	gameclk.h	\
	gamedat.cc	\
	gamerend.cc	\
	gametxt.cc	\
	gamewin.cc	\
	gamewin.h	\
	gumps.cc	\
	gumps.h		\
	imagescl.cc	\
	imagewin.cc	\
	imagewin.h	\
	items.cc	\
	items.h		\
	lists.cc	\
	lists.h		\
	menulist.cc	\
	menulist.h	\
	mouse.cc	\
	mouse.h		\
	npcnear.cc	\
	npcnear.h	\
	npctime.cc	\
	npctime.h	\
	objiter.h	\
	objs.cc		\
	objs.h		\
	palette.cc	\
	palette.h	\
	paths.cc	\
	paths.h		\
	readnpcs.cc	\
	ready.h		\
	rect.h		\
	scale.cc	\
	scale.h		\
	schedule.cc	\
	schedule.h	\
	segfile.cc	\
	segfile.h	\
	shapeid.h	\
	sigame.cc	\
	spells.cc	\
	spells.h	\
	tiles.h		\
	tqueue.cc	\
	tqueue.h	\
	txtscroll.cc	\
	txtscroll.h	\
	usecode.cc	\
	usecode.h	\
	useval.cc	\
	useval.h	\
	vec.h		\
	vgafile.cc	\
	vgafile.h	\
	virstone.cc	\
	virstone.h

exult_LDADD = 			\
	pathfinder/libpathfinder.la		\
	audio/libaudio.la	\
	audio/midi_drivers/libmididrv.la	\
	flic/libflic.la		\
	conf/libconf.la		\
	files/libu7file.la	\
	$(SDL_LIBS) $(KMID_LIBS) $(SYSLIBS) $(x_libraries) $(ICON_FILE)

exult_DEPENDENCIES = $(ICON_FILE)

EXTRA_DIST = 	\
	README.win32 \
	Makefile.win32 \
	Makefile.be \
	exult.spec

exultico.o: win32/exultico.rc win32/exult.ico
	windres win32/exultico.rc exultico.o
