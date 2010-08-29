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

bool Gump_ToggleButton::activate(int button)
{
	int delta;
	if (button == 1)
		delta = 2;
	else if (button == 3)
		delta = 2*numselections-2;
	else
		return false;

	set_frame((get_framenum() + delta)%(2*numselections));
	toggle(get_framenum()/2);
	paint();
	gwin->set_painted();
	return true;
}

bool Gump_ToggleButton::push(int button)
{
	if (button == 1 || button == 3)
	{
		set_pushed(button);
		paint();
		gwin->set_painted();
		return true;
	}
	return false;
}


void Gump_ToggleButton::unpush
	(
	int button
	)
{
	if (button == 1 || button == 3)
	{
		set_pushed(false);
		paint();
		gwin->set_painted();
	}
}

bool Gump_ToggleTextButton::activate(int button)
{
	int delta;
	if (button == 1)
		delta = 1;
	else if (button == 3)
		delta = numselections-1;
	else
		return false;

	set_frame((get_framenum() + delta)%numselections);
	text = selections[get_framenum()];
	init();
	toggle(get_framenum());
	paint();
	gwin->set_painted();
	return true;
}

bool Gump_ToggleTextButton::push(int button)
{
	if (button == 1 || button == 3)
	{
		set_pushed(button);
		paint();
		gwin->set_painted();
		return true;
	}
	return false;
}


void Gump_ToggleTextButton::unpush
	(
	int button
	)
{
	if (button == 1 || button == 3)
	{
		set_pushed(false);
		paint();
		gwin->set_painted();
	}
}
