dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.53)
AC_INIT(exult.cc)


# ---------------------------------------------------------------------
# System/version info
# ---------------------------------------------------------------------

PACKAGE=exult
VER_MAJOR=1
VER_MINOR=5
VER_EXTRA=.0svn
VERSION=$VER_MAJOR.$VER_MINOR$VER_EXTRA

# Do iPhone stuff first, as we may need to manipulate the host variable
AC_ARG_WITH(iphone_sdk, AS_HELP_STRING([--with-iphone-sdk=SDK_NAME], [Compile using iPhone SDK]),,)
if test x$with_iphone_sdk != x; then
        osplat="/Developer/Platforms"
	iphone_sdk="$with_iphone_sdk"
        if test "$(echo $iphone_sdk | grep -c 'iPhoneSimulator')" != "0"; then
                DEV_iOS="${osplat}/iPhoneSimulator.platform/Developer"
                SDK_iOS="${DEV_iOS}/SDKs/${iphone_sdk}.sdk"
                host_alias="i686-apple-darwin10"
                ARCH_iOS="i686"
        else
                DEV_iOS="${osplat}/iPhoneOS.platform/Developer"
                SDK_iOS="${DEV_iOS}/SDKs/${iphone_sdk}.sdk"
                host_alias="arm-apple-darwin10"
                ARCH_iOS="armv6"
        fi
        #LD_LIBRARY_PATH=${COMPILER_iOS}/usr/lib
        COMPILER_iOS=${DEV_iOS}/usr/bin
	CC=${COMPILER_iOS}/gcc
        CXX=${COMPILER_iOS}/g++
        LDFLAGS="-arch ${ARCH_iOS} -pipe -Os -gdwarf-2 -no-cpp-precomp -mthumb -isysroot ${SDK_iOS}"
        CFLAGS=${LDFLAGS}
        CXXFLAGS=${LDFLAGS}
        #CPPFLAGS="-DSDL_VER_1_3 -D__IPHONEOS__"
        AC_DEFINE(__IPHONEOS__, 1, [Using iPhone])
        AC_DEFINE(SDL_VER_1_3, 1, [SDL 1.3])
        enable_sdltest="no"
        if test ! -d "${SDK_iOS}"; then
                AC_MSG_ERROR([*** iPhone SDK directory does not exist! -> ${SDK_iOS}])
        fi
	echo "iPhone SDK ... ${iphone_sdk} (${host_alias})"
fi

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
AC_MSG_CHECKING([windowing system])
case "$host_os" in
	linux*)
		WINDOWING_SYSTEM="-DXWIN"
		if test "$host_vendor" = "embeddix"; then
			AC_MSG_RESULT([Qtopia (Sharp Zaurus)])
			CXXFLAGS="$CXXFLAGS -fsigned-char -DDISABLE_X11 -DQWS -D__zaurus__ -DUSE_FMOPL_MIDI"
			enable_timidity="no"
			enable_exult_studio_support="no"
			enable_exult_studio="no"
			enable_gimp_plugin="no"
			enable_debug="no"
			enable_data="yes"
		else
			AC_MSG_RESULT([X11 (GNU/Linux)])
		fi
		;;
	beos*)
		WINDOWING_SYSTEM="-DBEOS"
		AC_MSG_RESULT([BeOS])
		SYSLIBS="-lbe -lmidi"
		enable_timidity="no"
		;;
	mingw32* )
		WINDOWING_SYSTEM="-DWIN32"
		AC_MSG_RESULT([Win32 (mingw32)])
		SYSLIBS="-lwinmm -lstdc++"
		ICON_FILE="win32/exultico.o"
		enable_timidity="no"
		;;
	cygwin* )
		WINDOWING_SYSTEM="-DWIN32"
		AC_DEFINE(CYGWIN, 1, [Using Cygwin])
		AC_MSG_RESULT([Win32 (cygwin)])
		CXXFLAGS="$CXXFLAGS -mno-cygwin"
		SYSLIBS="-lwinmm"
		ICON_FILE="win32/exultico.o"
		enable_timidity="no"
		;;
	openbsd* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_DEFINE(OPENBSD, 1, [Using OpenBSD])
		AC_MSG_RESULT([X11 (OpenBSD)])
		SYSLIBS="-L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga"
		;;
	freebsd* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_DEFINE(NETBSD, 1, [Using NetBSD])
		AC_MSG_RESULT([X11 (FreeBSD)])
		CXXFLAGS="$CXXFLAGS -I/usr/local/include"
		;;
	netbsd* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT([X11 (NetBSD)])
		CXXFLAGS="$CXXFLAGS -I/usr/X11R6/include"
		;;
	solaris* )
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT([X11 (Solaris)])
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
		AC_MSG_RESULT([Mac OS X])
		if test x$iphone_sdk != x; then
		   SYSLIBS="-framework CoreFoundation -framework CoreAudio  -framework CoreMIDI"
	        else	
		   SYSLIBS="-framework CoreFoundation -framework AudioUnit -framework CoreMIDI"
		fi
		CXXFLAGS="$CXXFLAGS"
		EXULT_DATADIR="/Library/Application\ Support/Exult/data"
		ARCH=macosx
		;;
	*)
		WINDOWING_SYSTEM="-DXWIN"
		AC_MSG_RESULT([not sure... guessing X11])
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

