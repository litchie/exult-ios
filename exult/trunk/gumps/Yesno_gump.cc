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

#include "SDL_events.h"

#include "Yesno_gump.h"
#include "gamewin.h"
#include "mouse.h"
#include "game.h"
#include "Gump_button.h"
#include "Gump_manager.h"

/*
 *	Statics:
 */

short Yesno_gump::yesx = 63;
short Yesno_gump::yesnoy = 45;
short Yesno_gump::nox = 84;


/*
 *	A 'yes' or 'no' button.
 */

class Yesno_button : public Gump_button
{
	int isyes;			// 1 for 'yes', 0 for 'no'.
public:
	Yesno_button(Gump *par, int px, int py, int yes)
		: Gump_button(par, yes ? 
			game->get_shape("gumps/yesbtn") 
			: game->get_shape("gumps/nobtn"), px, py),
		  isyes(yes)
		{  }
					// What to do when 'clicked':
	virtual void activate();
};


/*
 *	Handle 'yes' or 'no' button.
 */

void Yesno_button::activate
	(
	)
{
	((Yesno_gump *) parent)->set_answer(isyes);
}


/*
 *	Create a yes/no box.
 */

Yesno_gump::Yesno_gump
	(
	const std::string &txt
	) : Modal_gump(0, game->get_shape("gumps/yesnobox")), text(txt), answer(-1)
{
	set_object_area(Rectangle(6, 6, 116, 30));
	yes_button = new Yesno_button(this, yesx, yesnoy, 1);
	no_button = new Yesno_button(this, nox, yesnoy, 0);
}

/*
 *	Done with yes/no box.
 */

Yesno_gump::~Yesno_gump
	(
	)
{
	delete yes_button;
	delete no_button;
}

/*
 *	Paint on screen.
 */

void Yesno_gump::paint
	(
	)
{
					// Paint the gump itself.
	paint_shape(x, y);
					// Paint buttons.
	yes_button->paint();
	no_button->paint();
					// Paint text.
	sman->paint_text_box(2, text.c_str(), x + object_area.x, 
			y + object_area.y, object_area.w, object_area.h, 2);
	gwin->set_painted();
}

/*
 *	Handle mouse-down events.
 */

void Yesno_gump::mouse_down
	(
	int mx, int my			// Position in window.
	)
{
					// Check buttons.
	if (yes_button->on_button(mx, my))
		pushed = yes_button;
	else if (no_button->on_button(mx, my))
		pushed = no_button;
	else
	{
		pushed = 0;
		return;
	}
	pushed->push();		// Show it.
}

/*
 *	Handle mouse-up events.
 */

void Yesno_gump::mouse_up
	(
	int mx, int my			// Position in window.
	)
{
	if (pushed)			// Pushing a button?
	{
		pushed->unpush();
		if (pushed->on_button(mx, my))
			pushed->activate();
		pushed = 0;
	}
}

/*
 *	Handle ASCII character typed.
 */

void Yesno_gump::key_down(int chr, SDL_Event& ev)
{
	if (chr == 'y' || chr == 'Y' || chr == SDLK_RETURN)
		set_answer(1);
	else if (chr == 'n' || chr == 'N' || chr == SDLK_ESCAPE)
		set_answer(0);
}

/*
 *	Get an answer to a question.
 *
 *	Output:	1 if yes, 0 if no or ESC.
 */

int Yesno_gump::ask
	(
	const char *txt			// What to ask.
	)
{
	Yesno_gump *dlg = new Yesno_gump(txt);
	int answer;
	if (!gumpman->Do_Modal_gump(dlg, Mouse::hand))
		answer = 0;
	else
		answer = dlg->get_answer();
	delete dlg;
	return (answer);
}
