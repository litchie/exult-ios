/*
 *  Gump_manager.cc - Object that manages all available gumps
 *
 *  Copyright (C) 2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_events.h"
#include "SDL_keyboard.h"

#include "Configuration.h"
#include "exult.h"
#include "Gump.h"
#include "Gump_manager.h"
#include "gamewin.h"

#include "Actor_gump.h"
#include "Paperdoll_gump.h"
#include "Spellbook_gump.h"
#include "Stats_gump.h"
#include "CombatStats_gump.h"
#include "Jawbone_gump.h"
#include "npcnear.h"
#include "actors.h"
#include "game.h"
#include "Audio.h"
#include "Yesno_gump.h"
#include "gump_utils.h"
#include "Slider_gump.h"

using std::cout;
using std::endl;

Gump_manager::Gump_manager()
	: open_gumps(0), non_persistent_count(0), right_click_close(true)
{
	std::string str;
	config->value("config/gameplay/right_click_closes_gumps", str, "yes");
	if (str == "no")
		right_click_close = false;
	config->set("config/gameplay/right_click_closes_gumps", str, true);
}


/*
 *	Showing gumps.
 */

bool Gump_manager::showing_gumps(bool no_pers) const
{
	// If no gumps, or we do want to check for persistent, just check to see if any exist
	if (!no_pers || !open_gumps) return open_gumps != 0;

	// If we don't want to check for persistend
	for (Gump_list *gump = open_gumps; gump; gump = gump->next)
		if (!gump->gump->is_persistent()) return true;

	return false;
}


/*
 *	Find the highest gump that the mouse cursor is on.
 *
 *	Output:	->gump, or null if none.
 */

Gump *Gump_manager::find_gump
	(
	int x, int y,			// Pos. on screen.
	bool pers				// Persistent?
	)
{
	Gump_list *gmp;
	Gump *found = 0;		// We want last found in chain.
	for (gmp = open_gumps; gmp; gmp = gmp->next)
	{
		Gump *gump = gmp->gump;
		if (gump->has_point(x,y) && (pers || !gump->is_persistent()))
			found = gump;
	}
	return (found);
}

/*
 *	Find gump containing a given object.
 */

Gump *Gump_manager::find_gump
	(
	Game_object *obj
	)
{
					// Get container object is in.
	Game_object *owner = obj->get_owner();
	if (!owner)
		return (0);
					// Look for container's gump.
	for (Gump_list *gmp = open_gumps; gmp; gmp = gmp->next)
		if (gmp->gump->get_container() == owner)
			return (gmp->gump);
	return (0);
}

/*
 *	Find gump with a given owner & shapenum.
 */

Gump *Gump_manager::find_gump
	(
	Game_object *owner,
	int shapenum
	)
{
	Gump_list *gmp;			// See if already open.
	for (gmp = open_gumps; gmp; gmp = gmp->next)
		if (gmp->gump->get_owner() == owner &&
		    gmp->gump->get_shapenum() == shapenum)
			return gmp->gump;
	return (0);
}

/*
 *	Add a gump to the end of a chain.
 */

void Gump_manager::add_gump(Gump *gump)
{
	Gump_list *g = new Gump_list(gump);

	if (!open_gumps)
		open_gumps = g;		// First one.
	else
	{
		Gump_list *last = open_gumps;
		while (last->next) last = last->next;
		last->next = g;
	}
	if (!gump->is_persistent())	// Count 'gump mode' gumps.
		{			// And pause the game.
		non_persistent_count++;
		gwin->get_tqueue()->pause(Game::get_ticks());
		}
}

/*
 *	Close a gump and delete it
 */

bool Gump_manager::close_gump(Gump *gump)
{
	bool ret = remove_gump(gump);
	delete gump;
	return ret;
}

/*
 *	Remove a gump from the chain
 */