AC_ARG_WITH(cxx, AS_HELP_STRING([--with-cxx=COMMAND], [Explicitly specify the C++ compiler to use]), CXX="")

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

AC_DEFUN([AC_CHECK_ALPHA_LINUX_CXX],
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
# ---------------------------------------------------------------------
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(int*)

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
AC_CHECK_HEADERS(hash_map hash_set ext/hash_map ext/hash_set tr1/unordered_map tr1/unordered_set sstream)
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

AC_MSG_CHECKING([for getaddrinfo()])
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

AC_MSG_CHECKING([for mkstemp()])
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

AC_MSG_CHECKING([for snprintf()])
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
AC_MSG_CHECKING([for special X11 libraries])
if test x$x_libraries = xNONE; then
	AC_MSG_RESULT(no)
	unset x_libraries
else
	x_libraries="-L$x_libraries -lX11 -lXext"
	AC_MSG_RESULT($x_libraries)
	AC_SUBST(x_libraries)
fi

# ---------------------------------------------------------------------
# Statically link libraries?
# ---------------------------------------------------------------------

AC_ARG_ENABLE(static-libraries, AS_HELP_STRING([--enable-static-libraries], [Enable static linking of libraries]),,enable_static_libs=no)
AC_MSG_CHECKING([if we want to statically compile libraries])
if test x$enable_static_libs != xno; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

# ---------------------------------------------------------------------
# Check for pkg-config
# ---------------------------------------------------------------------
PKG_PROG_PKG_CONFIG

# ---------------------------------------------------------------------
# SDL
# ---------------------------------------------------------------------

SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
	:,
	AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)

# ---------------------------------------------------------------------
# libogg, libvorbis, libvorbisfile
# ---------------------------------------------------------------------

if test "x$PKG_CONFIG" != "x"; then
	PKG_CHECK_MODULES(OGG, ogg >= 1.0 vorbis >= 1.0.1 vorbisfile, , AC_MSG_ERROR([*** must have Ogg/Vorbis installed!]))
else
	dnl old school test
	XIPH_PATH_OGG(
		,
		AC_MSG_ERROR([*** must have Ogg installed!])
	)
	XIPH_PATH_VORBIS(
		,
		AC_MSG_ERROR([*** must have Vorbis installed!])
	)
fi

# ---------------------------------------------------------------------
# libglade (for ES)
# ---------------------------------------------------------------------

if test "x$PKG_CONFIG" != "x"; then
	PKG_CHECK_MODULES(GLADE, libglade-2.0 >= 2.0, have_glade=yes, have_glade=no)
else
	have_glade=no
fi

# ---------------------------------------------------------------------
# libgnomeui (for Gnome shp thumbnailer)
# ---------------------------------------------------------------------

if test "x$PKG_CONFIG" != "x"; then
	PKG_CHECK_MODULES(LIBGNOMEUI, libgnomeui-2.0 >= 2.16, have_gnomeui=yes, have_gnomeui=no)
