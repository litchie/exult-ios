/*
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Do we have a version of SDL that we can compile against? */
#define HAVE_SDL 1

/* Do we have a version of libkmidi that we can compile against? */
/* #undef HAVE_LIBKMIDI */

      

// missing defines:
#define PACKAGE "exult"
#define MAJOR	"0.94"
#define	RELNUM	"alpha5"
#define VERSION MAJOR RELNUM

/* Do we appear to have a runnable copy of Timidity in our path? */
/* #undef HAVE_TIMIDITY_BIN */

/* Define if you have the <dirent.h> header file.  */
/* #undef HAVE_DIRENT_H */

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/time.h> header file.  */
/* #undef HAVE_SYS_TIME_H */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if the C++ compiler supports BOOL */
#define HAVE_BOOL 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

#define HAVE_HASH_MAP 1
#define HAVE_HASH_SET 1
#define SIZEOF_SHORT 2
#define SIZEOF_LONG 4
