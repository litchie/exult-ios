/*
 *  Copyright (C) 2000-2002  The Exult Team
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

#include <iostream>
#include <algorithm>

#include "SDL_mouse.h"
#include "cheat.h"
#include "exult.h"
#include "gamewin.h"
#include "ucmachine.h"
#include "Configuration.h"
#include "game.h"
#include "actors.h"
#include "mouse.h"
#include "browser.h"
#include "soundtest.h"
#include "cheat_screen.h"
#include "Gump_manager.h"
#include "Gump.h"
#include "drag.h"
#include "effects.h"

#ifdef USE_EXULTSTUDIO  /* Only needed for exult studio. */
#include "server.h"
#include "servemsg.h"

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif // WIN32

#endif //USE_EXULTSTUDIO 

using std::cout;
using std::endl;
using std::strcpy;
using std::strcat;

Cheat cheat;


Cheat::Cheat() {
	enabled = false;

	god_mode = false;
	wizard_mode = false;
	map_editor = false;
	tile_grid = false;
	edit_mode = move;
	edit_lift = 0;
	edit_shape = edit_frame = 0;
	infravision = false;
	pickpocket = false;
	grab_actor = true;

	browser = NULL;
	tester = NULL;
}

Cheat::~Cheat() {
	if (browser)
		delete browser;
	if (tester)
		delete tester;
	if (cscreen)
		delete cscreen;
}

void Cheat::init (void) {
	enabled = false;
	std::string cheating;
	config->value("config/gameplay/cheat",cheating,"no");
	if (cheating == "yes")
		enabled = true;
}

void Cheat::finish_init (void) {
	gwin = Game_window::get_instance();
	eman = gwin->get_effects();

	browser = new ShapeBrowser();
	tester = new SoundTester();
	cscreen = new CheatScreen();

	if (enabled)
		cout << "Cheats enabled." << endl;
}


void Cheat::set_enabled (bool en) {
	enabled = en;
	std::string cheating;
	if(enabled)
		cheating = "yes";
	else
		cheating = "no";
	config->set("config/gameplay/cheat",cheating,true);
}

void Cheat::toggle_god (void) {
	if (!enabled) return;

	god_mode = !god_mode;
	if (god_mode)
		{
			eman->center_text("God Mode Enabled");
			Actor *party[9];		// Set attack mode to 'nearest'.
			int cnt = gwin->get_party(party, 1);
			for (int i = 0; i < cnt; i++)
				party[i]->set_attack_mode(Actor::nearest);
		}
	else
		eman->center_text("God Mode Disabled");
}

void Cheat::toggle_wizard (void) {
	if (!enabled) return;

	wizard_mode = !wizard_mode;
	if (wizard_mode)
		eman->center_text("Archwizard Mode Enabled");
	else
		eman->center_text("Archwizard Mode Disabled");
}

void Cheat::toggle_map_editor (void) {
	if (!enabled) return;

	map_editor = !map_editor;
	if (map_editor)
		{
			eman->center_text("Map Editor Mode Enabled");
					// Stop time.
			gwin->set_time_stopped(-1);
#ifdef USE_EXULTSTUDIO			/* Launch ExultStudio! */
			if (!gwin->paint_eggs)	// Show eggs too.
				{
					gwin->paint_eggs = 1;
					gwin->paint();
				}
			if (client_socket < 0 && 
					!gwin->get_win()->is_fullscreen())
				{
					char cmnd[256];		// Set up command.
					strcpy(cmnd, "exult_studio -x");
					std::string data_path;
					config->value("config/disk/data_path",data_path,EXULT_DATADIR);
					strcat(cmnd, data_path.c_str());// Path to where .glade file should be.
					strcat(cmnd, " -g");	// Now want game name.
					strcat(cmnd, Game::get_gametitle().c_str());
					strcat(cmnd, " &");
					cout << "Executing: " << cmnd << endl;
#ifndef WIN32
					int ret = system(cmnd);
					if (ret == 127 || ret == -1)
						cout << "Couldn't run Exult Studio" << endl;
#else
					PROCESS_INFORMATION	pi;
					STARTUPINFO		si;

					std::memset (&si, 0, sizeof(si));
					si.cb = sizeof(si);

					int ret = CreateProcess (NULL, cmnd, NULL, NULL,
							FALSE, 0,
							NULL, NULL, &si, &pi);
					if (!ret) cout << "Couldn't run Exult Studio" << endl;
#endif
				}
#endif
		}
	else
		{
		clear_selected();	// Selection goes away.
		eman->center_text("Map Editor Mode Disabled");
					// Stop time-stop.
		gwin->set_time_stopped(0);
		}
}

