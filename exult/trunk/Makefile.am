INCLUDES = -I. -Iconf -Ifiles -Iaudio -Ipathfinder \
	$(SDL_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) \
	$(DEBUG_FLAGS) $(CPPFLAGS) \
	-DRELNUM=$(RELNUM)

CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $(EXE_TARGET)

SUBDIRS = audio conf pathfinder files flic

bin_PROGRAMS = exult

exult_SOURCES =	\
	actions.cc	\
	actions.h	\
	actorio.cc	\
	actors.cc	\
	actors.h	\
	args.cc		\
	args.h		\
	delobjs.cc	\
	delobjs.h	\
	drag.cc		\
	effects.cc	\
	effects.h	\
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
	objs.cc		\
	objs.h		\
	readnpcs.cc	\
	ready.h		\
	segfile.cc	\
	segfile.h	\
	tiles.h		\
	tqueue.cc	\
	tqueue.h	\
	usecode.cc	\
	usecode.h	\
	utils.cc	\
	utils.h		\
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
	Makefile.win32
