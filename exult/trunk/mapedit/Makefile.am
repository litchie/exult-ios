INCLUDES = -I.. -I../shapes -I../imagewin -I../files \
	$(GTK_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

if GTK_INTERFACE
GTK_APPS = shapetest paltest
GTK_LIBS = -lgtk -lgdk
OUTPUT_LIBS = libmapedit.la
else
GTK_APPS = 
GTK_LIBS =
OUTPUT_LIBS =
endif

shapetest_SOURCES = shapelst.cc shapelst.h

paltest_SOURCES = paledit.cc paledit.h

bin_PROGRAMS = $(GTK_APPS)

shapetest_LDADD = 			\
	../files/libu7file.la	\
	../imagewin/libimagewin.la	\
	../shapes/libshapes.la \
	$(GTK_LIBS) $(SYSLIBS) $(x_libraries)

paltest_LDADD = $(shapetest_LDADD)

noinst_LTLIBRARIES = $(OUTPUT_LIBS)

libmapedit_la_SOURCES = \
	paledit.cc \
	paledit.h \
	shapelst.cc \
	shapelst.h
