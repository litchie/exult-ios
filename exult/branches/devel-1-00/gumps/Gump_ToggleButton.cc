/*
Copyright (C) 2001 The Exult Team

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

#include "Gump_ToggleButton.h"
#include "gamewin.h"
#include "Gump.h"

void Gump_ToggleButton::activate(Game_window *gwin)
{
	set_frame(get_framenum() + 2);
	if (get_framenum() >= 2*numselections) set_frame(0);
	toggle(get_framenum()/2);
	paint(gwin);
	gwin->set_painted();
}


void Gump_ToggleTextButton::activate(Game_window *gwin)
{
	set_frame(get_framenum() + 1);
	if (get_framenum() >= numselections) set_frame(0);
	text = selections[get_framenum()];
	init();
	toggle(get_framenum());
	paint(gwin);
	gwin->set_painted();
}