else
	have_gnomeui=no
fi

# ---------------------------------------------------------------------
# Black magic for static linking.
# ---------------------------------------------------------------------
if test x$ARCH = xmacosx; then
	AC_ARG_WITH(macosx-static-lib-path,
		AS_HELP_STRING([--with-macosx-static-lib-path=path], [path to location of static library files (Mac OS X bundles only)]),
		[with_macosx_static_lib_path="$withval"],
		[with_macosx_static_lib_path=""])
	if test x$with_macosx_static_lib_path != x; then
		if test ! -d $with_macosx_static_lib_path; then
			AC_MSG_ERROR([*** a directory is expected for --macosx-static-lib-path!])
		fi
		with_macosx_static_lib_path=`echo "$with_macosx_static_lib_path" | sed 's/\/*$//'`
	else
		enable_static_libs=no
	fi
fi

if test x$enable_static_libs != xno; then
	if test x$ARCH != xmacosx; then
		# Assuming GCC. This is probably not portable.
		STATIC_LD="-static"
		LDFLAGS="$LDFLAGS -static"
	else
		# Apple has done its best to prevent the above from working, so we
		# need this stuff here.
		# Using -static doesn't work because of system libraries that cannot be
		# linked statically; using the usual -Wl,-static -lvorbis -Wl,-dynamic
		# also fails if there is a non-static version of libvorbis in the lib
		# search path before or at the same location as the static lib because
		# the linker tries to use these even with the flags. The only way to get
		# reliable static compiles in Mac OS X is to hard-code the path to the
		# static library and link to *that*.
		# Damn you, Apple!

		# SDL MacOS X readme lists this as needed.
		# We should have sdl-config by this point, or we would have
		# died in AM_PATH_SDL.
		SDL_LIBS=`$SDL_CONFIG $sdl_config_args --static-libs`
		OGG_LIBS="$with_macosx_static_lib_path/libogg.a"
		VORBISFILE_LIBS="$with_macosx_static_lib_path/libvorbisfile.a"
		VORBIS_LIBS="$with_macosx_static_lib_path/libvorbis.a -lm"
		ZLIB_LIBS="$with_macosx_static_lib_path/libz.a"
	fi
fi

# ---------------------------------------------------------------------
# Optional components
# ---------------------------------------------------------------------

# Timidity midi driver
AC_ARG_ENABLE(timidity_midi, AS_HELP_STRING([--disable-timidity-midi], [Disable built-in timidity midi]),,enable_timidity_midi=yes)
AC_ARG_WITH(timidity, AS_HELP_STRING([--with-timidity=path], [path to timidity.cfg (optional)]),,)
AC_MSG_CHECKING([if we want to use timidity midi])
if test x$enable_timidity_midi = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_TIMIDITY_MIDI, 1, [Enable timidity midi])

	if test x$with_timidity != x; then
		if test ! -d $with_timidity; then
			with_timidity=`echo "$with_timidity" | sed 's/timidity.cfg//'`
		fi
		AC_DEFINE_UNQUOTED(DEFAULT_TIMIDITY_PATH, "$with_timidity", [Default timidity path])
	fi
else
	AC_MSG_RESULT(no)
fi

# ALSA midi driver
AC_CHECK_HEADER(alsa/asoundlib.h, HAVEALSA=yes, HAVEALSA=no)
AC_ARG_ENABLE(alsa, AS_HELP_STRING([--disable-alsa], [Disable ALSA midi support]),,enable_alsa=yes)
AC_MSG_CHECKING([if we want to use ALSA midi])
if test x$HAVEALSA = xyes; then
	if test x$enable_alsa = xyes; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_ALSA_MIDI, 1, [Enable ALSA midi])
		LIBS="$LIBS -lasound"
	else
		AC_MSG_RESULT(no)
	fi
else
	AC_MSG_RESULT([no; libasound not found])
fi

