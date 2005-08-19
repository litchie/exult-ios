/*
Copyright (C) 2000-2002 The Exult Team

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

#include "actors.h"
#include "game.h"
#include "gamewin.h"
#include "misc_buttons.h"
#include "Stats_gump.h"
#include "Gump_manager.h"
#include "exult_flx.h"

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
const int num_extra_spots = 10;
const int att_name_textx = 15;

/*
 *	A secondary gump for showing custom attributes (for mods, new games).
 */
class Stats_extra_gump : public Stats_gump
	{
	Actor::Atts_vector atts;
public:
	Stats_extra_gump(Container_game_object *cont, int initx, int inity,
			Actor::Atts_vector allatts, int first)
		: Stats_gump(cont, initx, inity, EXULT_FLX_STATS_EXTRA_SHP,
				SF_EXULT_FLX)
		{
		int cnt = allatts.size() - first;
		if (cnt > num_extra_spots)
			cnt = num_extra_spots;
		atts.resize(cnt);
		for (int i = first; i < cnt; ++i)
			atts[i] = allatts[first + i];
		}
	~Stats_extra_gump()
		{  }

	virtual void paint();
};

/*
 *	Paint on screen.
 */

void Stats_extra_gump::paint
	(
	)
{
	const int font = 2;
	Gump_manager* gman = gumpman;

					// Area to print name in.
	const int namex = 30, namey = 6, namew = 95;
	Actor *act = get_actor();
					// Paint the gump itself.
	paint_shape(x, y);
					// Paint red "checkmark".
	check_button->paint();
					// Show statistics.
	std::string nm = act->get_name();
	sman->paint_text(2, nm.c_str(), x + namex +
		(namew - sman->get_text_width(2, nm.c_str()))/2, y + namey);
	int cnt = atts.size();
	for (int i = 0; i < cnt; ++i)
		{
		sman->paint_text(font, atts[i].first, x + att_name_textx, y);
		gman->paint_num(atts[i].second, x + textx, y + texty[0]);
		}
}

/*
 *	Show one of the atts.
 *
 *	Output:	Amount to increment x-pos for the next one.
 */

static int Show_atts
	(
	int x, int y,			// Pos. on screen.
	int framenum
	)
{
	Shape_manager *sman = Shape_manager::get_instance();
	ShapeID sid(game->get_shape("gumps/statatts"), framenum, SF_GUMPS_VGA);
	Shape_frame *s = sid.get_shape();
	sman->paint_shape(x + s->get_xleft(),
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
	set_object_area(Rectangle(0,0,0,0), 6, 136);
}
Stats_gump::Stats_gump
	(
	Container_game_object *cont, 
	int initx, int inity,
	int shnum,
	ShapeFile shfile
	) : Gump(cont, initx, inity, shnum, shfile)
{
	set_object_area(Rectangle(0,0,0,0), 6, 136);
}

/*
 *	Paint on screen.
 */

void Stats_gump::paint
	(
	)
{
	Gump_manager* gman = gumpman;

					// Area to print name in.
	const int namex = 30, namey = 6, namew = 95;
	Actor *act = get_actor();	// Check for freezing (SI).
	if (gwin->get_main_actor()->get_flag(Obj_flags::freeze))
		{
		int temp = act->get_temperature();
		int framenum = temp/10;	// Want it 1-5.
		if (framenum <= 0)
			framenum = 1;
		else if (framenum > 5)
			framenum = 5;
		set_frame(framenum);
		}
					// Paint the gump itself.
	paint_shape(x, y);
					// Paint red "checkmark".
	check_button->paint();
					// Show statistics.
	std::string nm = act->get_name();
	sman->paint_text(2, nm.c_str(), x + namex +
		(namew - sman->get_text_width(2, nm.c_str()))/2, y + namey);
	gman->paint_num(act->get_effective_prop(Actor::strength),
						x + textx, y + texty[0]);
	gman->paint_num(act->get_effective_prop(Actor::dexterity),
						x + textx, y + texty[1]);
	gman->paint_num(act->get_effective_prop(Actor::intelligence),
						x + textx, y + texty[2]);
  	gman->paint_num(act->get_effective_prop(Actor::combat),
						x + textx, y + texty[3]);
  	gman->paint_num(act->get_property(Actor::magic),
						x + textx, y + texty[4]);
  	gman->paint_num(act->get_property(Actor::health),
						x + textx, y + texty[5]);
  	gman->paint_num(act->get_property(Actor::mana),
						x + textx, y + texty[6]);
  	gman->paint_num(act->get_property(Actor::exp),
						x + textx, y + texty[7]);
	gman->paint_num(act->get_level(), x + textx, y + texty[8]);
  	gman->paint_num(act->get_property(Actor::training),
						x + textx, y + texty[9]);
					// Now show atts. at bottom.
	const int attsy = 130, attsx0 = 29;
	int attsx = attsx0;
	if (act->get_flag(Obj_flags::asleep))
		attsx += Show_atts(x + attsx, y + attsy, ASLEEP);
	if (act->get_flag(Obj_flags::poisoned))
		attsx += Show_atts(x + attsx, y + attsy, POISONED);
	if (act->get_flag(Obj_flags::charmed))
		attsx += Show_atts(x + attsx, y + attsy, CHARMED);
	if (act->get_property((int) Actor::food_level) <= 4)
		attsx += Show_atts(x + attsx, y + attsy, HUNGRY);
	if (act->get_flag(Obj_flags::protection))
		attsx += Show_atts(x + attsx, y + attsy, PROTECTED);
	if (act->get_flag(Obj_flags::cursed))
		attsx += Show_atts(x + attsx, y + attsy, CURSED);
	if (act->get_flag(Obj_flags::paralyzed))
		attsx += Show_atts(x + attsx, y + attsy, PARALYZED);
}