void Cheat::toggle_tile_grid (void) {
	if (!enabled) return;
	tile_grid = !tile_grid;
	gwin->set_all_dirty();
}

void Cheat::set_edit_lift(int lift) {
	if (!enabled) return;
	edit_lift = lift;
	gwin->set_all_dirty();
}

void Cheat::set_edit_shape(int sh, int fr) {
	edit_shape = sh;
	edit_frame = fr;
}

void Cheat::toggle_infravision (void) {
	if (!enabled) return;

	infravision = !infravision;
	if (infravision) {
		eman->center_text("Infravision Enabled");
		gwin->get_pal()->set(0);
	} else
		eman->center_text("Infravision Disabled");	
}

void Cheat::toggle_pickpocket (void) {
	if (!enabled) return;

	pickpocket = !pickpocket;
	if (pickpocket) {
		eman->center_text("Pick Pocket Enabled");
		gwin->get_pal()->set(0);
	} else
		eman->center_text("Pick Pocket Disabled");	
}

void Cheat::toggle_hack_mover (void) {
	if (!enabled) return;

	hack_mover = !hack_mover;
	if (hack_mover) {
		eman->center_text("Hack mover Enabled");
	} else {
		eman->center_text("Hack mover Disabled");
	}
}

void Cheat::change_gender (void) const {
	if (!enabled) return;

	if (gwin->get_main_actor()->get_type_flag(Actor::tf_sex)) {
		gwin->get_main_actor()->clear_type_flag(Actor::tf_sex);
		eman->center_text("Avatar is now male");
	} else {
		gwin->get_main_actor()->set_type_flag(Actor::tf_sex);
		eman->center_text("Avatar is now female");
	} 
	gwin->set_all_dirty();
}

void Cheat::toggle_eggs (void) const {
	if (!enabled) return;

	gwin->paint_eggs = !gwin->paint_eggs;
	if(gwin->paint_eggs)
		eman->center_text("Eggs display enabled");
	else
		eman->center_text("Eggs display disabled");
	gwin->paint();
}

void Cheat::toggle_Petra (void) const {
	if (!enabled || (Game::get_game_type() != SERPENT_ISLE)) return;

	if (gwin->get_main_actor()->get_flag(Obj_flags::petra))
		gwin->get_main_actor()->clear_flag(Obj_flags::petra);
	else
		gwin->get_main_actor()->set_flag(Obj_flags::petra);
	gwin->set_all_dirty();
}

void Cheat::toggle_naked (void) const {
	if (!enabled || (Game::get_game_type() != SERPENT_ISLE)) return;

	if (gwin->get_main_actor()->get_siflag(Actor::naked))
		gwin->get_main_actor()->clear_siflag(Actor::naked);
	else
		gwin->get_main_actor()->set_siflag(Actor::naked);
	gwin->set_all_dirty();
}

void Cheat::change_skin (void) const {
	if (!enabled || (Game::get_game_type() != SERPENT_ISLE && !Shape_manager::get_instance()->can_use_multiracial())) return;

	int color = gwin->get_main_actor()->get_skin_color();
  
	if (GAME_BG) color = (color+1) %4;
	else color = (color+1) %3;

	gwin->get_main_actor()->set_skin_color(color);
	gwin->set_all_dirty();
}