# fluidsynth midi driver
AC_CHECK_HEADER(fluidsynth.h, HAVEFLUIDSYNTH=yes, HAVEFLUIDSYNTH=no)
AC_ARG_ENABLE(fluidsynth, AS_HELP_STRING([--disable-fluidsynth], [Disable fluidsynth midi support]),,enable_fluidsynth=yes)
AC_MSG_CHECKING([if we want to use fluidsynth midi])
if test x$HAVEFLUIDSYNTH = xyes; then
        if test x$enable_fluidsynth = xyes; then
                AC_MSG_RESULT(yes)
                AC_DEFINE(USE_FLUIDSYNTH_MIDI, 1, [Enable fluidsynth midi])
                LIBS="$LIBS -lfluidsynth"
        else
                AC_MSG_RESULT(no)
        fi
else
        AC_MSG_RESULT([no; fluidsynth.h not found])
fi

# mt32emu midi driver
AC_ARG_ENABLE(mt32emu, AS_HELP_STRING([--enable-mt32emu], [Enable built-in mt32emu support]),,enable_mt32emu=no)
AC_MSG_CHECKING([if we should build mt32emu])
if test x$enable_mt32emu = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_MT32EMU_MIDI, 1, [Enable mt32emu])
	AM_CONDITIONAL(BUILD_MT32EMU, true)
else
	AC_MSG_RESULT(no)
	AM_CONDITIONAL(BUILD_MT32EMU, false)
fi


# zipped savegame support

AC_ARG_ENABLE(zip-support, AS_HELP_STRING([--enable-zip-support], [Enable zipped savegame support @<:@default yes@:>@]),,enable_zip_support=yes)

if test x$enable_zip_support = xyes ; then
	AC_CHECK_HEADER(zlib.h,,enable_zip_support=no)
fi

AC_MSG_CHECKING([for zipped savegame support])

if test x$enable_zip_support = xyes ; then
	# disabled for now (non-portable):

	# link statically against zlib if using gcc
	# if test x$GCC = xyes ; then
	#	ZLIB_LIBS="-Wl,-Bstatic -lz -Wl,-Bdynamic"
	# else
	if test x$enable_static_libs = xno -o x$ARCH != xmacosx; then
		ZLIB_LIBS="-lz"
	fi
	# fi
	AC_DEFINE(HAVE_ZIP_SUPPORT, 1, [Have zip support])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

# HQnX scaler
AC_ARG_ENABLE(all_hq_scalers, AS_HELP_STRING([--enable-all-hq-scalers], [Enable hq2x, hq3x, hq4x scaler support @<:@default yes@:>@]),,enable_all_hq_scalers=yes)
AC_MSG_CHECKING([if we should build all hq scaler])
if test x$enable_all_hq_scalers = xyes; then
	enable_hq2x=yes
	enable_hq3x=yes
	enable_hq4x=yes
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

# HQ2X scaler
AC_ARG_ENABLE(hq2x, AS_HELP_STRING([--enable-hq2x],[Enable hq2x scaler support @<:@default no@:>@]),,enable_hq2x=no)
AC_MSG_CHECKING(if we should build the hq2x scaler)
if test x$enable_hq2x = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_HQ2X_SCALER, 1, [Build hq2x scaler])
else
	AC_MSG_RESULT(no)
fi

# HQ3X scaler
AC_ARG_ENABLE(hq3x, AS_HELP_STRING([--enable-hq3x],[Enable hq3x scaler support @<:@default no@:>@]),,enable_hq3x=no)
AC_MSG_CHECKING(if we should build the hq3x scaler)
if test x$enable_hq3x = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_HQ3X_SCALER, 1, [Build hq3x scaler])
else
	AC_MSG_RESULT(no)
fi

# HQ4X scaler
AC_ARG_ENABLE(hq4x, AS_HELP_STRING([--enable-hq4x],[Enable hq4x scaler support @<:@default no@:>@]),,enable_hq4x=no)
AC_MSG_CHECKING(if we should build the hq4x scaler)
if test x$enable_hq4x = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_HQ4X_SCALER, 1, [Build hq4x scaler])
else
	AC_MSG_RESULT(no)
fi

# OpenGL rendering

AC_ARG_ENABLE(opengl, AS_HELP_STRING([--enable-opengl], [Enable OpenGL rendering support @<:@EXPERIMENTAL@:>@]),,enable_opengl=no)

