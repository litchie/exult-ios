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

#include "actors.h"
#include "game.h"
#include "gamewin.h"
#include "gump_utils.h"
#include "misc_buttons.h"
#include "Stats_gump.h"


/*
 *	Some gump shape numbers:
 */

const int ASLEEP = 0, POISONED = 1, CHARMED = 2, HUNGRY = 3,
		  PROTECTED = 4, CURSED = 5, PARALYZED = 6;

/*
 *	Statics:
 */

short Stats_gump::textx = 123;
short Stats_gump::texty[10] = {17, 26, 35, 46, 55, 67, 76, 86,
							95, 104};

/*
 *	Show one of the atts.
 *
 *	Output:	Amount to increment x-pos for the next one.
 */

static int Show_atts
	(
	Game_window *gwin,
	int x, int y,			// Pos. on screen.
	int framenum
	)
{
	Shape_frame *s = gwin->get_gump_shape(game->get_shape(
						"gumps/statatts"), framenum);
	gwin->paint_shape(x + s->get_xleft(),
				 y + s->get_ybelow(), s, 1);
	return s->get_width() + 2;
}

/*
 *	Create stats display.
 */
Stats_gump::Stats_gump
	(
	Container_game_object *cont, 
	int initx, int inity
	) : Gump(cont, initx, inity, game->get_shape("gumps/statsdisplay"))
{
}

/*
 *	Paint on screen.
 */

void Stats_gump::paint
	(
	Game_window *gwin
	)
{
					// Area to print name in.
	const int namex = 30, namey = 6, namew = 95;
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
					// Paint red "checkmark".
	paint_button(gwin, check_button);
	Actor *act = get_actor();	// Show statistics.
	std::string nm = act->get_name();
	gwin->paint_text(2, nm.c_str(), x + namex +
		(namew - gwin->get_text_width(2, nm.c_str()))/2, y + namey);
	Paint_num(gwin, act->get_property(Actor::strength),
						x + textx, y + texty[0]);
	Paint_num(gwin, act->get_property(Actor::dexterity),
						x + textx, y + texty[1]);
	Paint_num(gwin, act->get_property(Actor::intelligence),
						x + textx, y + texty[2]);
  	Paint_num(gwin, act->get_property(Actor::combat),
						x + textx, y + texty[3]);
  	Paint_num(gwin, act->get_property(Actor::magic),
						x + textx, y + texty[4]);
  	Paint_num(gwin, act->get_property(Actor::health),
						x + textx, y + texty[5]);
  	Paint_num(gwin, act->get_property(Actor::mana),
						x + textx, y + texty[6]);
  	Paint_num(gwin, act->get_property(Actor::exp),
						x + textx, y + texty[7]);
	Paint_num(gwin, act->get_level(), x + textx, y + texty[8]);
  	Paint_num(gwin, act->get_property(Actor::training),
						x + textx, y + texty[9]);
					// Now show atts. at bottom.
	const int attsy = 130, attsx0 = 29;
	int attsx = attsx0;
	if (act->get_flag(Actor::asleep))
		attsx += Show_atts(gwin, x + attsx, y + attsy, ASLEEP);
	if (act->get_flag(Actor::poisoned))
		attsx += Show_atts(gwin, x + attsx, y + attsy, POISONED);
	if (act->get_flag(Actor::charmed))
		attsx += Show_atts(gwin, x + attsx, y + attsy, CHARMED);
	if (act->get_property((int) Actor::food_level) <= 4)
		attsx += Show_atts(gwin, x + attsx, y + attsy, HUNGRY);
	if (act->get_flag(Actor::protection))
		attsx += Show_atts(gwin, x + attsx, y + attsy, PROTECTED);
	if (act->get_flag(Actor::cursed))
		attsx += Show_atts(gwin, x + attsx, y + attsy, CURSED);
	if (act->get_flag(Actor::paralyzed))
		attsx += Show_atts(gwin, x + attsx, y + attsy, PARALYZED);
}
