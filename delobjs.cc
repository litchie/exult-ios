/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Delobjs.cc - Game objects that have been removed, but need deleting.
 **
 **	Written: 5/25/2000 - JSF
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

#include "alpha_kludges.h"

#include "objs.h"
#include "delobjs.h"

/*
 *	Remove and delete all objects.
 */
void Deleted_objects::flush
	(
	)
	{
	clear();			// I think this deletes them.
	}

