INCLUDES = -I. -Iconf -Ifiles -Iaudio -Ipathfinder \
	$(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) \
	$(DEBUG_FLAGS) $(CPPFLAGS)

CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $(EXE_TARGET)

SUBDIRS = audio conf pathfinder files flic tools

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
	bodies.cc	\
	bodies.h	\
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
	gameclk.cc	\
	gameclk.h	\
	gamedat.cc	\
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
	mouse.cc	\
	mouse.h		\
	npcnear.cc	\
	npcnear.h	\
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
	spells.cc	\
	spells.h	\
	tiles.h		\
	titles.cc	\
	titles.h	\
	tqueue.cc	\
	tqueue.h	\
	usecode.cc	\
	usecode.h	\
	useval.cc	\
	useval.h	\
	vec.cc		\
	vec.h		\
	vgafile.cc	\
	vgafile.h

exult_LDADD = 			\
	pathfinder/libpathfinder.la		\
	audio/libaudio.la	\
	audio/midi_drivers/libmididrv.la	\
	flic/libflic.la		\
	conf/libconf.la		\
	files/libu7file.la	\
	$(SDL_LIBS) $(KMID_LIBS) $(SYSLIBS) $(x_libraries)

EXTRA_DIST = 	\
	README.win32 \
	Makefile.win32 \
	Makefile.be \
	exult.spec
