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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Do we have a version of SDL that we can compile against? */
#define HAVE_SDL 1


// missing defines:
#define PACKAGE "exult"
#define VERSION "0.99cvs"
#define VER_MAJOR "0"
#define VER_MINOR "99"
#define VER_EXTRA "cvs"


/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if the C++ compiler supports BOOL */
#define HAVE_BOOL 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Do we have zlib? (for compressed savegames) */
#define HAVE_ZIP_SUPPORT 1


#define HAVE_HASH_MAP 1
#define HAVE_HASH_SET 1
#define SIZEOF_SHORT 2
#define SIZEOF_LONG 4