bool Gump_manager::remove_gump(Gump *gump)
{
	if (open_gumps)
	{
		if (open_gumps->gump == gump)
		{
			Gump_list *p = open_gumps->next;
			delete open_gumps;
			open_gumps = p;
		}
		else
		{
			Gump_list *p = open_gumps;		// Find prev. to this.
			while (p->next != 0 && p->next->gump != gump) p = p->next;

			if (p->next)
			{
				Gump_list *g = p->next->next;
				delete p->next;
				p->next = g;
			}
			else
				return true;
		}
		if (!gump->is_persistent())	// Count 'gump mode' gumps.
			{			// And resume queue if last.
			non_persistent_count--;
			gwin->get_tqueue()->resume(Game::get_ticks());
			}
	}

	return false;
}

/*
 *	Show a gump.
 */

void Gump_manager::add_gump
	(
	Game_object *obj,		// Object gump represents.
	int shapenum			// Shape # in 'gumps.vga'.
	)
{
	int paperdoll = 0;
	
	if (shapenum >= ACTOR_FIRST_GUMP && shapenum <= ACTOR_LAST_GUMP
		&& Game::get_game_type() == BLACK_GATE)
		paperdoll = 1;

	// overide for paperdolls
	if (shapenum == 123 && (Game::get_game_type() == SERPENT_ISLE ||
		(sman->can_use_paperdolls() && sman->get_bg_paperdolls())))
		paperdoll=2;
	
	Gump *dragged = gwin->get_dragging_gump();
	
	// If we are dragging the same, just return
	if (dragged && dragged->get_owner() == obj && dragged->get_shapenum() == shapenum)
		return;

	static int cnt = 0;		// For staggering them.
	Gump_list *gmp;			// See if already open.
	for (gmp = open_gumps; gmp; gmp = gmp->next)
		if (gmp->gump->get_owner() == obj &&
		    gmp->gump->get_shapenum() == shapenum)
			break;

	if (gmp)			// Found it?
	{			// Move it to end.
		if (gmp->next)
		{
			Gump *gump = gmp->gump;
			remove_gump(gump);
			add_gump(gump);
		}
		gwin->paint();
		return;
	}

	int x = (1 + cnt)*gwin->get_width()/10, 
	    y = (1 + cnt)*gwin->get_height()/10;

	ShapeID s_id(shapenum, 0, paperdoll == 2 ? SF_PAPERDOL_VGA : SF_GUMPS_VGA);
    	Shape_frame *shape = s_id.get_shape();
		
	if (x + shape->get_xright() > gwin->get_width() ||
	    y + shape->get_ybelow() > gwin->get_height())
	{
		cnt = 0;
		x = gwin->get_width()/10;
		y = gwin->get_width()/10;
	}

	Gump *new_gump = 0;
	Actor *npc = 0;
    if (obj)
        npc = obj->as_actor();
	if (npc && paperdoll == 2)
		new_gump = new Paperdoll_gump(npc, x, y, npc->get_npc_num());
	else if (npc && paperdoll) 
		new_gump = new Actor_gump(npc, x, y, shapenum);
	else if (shapenum == game->get_shape("gumps/statsdisplay"))
		new_gump = new Stats_gump((Container_game_object *) obj, x, y);
	else if (shapenum == game->get_shape("gumps/spellbook"))
		new_gump = new Spellbook_gump((Spellbook_object *) obj);
	else if (Game::get_game_type() == SERPENT_ISLE)
	{
		if (shapenum == game->get_shape("gumps/spell_scroll"))
			new_gump = new Spellscroll_gump(obj);
		else if (shapenum >= game->get_shape("gumps/cstats/1")&&
				shapenum <= game->get_shape("gumps/cstats/6"))
			new_gump = new CombatStats_gump(x, y);
		else if (shapenum == game->get_shape("gumps/jawbone"))
			new_gump = new Jawbone_gump(
					(Jawbone_object*) obj, x, y);
	}

	if (!new_gump)
		new_gump = new Container_gump((Container_game_object *) obj, x, y, shapenum);

					// Paint new one last.
	add_gump(new_gump);
	if (++cnt == 8)
		cnt = 0;
	int sfx = Audio::game_sfx(14);
	Audio::get_ptr()->play_sound_effect(sfx);	// The weird noise.
	gwin->paint();			// Show everything.
}