if test x$enable_opengl = xyes; then
	# Check for header presence. Damn you, Apple!
	if test x$ARCH != xmacosx; then
		AC_CHECK_HEADER(GL/gl.h,,enable_opengl=no)
	else
		AC_CHECK_HEADER(OpenGL/gl.h,,enable_opengl=no)
	fi
fi

AC_MSG_CHECKING([for OpenGL rendering support])

if test x$enable_opengl = xyes ; then
	AC_DEFINE(HAVE_OPENGL, 1, [Have OpenGL])
	
	# Mac OS X gets OpenGL through a framework.
	if test x$ARCH != xmacosx; then
		OPENGL_LIBS="-lGL"
	else
		OPENGL_LIBS="-framework OpenGL"
	fi
	
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

# Midi Sfx

AC_ARG_ENABLE(midi-sfx, AS_HELP_STRING([--enable-midi-sfx], [Support for Midi Sfx (sounds horrible) @<:@default no@:>@]),,enable_midi_sfx=no)
AC_MSG_CHECKING([whether to enable midi sfx])
if test x$enable_midi_sfx = xyes; then
	AC_DEFINE(ENABLE_MIDISFX, 1, [Enable Midi Sfx])
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi

# ---------------------------------------------------------------------
# support for Exult Studio
# ---------------------------------------------------------------------
AC_MSG_CHECKING([whether to enable support for Exult Studio])
AC_ARG_ENABLE(exult-studio-support, AS_HELP_STRING([--enable-exult-studio-support], [Enable ExultStudio support @<:@default yes@:>@]),,enable_exult_studio_support=yes)
if test "$WINDOWING_SYSTEM" != -DXWIN ; then
	enable_exult_studio_support=no
fi

AC_ARG_ENABLE(macosx-studio-support, AS_HELP_STRING([--enable-macosx-studio-support], [Force ExultStudio support in Mac OS X @<:@EXPERIMENTAL@:>@ @<:@default no@:>@]),,enable_macosx_studio_support=no)
if test "$WINDOWING_SYSTEM" != -DMACOSX; then
	enable_macosx_studio_support=no
fi

if test x$enable_macosx_studio_support = xyes ; then
	enable_exult_studio_support=yes
	SYSLIBS="$SYSLIBS -L/usr/X11R6/lib -lX11"
	AC_DEFINE(XWIN, 1, [X11 (needed by Exult Studio support in Mac OS X)])
fi

if test x$enable_exult_studio_support = xyes ; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(USE_EXULTSTUDIO, 1, [Use Exult Studio])
	
	# due to bug 2129731 and 3139441 some Linux distributions need -lX11 explicitly set 
	# when enabling Exult Studio support or linking fails
	if test "$WINDOWING_SYSTEM" = -DXWIN ; then
		SYSLIBS="$SYSLIBS -lX11"
	fi
	
else
	AC_MSG_RESULT(no)
fi


# ---------------------------------------------------------------------
# Alternative directories
# ---------------------------------------------------------------------

DESKTOPDIR="${datadir}/applications"
AC_ARG_WITH([desktopdir],
            AS_HELP_STRING([--with-desktopdir=DIR],[change desktop directory]),
	[case "${withval}" in
		yes)
		;;
		no)
		;;	
		*)
			DESKTOPDIR="${withval}"
		;;
	esac])
AC_SUBST([DESKTOPDIR])

ICONDIR="${datadir}/icons"
AC_ARG_WITH([icondir],
            AS_HELP_STRING([--with-icondir=DIR],[change icon directory]),
	[case "${withval}" in
		yes)
		;;
		no)
		;;
		*)
			ICONDIR="${withval}"
		;;
	esac])
AC_SUBST([ICONDIR])


# ---------------------------------------------------------------------
# Debugging options
# ---------------------------------------------------------------------

# basic debugging mode
AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug], [Enable debug messages @<:@default no@:>@]),,enable_debug=no)
AC_MSG_CHECKING([whether to enable debugging mode])
if test x$enable_debug = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Enable debug mode])
	DEBUG_FLAGS="-g"
else
	AC_MSG_RESULT(no)
	DEBUG_FLAGS="-O2"
