/*
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifdef USE_EXULTSTUDIO  /* Only needed for exult studio. */
#include "server.h"

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
	gwin = Game_window::get_game_window();

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
			gwin->center_text("God Mode Enabled");
			Actor *party[9];		// Set attack mode to 'nearest'.
			int cnt = gwin->get_party(party, 1);
			for (int i = 0; i < cnt; i++)
				party[i]->set_attack_mode(Actor::nearest);
		}
	else
		gwin->center_text("God Mode Disabled");
}

void Cheat::toggle_wizard (void) {
	if (!enabled) return;

	wizard_mode = !wizard_mode;
	if (wizard_mode)
		gwin->center_text("Archwizard Mode Enabled");
	else
		gwin->center_text("Archwizard Mode Disabled");
}

void Cheat::toggle_map_editor (void) {
	if (!enabled) return;

	map_editor = !map_editor;
	if (map_editor)
		{
			gwin->center_text("Map Editor Mode Enabled");
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
		gwin->center_text("Map Editor Mode Disabled");
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
		gwin->center_text("Infravision Enabled");
		gwin->set_palette(0);
	} else
		gwin->center_text("Infravision Disabled");	
}

void Cheat::toggle_pickpocket (void) {
	if (!enabled) return;

	pickpocket = !pickpocket;
	if (pickpocket) {
		gwin->center_text("Pick Pocket Enabled");
		gwin->set_palette(0);
	} else
		gwin->center_text("Pick Pocket Disabled");	
}

void Cheat::toggle_hack_mover (void) {
	if (!enabled) return;

	hack_mover = !hack_mover;
	if (hack_mover) {
		gwin->center_text("Hack mover Enabled");
	} else {
		gwin->center_text("Hack mover Disabled");
	}
}

void Cheat::change_gender (void) const {
	if (!enabled) return;

	if (gwin->get_main_actor()->get_type_flag(Actor::tf_sex)) {
		gwin->get_main_actor()->clear_type_flag(Actor::tf_sex);
		gwin->center_text("Avatar is now male");
	} else {
		gwin->get_main_actor()->set_type_flag(Actor::tf_sex);
		gwin->center_text("Avatar is now female");
	} 
	gwin->set_all_dirty();
}

void Cheat::toggle_eggs (void) const {
	if (!enabled) return;

	gwin->paint_eggs = !gwin->paint_eggs;
	if(gwin->paint_eggs)
		gwin->center_text("Eggs display enabled");
	else
		gwin->center_text("Eggs display disabled");
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
	if (!enabled || (Game::get_game_type() != SERPENT_ISLE && !gwin->can_use_multiracial())) return;

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
		gwin->center_text("Level up!");
	} else {
		gwin->center_text("Maximum level reached");
	}
}

void Cheat::fake_time_period (void) const {
	if (!enabled) return;

	gwin->fake_next_period();
	gwin->center_text("Game clock incremented");
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

void Cheat::map_teleport (void) const {
	if (!enabled) return;

	Shape_frame *map;
	// display map

#if 0
	map = ShapeID(game->get_shape("sprites/map"), 0, SF_SPRITES_VGA).get_shape();
#else
	map = ShapeID(game->get_shape("sprites/cheatmap"), 1, SF_GAME_FLX).get_shape();
#endif

	// Get coords. for centered view.
	int x = (gwin->get_width() - map->get_width())/2 + map->get_xleft();
	int y = (gwin->get_height() - map->get_height())/2 + map->get_yabove();
	gwin->paint_shape(x, y, map, 1);
  
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
	gwin->center_text("Teleport!!!");
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
	gwin->center_text("Teleport!!!");
}

void Cheat::create_coins (void) const {
	if (!enabled) return;

	gwin->get_main_actor()->add_quantity(100, 644);
	gwin->center_text("Added 100 gold coins");
}

void Cheat::create_last_shape (void) const {
	if (!enabled) return;

	int current_shape = 0;
	int current_frame = 0;
	if(browser->get_shape(current_shape, current_frame)) {
		gwin->get_main_actor()->add(gwin->create_ireg_object(current_shape, current_frame), 1);
		gwin->center_text("Object created");
	} else
		gwin->center_text("Can only create from 'shapes.vga'");
}

void Cheat::delete_object (void) const {
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
		obj->remove_this();
		gwin->center_text("Object deleted");
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
		Dead_body *body = dynamic_cast<Dead_body *> (
						gwin->get_body(npc_num));
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
 
	gwin->center_text("Party healed");
	gwin->paint();
}

void Cheat::shape_browser (void) const {
	if (!enabled) return;

	browser->browse_shapes();
	gwin->paint();
	gwin->set_palette(-1,-1);
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
		gwin->center_text("NPC Tool Actor Grabbing Enabled");
	else
		gwin->center_text("NPC Tool Actor Grabbing Disabled");
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
		gwin->center_text("NPC Numbers Enabled");
	else
		gwin->center_text("NPC Numbers Disabled");
}
