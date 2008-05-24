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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "Book_gump.h"
#include "gamewin.h"
#include "game.h"


/*
 *	Create book display.
 */

Book_gump::Book_gump
	(
	bool serp
	) : Text_gump(game->get_shape("gumps/book"), serp)
{
}

/*
 *	Paint book.  Updates curend.
 */

void Book_gump::paint
	(
	Game_window *gwin
	)
{
					// Paint the gump itself.
	gwin->paint_shape(x, y, get_shape());
					// Paint left page.
	curend = paint_page(gwin, Rectangle(36, 10, 122, 130), curtop);
					// Paint right page.
	curend = paint_page(gwin, Rectangle(174, 10, 122, 130), curend);
}