fi

# optimized debugging mode
AC_ARG_ENABLE(optimized-debug, AS_HELP_STRING([--enable-optimized-debug], [Enable optimized debug build @<:@default no@:>@]),,enable_optimized_debug=no)
AC_MSG_CHECKING([whether to do an optimized debug build])
if test x$enable_optimized_debug = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Enable debug mode])
	DEBUG_FLAGS="-g -O2 -fno-default-inline -fno-inline"
else
	AC_MSG_RESULT(no)
fi

# heavy unoptimized GDB  debugging mode
AC_ARG_ENABLE(heavy-gdb-debug, AS_HELP_STRING([--enable-heavy-gdb-debug], [Enable debug messages with GDB symbols @<:@default no@:>@]),,enable_heavy_gdb_debug=no)
AC_MSG_CHECKING([whether to enable unoptimized debugging mode with GDB symbols])
if test x$enable_heavy_gdb_debug = xyes; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DEBUG, 1, [Enable unoptimized debug mode plus GDB symbols])
	DEBUG_FLAGS="-ggdb3"
	CXXFLAGS="-O0"
	CFLAGS="-O0"
else
	AC_MSG_RESULT(no)
fi

# SDL parachute?
AC_ARG_ENABLE(sdl-parachute, AS_HELP_STRING([--enable-sdl-parachute], [Use SDL parachute @<:@default yes@:>@]),,enable_sdl_parachute=yes)
AC_MSG_CHECKING([if we should disable the SDL parachute])
if test x$enable_sdl_parachute = xno; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(NO_SDL_PARACHUTE, 1, [Disable SDL parachute])
else
  AC_MSG_RESULT(no)
fi

# Usecode debugger
#AC_ARG_ENABLE(usecode-debugger, AS_HELP_STRING([--enable-usecode-debugger], [Support for usecode debugging @<:@default no@:>@]),,enable_usecode_debugger=no)
#AC_MSG_CHECKING([whether to enable the usecode debugger])
#if test x$enable_usecode_debugger = xyes; then
#	AC_MSG_RESULT(yes)
#	AC_DEFINE(USECODE_DEBUGGER, 1, [Enable Usecode debugging])
#else
#	AC_MSG_RESULT(no)
#fi


# ---------------------------------------------------------------------
# Warning level
# ---------------------------------------------------------------------

# determine warning level
AC_MSG_CHECKING([warning level])
WARNING_LEVEL="normal"

AC_ARG_ENABLE(warnings, AS_HELP_STRING([--enable-warnings], [Enable all warnings @<:@default no@:>@]),,enable_warnings=no)
# more warnings
AC_ARG_ENABLE(paranoid-warnings, AS_HELP_STRING([--enable-paranoid-warnings], [Enable paranoid warnings @<:@default no@:>@]),,enable_paranoid_warnings=no)
if test x$enable_paranoid_warnings = xyes; then
	WARNINGS="-O -Wall -Wextra -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Winline -Wdisabled-optimization -fcheck-new -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual"
#	DEBUG_FLAGS="$DEBUG_FLAGS -Wall -O -Wunused -Wuninitialized -Woverloaded-virtual -Wpointer-arith -Wcast-qual -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Winline"
	DEBUG_FLAGS="$DEBUG_FLAGS -O -Wall -Wextra -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls -Winline -Wdisabled-optimization -fcheck-new -Wctor-dtor-privacy -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wpointer-arith"
	WARNING_LEVEL="paranoid"
elif test x$enable_warnings = xyes; then
 	DEBUG_FLAGS="$DEBUG_FLAGS -O -Wall -Wunused -Wuninitialized -Woverloaded-virtual -Wpointer-arith -Wcast-qual -Wwrite-strings"
	WARNING_LEVEL="all"
fi

# Output result
AC_MSG_RESULT($WARNING_LEVEL)


