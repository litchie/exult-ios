INCLUDES = -I.. -I../shapes -I../imagewin -I../files \
	$(INCDIRS) $(WINDOWING_SYSTEM) $(DEBUG_FLAGS) $(CPPFLAGS)

bin_PROGRAMS = shapetest

shapetest_SOURCES = shapelst.cc shapelst.h

shapetest_LDADD = 			\
	../files/libu7file.la	\
	../imagewin/libimagewin.la	\
	../shapes/libshapes.la \
	-lgtk -lgdk -lglib $(SYSLIBS) $(x_libraries)

noinst_LTLIBRARIES = libmapedit.la

libmapedit_la_SOURCES = \
	shapelst.cc \
	shapelst.h \
	u7drag.cc \
	u7drag.h




