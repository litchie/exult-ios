/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Dir.h - Directions.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#ifndef INCL_DIR
#define INCL_DIR	1

/*
 *	Directions:
 */
enum Direction
	{
	north = 0,
	northeast = 1,
	east = 2,
	southeast = 3,
	south = 4,
	southwest = 5,
	west = 6,
	northwest = 7
	};

Direction Get_direction
	(
	int deltay,
	int deltax
	);
Direction Get_direction4
	(
	int deltay,
	int deltax
	);

int Get_direction16
	(
	int deltay,
	int deltax
	);

#endif