/*
 *	End gump mode.
 */

void Gump_manager::close_all_gumps
	(
	bool pers
	)
{
	bool removed = false;

	Gump_list *prev = 0;
	Gump_list *next = open_gumps;

	while (next)		// Remove all gumps.
		{
		Gump_list *gump = next;
		next = gump->next;

		// Don't delete if persistant or modal.
		if ((!gump->gump->is_persistent() || pers) &&
		    !gump->gump->is_modal())
		{
			if (!gump->gump->is_persistent())
				gwin->get_tqueue()->resume(Game::get_ticks());
			if (prev) prev->next = gump->next;
			else open_gumps = gump->next;
			delete gump->gump;
			delete gump;
			removed = true;
		}
		else
			prev = gump;
	}
	non_persistent_count = 0;
	gwin->get_npc_prox()->wait(4);		// Delay "barking" for 4 secs.
	if (removed) gwin->paint();
}


/*
 *	Handle a double-click.
 */

bool Gump_manager::double_clicked
	(
	int x, int y, 			// Coords in window.
	Game_object *&obj
	)
	{
	Gump *gump = find_gump(x, y);

	if (gump)
	{			// Find object in gump.
		obj = gump->find_object(x, y);
		if (!obj)		// Maybe it's a spell.
		{
		 	Gump_button *btn = gump->on_button(x, y);
			if (btn) btn->double_clicked(x, y);
			else if (gwin->get_double_click_closes_gumps())
			{
				gump->close();
				gwin->paint();
			}
		}
		return true;
	}

	return false;
}

/*
 *	Update the gumps
 */
void Gump_manager::update_gumps()
{
	for (Gump_list *gmp = open_gumps; gmp; gmp = gmp->next)
		gmp->gump->update_gump();
}

/*
 *	Paint the gumps
 */
void Gump_manager::paint()
{
	for (Gump_list *gmp = open_gumps; gmp; gmp = gmp->next)
		gmp->gump->paint();
}


/*
 *	Verify user wants to quit.
 *
 *	Output:	1 to quit.
 */
int Gump_manager::okay_to_quit()
{
	if (Yesno_gump::ask("Do you really want to quit?"))
		quitting_time = QUIT_TIME_YES;
	return quitting_time;
}


int Gump_manager::handle_modal_gump_event
	(
	Modal_gump *gump,
	SDL_Event& event
	)
{
	//	Game_window *gwin = Game_window::get_instance();
	int scale_factor = gwin->get_fastmouse() ? 1 
				: gwin->get_win()->get_scale();
	static bool rightclick;

	switch (event.type)
	{
	case SDL_MOUSEBUTTONDOWN:
#ifdef DEBUG
cout << "(x,y) rel. to gump is (" << ((event.button.x / scale_factor) - gump->get_x())
	 << ", " <<	((event.button.y / scale_factor) - gump->get_y()) << ")"<<endl;
#endif
		if (event.button.button == 1)
			gump->mouse_down(event.button.x / scale_factor, 
						event.button.y / scale_factor);
		else if (event.button.button == 2 && gwin->get_mouse3rd()) {
			gump->key_down(SDLK_RETURN);
			gump->text_input(SDLK_RETURN, SDLK_RETURN);
		} else if (event.button.button == 3)
			rightclick = true;
		else if (event.button.button == 4) // mousewheel up
			gump->mousewheel_up();
		else if (event.button.button == 5) // mousewheel down
			gump->mousewheel_down();
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 1)
			gump->mouse_up(event.button.x / scale_factor,
						event.button.y / scale_factor);
		else if (event.button.button == 3 && gwin->get_mouse3rd() && rightclick) {
			rightclick = false;
			if (gumpman->can_right_click_close()) return 0;
		}
		break;
	case SDL_MOUSEMOTION:
		Mouse::mouse->move(event.motion.x / scale_factor, event.motion.y / scale_factor);
		Mouse::mouse_update = true;
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			gump->mouse_drag(event.motion.x / scale_factor,
						event.motion.y / scale_factor);
		break;
	case SDL_QUIT:
		if (okay_to_quit())
			return (0);
	case SDL_KEYDOWN:
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
				return (0);
			if ((event.key.keysym.sym == SDLK_s) &&
				(event.key.keysym.mod & KMOD_ALT) &&
				(event.key.keysym.mod & KMOD_CTRL)) {
					make_screenshot(true);
					return 1;
			}

			int chr = event.key.keysym.sym;
#if 0
			gump->key_down((event.key.keysym.mod & KMOD_SHIFT)
						? toupper(chr) : chr, event);
#else
			gump->key_down(event.key.keysym.sym);
			gump->text_input(event.key.keysym.sym, event.key.keysym.unicode);
#endif

			break;
		}
	}
	return (1);
}

