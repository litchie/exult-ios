INCLUDES = -I.. -I../shapes -I../imagewin -I../files \
	$(GTK_INCLUDES) $(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

if GTK_INTERFACE
GTK_APPS = shapetest
else
GTK_APPS = 
endif

shapetest_SOURCES = shapelst.cc shapelst.h

bin_PROGRAMS = $(GTK_APPS)

shapetest_LDADD = 			\
	../files/libu7file.la	\
	../imagewin/libimagewin.la	\
	../shapes/libshapes.la \
	$(GTK_LIBS) $(SYSLIBS) $(x_libraries)

noinst_LTLIBRARIES = libmapedit.la

libmapedit_la_SOURCES = \
	shapelst.cc \
	shapelst.h





