/*
Copyright (C) 2000-2004 The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "Notebook_gump.h"
#include "game.h"
// #include "gamewin.h"

vector<One_note> Notebook_gump::notes;
bool Notebook_gump::initialized = false;	// Set when read in.

/*
 *	Create notebook gump.
 */

Notebook_gump::Notebook_gump
	(
	) : Gump(0, game->get_shape("gumps/book")),
		curnote(0), curoff(0)
	{
	// +++++++++Guessing obj. area.
	set_object_area(Rectangle(36, 10, 100, 100), 34, 36);
	}
Notebook_gump *Notebook_gump::create
	(
	)
	{
	// ++++++Initialize.
	return new Notebook_gump;
	}

/*
 *	Paint notebook.
 */

void Notebook_gump::paint
	(
	)
{
	Gump::paint();
	// +++++FINISH.

}