void Cheat::levelup_party (void) const {
	if (!enabled) return;

	Actor* party[9];
	int level, newexp;
	bool leveledup = false;

	// get party, including Avatar
	int cnt = gwin->get_party(party, 1);

	for (int i=0; i<cnt; i++) {
		level = party[i]->get_level();
		if (level < 10) {
			leveledup = true;
			newexp = 25 * (2 << level); // one level higher
			party[i]->set_property(Actor::exp, newexp);
		}
	}  

	if (leveledup) {
		eman->center_text("Level up!");
	} else {
		eman->center_text("Maximum level reached");
	}
}

void Cheat::fake_time_period (void) const {
	if (!enabled) return;

	gwin->fake_next_period();
	eman->center_text("Game clock incremented");
}

void Cheat::dec_skip_lift (void) const {
	if (!enabled) return;

	if (gwin->skip_lift == 16)
		gwin->skip_lift = 11;
	else
		gwin->skip_lift--;
	if (gwin->skip_lift < 0)	// 0 means 'terrain-editing'.
		gwin->skip_lift = 16;
#ifdef DEBUG
	cout << "Skip_lift = " << gwin->skip_lift << endl;
#endif
	gwin->paint();
}

void Cheat::set_skip_lift (int skip) const {
	if (!enabled) return;

	if ((skip >= 1 && skip <= 11) || skip == 16)
		gwin->skip_lift = skip;
#ifdef DEBUG
	cout << "Skip_lift = " << gwin->skip_lift << endl;
#endif
	gwin->paint();
}

/*
 *	Tell EStudio whether there's a selection and clipboard.
 */
void Cheat::send_select_status()
	{
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0)
		{
		unsigned char msg[2];
		msg[0] = selected.empty() ? 0 : 1;
		msg[1] = clipboard.empty() ? 0 : 1;
		Exult_server::Send_data(client_socket, 
				Exult_server::select_status, &msg[0], 2);
		}
#endif
	}

/*
 *	Add an object to the selected list without checking.
 */
void Cheat::append_selected(Game_object *obj) {
	selected.push_back(obj);
	if (selected.size() == 1)	// First one?
		send_select_status();
}

/*
 *	Toggle the selection of an object.
 */
void Cheat::toggle_selected(Game_object *obj) {
	if (!obj->get_owner())
		gwin->add_dirty(obj);
	else
		gwin->set_all_dirty();
					// In list?
	for (Game_object_vector::iterator it = selected.begin();
					it != selected.end(); ++it)
		if (*it == obj)
			{		// Yes, so remove it.
			selected.erase(it);
			if (selected.empty())	// Last one?
				send_select_status();
			return;
			}
	selected.push_back(obj);	// No, so add it.
	if (selected.size() == 1)	// 1st one?
		send_select_status();
}

/*
 *	Clear out selection.
 */
void Cheat::clear_selected() {
	if (selected.empty())
		return;
	for (Game_object_vector::iterator it = selected.begin();
					it != selected.end(); ++it)
		{
		Game_object *obj = *it;
		if (!obj->get_owner())
			gwin->add_dirty(obj);
		else
			gwin->set_all_dirty();
		}
	selected.clear();
	send_select_status();
}

/*
 *	Delete all selected objects.
 */
void Cheat::delete_selected() {
	if (selected.empty())
		return;
	while (!selected.empty())
		{
		Game_object *obj = selected.back();
		selected.pop_back();
		if (obj->get_owner())
			gwin->add_dirty(obj);
		else			// In a gump?
			gwin->set_all_dirty();
		obj->remove_this();
		}
	send_select_status();
}

/*
 *	Move the selected objects by given #tiles.  Objects inside another are
 *	treated as being at the location of their owner.
 */
