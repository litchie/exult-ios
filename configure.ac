dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.53)
AC_INIT(exult.cc)


# ---------------------------------------------------------------------
# System/version info
# ---------------------------------------------------------------------

PACKAGE=exult
VER_MAJOR=1
VER_MINOR=1
VER_EXTRA=.0cvs
VERSION=$VER_MAJOR.$VER_MINOR$VER_EXTRA

# check host/target systems
# (build = system we're building on, host = system we're building for,
# target = system the program we're building will build for)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE($PACKAGE,$VERSION,no-define)
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Package Name])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Package Version])
AC_DEFINE_UNQUOTED(VER_MAJOR, "$VER_MAJOR", [Version Number Major])
AC_DEFINE_UNQUOTED(VER_MINOR, "$VER_MINOR", [Version Number Minor])
AC_DEFINE_UNQUOTED(VER_EXTRA, "$VER_EXTRA", [Version Number Extra])

AM_CONFIG_HEADER(config.h)

AH_BOTTOM([
#ifdef ALPHA_LINUX_CXX
#include "alpha_kludges.h"
#endif
 
#define USE_FMOPL_MIDI
])


# ---------------------------------------------------------------------
# Host system settings
# ---------------------------------------------------------------------

AC_EXEEXT

SYSLIBS=""
ICON_FILE=""
EXE_TARGET="exult$EXEEXT"
EXULT_DATADIR="$datadir/exult"
ARCH=""

# determine windowing system from 'host'
AC_MSG_CHECKING("windowing system")
case "$host_os" in
	linux*)
		WINDOWING_SYSTEM="-DXWIN"
		if test "$host_vendor" = "embeddix"; then
			AC_MSG_RESULT(Qtopia (Sharp Zaurus))
			CXXFLAGS="$CXXFLAGS -fsigned-char -DDISABLE_X11 -DQWS -D__zaurus__ -DUSE_FMOPL_MIDI"
			enable_kmid="no"
			enable_timidity="no"
			enable_exult_studio_support="no"
			enable_exult_studio="no"
			enable_gimp_plugin="no"
			enable_debug="no"
			enable_data="yes"
		else
			AC_MSG_RESULT(X11 (GNU/Linux))
		fi
		;;
	beos*)
		WINDOWING_SYSTEM="-DBEOS"
		AC_MSG_RESULT(BeOS)
		SYSLIBS="-lbe -lmidi"
		enable_kmid="no"
		enable_timidity="no"
		;;
	mingw32* )
		WINDOWING_SYSTEM="-DWIN32"
		AC_MSG_RESULT(Win32 (mingw32))
		SYSLIBS="-lwinmm -lstdc++"
		ICON_FILE="win32/exultico.o"
		enable_kmid="no"
		enable_timidity="no"
		;;
	cygwin* )
		WINDOWING_SYSTEM="-DWIN32"
		AC_DEFINE(CYGWIN, 1, [Using Cygwin])
		AC_MSG_RESULT(Win32 (cygwin))
		SYSLIBS="-lwinmm"
		ICON_FILE="win32/exultico.o"
		enable_kmid="no"
		enable_timidity="no"
		;;
	openbsd* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_DEFINE(OPENBSD, 1, [Using OpenBSD])
		AC_MSG_RESULT(X11 (OpenBSD))
		SYSLIBS="-L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga"
		;;
	solaris* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT(X11 (Solaris))
		SYSLIBS="-lsocket -lX11"
		;;
	darwin*)
		dnl We have a problem here: both MacOS X and Darwin report 
		dnl the same signature "powerpc-apple-darwin*" - so we have
		dnl to do more to distinguish them. Plain Darwin will propably
		dnl use X-Windows; and it is of course lacking Cocoa. For
		dnl now I am lazy and do not add proper detection code.
		
		WINDOWING_SYSTEM="-DMACOSX"
		AC_DEFINE(MACOSX, 1, [Using MacOSX])
		AC_MSG_RESULT(Mac OS X)
		SYSLIBS="-framework QuickTime"
		CXXFLAGS="$CXXFLAGS"
		EXULT_DATADIR="data"
		ARCH=macosx
		;;
	*)
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT(not sure... guessing X11)
		;;
