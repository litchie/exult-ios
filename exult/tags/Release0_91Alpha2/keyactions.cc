/*
 *  Copyright (C) 2001 The Exult Team
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

#include "keys.h"
#include "gump_utils.h"
#include "gamewin.h"
#include "mouse.h"
#include "actors.h"
#include "game.h"
#include "exult_types.h"
#include "exult_constants.h"
#include "File_gump.h"
#include "Scroll_gump.h"
#include "cheat.h"
#include "ucmachine.h"
#include "Audio.h"

extern void increase_resolution();
extern void decrease_resolution();
extern void make_screenshot(bool silent = false);
extern void change_gamma(bool down);
extern void Set_mouse_and_speed(int mx, int my);
extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);

extern Game_window *gwin;
extern Game* game;
extern Cheat cheat;
extern KeyBinder* keybinder;

/*
 *	Get the i'th party member, with the 0'th being the Avatar.
 */

static Actor *Get_party_member
(
 int num				// 0=avatar.
)
{
  int npc_num = 0;	 	// Default to Avatar
  if (num > 0)
    npc_num = gwin->get_usecode()->get_party_member(num - 1);
  return gwin->get_npc(npc_num);
}


//  { ActionQuit, 0, "Quit", true, false, NONE },
void ActionQuit(int *params)
{
  Okay_to_quit();
}

// { ActionFileGump, 0, "Save/restore", true, false, NONE },
void ActionFileGump(int *params)
{
  File_gump *fileio = new File_gump();
  Do_Modal_gump(fileio, Mouse::hand);
  delete fileio;
}

//  { ActionQuicksave, 0, "Quick-save", true, false, NONE },
void ActionQuicksave(int *params)
{
  try
  {
    gwin->write();
  }
  catch(...)
  {
    gwin->center_text("Saving game failed!");
    return;
  }
  gwin->center_text("Game saved");
}

//  { ActionQuickrestore, 0, "Quick-restore", true, false, NONE },
void ActionQuickrestore(int *params)
{
  try
  {
    gwin->read();
  }
  catch(...)
  {
    gwin->center_text("Restoring game failed!");
    return;
  }
  gwin->center_text("Game restored");
  gwin->paint();
}

//  { ActionAbout, 0, "About Exult", true, false, NONE },
void ActionAbout(int *params)
{
  Scroll_gump *scroll;
  scroll = new Scroll_gump();
  
  scroll->add_text("Exult V"VERSION"\n");
  scroll->add_text("(C) 1998-2001 Exult Team\n\n");
  scroll->add_text("Available under the terms of the ");
  scroll->add_text("GNU General Public License\n\n");
  scroll->add_text("http://exult.sourceforge.net\n");
  
  scroll->paint(gwin);
  do
  {
    int x, y;
    Get_click(x,y, Mouse::hand);
  } while (scroll->show_next_page(gwin));
  gwin->paint();
  delete scroll;
}

//  { ActionHelp, 0, "List keys", true, false, NONE },
void ActionHelp(int *params)
{
  keybinder->ShowHelp();
}

//  { ActionCloseGumps, 0, "Close gumps", false, false, NONE },
void ActionCloseGumps(int *params)
{
  gwin->end_gump_mode();
}

//  { ActionScreenshot, 0, "Take screenshot", true, false, NONE },
void ActionScreenshot(int *params)
{
  make_screenshot();
}

//  { ActionRepaint, 0, "Repaint screen", false, false, NONE },
void ActionRepaint(int *params)
{
  gwin->paint();
}

//  { ActionResIncrease, 0, "Increase resolution", true, true, NONE },
void ActionResIncrease(int *params)
{
  increase_resolution();
}

//  { ActionResDecrease, 0, "Decrease resolution", true, true, NONE },
void ActionResDecrease(int *params)
{
  decrease_resolution();
}

//  { ActionBrighter, 0, "Increase brightness", true, false, NONE },
void ActionBrighter(int *params)
{
  change_gamma(true);
}

//  { ActionDarker, 0, "Decrease brightness", true, false, NONE },
void ActionDarker(int *params)
{
  change_gamma(false);
}

//  { ActionFullscreen, 0, "Toggle fullscreen", true, false, NONE },
void ActionFullscreen(int *params)
{
  gwin->get_win()->toggle_fullscreen();
  gwin->paint();
}

//  { ActionUseItem, 1, "Use item", false, false, NONE },
// params[0] = shape nr.
void ActionUseItem(int *params)
{
  gwin->activate_item(params[0]);
}

//  { ActionCombat, 0, "Toggle combat", true, false, NONE },
void ActionCombat(int *params)
{
  gwin->toggle_combat();
  gwin->paint();
  int mx, my;			// Update mouse.
  SDL_GetMouseState(&mx, &my);
  Set_mouse_and_speed(mx, my);
}