void Cheat::move_selected(int dx, int dy, int dz) {
	if (selected.empty())
		return;			// Nothing to do.
	std::vector<Tile_coord> tiles;	// Store locations here.
	int lowz = 1000, highz = -1000;	// Get min/max lift.
					// Remove & store old locations.
	Game_object_vector::iterator it;
	for (it = selected.begin(); it != selected.end(); ++it)
		{
		Game_object *obj = *it;
		Game_object *owner = obj->get_outermost();
					// Get location.  Use owner if inside.
		Tile_coord tile = owner->get_tile();
		tiles.push_back(tile);
		if (obj == owner)	// Not inside?
			gwin->add_dirty(obj);
		else			// In a gump.  Repaint all for now.
			gwin->set_all_dirty();
		obj->remove_this(true);
		if (tile.tz < lowz)
			lowz = tile.tz;
		if (tile.tz > highz)
			highz = tile.tz;
		}
	if (lowz + dz < 0)		// Too low?
		dz = -lowz;
	if (highz + dz > 15)		// Too high?
		dz = 15 - highz;
					// Add back in new locations.
	for (it = selected.begin(); it != selected.end(); ++it)
		{
		Tile_coord tile = tiles[it - selected.begin()];
		int newtx = (tile.tx + dx + c_num_tiles)%c_num_tiles;
		int newty = (tile.ty + dy + c_num_tiles)%c_num_tiles;
		int newtz = (tile.tz + dz + 16)%16;
		(*it)->set_invalid();
		(*it)->move(newtx, newty, newtz);
		}
	}

/*
 *	Want lowest, southmost, then eastmost first.
 */
class Clip_compare
	{
public:
	bool operator()(const Game_object *o1, const Game_object *o2)
		{
		Tile_coord t1 = o1->get_tile(),
			   t2 = o2->get_tile();
		if (t1.tz != t2.tz)
			return t1.tz < t2.tz;
		else if (t1.ty != t2.ty)
			return t1.ty > t2.ty;
		else return t1.tx >= t2.tx;
		}
	};

/*
 *	Cut/copy.
 */
void Cheat::cut(bool copy)
	{
	if (selected.empty())
		return;			// Nothing selected.
	bool clip_was_empty = clipboard.empty();
	Game_object_vector::iterator it;
					// Clear out old clipboard.
	for (it = clipboard.begin(); it != clipboard.end(); ++it)
		gwin->delete_object(*it);	// Careful here.
	clipboard.resize(0);
	clipboard.reserve(selected.size());
	if (!copy)			// Removing?  Force repaint.
		gwin->set_all_dirty();
					// Go through selected objects.
	for (it = selected.begin(); it != selected.end(); ++it)
		{
		Game_object *obj = *it;
		Tile_coord t = obj->get_outermost()->get_tile();
		if (copy)
					// TEST+++++REALLY want a 'clone()'.
			obj = gwin->create_ireg_object(obj->get_shapenum(),
							obj->get_framenum());
		else			// Cut:  Remove but don't delete.
			obj->remove_this(true);
					// Set pos. & add to list.
		obj->set_shape_pos(t.tx%c_tiles_per_chunk,
				   t.ty%c_tiles_per_chunk);
		obj->set_chunk(t.tx/c_tiles_per_chunk, t.ty/c_tiles_per_chunk);
		clipboard.push_back(obj);
		}
					// Sort.
	std::sort(selected.begin(), selected.end(), Clip_compare());
	if (!copy)			// Cut?  Remove selection.
		clear_selected();	// (This will send status.)
	else if (clip_was_empty)	// First clipboard object?
		send_select_status();
	}

/*
 *	Create an object as moveable (IREG) or fixed.
 *	++++++++Goes away when we have obj->clone()++++++++++
 */

static Game_object *Create_object
	(
	Game_window *gwin,
	int shape, int frame		// What to create.
	)
	{
	Shape_info& info = ShapeID::get_info(shape);
	int sclass = info.get_shape_class();
					// Is it an ireg (changeable) obj?
	bool ireg = (sclass != Shape_info::unusable &&
		sclass != Shape_info::building);
	Game_object *newobj;
	if (ireg)
		newobj = gwin->create_ireg_object(info, shape, frame, 0, 0, 0);
	else
		newobj = new Ifix_game_object(shape, frame, 0, 0, 0);
	return newobj;
	}

/*
 *	Paste selection.
 */
