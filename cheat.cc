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

#include <iostream>

#include "cheat.h"
#include "gamewin.h"
#include "Configuration.h"
#include "game.h"
#include "actors.h"
#include "mouse.h"
#include "browser.h"

extern Configuration* config;
extern int scale;
extern ShapeBrowser* browser;

int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);


Cheat::Cheat() {
  enabled = false;

  god_mode = false;
  wizard_mode = false;
  hack_mover = false;
  infravision = false;
}

Cheat::~Cheat() {

}

void Cheat::init (void) {
  string cheating;

  gwin = Game_window::get_game_window();

  config->value("config/gameplay/cheat",cheating,"yes");
  enabled = true;
  if (cheating == "no")
    enabled = false;

  if (enabled)
    cout << "Cheats enabled." << endl;
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

void Cheat::toggle_hack_mover (void) {
  if (!enabled) return;

  hack_mover = !hack_mover;
  if (hack_mover)
    gwin->center_text("Hack-mover Enabled");
  else
    gwin->center_text("Hack-mover Disabled");			
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

void Cheat::change_gender (void) {
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

void Cheat::toggle_eggs (void) {
  if (!enabled) return;

  gwin->paint_eggs = !gwin->paint_eggs;
  if(gwin->paint_eggs)
    gwin->center_text("Eggs display enabled");
  else
    gwin->center_text("Eggs display disabled");
  gwin->paint();
}

void Cheat::toggle_Petra (void) {
  if (!enabled || (Game::get_game_type() != SERPENT_ISLE)) return;

  if (gwin->get_main_actor()->get_flag(Actor::petra))
    gwin->get_main_actor()->clear_flag(Actor::petra);
  else
    gwin->get_main_actor()->set_flag(Actor::petra);
  gwin->set_all_dirty();
}

void Cheat::toggle_naked (void) {
  if (!enabled || (Game::get_game_type() != SERPENT_ISLE)) return;

  if (gwin->get_main_actor()->get_siflag(Actor::naked))
    gwin->get_main_actor()->clear_siflag(Actor::naked);
  else
    gwin->get_main_actor()->set_siflag(Actor::naked);
  gwin->set_all_dirty();
}

void Cheat::change_skin (void) {
  if (!enabled || (Game::get_game_type() != SERPENT_ISLE)) return;

  int color = gwin->get_main_actor()->get_skin_color();
  
  if (color < 0 || color > 2)
    return;
  color = (color + 4) %3;
  gwin->get_main_actor()->set_skin_color(color);
  gwin->set_all_dirty();
}

void Cheat::fake_time_period (void) {
  if (!enabled) return;

  gwin->fake_next_period();
  gwin->center_text("Game clock incremented");
}

void Cheat::dec_skip_lift (void) {
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

void Cheat::map_teleport (void) {
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
  
  //the 5 and 10 below are the map-borders, 3072 dimensions of the world
  //the +1 _seems_ to improve location, maybe something to do with "/ 3072"?
  xx = ((tx * (map->get_width() - 10)) / 3072) + (5 + x - map->get_xleft()) + 1;
  yy = ((ty * (map->get_height() - 10)) / 3072) + (5 + y - map->get_yabove()) + 1;
  gwin->get_win()->fill8(255, 1, 5, xx, yy - 2);
  gwin->get_win()->fill8(255, 5, 1, xx - 2, yy); // ++++ is this the right yellow?
  
  gwin->show(1);
  if (!Get_click(xx, yy, Mouse::greenselect)) {
    gwin->paint();
    return;
  }
  
  //the 5 and 10 below are the map-borders, 3072 dimensions of the world
  tx = ((xx - (5 + x - map->get_xleft()))*3072) / (map->get_width() - 10);
  ty = ((yy - (5 + y - map->get_yabove()))*3072) / (map->get_height() - 10);
  cout << "Teleporting to " << tx << "," << ty << "!" << endl;
  Tile_coord t(tx,ty,0);
  gwin->teleport_party(t);
  gwin->center_text("Teleport!!!");
}

void Cheat::cursor_teleport (void) {
  if (!enabled) return;

  int x, y;
  SDL_GetMouseState(&x, &y);
  x = x>>scale;
  y = y>>scale;
  Tile_coord t(gwin->get_scrolltx() + x/tilesize,
	       gwin->get_scrollty() + y/tilesize, 0);
  gwin->teleport_party(t);
  gwin->center_text("Teleport!!!");
}

void Cheat::create_coins (void) {
  if (!enabled) return;

  gwin->get_main_actor()->add_quantity(100, 644);
  gwin->center_text("Added 100 gold coins");
}


void Cheat::create_last_shape (void) {
  if (!enabled) return;

  int current_shape = 0;
  int current_frame = 0;
  if(browser->get_shape(current_shape, current_frame)) {
    gwin->get_main_actor()->add(new Ireg_game_object(current_shape, current_frame, 0, 0), 1);
    gwin->center_text("Object created");
  } else
    gwin->center_text("Can only create from 'shapes.vga'");
}

void Cheat::delete_object (void) {
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

void Cheat::shape_browser (void) {
  if (!enabled) return;

  browser->browse_shapes();
  gwin->paint();
  gwin->set_palette(-1,-1);
}
