/*
Copyright (C) 2000  Willem Jan Palenstijn

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

#include <iostream>

#include "SDL_mouse.h"
#include "cheat.h"
#include "gamewin.h"
#include "Configuration.h"
#include "game.h"
#include "actors.h"
#include "mouse.h"
#include "browser.h"
#include "soundtest.h"
#include "cheat_screen.h"

using std::cout;
using std::endl;

extern Configuration* config;
extern int scale;

int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);


Cheat::Cheat() {
  enabled = false;

  god_mode = false;
  wizard_mode = false;
  map_editor = false;
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
  config->value("config/gameplay/cheat",cheating,"yes");
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
    gwin->center_text("God Mode Enabled");
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
#ifdef XWIN			/* Launch ExultStudio! */
    static bool launched = false;
    if (!launched && !gwin->get_win()->is_fullscreen())
	{
	launched = true;
	char cmnd[256];		// Set up command.
	strcpy(cmnd, "exult_studio -x");
	std::string data_path;
	config->value("config/disk/data_path",data_path,EXULT_DATADIR);
	strcat(cmnd, data_path.c_str());// Path to where .glade file should be.
	strcat(cmnd, " -d");	// Now want path to game.
	std::string gametitle = Game::get_game_type() == BLACK_GATE ?
		"blackgate" : "serpentisle";
	std::string d = "config/disk/game/" + gametitle + "/path";
	std::string game_path;
	config->value(d.c_str(), game_path, ".");
	strcat(cmnd, game_path.c_str());
	strcat(cmnd, " &");
	cout << "Executing: " << cmnd << endl;
	int ret = system(cmnd);
	if (ret == 127 || ret == -1)
		cout << "Couldn't run Exult Studio" << endl;
	}
#endif
    }
  else
    gwin->center_text("Map Editor Mode Disabled");			
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
  if (!enabled || (Game::get_game_type() != SERPENT_ISLE)) return;

  int color = gwin->get_main_actor()->get_skin_color();
  
  if (color < 0 || color > 2)
    return;
  color = (color + 4) %3;
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
  if (gwin->skip_lift <= 0)
    gwin->skip_lift = 16;
#if DEBUG
  cout << "Skip_lift = " << gwin->skip_lift << endl;
#endif
  gwin->paint();
}

void Cheat::set_skip_lift (int skip) const {
  if (!enabled) return;

  if ((skip >= 1 && skip <= 11) || skip == 16)
    gwin->skip_lift = skip;
#if DEBUG
  cout << "Skip_lift = " << gwin->skip_lift << endl;
#endif
  gwin->paint();
}

void Cheat::map_teleport (void) const {
  if (!enabled) return;

  // display map

  //++++Black gate
  Shape_frame *map = gwin->get_sprite_shape(22, 0);
  // Get coords. for centered view.
  int x = (gwin->get_width() - map->get_width())/2 + map->get_xleft();
  int y = (gwin->get_height() - map->get_height())/2 + map->get_yabove();
  gwin->paint_shape(x, y, map, 1);
  
  // mark current location
  int tx, ty, z, xx, yy;
  gwin->get_main_actor()->get_abs_tile(tx, ty, z);
  
  // these may need some tweaking for SI
  int border = (Game::get_game_type()==SERPENT_ISLE ? 12 : 5);
  const int worldsize = c_tiles_per_chunk * c_num_chunks;
  int correction = (Game::get_game_type()==SERPENT_ISLE ? 0 : 1);
  int correctx = (Game::get_game_type()==SERPENT_ISLE ? 9 : 0);
  int correcty = (Game::get_game_type()==SERPENT_ISLE ? 0 : 0);
  int correctscale = (Game::get_game_type()==SERPENT_ISLE ? 10 : 0);

  xx = ((tx * (map->get_width() - border*2 + correctscale)) / worldsize) + (border + x - map->get_xleft()) + correction + correctx;
  yy = ((ty * (map->get_height() - border*2 + correctscale)) / worldsize) + (border + y - map->get_yabove()) + correction + correcty;
  gwin->get_win()->fill8(255, 1, 5, xx, yy - 2);
  gwin->get_win()->fill8(255, 5, 1, xx - 2, yy);
  
  gwin->show(1);
  if (!Get_click(xx, yy, Mouse::greenselect)) {
    gwin->paint();
    return;
  }
  
  tx = ((xx - correctx - (border + x - map->get_xleft()))*worldsize) / (map->get_width() - 2*border + correctscale);
  ty = ((yy - correcty - (border + y - map->get_yabove()))*worldsize) / (map->get_height() - 2*border + correctscale);
					// World-wrapping.
  tx = (tx + c_num_tiles)%c_num_tiles;
  ty = (ty + c_num_tiles)%c_num_tiles;
  cout << "Teleporting to " << tx << "," << ty << "!" << endl;
  Tile_coord t(tx,ty,0);
  gwin->teleport_party(t);
  gwin->center_text("Teleport!!!");
}

void Cheat::cursor_teleport (void) const {
  if (!enabled) return;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x = x>>scale;
  y = y>>scale;
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
    gwin->get_main_actor()->add(new Ireg_game_object(current_shape, current_frame, 0, 0), 1);
    gwin->center_text("Object created");
  } else
    gwin->center_text("Can only create from 'shapes.vga'");
}

void Cheat::delete_object (void) const {
  if (!enabled) return;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x = x>>scale;
  y = y>>scale;
  Game_object *obj = gwin->find_object(x, y);
  if (obj) {
    obj->remove_this();
    gwin->center_text("Object deleted");
    gwin->paint();
  }
}

void Cheat::heal_party (void) const {
  if (!enabled) return;

  // resurrect dead party members
  Dead_body *bodies[9];
  int count = Dead_body::find_dead_companions(bodies);
  for (int i = 0; i < count; i++) {
    int npc_num = bodies[i]->get_live_npc_num();
    if (npc_num < 0)
      continue;
    Actor *live_npc = gwin->get_npc(npc_num);
    if (live_npc)
      live_npc->resurrect(bodies[i]);;
  }

  // heal everyone
  Actor* party[9];
  count = gwin->get_party(party, 1);
  for (int i = 0; i < count; i++) {
    if (!party[i]->is_dead()) {
      // heal
      party[i]->set_property(Actor::health, party[i]->get_property(Actor::strength));
      // cure poison
      party[i]->clear_flag(Obj_flags::poisoned);

      // remove hunger  +++++ what is "normal" food level??
      party[i]->set_property(Actor::food_level, 30);
    }
  }  
 
  gwin->center_text("Party healed");
  gwin->paint();
}

void Cheat::shape_browser (void) const {
  if (!enabled) return;

  browser->browse_shapes();
  gwin->paint();
  gwin->set_palette(-1,-1);
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

void Cheat::toggle_number_npcs (void) {
	if (!enabled) return;

	npc_numbers = !npc_numbers;
	if (npc_numbers)
		gwin->center_text("NPC Numbers Enabled");
	else
		gwin->center_text("NPC Numbers Disabled");
}