esac

AM_CONDITIONAL(MACOSX, test x$ARCH = xmacosx)

# ---------------------------------------------------------------------
# Compilers and other tools
# ---------------------------------------------------------------------

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AM_PROG_LEX
AC_PROG_YACC
AM_CONDITIONAL(LEXYACC, test -n "$YACC")

AC_ARG_WITH(cxx, [  --with-cxx=COMMAND	  Explicitly specify the C++ compiler to use], CXX="")

if test -z "$CXX"; then
	AC_PROG_CXX
else
	AC_PROG_CXX([$CXX])
fi

AM_DISABLE_SHARED

AM_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)


# ---------------------------------------------------------------------
# Compilation environment
# ---------------------------------------------------------------------

AC_DEFUN(AC_CHECK_ALPHA_LINUX_CXX,
[
AC_MSG_CHECKING([for Alpha/Linux with DEC CXX])
AC_TRY_RUN([
	int main(int argc, char **argv)
	{
		#if defined (__DECCXX) && defined(__linux__) && defined(__alpha__)
		return 0;
		#endif
		return 1;
	}], [
		AC_MSG_RESULT([yes])
		AC_DEFINE(ALPHA_LINUX_CXX, 1, [Using Alpha Linux with cxx])
	], AC_MSG_RESULT([no]), AC_MSG_RESULT([cross compiling- assuming \"no\"]))
])


# ---------------------------------------------------------------------
# Check sizes of integer types.
# Cross-compiling sizes set to 2,4,4,8. (May need to be changed...)
# ---------------------------------------------------------------------
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)

# ---------------------------------------------------------------------
# Checks for header files.
# ---------------------------------------------------------------------
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(limits.h sys/time.h unistd.h)
AC_CHECK_HEADERS(sys/types.h sys/socket.h netdb.h)
AC_CHECK_HEADERS(sys/wait.h signal.h)
AM_CONDITIONAL(HAVE_PNG, false)
AC_CHECK_HEADERS(png.h, AM_CONDITIONAL(HAVE_PNG,true))
AC_CHECK_ALPHA_LINUX_CXX
AC_LANG_CPLUSPLUS
AC_CHECK_HEADERS(hash_map hash_set ext/hash_map ext/hash_set sstream)
AC_LANG_C

# ---------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# ---------------------------------------------------------------------
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl If this is a gnu compiler, pass -Wall
if test "$ac_cv_c_compiler_gnu" = "yes"; then
	CFLAGS="$CFLAGS -Wall"
fi

# ---------------------------------------------------------------------
# Checks for library functions
# ---------------------------------------------------------------------
dnl Disabled this for now (undefined in autoconf < 2.5)
dnl AC_FUNC_MALLOC

AC_FUNC_MEMCMP

AC_CHECK_FUNCS([atexit dup2 getcwd isascii memchr memmove memset mkdir pow select socket strcasecmp strchr strstr strtol strtoul])

AC_MSG_CHECKING(for getaddrinfo())
AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
],
[
	struct addrinfo a;
	//getaddrinfo(0, 0, 0, 0);
],
  ac_cv_func_getaddrinfo=yes,
  ac_cv_func_getaddrinfo=no)
AC_MSG_RESULT($ac_cv_func_getaddrinfo)
if test x$ac_cv_func_getaddrinfo = xyes ; then
	AC_DEFINE(HAVE_GETADDRINFO, 1, [Have addrinfo/getaddrinfo])
fi

AC_MSG_CHECKING(for mkstemp())
AC_TRY_LINK([
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
],
[
	mkstemp(0);
],
  ac_cv_func_mkstemp=yes,
  ac_cv_func_mkstemp=no)
AC_MSG_RESULT($ac_cv_func_mkstemp)
if test x$ac_cv_func_mkstemp = xyes ; then
	AC_DEFINE(HAVE_MKSTEMP, 1, [Have mkstemp])