/*
 *	Handle a modal gump, like the range slider or the save box, until
 *	the gump self-destructs.
 *
 *	Output:	0 if user hit ESC.
 */

int Gump_manager::do_modal_gump
	(
	Modal_gump *gump,	// What the user interacts with.
	Mouse::Mouse_shapes shape	// Mouse shape to use.
	)
{
	SDL_EnableUNICODE(1); // enable unicode translation for text input


	//	Game_window *gwin = Game_window::get_instance();

	// maybe make this selective? it's nice for menus, but annoying for sliders
	//	gwin->end_gump_mode();

	Mouse::Mouse_shapes saveshape = Mouse::mouse->get_shape();
	if (shape != Mouse::dontchange)
		Mouse::mouse->set_shape(shape);
	gwin->show(true);
	int escaped = 0;
					// Get area to repaint when done.
	Rectangle box = gump->get_rect();
	box.enlarge(2);
	box = gwin->clip_to_win(box);
					// Create buffer to backup background.
	Image_buffer *back = gwin->get_win()->create_buffer(box.w, box.h);
	Mouse::mouse->hide();			// Turn off mouse.
					// Save background.
	gwin->get_win()->get(back, box.x, box.y);
	gump->paint();			// Paint gump.
	add_gump(gump);
	Mouse::mouse->show();
	gwin->show();
	do
	{
		Delay();		// Wait a fraction of a second.
		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;
		SDL_Event event;
		while (!escaped && !gump->is_done() && SDL_PollEvent(&event))
			escaped = !handle_modal_gump_event(gump, event);
		if (GL_manager::get_instance())
			gwin->paint();	// OpenGL?  Paint each cycle.
		Mouse::mouse->show();		// Re-display mouse.
		if (!gwin->show() &&	// Blit to screen if necessary.
		    Mouse::mouse_update)	// If not, did mouse change?
			Mouse::mouse->blit_dirty();
	}
	while (!gump->is_done() && !escaped);
	Mouse::mouse->hide();
	remove_gump(gump);
					// Restore background, if wanted.
	if (gump->want_restore_background())
		gwin->get_win()->put(back, box.x, box.y);
	delete back;
	Mouse::mouse->set_shape(saveshape);
					// Leave mouse off.
	gwin->show(true);

	SDL_EnableUNICODE(0);

	return (!escaped);
}


/*
 *	Prompt for a numeric value using a slider.
 *
 *	Output:	Value, or 0 if user hit ESC.
 */

int Gump_manager::prompt_for_number
	(
	int minval, int maxval,		// Range.
	int step,
	int defval			// Default to start with.
	)
{
	Slider_gump *slider = new Slider_gump(minval, maxval,
							step, defval);
	int ok = do_modal_gump(slider, Mouse::hand);
	int ret = !ok ? 0 : slider->get_val();
	delete slider;
	return (ret);
}


/*
 *	Show a number.
 */

void Gump_manager::paint_num
	(
	int num,
	int x,				// Coord. of right edge of #.
	int y				// Coord. of top of #.
	)
{
	//	Shape_manager *sman = Shape_manager::get_instance();
	const int font = 2;
	char buf[20];
  	std::snprintf(buf, 20, "%d", num);
	sman->paint_text(font, buf, x - sman->get_text_width(font, buf), y);
}