//  { ActionTarget, 0, "Target mode", true, false, NONE },
void ActionTarget(int *params)
{
  int x, y;
  if (!Get_click(x, y, Mouse::greenselect))
    return;
  gwin->double_clicked(x, y);
  if (gwin->get_mode() == Game_window::gump)
    Mouse::mouse->set_shape(Mouse::hand);
}

//  { ActionInventory, 1, "Show inventory", true, false, NONE },
// params[0] = party member (0-7), or -1 for 'next'
void ActionInventory(int *params)
{
  static int inventory_page = -1;

  if (params[0] == -1) {
    if (gwin->get_mode() != Game_window::gump)
      inventory_page = -1;
    if(inventory_page<gwin->get_usecode()->get_party_count())
      ++inventory_page;
    else
      inventory_page = 0;
  } else {
    inventory_page = params[0];
    if (inventory_page < 0 || inventory_page >= gwin->get_usecode()->get_party_count())
      inventory_page = 0;
  }
  
  Actor *actor = Get_party_member(inventory_page);
  if (actor)
    actor->activate(gwin->get_usecode());
  if (gwin->get_mode() == Game_window::gump)
    Mouse::mouse->set_shape(Mouse::hand);
}

//  { ActionTryKeys, 0, "Try keys", true, false, NONE },
void ActionTryKeys(int *params)
{
  int x, y;
  if (!Get_click(x, y, Mouse::greenselect))
    return;
  // Look for obj. in open gump.
  Gump *gump = gwin->find_gump(x, y);
  Game_object *obj;
  if (gump)
    obj = gump->find_object(x, y);
  else				// Search rest of world.
    obj = gwin->find_object(x, y);
  if (!obj)
    return;
  int qual = obj->get_quality();	// Key quality should match.
  Actor *party[10];		// Get ->party members.
  int party_cnt = gwin->get_party(&party[0], 1);
  for (int i = 0; i < party_cnt; i++)
  {
    Actor *act = party[i];
    Game_object_vector keys;		// Get keys.
    if (act->get_objects(keys, 641, qual, c_any_framenum))
    {
      keys[0]->activate(gwin->get_usecode());
      return;
    }
  }
  Mouse::mouse->flash_shape(Mouse::redx);	// Nothing matched.
}

//  { ActionStats, 1, "Show stats", true, false, NONE },
// params[0] = party member (0-7), or -1 for 'next'
void ActionStats(int *params)
{
  static int stats_page = -1;
  
  if (params[0] == -1) {
    if (gwin->get_mode() != Game_window::gump)
      stats_page = -1;
    if (stats_page < gwin->get_usecode()->get_party_count())
      ++stats_page;
    else
      stats_page = 0;
  } else {
    stats_page = params[0];
    if (stats_page < 0 || stats_page >= gwin->get_usecode()->get_party_count())
      stats_page = 0;
  }

  Actor *actor = Get_party_member(stats_page);
  if (actor)
    gwin->show_gump(actor, game->get_shape("gumps/statsdisplay"));
  if (gwin->get_mode() == Game_window::gump)
    Mouse::mouse->set_shape(Mouse::hand);
}

//  { ActionSIIntro, 0,  "Show SI intro", true, true, SERPENT_ISLE },
void ActionSIIntro(int *params)
{
  game->set_jive();
  game->play_intro();
  game->clear_jive();
  gwin->set_palette(0);
  gwin->paint();
  gwin->fade_palette (50, 1, 0);
}

//  { ActionEndgame, 1, "Show endgame", true, true, BLACK_GATE },
// params[0] = -1,0 = won, 1 = lost
void ActionEndgame(int *params)
{
  game->end_game(params[0] != 1);
  gwin->set_palette(0);
  gwin->paint();
  gwin->fade_palette (50, 1, 0);
}

//  { ActionScrollLeft, 0, "Scroll left", true, true, NONE },
void ActionScrollLeft(int *params)
{
  for (int i = 16; i; i--)
    gwin->view_left();
}

//  { ActionScrollRight, 0, "Scroll right", true, true, NONE },
void ActionScrollRight(int *params)
{
  for (int i = 16; i; i--)
    gwin->view_right();
}

//  { ActionScrollUp, 0, "Scroll up", true, true, NONE },
void ActionScrollUp(int *params)
{
  for (int i = 16; i; i--)
    gwin->view_up();
}

//  { ActionScrollDown, 0, "Scroll down", true, true, NONE },
void ActionScrollDown(int *params)
{
  for (int i = 16; i; i--)
    gwin->view_down();
}

//  { ActionCenter, 0, "Center screen", true, true, NONE },
void ActionCenter(int *params)
{
  gwin->center_view(gwin->get_main_actor()->get_abs_tile_coord());
  gwin->paint();
}

