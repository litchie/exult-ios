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

#include "actors.h"
#include "File_gump.h"
#include "game.h"
#include "gamewin.h"
#include "gump_utils.h"
#include "misc_buttons.h"
#include "Modal_gump.h"
#include "mouse.h"


/*
 *	A checkmark for closing its parent:
 */
Checkmark_button::Checkmark_button(Gump *par, int px, int py)
	: Gump_button(par, game->get_shape("gumps/check"), px, py)
{
}

/*
 *	Handle click on a 'checkmark'.
 */

void Checkmark_button::activate
	(
	Game_window *gwin
	)
{
	parent->close(gwin);
}

/*
 *	A 'heart' button for bringing up stats.
 */

Heart_button::Heart_button(Gump *par, int px, int py)
	: Gump_button(par, game->get_shape("gumps/heart"), px, py)
{
}

/*
 *	Handle click on a heart.
 */

void Heart_button::activate
	(
	Game_window *gwin
	)
{
	gwin->show_gump(parent->get_container(), game->get_shape("gumps/statsdisplay"));
}

/*
 *	A diskette for bringing up the 'save' box.
 */

Disk_button::Disk_button(Gump *par, int px, int py)
	: Gump_button(par, game->get_shape("gumps/disk"), px, py)
{
}

/*
 *	Handle click on a diskette.
 */

void Disk_button::activate
	(
	Game_window *gwin
	)
{
	File_gump *fileio = new File_gump();
	Do_Modal_gump(fileio, Mouse::hand);
	delete fileio;
}

/*
 *	The combat toggle button.
 */

Combat_button::Combat_button(Gump *par, int px, int py)
	: Gump_button(par, game->get_shape("gumps/combat"),
		px, py)
{
	pushed = Game_window::get_game_window()->in_combat();
}

/*
 *	Handle click on a combat toggle button.
 */

void Combat_button::activate
	(
	Game_window *gwin
	)
{
	gwin->toggle_combat();
	pushed = gwin->in_combat();
	parent->paint(gwin);
}

/*
 *	The halo button.
 */

Halo_button::Halo_button(Gump *par, int px, int py)
	: Gump_button(par, game->get_shape("gumps/halo"), px, py)
{
}

/*
 *	Handle click on a halo toggle button.
 */

void Halo_button::activate
	(
	Game_window *gwin
	)
{
					// ++++++Later.
}

/*
 *	Combat mode.  Has 10 frames corresponding to Actor::Attack_mode.
 */

Combat_mode_button::Combat_mode_button(Gump *par, int px, int py, Actor *a)
	: Gump_button(par, game->get_shape("gumps/combatmode"), px, py), actor(a)
{
	framenum = (int) actor->get_attack_mode();
}

/*
 *	Handle click on a combat toggle button.
 */

void Combat_mode_button::activate
	(
	Game_window *gwin
	)
{
	framenum = (framenum + 1)%10;
	actor->set_attack_mode((Actor::Attack_mode) framenum);
	parent->paint_button(gwin, this);
	gwin->set_painted();
}
