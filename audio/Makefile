# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/exult
pkglibdir = $(libdir)/exult
pkgincludedir = $(includedir)/exult

top_builddir = ..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = i586-pc-linux-gnu
host_triplet = i586-pc-linux-gnu
AS = @AS@
CC = gcc
CPP = gcc -E
CXX = g++
DLLTOOL = @DLLTOOL@
EXEEXT = 
FREETYPE_DIR = /usr/include/freetype
HAVE_SDL_CONFIG = yes
KMID_INCLUDES = -I/usr/local/include/libkmid
KMID_LIBS = -L/usr/local/lib -lkmid
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBTOOL_DEPS = ./ltconfig ./ltmain.sh
LN_S = ln -s
MAKEINFO = makeinfo
OBJDUMP = @OBJDUMP@
OBJEXT = o
PACKAGE = exult
RANLIB = ranlib
RELNUM = 12
SDL_INCLUDES = -I/usr/include/SDL -D_REENTRANT
SDL_LIBS = -L/usr/lib -lSDL -lpthread
STRIP = strip
VERSION = $MAJOR.$RELNUM
WINDOWING_SYSTEM = -DXWIN

AUTOMAKE_OPTIONS = foreign
INCLUDES =  $(SDL_INCLUDES) $(KMID_INCLUDES)

CXXFLAGS =  $(INCDIRS) -Wall 

#LIBS +=  -L/usr/X11R6/lib/ -lc -lstdc++ -lgcc -lm -lXi -lXext -lX11 

OFILES = Mixer.o Midi.o Audio.o Flex.o IFF.o SDL_mapping.o
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../autoconfig.h
CONFIG_CLEAN_FILES = 
DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --foreign audio/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = audio

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --foreign audio/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am:
uninstall: uninstall-am
all-am: Makefile all-local
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am
	-rm -f libtool

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: tags distdir info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-local all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


all-local:	libaudio.a 

libaudio.a:	$(OFILES)
	ar rs libaudio.a $(OFILES)

clean:
	rm -f libaudio.a core $(OFILES)

tar:
	rm -rf /tmp/audio
	mkdir /tmp/audio
	cp Audio.cc Audio.h Flex.cc Flex.h Midi.cc Midi.h Mixer.cc Mixer.h SDL_mapping.h Makefile /tmp/audio
	cd /tmp && tar czvpf exult_audio_lib.tar.gz audio

%.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

# Dependencies

Audio.o: Audio.cc Audio.h ../autoconfig.h SDL_mapping.h Flex.h common.h Mixer.h Midi.h
Flex.o: Flex.cc Flex.h ../autoconfig.h common.h
IFF.o: IFF.cc IFF.h ../autoconfig.h common.h
Midi.o: Midi.cc Midi.h ../autoconfig.h SDL_mapping.h Flex.h common.h Mixer.h
Mixer.o: Mixer.cc Mixer.h ../autoconfig.h SDL_mapping.h Flex.h common.h Audio.h Midi.h
SDL_mapping.o: SDL_mapping.cc SDL_mapping.h ../autoconfig.h
test.o: test.cc Audio.h ../autoconfig.h SDL_mapping.h Flex.h common.h Mixer.h Midi.h

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
