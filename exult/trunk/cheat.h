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

#ifndef CHEAT_H
#define CHEAT_H

class Game_window;
class ShapeBrowser;
class SoundTester;
class CheatScreen;
class Actor;

class Cheat
{
 public:
  Cheat();
  ~Cheat();

  void init (void);
  void finish_init (void);

private:
  Game_window *gwin;
  ShapeBrowser *browser;
  SoundTester *tester;
  CheatScreen *cscreen;

  bool god_mode;
  bool wizard_mode;
  bool map_editor;
  bool infravision;
  bool pickpocket;
  bool grab_actor;
  bool npc_numbers;
  bool hack_mover;

  bool enabled;

public:
  bool operator() (void) const { return enabled; }
  void set_enabled(bool en);

  bool in_god_mode (void) const { return god_mode; }
  bool in_wizard_mode (void) const { return wizard_mode; }
  bool in_map_editor(void) const { return map_editor; }
  bool in_infravision (void) const { return infravision; }
  bool in_pickpocket (void) const {return pickpocket; }
  bool in_hack_mover (void) const { return (hack_mover || map_editor); }
  
  void toggle_god (void);
  void set_god (bool god) { god_mode = god; }
  void toggle_wizard (void);
  void set_wizard (bool wizard) { wizard_mode = wizard; }
  void toggle_map_editor (void);
  void set_map_editor (bool map) { map_editor = map; }
  void toggle_infravision (void);
  void set_infravision (bool infra) { infravision = infra; }
  void toggle_pickpocket (void);
  void set_pickpocket (bool pick) { pickpocket = pick; }
  void toggle_hack_mover (void);
  void set_hack_mover (bool hm) { hack_mover = hm; }

  void toggle_eggs (void) const;
  void change_gender (void) const;

  void toggle_Petra (void) const;
  void toggle_naked (void) const;
  void change_skin (void) const;

  void levelup_party (void) const;
  void heal_party (void) const;

  void fake_time_period (void) const;
  void dec_skip_lift (void) const;
  void set_skip_lift (int skip) const;

  void map_teleport (void) const;
  void cursor_teleport (void) const;

  void create_coins (void) const;
  void create_last_shape (void) const;
  void delete_object (void) const;
  void shape_browser (void) const;
  void sound_tester (void) const;

  void cheat_screen (void) const;

  bool grabbing_actor (void) const { return grab_actor; }
  void toggle_grab_actor(void);
  void set_grab_actor(bool grab) { grab_actor = grab; }
  void set_grabbed_actor (Actor *actor) const;

  bool number_npcs (void) const { return npc_numbers; }
  void toggle_number_npcs(void);
  void set_number_npcs(bool num) { npc_numbers = num; }
};

extern Cheat cheat;

#endif
