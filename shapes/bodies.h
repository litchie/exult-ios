/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Bodies.h - Associate bodies with 'live' shapes.
 **
 **	Written: 6/28/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#ifndef INCL_BODIES
#define INCL_BODIES 1

/*
 *	A class to get the dead-body shape/frame for a given 'live' shape.
 */
class Body_lookup
	{
	static short table[];		// Table of values.
public:
	static int find(int liveshape, int& deadshape, int& deadframe);
	};

#endif
