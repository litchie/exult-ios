/**
 **	Combo.cc - A combination of multiple objects.
 **
 **	Written: 4/26/02 - JSF
 **/

/*
Copyright (C) 2002 The Exult Team

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

#include "combo.h"
#include "shapedraw.h"
#include "exult_constants.h"

/*
 *	Create empty combo.
 */

Combo::Combo
	(
	) : tx(0), ty(0), xtiles(0), ytiles(0), ztiles(0), hot_index(-1)
	{
	}

/*
 *	Clean up.
 */

Combo::~Combo
	(
	)
	{
	for (vector<Combo_member *>::iterator it = members.begin();
					it != members.end(); ++it)
		delete *it;
	}

/*
 *	Add a new object.
 */

void Combo::add
	(
	int tx, int ty, int tz,		// Location rel. to top-left.
	int shnum, int frnum		// Shape
	)
	{
	//+++++++++++
	}

/*
 *	Remove i'th object.
 */

void Combo::remove
	(
	int i
	)
	{
	//+++++++++++
	}

/*
 *	Paint in a drawing area.
 */

void Combo::draw
	(
	Shape_draw *draw
	)
	{
	for (vector<Combo_member *>::iterator it = members.begin();
					it != members.end(); ++it)
		{
		Combo_member *m = *it;
					// Figure pixels up-left for lift.
		int lft = m->tz*(c_tilesize/2);
					// Figure relative tile.
		int mtx = m->tx - tx,
		    mty = m->ty - ty;
		int x = mtx*c_tilesize - lft,
		    y = mty*c_tilesize - lft;
		draw->draw_shape(m->shapenum, m->framenum, x, y);
		}
	}
