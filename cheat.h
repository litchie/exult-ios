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

#include "gamewin.h"

class Cheat {
 public:
  Cheat();
  ~Cheat();

  void init (void);

private:
  Game_window *gwin;

  bool god_mode;
  bool wizard_mode;
  bool hack_mover;
  bool infravision;

  bool enabled;

public:
  bool operator() (void) const { return enabled; }

  bool in_god_mode (void) const { return god_mode; }
  bool in_wizard_mode (void) const { return wizard_mode; }
  bool in_hack_mover (void) const { return hack_mover; }
  bool in_infravision (void) const { return infravision; }
  
  void toggle_god (void);
  void toggle_wizard (void);
  void toggle_hack_mover (void);
  void toggle_infravision (void);

  void toggle_eggs (void);
  void change_gender (void);

  void toggle_Petra (void);
  void toggle_naked (void);
  void change_skin (void);

  void fake_time_period (void);
  void dec_skip_lift (void);

  void map_teleport (void);
  void cursor_teleport (void);

  void create_coins (void);
  void create_last_shape (void);
  void delete_object (void);
  void shape_browser (void);
};

#endif
