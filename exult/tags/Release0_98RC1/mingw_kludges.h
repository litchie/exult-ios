/*
Copyright (C) 2000 The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/******************************************************************************

This file is for Kludges that are specific for compiling Exult with MinGW32

The file is automatically included in every sourcefile by the makefile

This file shouldn't be manually included as it may cause problems for other
compilers.

******************************************************************************/

#ifndef MINGW_KLUDGES_H
#define MINGW_KLUDGES_H

// Mingw doesn't have snprintf, but does have _snprintf
// Exult uses snprintf
#define snprintf _snprintf

#endif //MINGW_KLUDGES_H