//  { ActionShapeBrowser, 0, "Shape browser", true, true, NONE },
void ActionShapeBrowser(int *params)
{
  cheat.shape_browser();
}

//  { ActionCreateShape, 3, "Create last shape", true, true, NONE },
// params[0] = shape nr., or -1 for 'last selected shape in browser'
// params[1] = frame nr.
// params[2] = quantity
// params[3] = quality
void ActionCreateShape(int *params)
{
  if (params[0] == -1) {
    cheat.create_last_shape();
  } else {
    if (params[1] == -1) params[1] = 0;
    if (params[2] == -1) params[2] = 1;
    if (params[3] == -1) params[3] = c_any_qual;
    gwin->get_main_actor()->add_quantity(params[2], params[0], params[3], params[1]);
    gwin->center_text("Object created");
  }
}
 
//  { ActionDeleteObject, 0, "Delete object", true, true, NONE },
void ActionDeleteObject(int *params)
{
  cheat.delete_object();
}

//  { ActionToggleEggs, 0, "Toggle egg display", true, true, NONE },
void ActionToggleEggs(int *params)
{
  cheat.toggle_eggs();
}

//  { ActionGodMode, 1, "Toggle god mode", true, true, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionGodMode(int *params)
{
  if (params[0] == -1)
    cheat.toggle_god();
  else
    cheat.set_god(params[0] != 0);
}

//  { ActionGender, 0, "Change gender", true, true, NONE },
void ActionGender(int *params)
{
  cheat.change_gender();
}

//  { ActionCheatHelp, 0, "List cheat keys", true, true, NONE },
void ActionCheatHelp(int *params)
{
  keybinder->ShowCheatHelp();
}

//  { ActionInfravision, 1, "Toggle infravision", true, true, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionInfravision(int *params)
{
  if (params[0] == -1)
    cheat.toggle_infravision();
  else
    cheat.set_infravision(params[0] != 0);
}

//  { ActionSkipLift, 1, "Change skiplift", true, true, NONE },
// params[0] = level, or -1 to decrease one
void ActionSkipLift(int *params)
{
  if (params[0] == -1)
    cheat.dec_skip_lift();
  else
    cheat.set_skip_lift(params[0]);
}

//  { ActionLevelup, 1, "Level-up party", true, true, NONE },
// params[0] = nr. of levels (or -1 for one)
void ActionLevelup(int *params)
{
  if (params[0] == -1) params[0] = 1;
  for (int i=0; i < params[0]; i++)
    cheat.levelup_party();
}

//  { ActionMapEditor, 1, "Toggle map-editor mode", true, true, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionMapEditor(int *params)
{
  if (params[0] == -1)
    cheat.toggle_map_editor();
  else
    cheat.set_map_editor(params[0] != 0);
}

//  { ActionMapTeleport, 0, "Map teleport", true, true, NONE },
void ActionMapTeleport(int *params)
{
  cheat.map_teleport();
}

//  { ActionTeleport, 0, "Teleport to cursor", true, true, NONE },
void ActionTeleport(int *params)
{
  cheat.cursor_teleport();
}

//  { ActionTime, 0, "Next time period", true, true, NONE },
void ActionTime(int *params)
{
  cheat.fake_time_period();
}

//  { ActionWizard, 1, "Toggle archwizard mode", true, true, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionWizard(int *params)
{
  if (params[0] == -1)
    cheat.toggle_wizard();
  else
    cheat.set_wizard(params[0] != 0);
}

//  { ActionHeal, 0, "Heal party", true, true, NONE },
void ActionHeal(int *params)
{
  cheat.heal_party();
}

//  { ActionPlayMusic, 1, "Play song", false, true, NONE },
// params[0] = song nr., or -1 for next, -2 for previous
void ActionPlayMusic(int *params)
{
  static int mnum = 0;

  if (params[0] == -1) {
    Audio::get_ptr()->start_music(mnum++, 0);
  } else if (params[0] == -2) {
    if (mnum > 0)
      Audio::get_ptr()->start_music(--mnum, 0);
  } else {
    mnum = params[0];
    Audio::get_ptr()->start_music(mnum, 0);
  }
}

//  { ActionNaked, 0, "Toggle naked mode", true, true, SERPENT_ISLE },
void ActionNaked(int  *params)
{
  cheat.toggle_naked();
}

//  { ActionPetra, 0, "Toggle Petra mode", true, true, SERPENT_ISLE },
void ActionPetra(int *params)
{
  cheat.toggle_Petra();
}

//  { ActionSkinColour, 0 "Change skin colour", true, true, SERPENT_ISLE },
void ActionSkinColour(int *params)
{
  cheat.change_skin();
}

//  { ActionSoundTester, 0, "Sound tester", false, true, NONE }
void ActionSoundTester(int *params)
{
  cheat.sound_tester();
}
