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

#include "../alpha_kludges.h"

#include "gamewin.h"
#include "Gump_button.h"
#include "Gump.h"


/*
 *	Redisplay as 'pushed'.
 */

void Gump_button::push
	(
	Game_window *gwin
	)
{
	pushed = 1;
	parent->paint_button(gwin, this);
	gwin->set_painted();
}

/*
 *	Redisplay as 'unpushed'.
 */

void Gump_button::unpush
	(
	Game_window *gwin
	)
{
	pushed = 0;
	parent->paint_button(gwin, this);
	gwin->set_painted();
}

/*
 *	Default method for double-click.
 */

void Gump_button::double_clicked
	(
	Game_window *gwin
	)
{
}