fi

AC_MSG_CHECKING(for snprintf())
AC_TRY_LINK([
#include <stdio.h>
],
[
	snprintf(0,0,0,0);
],
  ac_cv_func_snprintf=yes,
  ac_cv_func_snprintf=no)
AC_MSG_RESULT($ac_cv_func_snprintf)
if test x$ac_cv_func_snprintf = xyes ; then
	AC_DEFINE(HAVE_SNPRINTF, 1, [Have snprintf])
	AM_CONDITIONAL(HAVE_SNPRINTF,true)
else
	AM_CONDITIONAL(HAVE_SNPRINTF,false)
fi

# do we need special X11 libraries?
AC_MSG_CHECKING(for special X11 libraries)
if test x$x_libraries = xNONE; then
	AC_MSG_RESULT(no)
	unset x_libraries
else
	x_libraries="-L$x_libraries -lX11 -lXext"
	AC_MSG_RESULT($x_libraries)
	AC_SUBST(x_libraries)
fi


# ---------------------------------------------------------------------
# SDL
# ---------------------------------------------------------------------

SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
	:,
	AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)

AC_CHECK_LIB(SDL_mixer, Mix_QuickLoad_RAW,
	SDL_LIBS="$SDL_LIBS -lSDL_mixer",
	AC_MSG_ERROR([*** SDL_mixer version 1.2.4 or later not found!]),
	$SDL_LIBS
)


# ---------------------------------------------------------------------
# Optional components
# ---------------------------------------------------------------------

# Timidity MIDI driver
AC_MSG_CHECKING(whether to enable timidity)
AC_ARG_ENABLE(timidity, [  --enable-timidity       Use timidity [defult yes]],,enable_timidity=yes)
if test x$enable_timidity = xyes; then
	AC_MSG_RESULT(yes)
	AC_CHECK_PROG(HAVE_TIMIDITY_BIN, timidity, found it, not found)
	if test "$HAVE_TIMIDITY_BIN" = "found it"; then
		AC_DEFINE(HAVE_TIMIDITY_BIN, 1, [Have timidity binary])
	fi
else
	AC_MSG_RESULT(no)
fi

# zipped savegame support

AC_ARG_ENABLE(zip-support, [  --enable-zip-support    Enable zipped savegame support [default yes]],,enable_zip_support=yes)

if test x$enable_zip_support = xyes ; then
	AC_CHECK_HEADER(zlib.h,,enable_zip_support=no)
fi

AC_MSG_CHECKING(for zipped savegame support)

if test x$enable_zip_support = xyes ; then
	# disabled for now (non-portable):

	# link statically against zlib if using gcc
	# if test x$GCC = xyes ; then
	#	ZLIB_LIBS="-Wl,-Bstatic -lz -Wl,-Bdynamic"
	# else
		ZLIB_LIBS="-lz"
	# fi
	AC_DEFINE(HAVE_ZIP_SUPPORT, 1, [Have zip support])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi


# OpenGL rendering

AC_ARG_ENABLE(opengl, [  --enable-opengl         Enable OpenGL rendering support [EXPERIMENTAL]],,enable_opengl=no)

AC_MSG_CHECKING(for OpenGL rendering support)

if test x$enable_opengl = xyes; then
	AC_DEFINE(HAVE_OPENGL, 1, [Have OpenGL])
	OPENGL_LIBS="-lGL"
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi


# ---------------------------------------------------------------------
# support for Exult Studio
# ---------------------------------------------------------------------
AC_MSG_CHECKING(whether to enable support for Exult Studio)
AC_ARG_ENABLE(exult-studio-support, [  --enable-exult-studio-support   Enable ExultStudio support [default yes]],,enable_exult_studio_support=yes)
if test "$WINDOWING_SYSTEM" != -DXWIN ; then
	enable_exult_studio_support=no
fi

if test x$enable_exult_studio_support = xyes ; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_EXULTSTUDIO, 1, [Use ExultStudio])
else
	AC_MSG_RESULT(no)
fi