# Ignore long-long warnings (for SDL header files...)
AC_ARG_ENABLE(long-long-warnings, AS_HELP_STRING([--enable-long-long-warnings], [Enable long long warnings- needed for Compaq's cxx with GNU ld @<:@default no@:>@]),
	,enable_long_long_warnings=no)
AC_MSG_CHECKING([if we should disable long-long warnings])
if test x$enable_long_long_warnings = xyes; then
	AC_MSG_RESULT(no)
else
	AC_MSG_RESULT(yes)
	DEBUG_FLAGS="$DEBUG_FLAGS -Wno-long-long"
fi

# --------------------
# External features
# --------------------

# Build any external programs?
AC_ARG_ENABLE(tools, AS_HELP_STRING([--disable-tools], [Only build the main program]),,enable_tools=yes)
AC_MSG_CHECKING([whether to build only the main program])
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
AC_ARG_ENABLE(compiler, AS_HELP_STRING([--enable-compiler], [Build the usecode compiler @<:@default yes@:>@]),,enable_compiler=yes)
AC_MSG_CHECKING([whether to build the usecode compiler])
if test x$enable_compiler = xno; then
	AC_MSG_RESULT(no)
	AM_CONDITIONAL(BUILD_COMPILER, false)
else
	AC_MSG_RESULT(yes)
	AM_CONDITIONAL(BUILD_COMPILER, true)
fi

# Build data files?
AC_ARG_ENABLE(data, AS_HELP_STRING([--enable-data], [Create the data files @<:@default yes@:>@]),,enable_data=yes)
AC_MSG_CHECKING([whether to build the data files])
if test x$enable_data = xno; then
	AC_MSG_RESULT(no)
	AM_CONDITIONAL(DATA_FILES, false)
else
	AM_CONDITIONAL(DATA_FILES, true)
	AC_MSG_RESULT(yes)
fi

# Build mods?
AC_ARG_ENABLE(mods, AS_HELP_STRING([--enable-mods], [Build the Exult mods (requires usecode compiler) @<:@default no@:>@]),,enable_mods=no)
AC_MSG_CHECKING([whether to build the Exult mods])
if test x$enable_mods = xno -o x$enable_compiler = xno; then
	AC_MSG_RESULT(no)
	AM_CONDITIONAL(BUILD_MODS, false)
else
	AM_CONDITIONAL(BUILD_MODS, true)
	AC_MSG_RESULT(yes)
fi

# Freetype2 (optional, used in ExultStudio, shapes/fontgen.cc)
AC_PATH_PROG(FT2CONFIG, freetype-config)
if test -n "$FT2CONFIG"; then
	AC_DEFINE(HAVE_FREETYPE2, 1, [Have freetype2])
	FREETYPE2_LIBS=`$FT2CONFIG --libs`
	AC_SUBST(FREETYPE2_LIBS)
	FREETYPE2_INCLUDES=`$FT2CONFIG --cflags`
	AC_SUBST(FREETYPE2_INCLUDES)
fi

# exult-studio
AC_ARG_ENABLE(exult-studio, AS_HELP_STRING([--enable-exult-studio], [Build Exult Studio @<:@default no@:>@]),,enable_exult_studio=no)
AC_MSG_CHECKING([whether to build Exult Studio])
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

# gnome-shp-thumbnailer
AC_ARG_ENABLE(gnome-shp-thumbnailer, AS_HELP_STRING([--enable-gnome-shp-thumbnailer], [Build Gnome SHP Thumbnailer @<:@default no@:>@]),,enable_gnome_shp_thumbnailer=no)
AC_MSG_CHECKING([whether to build the Gnome SHP Thumbnailer])
if test x$enable_gnome_shp_thumbnailer = xyes; then
	AC_MSG_RESULT(yes)
	if test x$have_gnomeui = xno; then
		echo "Umm, but we don't have any libgnomeui stuff."
		echo "Try again, either with libgnomeui, or with --disable-gnome-shp-thumbnailer"
		exit 1
	fi
	AM_CONDITIONAL(BUILD_GTHUMB, true)
else
	AM_CONDITIONAL(BUILD_GTHUMB, false)
	AC_MSG_RESULT(no)
fi

# GIMP plugin
AM_CONDITIONAL(GIMP_PLUGIN, false)
AC_ARG_ENABLE(gimp-plugin, AS_HELP_STRING([--enable-gimp-plugin], [Build the GIMP plugin @<:@default no@:>@]),,enable_gimp_plugin=no)
AC_MSG_CHECKING([whether to build the GIMP plugin])
if test x$enable_gimp_plugin = xyes; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING([for gimptool])
	AC_CHECK_PROGS(GIMPTOOL, gimptool-2.0 gimptool-1.3 gimptool-1.2 gimptool)
	if test -z "$GIMPTOOL"; then
		AC_MSG_RESULT([no, not building GIMP plugin])
	else
		AC_MSG_CHECKING([for GIMP version])
		gimp_version=`$GIMPTOOL --version |  awk 'BEGIN { FS = "."; } { print $1 * 1000 + $2*100+$3;}'`
		if test "$gimp_version" -ge 1312; then
			AC_MSG_RESULT([found >= 1.3.12])
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
		elif test "$gimp_version" -ge 1200 -a \
			  "$gimp_version" -lt 1300; then
			AC_MSG_RESULT([found 1.2.x])
			AC_SUBST(GIMPTOOL)
			AM_CONDITIONAL(GIMP_PLUGIN, true)
			CPPFLAGS="$save_cppflags"
			GIMP_PLUGIN_PREFIX=`$GIMPTOOL --gimpplugindir`
			GIMP_PLUGIN_PREFIX="$GIMP_PLUGIN_PREFIX/plug-ins"
			AC_SUBST(GIMP_PLUGIN_PREFIX)
			AC_DEFINE(HAVE_GIMP, 1, [Have GIMP])
			AC_DEFINE(HAVE_GIMP_1_2, 1, [Have GIMP 1.2.x])
			GIMP_INCLUDES=`$GIMPTOOL --cflags`
			GIMP_LIBS=`$GIMPTOOL --libs`
			AC_SUBST(GIMP_INCLUDES)
			AC_SUBST(GIMP_LIBS)
		else
			AC_MSG_RESULT([found < 1.3.12 - disabling plugin])
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
AC_SUBST(OGG_LIBS)
AC_SUBST(VORBISFILE_LIBS)
AC_SUBST(VORBIS_LIBS)
AC_SUBST(WINDOWING_SYSTEM)
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
audio/midi_drivers/timidity/Makefile
audio/midi_drivers/mt32emu/Makefile
conf/Makefile
content/Makefile
content/sifixes/Makefile
content/si/Makefile
content/bgkeyring/Makefile
content/bg/Makefile
content/islefaq/Makefile
files/Makefile
files/zip/Makefile
gamemgr/Makefile
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
shapes/shapeinf/Makefile
usecode/Makefile
usecode/compiler/Makefile
usecode/ucxt/Makefile
usecode/ucxt/Docs/Makefile
usecode/ucxt/data/Makefile
usecode/ucxt/include/Makefile
usecode/ucxt/src/Makefile
mapedit/Makefile
server/Makefile
])

dnl ****************
dnl Configure Status
dnl ****************

echo
echo Exult v$VERSION
echo
echo SDL ....................... : `$SDL_CONFIG --version`
if test x$have_glade = xyes; then
echo GLIB ...................... : `$PKG_CONFIG --modversion glib-2.0`
echo GTK+ ...................... : `$PKG_CONFIG --modversion gtk+-2.0`
echo libGlade .................. : `$PKG_CONFIG --modversion libglade-2.0`
fi
if test x$have_gnomeui = xyes; then
echo libGnomeui................. : `$PKG_CONFIG --modversion libgnomeui-2.0`
fi
echo 
echo Build tools................ : $enable_tools
echo Build usecode compiler..... : $enable_compiler
echo Build Exult mods........... : $enable_mods
echo Build Exult Studio......... : $enable_exult_studio
echo Exult Studio support....... : $enable_exult_studio_support
echo
if test x$enable_all_hq_scalers = xyes; then
echo Build all HQ scalers....... : $enable_all_hq_scalers
else
echo Build HQ2X scaler.......... : $enable_hq2x
echo Build HQ3X scaler.......... : $enable_hq3x
echo Build HQ4X scaler.......... : $enable_hq4x
fi
echo
echo "Now type 'make' to build Exult"
