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

  bool god_mode;
  bool wizard_mode;
  bool map_editor;
  bool infravision;

  bool enabled;

public:
  bool operator() (void) const { return enabled; }
  void set_enabled(bool en);

  bool in_god_mode (void) const { return god_mode; }
  bool in_wizard_mode (void) const { return wizard_mode; }
  bool in_map_editor(void) const { return map_editor; }
  bool in_infravision (void) const { return infravision; }
  
  void toggle_god (void);
  void toggle_wizard (void);
  void toggle_map_editor (void);
  void toggle_infravision (void);

  void toggle_eggs (void) const;
  void change_gender (void) const;

  void toggle_Petra (void) const;
  void toggle_naked (void) const;
  void change_skin (void) const;

  void levelup_party (void) const;
  void heal_party (void) const;

  void fake_time_period (void) const;
  void dec_skip_lift (void) const;

  void map_teleport (void) const;
  void cursor_teleport (void) const;

  void create_coins (void) const;
  void create_last_shape (void) const;
  void delete_object (void) const;
  void shape_browser (void) const;
  void sound_tester (void) const;
};

#endif
