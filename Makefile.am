INCLUDES = -I$(srcdir) -I$(srcdir)/imagewin -I$(srcdir)/shapes \
	-I$(srcdir)/server -I$(srcdir)/data \
	-I$(srcdir)/objs -I$(srcdir)/conf -I$(srcdir)/files -I$(srcdir)/gumps \
	-I$(srcdir)/audio -I$(srcdir)/pathfinder -I$(srcdir)/usecode \
	$(SDL_CFLAGS) $(INCDIRS) $(WINDOWING_SYSTEM) \
	$(DEBUG_FLAGS) $(CPPFLAGS) -DEXULT_DATADIR=\"$(EXULT_DATADIR)\"

CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(DEBUG_FLAGS) $(CXXFLAGS) $(LDFLAGS) -o $(EXE_TARGET)

SUBDIRS =  files tools data conf flic audio pathfinder gumps docs \
	desktop shapes imagewin usecode objs server mapedit

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
	bggame.h	\
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
	exult.h		\
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
	shapeid.cc	\
	shapeid.h	\
	sigame.cc	\
	sigame.h	\
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
	imagewin/libimagewin.la	\
	usecode/libusecode.la	\
	shapes/libshapes.la \
	objs/libobjs.la \
	gumps/libgumps.la \
	server/libserver.la \
	files/libu7file.la

exult_LDADD = 			\
	$(EXULTLIBS) \
	$(SDL_LIBS) $(KMID_LIBS) $(SYSLIBS) $(x_libraries) $(ICON_FILE)

exult_DEPENDENCIES = $(ICON_FILE) $(EXULTLIBS)

EXTRA_DIST = 	\
	FAQ \
	README.win32 \
	README.Alpha \
	README.BeOS \
	README.MacOS \
	Makefile.win32 \
	Makefile.be \
	Makefile.cygwin \
	alpha_kludges.h mingw_kludges.h \
	exult.spec \
	win32/exult.ico \
	win32/exultico.rc \
	beos/exult.rsrc \
	mac/exult_icon.rsrc.bin \
	mac/ExultMac.mcp.xml.sit \
	mac/exult_version.rsrc.bin \
	mac/mac_autoconfig.h \
	mac/mac_prefix.h \
	macosx/exult.icns \
	debian/changelog \
	debian/control \
	debian/copyright \
	debian/dirs \
	debian/README.debian \
	debian/rules

win32/exultico.o: $(srcdir)/win32/exultico.rc $(srcdir)/win32/exult.ico
	-mkdir win32
	cp $(srcdir)/win32/exult.ico win32
	windres $(srcdir)/win32/exultico.rc win32/exultico.o

CLEANFILES = *~

wrapper: ${EXE_TARGET}
	mkdir -p ${PACKAGE}.app/Contents/MacOS
	mkdir -p ${PACKAGE}.app/Contents/Resources
	echo "APPL${SDL_APPLE_CREATOR}" > ${PACKAGE}.app/Contents/PkgInfo
	cp Info.plist ${PACKAGE}.app/Contents/
	cp -r SDLMain.nib ${PACKAGE}.app/Contents/Resources/
	cp ${EXE_TARGET} ${PACKAGE}.app/Contents/MacOS/
	cp macosx/exult.icns ${PACKAGE}.app/Contents/Resources/



