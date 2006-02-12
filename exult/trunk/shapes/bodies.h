/*
 *  bodies.h - Associate bodies with 'live' shapes.
 *
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

#ifndef BODIES_H
#define BODIES_H 1

/*
 *	A class to get the dead-body shape/frame for a given 'live' shape.
 */
class Body_lookup
	{
	static void setup();
public:
	static int find(int liveshape, int& deadshape, int& deadframe);
	static int Is_body_shape(int shapeid);
	};


/*
 *	Recognize dead body shapes.
 */

inline int Is_body(int shapeid)
	{ return Body_lookup::Is_body_shape(shapeid); }


#endif