void Cheat::paste
	(
	int mx, int my			// Mouse position.
	)
	{
	if (clipboard.empty())
		return;			// Nothing there.
					// Use lowest/south/east for position.
	Tile_coord hot = clipboard[0]->get_tile();
	clear_selected();		// Remove old selected.
#if 0
					// First see if spot is in a gump.
	Gump *on_gump = gwin->get_gump_man()->find_gump(mx, my);
#endif
	Game_object_vector::iterator it;
	for (it = clipboard.begin(); it != clipboard.end(); ++it)
		{
		Game_object *obj = *it;
		Tile_coord t = obj->get_tile();
					// Figure spot rel. to hot-spot.
		int liftpix = ((t.tz - hot.tz)*c_tilesize)/2;
		int x = mx + (t.tx - hot.tx)*c_tilesize - liftpix,
		    y = my + (t.ty - hot.ty)*c_tilesize - liftpix;
					// +++++Use clone().
		obj = Create_object(gwin, obj->get_shapenum(),
						obj->get_framenum());
		Dragging_info drag(obj);
		if (drag.drop(x, y, true))	// (Dels if it fails.)
#if 0
		bool ok = false;
		if (on_gump)
			ok = on_gump->add(obj, mx, my, x, y)!=0;
		else			// Try to drop at increasing hts.
			for (int lift = edit_lift; !ok && lift <= 11; lift++)
				ok = gwin->drop_at_lift(obj, x, y, lift);
		if (ok)
#endif
			append_selected(obj);
#if 0
		else
			delete obj;
#endif
		}
	gwin->set_all_dirty();		// Just repaint all.
	}

/*
 *	Prompt for spot to paste to.
 */

void Cheat::paste()
	{
	if (clipboard.empty())
		return;
	int x, y;		// Allow dragging while here:
	if (Get_click(x, y, Mouse::greenselect, 0, true))
		paste(x, y);
	}

void Cheat::map_teleport (void) const {
	if (!enabled) return;

	Shape_frame *map;
	// display map

#if 0
	ShapeID mapid(game->get_shape("sprites/map"), 0, SF_SPRITES_VGA);
#else
	ShapeID mapid(game->get_shape("sprites/cheatmap"), 1, SF_GAME_FLX);
#endif
	map = mapid.get_shape();

	// Get coords. for centered view.
	int x = (gwin->get_width() - map->get_width())/2 + map->get_xleft();
	int y = (gwin->get_height() - map->get_height())/2 + map->get_yabove();
	mapid.paint_shape(x, y, 1);
  
	// mark current location
	int xx, yy;
	Tile_coord t = gwin->get_main_actor()->get_tile();
  
	const int worldsize = c_tiles_per_chunk * c_num_chunks;
	int border=2;

	xx = ((t.tx * (map->get_width() - border*2)) / worldsize);
	yy = ((t.ty * (map->get_height() - border*2)) / worldsize);


	xx += x - map->get_xleft() + border;
	yy += y - map->get_yabove() + border;
	gwin->get_win()->fill8(255, 1, 5, xx, yy - 2);
	gwin->get_win()->fill8(255, 5, 1, xx - 2, yy);
  
	gwin->show(1);
	if (!Get_click(xx, yy, Mouse::greenselect)) {
		gwin->paint();
		return;
	}

	xx -= x - map->get_xleft() + border;
	yy -= y - map->get_yabove() + border;

	t.tx = static_cast<int>(((xx + 0.5)*worldsize) / (map->get_width() - 2*border));
	t.ty = static_cast<int>(((yy + 0.5)*worldsize) / (map->get_height() - 2*border));

	// World-wrapping.
	t.tx = (t.tx + c_num_tiles)%c_num_tiles;
	t.ty = (t.ty + c_num_tiles)%c_num_tiles;
	cout << "Teleporting to " << t.tx << "," << t.ty << "!" << endl;
	t.tz = 0;
	gwin->teleport_party(t);
	eman->center_text("Teleport!!!");
}

void Cheat::cursor_teleport (void) const {
	if (!enabled) return;

	int x, y;
	SDL_GetMouseState(&x, &y);
	x /= gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();
	y /= gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();
	Tile_coord t(gwin->get_scrolltx() + x/c_tilesize,
				 gwin->get_scrollty() + y/c_tilesize, 0);
	gwin->teleport_party(t);
	eman->center_text("Teleport!!!");
}

