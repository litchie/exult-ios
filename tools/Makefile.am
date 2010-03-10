INCLUDES = -I$(srcdir)/../headers -I$(srcdir)/../files -I$(srcdir)/../usecode \
	-I$(srcdir)/.. -I$(srcdir)/../shapes -I$(srcdir)/../imagewin \
	-I$(srcdir)/../conf $(DEBUG_FLAGS) $(CPPFLAGS) $(SDL_CFLAGS) \
	$(GNOMEUI_INCLUDES)

if HAVE_PNG
IPACK = ipack
else
IPACK =
GTHUMB=
endif

if BUILD_GTHUMB
GTHUMB=gnome_shp_thumbnailer
else
GTHUMB=
endif

if BUILD_TOOLS
NOINSTTOOLS = wuc wud mklink rip ucdump cmanip
EXTRATOOLS = splitshp shp2pcx textpack
else
EXTRATOOLS =
NOINSTTOOLS =
endif

bin_PROGRAMS = expack $(EXTRATOOLS) $(IPACK) $(GTHUMB)
noinst_PROGRAMS = $(NOINSTTOOLS)

expack_SOURCES = \
	expack.cc

expack_LDADD = \
	../files/libu7file.la \
	$(SYSLIBS)

ipack_SOURCES = \
	ipack.cc

ipack_LDADD = \
	../files/libu7file.la \
	../shapes/libshapes.la \
	../imagewin/libimagewin.la \
	-lpng -lz $(SYSLIBS)

gnome_shp_thumbnailer_SOURCES = \
	gnome_shp_thumbnailer.cc

gnome_shp_thumbnailer_LDADD = \
	../shapes/libshapes.la \
	../imagewin/libimagewin.la \
	../files/libu7file.la \
	-lpng -lz $(SYSLIBS) $(GNOMEUI_LIBS)

wuc_SOURCES = \
	wuc.c \
	uctools.h

wud_SOURCES = wud.cc

wud_LDADD = \
	$(SYSLIBS) \
	../files/libu7file.la \
	../usecode/libusecode.la

mklink_SOURCES = mklink.c

rip_SOURCES = rip.c

#ucdump_SOURCES = ucdump.c

shp2pcx_SOURCES = shp2pcx.cc

shp2pcx_LDADD = $(SYSLIBS)

splitshp_SOURCES = splitshp.cc

splitshp_LDADD = $(SYSLIBS)

textpack_SOURCES = textpack.cc

textpack_LDADD = \
	../files/libu7file.la \
	$(SYSLIBS)

cmanip_SOURCES = \
	cmanip.cc 

cmanip_LDADD = \
	../conf/libconf.la \
	../files/libu7file.la \
	$(SYSLIBS)

# From gnome-xcf-thumbnailer makefile.
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = gnome_shp_thumbnailer.schemas.in
schemas_DATA = gnome_shp_thumbnailer.schemas

# Rule to make the service file with bindir expanded
$(schemas_DATA): $(schemas_in_files) Makefile
	@$(SED) -e "s|\@bindir\@|$(bindir)|" $< > $@

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p ; \
		done \
	fi; \
	echo "*** MIME types not installed. Run 'cd tools; make desktop-install; cd ..'"; \
	echo "*** (or 'cd tools; make home-desktop-install; cd ..' for installation in home directory)."
else
install-data-local:
endif

if BUILD_GTHUMB
desktop-install:
	if test "$(datadir)" = "/usr/share"; then \
		desktopdir=/usr/share; \
	else \
		desktopdir=/usr/local/share; \
	fi; \
	$(INSTALL_DATA) x-shapefile.xml $(DESTDIR)$$desktopdir/mime/packages; \
	if test -z "$(DESTDIR)"; then \
		echo "Updating mime."; \
		update-mime-database $$desktopdir/mime; \
	else \
		echo "*** Not updating mime database. After install, run:"; \
		echo "*** update-mime-database $$desktopdir/mime"; \
	fi

home-desktop-install:
	desktopdir=$(HOME)/.local/share; \
	$(INSTALL_DATA) x-shapefile.xml $(DESTDIR)$$desktopdir/mime/packages; \
	if test -z "$(DESTDIR)"; then \
		echo "Updating mime."; \
		update-mime-database $$desktopdir/mime; \
	else \
		echo "*** Not updating mime database. After install, run:"; \
		echo "*** update-mime-database $$desktopdir/mime"; \
	fi
else
desktop-install:

home-desktop-install:
endif

EXTRA_DIST = \
	intrins1.txt \
	intrins2.txt \
	README \
	u7bgflag.txt \
	u7siflag.txt \
	ucformat.txt \
	expack.txt

CLEANFILES = *~ gnome_shp_thumbnailer.schemas