# ---------------------------------------------------------------------
# Memory allocation scheme
# ---------------------------------------------------------------------
AC_MSG_CHECKING(checking memory allocation scheme)

# For the alternate allocator
AC_ARG_ENABLE(alternate-allocator,    [  --enable-alternate-allocator    Use alternate memory allocator [default no]],,enable_alternate_allocator=no)
if test x$enable_alternate_allocator = xyes; then
	AC_DEFINE(WANT_ALTERNATE_ALLOCATOR, 1, [Use alternate allocator])
fi

# For storage initialisation
AC_ARG_ENABLE(storage-initialisation, [  --enable-storage-initialisation Clear storage when allocated [default no]],,enable_storage_initialisation=no)
if test x$enable_storage_initialisation = xyes; then
	AC_DEFINE(WANT_ALTERNATE_ALLOCATOR, 1, [Use alternate allocator])
	AC_DEFINE(INITIALISE_ALLOCATED_BLOCKS,0, [Initialize allocated blocks])
fi

# For storage poisoning
AC_ARG_ENABLE(storage-poisoning,      [  --enable-storage-poisoning      Poison storage when allocated [default no]],,enable_storage_poisoning=no)
if test x$enable_storage_poisoning = xyes; then
	if test x$enable_storage_initialisation = xyes; then
		AC_MSG_RESULT(poisoned. (Disabling storage initialisation. Can't poison and init to zero.))
	fi
	AC_DEFINE(WANT_ALTERNATE_ALLOCATOR, 1, [Use alternate allocator])
	AC_DEFINE(POISON_ALLOCATED_BLOCKS, 1, [Poison allocated blocks])
fi

# output result
if test x$enable_storage_poisoning = xyes; then
	if test x$enable_storage_initialisation = xno; then
		AC_MSG_RESULT(poisoned)
	fi
else
	if test x$enable_storage_initialisation = xyes; then
		AC_MSG_RESULT(initialized)
	else
		if test x$enable_alternate_allocator = xyes; then
			AC_MSG_RESULT(alternate)
		else
			AC_MSG_RESULT(normal)
		fi
	  fi
fi


# ---------------------------------------------------------------------
# Debugging options
# ---------------------------------------------------------------------

# basic debugging mode
AC_ARG_ENABLE(debug, [  --enable-debug          Enable debug messages [default no]],,enable_debug=no)
AC_MSG_CHECKING(whether to enable debugging mode)
if test x$enable_debug = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Enable debug mode])
	DEBUG_FLAGS="-g"
else
	AC_MSG_RESULT(no)
	DEBUG_FLAGS="-O2"
fi

# optimized debugging mode
AC_ARG_ENABLE(optimized-debug,  [  --enable-optimized-debug        Enable optimized debug build [default no]],,enable_optimized_debug=no)
AC_MSG_CHECKING(whether to do an optimized debug build)
if test x$enable_optimized_debug = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Enable debug mode])
	DEBUG_FLAGS="-g -O2 -fno-default-inline -fno-inline"
else
	AC_MSG_RESULT(no)
fi

# SDL parachute?
AC_ARG_ENABLE(sdl-parachute,    [  --enable-sdl-parachute  Use SDL parachute [default yes]],,enable_sdl_parachute=yes)
AC_MSG_CHECKING(if we should disable the SDL parachute)
if test x$enable_sdl_parachute = xno; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(NO_SDL_PARACHUTE, 1, [Disable SDL parachute])
else
  AC_MSG_RESULT(no)
fi

# Usecode debugger
AC_ARG_ENABLE(usecode-debugger, [  --enable-usecode-debugger       Support for usecode debugging [default no]],,enable_usecode_debugger=no)
AC_MSG_CHECKING(whether to enable the usecode debugger)
if test x$enable_usecode_debugger = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USECODE_DEBUGGER, 1, [Enable Usecode debugging])
else
	AC_MSG_RESULT(no)
fi


# ---------------------------------------------------------------------
# Warning level
# ---------------------------------------------------------------------