void Cheat::create_coins (void) const {
	if (!enabled) return;

	gwin->get_main_actor()->add_quantity(100, 644);
	eman->center_text("Added 100 gold coins");
}

void Cheat::create_last_shape (void) const {
	if (!enabled) return;

	int current_shape = 0;
	int current_frame = 0;
	if(browser->get_shape(current_shape, current_frame)) {
		gwin->get_main_actor()->add(gwin->create_ireg_object(current_shape, current_frame), 1);
		eman->center_text("Object created");
	} else
		eman->center_text("Can only create from 'shapes.vga'");
}

void Cheat::delete_object (void) {
	if (!enabled) return;

	int x, y;
	SDL_GetMouseState(&x, &y);
	x /= gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();
	y /= gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();

	Game_object *obj;
	Gump *gump = gwin->get_gump_man()->find_gump(x, y);
	if (gump) {
		obj = gump->find_object(x, y);
	} else {			// Search rest of world.
		obj = gwin->find_object(x, y);
	}

	if (obj) {
		clear_selected();	// Unselect all.
		obj->remove_this();
		eman->center_text("Object deleted");
		gwin->paint();
	}
}

void Cheat::heal_party (void) const {
	if (!enabled) return;

	int	i;	// for MSVC
	Usecode_machine *uc = gwin->get_usecode();
				// NOTE:  dead_party_count decrs. as we
				//   resurrect.
	int count = uc->get_dead_party_count();
	int dead[16];			// Save in separate list.
	if (count > 16)
		count = 16;
	for (i = 0; i < count; i++)
		dead[i] = uc->get_dead_party_member(i);
	for (i = 0; i < count; i++) {
		int npc_num = dead[i];
		Dead_body *body = gwin->get_body(npc_num);
		Actor *live_npc = gwin->get_npc(npc_num);
		if (body && live_npc)
			live_npc->resurrect(body);
	}

	// heal everyone
	Actor* party[9];
	count = gwin->get_party(party, 1);
	for (i = 0; i < count; i++) {
		if (!party[i]->is_dead()) {
			// heal
			party[i]->set_property(Actor::health, party[i]->get_property(Actor::strength));
			// cure poison
			party[i]->clear_flag(Obj_flags::poisoned);

			// remove hunger  +++++ what is "normal" food level??
			party[i]->set_property(Actor::food_level, 30);
		}
	}

	// restore mana
	Main_actor* avatar = gwin->get_main_actor();
	avatar->set_property(Actor::mana, avatar->get_property(Actor::magic));
 
	eman->center_text("Party healed");
	gwin->paint();
}

void Cheat::shape_browser (void) const {
	if (!enabled) return;

	browser->browse_shapes();
	gwin->paint();
	gwin->get_pal()->set(-1,-1);
}

bool Cheat::get_browser_shape (int &shape, int &frame) const {
	if (!enabled) return false;

	return browser->get_shape(shape, frame);
}

void Cheat::sound_tester (void) const {
	if (!enabled) return;

	tester->test_sound();
	gwin->paint();
}


void Cheat::cheat_screen (void) const {
	if (!enabled) return;

	cscreen->show_screen();
	gwin->set_all_dirty();
	gwin->paint();
}

void Cheat::toggle_grab_actor (void) {
	if (!enabled) return;

	grab_actor = !grab_actor;
	if (grab_actor)
		eman->center_text("NPC Tool Actor Grabbing Enabled");
	else
		eman->center_text("NPC Tool Actor Grabbing Disabled");
}

void Cheat::set_grabbed_actor (Actor *actor) const {
	if (!enabled||!cscreen) return;

	cscreen->SetGrabbedActor(actor);	
}

void Cheat::clear_this_grabbed_actor(Actor *actor) const {
	if (!enabled||!cscreen) return;

	cscreen->ClearThisGrabbedActor(actor);
}

void Cheat::toggle_number_npcs (void) {
	if (!enabled) return;

	npc_numbers = !npc_numbers;
	if (npc_numbers)
		eman->center_text("NPC Numbers Enabled");
	else
		eman->center_text("NPC Numbers Disabled");
}