# determine warning level
AC_MSG_CHECKING(warning level)
WARNING_LEVEL="normal"

# more warnings
AC_ARG_ENABLE(warnings, [  --enable-warnings       Enable all warnings [default no]],,enable_warnings=no)
if test x$enable_warnings = xyes; then
 	DEBUG_FLAGS="$DEBUG_FLAGS -Wall -O -Wunused -Wuninitialized -Woverloaded-virtual -Wpointer-arith -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Winline"
	WARNING_LEVEL="all"
fi

# Weep into your beer warnings
AC_ARG_ENABLE(awful-warnings, [  --enable-awful-warnings Enable warnings for violations of Scott Meyers' Effective C++ style guidelines [default no]],,enable_awful_warnings=no)
if test x$enable_awful_warnings = xyes; then
	DEBUG_FLAGS="$DEBUG_FLAGS -Weffc++"
	if test x$WARNING_LEVEL = xall; then
		WARNING_LEVEL="all, including Scott Meyers' 'effective C++'"
	else
		WARNING_LEVEL="Scott Meyers' 'effective C++'"
	fi
fi

AC_ARG_ENABLE(paranoid-warnings, [  --enable-paranoid-warnings      Enable paranoid warnings [default no]],,enable_paranoid_warnings=no)
if test x$enable_paranoid_warnings = xyes; then
	WARNINGS="-Wall -W -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Winline -Wdisabled-optimization -fcheck-new -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual"
#	DEBUG_FLAGS="$DEBUG_FLAGS -Wall -O -Wunused -Wuninitialized -Woverloaded-virtual -Wpointer-arith -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Winline"
	WARNING_LEVEL="paranoid"
fi

# Output result
AC_MSG_RESULT($WARNING_LEVEL)


# Ignore long-long warnings (for SDL header files...)
AC_ARG_ENABLE(long-long-warnings, [  --enable-long-long-warnings     Enable long long warnings- needed for Compaq's cxx with GNU ld [default no]],
	,,enable_long_long_warnings=no)
AC_MSG_CHECKING(if we should disable long-long warnings)
if test x$enable_long_long_warnings = xyes; then
	AC_MSG_RESULT(no)
else
	AC_MSG_RESULT(yes)
	DEBUG_FLAGS="$DEBUG_FLAGS -Wno-long-long"
fi

# Heavy namespaces. Most installations probably can't take this option
AC_ARG_ENABLE(std-namespace, [  --enable-std-namespace  Honour the std namespace. [default no]],,enable_std_namespace=no)
AC_MSG_CHECKING(if we should honour the std namespace)
if test x$enable_std_namespace = xyes; then
	AC_MSG_RESULT(yes)
	DEBUG_FLAGS="$DEBUG_FLAGS -fhonor-std"
else
	AC_MSG_RESULT(no)
fi

# repo
AC_ARG_ENABLE(repo, [  --enable-repo Repo type template instantiation. [default no]],,enable_repo=no)
AC_MSG_CHECKING(if we should use repo type template instantiation)
if test x$enable_repo = xyes; then
	AC_MSG_RESULT(yes)
	DEBUG_FLAGS="$DEBUG_FLAGS -frepo"
else
	AC_MSG_RESULT(no)
fi

# external templates (alt)
AC_ARG_ENABLE(external-templates, [  --enable-external-templates	external template instantiation. [default no]],,enable_external_templates=no)
AC_MSG_CHECKING(if we should use (alternate) external template instantiation)
if test x$enable_external_templates = xyes; then
	AC_MSG_RESULT(yes)
	DEBUG_FLAGS="$DEBUG_FLAGS -falt-external-templates"
else
	AC_MSG_RESULT(no)
fi



# Architecture dependent optimisations
AC_MSG_CHECKING(if we should optimise for MMX)
AC_ARG_ENABLE(timidity, [  --enable-mmx       Use MMX optimisations [defult no]],,enable_mmx=no)
if test x$enable_mmx = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -mmmx"
else
	AC_MSG_RESULT(no)
fi

# Architecture dependent optimisations
AC_MSG_CHECKING(if we should optimise for 3DNOW)
AC_ARG_ENABLE(timidity, [  --enable-3dnow       Use 3DNOW optimisations [defult no]],,enable_3dnow=no)
if test x$enable_3dnow = xyes; then
	AC_MSG_RESULT(yes)
	CXXFLAGS="$CXXFLAGS -m3dnow"
else
	AC_MSG_RESULT(no)
fi

# --------------------
# External features
# --------------------

# Build any external programs?
AC_ARG_ENABLE(tools,    [  --disable-tools         Only build the main program],,enable_tools=yes)
AC_MSG_CHECKING(whether to build only the main program)
if test x$enable_tools = xno; then
	AC_MSG_RESULT(yes)
	AM_CONDITIONAL(BUILD_TOOLS, false)
	enable_gtk_interface=no
	enable_gimp_plugin=no
	enable_compiler=no
else
	AM_CONDITIONAL(BUILD_TOOLS, true)
	AC_MSG_RESULT(no)
fi

# Build compiler?
AC_ARG_ENABLE(compiler, [  --enable-compiler       Build the usecode compiler [default yes]],,enable_compiler=yes)
AC_MSG_CHECKING(whether to build the usecode compiler)
if test x$enable_compiler = xno; then
	AC_MSG_RESULT(no)
	AM_CONDITIONAL(BUILD_COMPILER, false)
else
	AC_MSG_RESULT(yes)
	AM_CONDITIONAL(BUILD_COMPILER, true)
fi

# Build data files?
AC_ARG_ENABLE(data,          [  --enable-data           Create the data files [default yes]],,enable_data=yes)
AC_MSG_CHECKING(whether to build the data files)
if test x$enable_data = xno; then
	AC_MSG_RESULT(no)
	AM_CONDITIONAL(DATA_FILES, false)
else
	AM_CONDITIONAL(DATA_FILES, true)
	AC_MSG_RESULT(yes)
fi

# pkg-config
AC_MSG_CHECKING(for pkg-config)
if test "x`pkg-config --version`" = "x"; then
	AC_MSG_RESULT(no pkg-config)
else
	AC_MSG_RESULT(ok)
	AC_MSG_CHECKING(for GTK+/GLADE development files)
	if pkg-config --atleast-version 2.0 libglade-2.0; then
		have_glade=yes
		AC_MSG_RESULT(yes...)
		AC_DEFINE(HAVE_GLADE, 1, [Have Glade support libraries])
		GLADE_INCLUDES=`pkg-config --cflags libglade-2.0`
		GLADE_LIBS=`pkg-config --libs libglade-2.0`
		AC_SUBST(GLADE_INCLUDES)
		AC_SUBST(GLADE_LIBS)
	else
		have_glade=no
		AC_MSG_RESULT(no gtk+/libglade)
	fi
fi

# Freetype2 (optional, used in ExultStudio, shapes/fontgen.cc)
AC_PATH_PROG(FT2CONFIG, freetype-config)
if test -n "$FT2CONFIG"; then
	AC_DEFINE(HAVE_FREETYPE2, 1, [Have freetype2])
	FREETYPE2_LIBS=`freetype-config --libs`
	AC_SUBST(FREETYPE2_LIBS)
	FREETYPE2_INCLUDES=`freetype-config --cflags`
	AC_SUBST(FREETYPE2_INCLUDES)
fi

# exult-studio
AC_ARG_ENABLE(exult-studio, [  --enable-exult-studio   Build Exult Studio [default no]],,enable_exult_studio=no)
AC_MSG_CHECKING(whether to build Exult Studio)
if test x$enable_exult_studio = xyes; then
	AC_MSG_RESULT(yes)
	if test x$have_glade = xno; then
		echo "Umm, but we don't have any libglade stuff."
		echo "Try again, either with libglade, or with --disable-exult-studio"
		exit 1
	fi
	AM_CONDITIONAL(BUILD_STUDIO, true)
else
	AM_CONDITIONAL(BUILD_STUDIO, false)
	AC_MSG_RESULT(no)
fi

# GNOME desktop files
AC_PATH_PROG(GNOMECONFIG, gnome-config)
if test -n "$GNOMECONFIG"; then
	AM_CONDITIONAL(GNOME_INSTALLED, true)
else
	AM_CONDITIONAL(GNOME_INSTALLED, false)
fi

# GIMP plugin
AM_CONDITIONAL(GIMP_PLUGIN, false)
AC_ARG_ENABLE(gimp-plugin,   [  --enable-gimp-plugin    Build the GIMP plugin [default no]],,enable_gimp_plugin=no)
AC_MSG_CHECKING(whether to build the GIMP plugin)
if test x$enable_gimp_plugin = xyes; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING(for gimptool)
	AC_CHECK_PROGS(GIMPTOOL, gimptool-1.3 gimptool)
	if test -z "$GIMPTOOL"; then
		AC_MSG_RESULT(no, not building GIMP plugin)
	else
		AC_MSG_CHECKING(for GIMP version)
		gimp_version=`$GIMPTOOL --version |  awk 'BEGIN { FS = "."; } { print $1 * 1000 + $2*100+$3;}'`
		if test "$gimp_version" -ge 1312; then
			AC_MSG_RESULT(found >= 1.3.12)
			AC_SUBST(GIMPTOOL)
			AM_CONDITIONAL(GIMP_PLUGIN, true)
			CPPFLAGS="$save_cppflags"
			GIMP_PLUGIN_PREFIX=`$GIMPTOOL --gimpplugindir`
			GIMP_PLUGIN_PREFIX="$GIMP_PLUGIN_PREFIX/plug-ins"
			AC_SUBST(GIMP_PLUGIN_PREFIX)
			AC_DEFINE(HAVE_GIMP, 1, [Have GIMP])
			GIMP_INCLUDES=`$GIMPTOOL --cflags`
			GIMP_LIBS=`$GIMPTOOL --libs`
			AC_SUBST(GIMP_INCLUDES)
			AC_SUBST(GIMP_LIBS)
		else
			AC_MSG_RESULT(found < 1.3.12 - disabling plugin)
		fi
	fi
else
	AC_MSG_RESULT(no)
fi


# ------------------
# Generate output
# ------------------

AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)
AC_SUBST(WINDOWING_SYSTEM)
AC_SUBST(KMID_INCLUDES)
AC_SUBST(KMID_LIBS)
AC_SUBST(EXE_TARGET)
AC_SUBST(EXULT_DATADIR)
AC_SUBST(SYSLIBS)
AC_SUBST(ICON_FILE)
AC_SUBST(DEBUG_FLAGS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(OPENGL_LIBS)
AC_SUBST(WARNINGS)

AC_OUTPUT([
exult.spec
Info.plist
Makefile
audio/Makefile
audio/midi_drivers/Makefile
conf/Makefile
files/Makefile
files/zip/Makefile
gumps/Makefile
pathfinder/Makefile
flic/Makefile
tools/Makefile
data/Makefile
docs/Makefile
desktop/Makefile
objs/Makefile
imagewin/Makefile
shapes/Makefile
usecode/Makefile
usecode/compiler/Makefile
usecode/ucxt/Makefile
usecode/ucxt/Docs/Makefile
usecode/ucxt/data/Makefile
usecode/ucxt/include/Makefile
usecode/ucxt/src/Makefile
mapedit/Makefile
server/Makefile
debian/Makefile
])

dnl ****************
dnl Configure Status
dnl ****************

echo
echo Exult v$VERSION
echo
echo SDL ....................... : `$SDL_CONFIG --version`
if test x$have_glade = xyes; then
echo GLIB ...................... : `pkg-config --modversion glib-2.0`
echo GTK+ ...................... : `pkg-config --modversion gtk+-2.0`
echo libGlade .................. : `pkg-config --modversion libglade-2.0`
echo 
fi
echo Build tools................ : $enable_tools
echo Build ExultStudio.......... : $enable_exult_studio

echo
echo "Now type 'make' to build Exult"
