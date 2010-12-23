/*
 *  Copyright (C) 2001-2010 The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "keyactions.h"
#include "keys.h"
#include "gump_utils.h"
#include "gamewin.h"
#include "gamemap.h"
#include "mouse.h"
#include "actors.h"
#include "game.h"
#include "exult.h"
#include "exult_types.h"
#include "exult_constants.h"
#include "File_gump.h"
#include "Scroll_gump.h"
#include "cheat.h"
#include "combat_opts.h"
#include "ucmachine.h"
#include "party.h"
#include "Audio.h"
#include "Gamemenu_gump.h"
#include "Newfile_gump.h"
#include "Face_stats.h"
#include "Notebook_gump.h"
#include "Gump_manager.h"
#include "effects.h"
#include "palette.h"

#ifdef UNDER_CE
  #include "Keyboard_gump.h"
  #include "Touchscreen.h"
#endif

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
		npc_num = Game_window::get_instance()->get_party_man()->get_member(num - 1);
	return Game_window::get_instance()->get_npc(npc_num);
}


//  { ActionQuit, 0, "Quit", normal_keys, NONE },
void ActionQuit(int *params)
{
	Game_window::get_instance()->get_gump_man()->okay_to_quit();
}

// { ActionOldFileGump, 0, "Save/restore", normal_keys, NONE },
void ActionOldFileGump(int *params)
{
	File_gump *fileio = new File_gump();
	Game_window::get_instance()->get_gump_man()->do_modal_gump(fileio, 
															   Mouse::hand);
	delete fileio;
}

#ifdef UNDER_CE
// { ActionMinimizeGame, 0, "Minimize game", normal_keys, NONE },
void ActionMinimizeGame(int *params)
{
	//gwin->clear_screen(true);
	minimized = true;
	SDL_WM_IconifyWindow();
	SDL_Delay(250);
	SDL_WM_IconifyWindow();
}

// { ActionTouchscreenMode, 0, "Touchscreen mode", normal_keys, NONE },
void ActionTouchscreenMode(int *params)
{
	int Right,Double;
	Touchscreen->getModes(&Right, &Double);
	if (params[0] == Right)
		params[0] = 0;
	if (params[1] == Double)
		params[1] = 0;
	Touchscreen->setModes(params[0], params[1]);
}

// { "KEYBOARD_POSITION", ActionKeyboardPosition, 0, "Keyboard position", normal_keys, NONE, true },
void ActionKeyboardPosition(int *params)
{
	// 0 = upper left, 1 = upper right, 2 = lower left, 3 = lower right
	int corner = params[0];
	if (corner == -1)
		corner = gkeyboard->getCorner() + 1;
	if (corner > 3 || corner < 0)
		corner = 0;

	gkeyboard->show(corner, -1);
}

// { "KEYBOARD_MODE", ActionKeyboardMode, 0, "Keyboard mode", normal_keys, NONE, true },
void ActionKeyboardMode(int *params)
{
	int newstate = params[0];
	int curstate = gkeyboard->getState();
	if (newstate == curstate)
	{
		if (newstate == KEYG_HIDDEN)
			newstate = -1;
		else
			newstate = KEYG_MINIMIZED;
	}
	else if (newstate == -1)
	{
		newstate = curstate + 1;
		(newstate > KEYG_MINIMIZED) && (newstate = KEYG_KEYBOARD);
	}
	else if (newstate < -1 || newstate > KEYG_HIDDEN)
	{
		newstate = KEYG_KEYBOARD;
	}
	switch(newstate)
	{
		case KEYG_MINIMIZED:	gkeyboard->minimize();			break;
		case KEYG_HIDDEN:		gkeyboard->hide();				break;
		default:				gkeyboard->show(-1, newstate);	break;
	}
}

#endif

// { ActionMenuGump, 0, "GameMenu", normal_keys, NONE },
void ActionMenuGump(int *params)
{
	Gamemenu_gump *gmenu = new Gamemenu_gump();
	Game_window::get_instance()->get_gump_man()->do_modal_gump(gmenu,
															   Mouse::hand);
	delete gmenu;
}

// { ActionFileGump, 0, "Save/restore", normal_keys, NONE },
void ActionFileGump(int *params)
{
	Newfile_gump *fileio = new Newfile_gump();
	Game_window::get_instance()->get_gump_man()->do_modal_gump(fileio,
															   Mouse::hand);
	delete fileio;
}
//  { ActionQuicksave, 0, "Quick-save", normal_keys, NONE },
void ActionQuicksave(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	try {
		gwin->write();
	}
	catch(exult_exception &/*e*/) {
		gwin->get_effects()->center_text("Saving game failed!");
		return;
	}
	gwin->get_effects()->center_text("Game saved");
}

//  { ActionQuickrestore, 0, "Quick-restore", normal_keys, NONE },
void ActionQuickrestore(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	try {
		gwin->read();
	}
	catch(exult_exception &/*e*/) {
		gwin->get_effects()->center_text("Restoring game failed!");
		return;
	}
	gwin->get_effects()->center_text("Game restored");
	gwin->paint();
}

//  { ActionAbout, 0, "About Exult", normal_keys, NONE },
void ActionAbout(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	Scroll_gump *scroll;
	scroll = new Scroll_gump();
	
	scroll->add_text("Exult V"VERSION"\n");
	scroll->add_text("(C) 1998-2010 Exult Team\n\n");
	scroll->add_text("Available under the terms of the ");
	scroll->add_text("GNU General Public License\n\n");
	scroll->add_text("http://exult.sourceforge.net\n");
	
	scroll->paint();
	do {
		int x, y;
		Get_click(x,y, Mouse::hand);
	} while (scroll->show_next_page());
	gwin->paint();
	delete scroll;
}

//  { ActionHelp, 0, "List keys", normal_keys, NONE },
void ActionHelp(int *params)
{
	keybinder->ShowHelp();
}

//  { ActionCloseGumps, 0, "Close gumps", dont_show, NONE },
void ActionCloseGumps(int *params)
{
	Game_window::get_instance()->get_gump_man()->close_all_gumps();
}

//  { ActionCloseOrMenu, "Game menu", normal_keys, NONE },
void ActionCloseOrMenu(int* params)
{
	Game_window *gwin = Game_window::get_instance();
	if (gwin->get_gump_man()->showing_gumps(true))
		gwin->get_gump_man()->close_all_gumps();
	else
		ActionMenuGump(0);
}

//  { ActionScreenshot, 0, "Take screenshot", normal_keys, NONE },
void ActionScreenshot(int *params)
{
	make_screenshot();
}

//  { ActionRepaint, 0, "Repaint screen", dont_show, NONE },
void ActionRepaint(int *params)
{
	Game_window::get_instance()->paint();
}

//  { ActionResIncrease, 0, "Increase resolution", cheat_keys, NONE },
void ActionResIncrease(int *params)
{
	increase_resolution();
}

//  { ActionResDecrease, 0, "Decrease resolution", cheat_keys, NONE },
void ActionResDecrease(int *params)
{
	decrease_resolution();
}

//  { ActionBrighter, 0, "Increase brightness", normal_keys, NONE },
void ActionBrighter(int *params)
{
	change_gamma(true);
}

//  { ActionDarker, 0, "Decrease brightness", normal_keys, NONE },
void ActionDarker(int *params)
{
	change_gamma(false);
}

//  { ActionFullscreen, 0, "Toggle fullscreen", normal_keys, NONE },
void ActionFullscreen(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	gwin->get_win()->toggle_fullscreen();
#ifdef HAVE_OPENGL
	if (GL_manager::get_instance())
		Set_renderer(gwin->get_win(), gwin->get_pal(), true);
#endif
	gwin->paint();
}

//  { ActionUseItem, 3, "Use item", dont_show, NONE },
// params[0] = shape nr.
// params[1] = frame nr.
// params[2] = quality
void ActionUseItem(int *params)
{
	if (params[1] == -1) params[1] = c_any_framenum;
	if (params[2] == -1) params[2] = c_any_qual;
	Game_window::get_instance()->activate_item(params[0], params[1], params[2]);

	Mouse::mouse->set_speed_cursor();
}

//  { ActionUseItem, 3, "Use food", dont_show, NONE },
void ActionUseFood(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	if (!gwin->activate_item(377) &&	// First try normal food items.
	    GAME_SI)			// SI has 'everlasting goblet'.
		gwin->activate_item(0x268, 20);
	Mouse::mouse->set_speed_cursor();
}

//  { ActionCallUsecode, 2, "Call usecode", dont_show, NONE },
// params[0] = usecode function.
// params[1] = event id.
void ActionCallUsecode(int *params)
{
	Usecode_machine *usecode = Game_window::get_instance()->get_usecode();
	usecode->call_usecode(params[0], (Game_object *)0,
		static_cast<Usecode_machine::Usecode_events>(params[1]));

	Mouse::mouse->set_speed_cursor();
}

//  { ActionCombat, 0, "Toggle combat", normal_keys, NONE },
void ActionCombat(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	gwin->toggle_combat();
	gwin->paint();
	Mouse::mouse->set_speed_cursor();
}

//  { ActionCombatPause, 0, "Pause combat", normal_keys, NONE },
void ActionCombatPause(int *params)
{
	Combat::toggle_pause();
	Mouse::mouse->set_speed_cursor();
}

//  { ActionTarget, 0, "Target mode", normal_keys, NONE },
void ActionTarget(int *params)
{
	int x, y;
	if (!Get_click(x, y, Mouse::greenselect))
		return;
	Game_window::get_instance()->double_clicked(x, y);
	Mouse::mouse->set_speed_cursor();
}

//  { ActionInventory, 1, "Show inventory", normal_keys, NONE },
// params[0] = party member (0-7), or -1 for 'next'
void ActionInventory(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	static int inventory_page = -1;
	Actor *actor;
	
	if (params[0] == -1) {
		Gump_manager *gump_man = gwin->get_gump_man();
		int party_count = gwin->get_party_man()->get_count();
		int shapenum;
		
		for(int i=0;i<=party_count;++i)
		{
			actor = Get_party_member(i);
			if (!actor)
				continue;
			bool skip;
			shapenum = actor->inventory_shapenum();
			// Check if this actor's inventory page is open or not
			if (!gump_man->find_gump(actor, shapenum) && skip)
			{
				skip = false;
				if (actor->can_act_charmed()){
					gump_man->add_gump(actor, shapenum, true); //force showing inv.
					inventory_page = i;
					return;
				}else
					skip = true;
			}
		}
		inventory_page = (inventory_page+1)%(party_count+1);
	} else {
		inventory_page = params[0];
		if (inventory_page < 0 ||
				inventory_page > gwin->get_party_man()->get_count())
			return;
	}
	
	actor = Get_party_member(inventory_page);
	if (actor && actor->can_act_charmed()) {
		actor->show_inventory(); //force showing inv.
	}

	Mouse::mouse->set_speed_cursor();
}

//  { ActionTryKeys, 0, "Try keys", normal_keys, NONE },
void ActionTryKeys(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int x, y;			// Allow dragging.
	if (!Get_click(x, y, Mouse::greenselect, 0, true))
		return;
	// Look for obj. in open gump.
	Gump *gump = gwin->get_gump_man()->find_gump(x, y);
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
	for (int i = 0; i < party_cnt; i++) {
		Actor *act = party[i];
		Game_object_vector keys;		// Get keys.
		if (act->get_objects(keys, 641, qual, c_any_framenum))
			{
			for (size_t i = 0; i < keys.size(); i++)
				if (!keys[i]->inside_locked())
					{
					// intercept the click_on_item call made by the key-usecode
					Usecode_machine *ucmachine = gwin->get_usecode();
					Game_object *oldtarg;
					Tile_coord *oldtile;
					ucmachine->save_intercept(oldtarg, oldtile);
					ucmachine->intercept_click_on_item(obj);
					keys[0]->activate();
					ucmachine->restore_intercept(oldtarg, oldtile);
					return;
					}
			}
	}
	Mouse::mouse->flash_shape(Mouse::redx);	// Nothing matched.
}

//  { ActionStats, 1, "Show stats", normal_keys, NONE },
// params[0] = party member (0-7), or -1 for 'next'
void ActionStats(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	static int stats_page = -1;
	Actor *actor;
	
	if (params[0] == -1) {
		Gump_manager *gump_man = gwin->get_gump_man();
		int party_count = gwin->get_party_man()->get_count();
		int shapenum = game->get_shape("gumps/statsdisplay");

		for(int i=0;i<=party_count;++i)
		{
			actor = Get_party_member(i);
			if (!actor)
				continue;

			// Check if this actor's stats page is open or not
			if (!gump_man->find_gump(actor, shapenum))
			{
				gump_man->add_gump(actor, shapenum); //force showing stats.
				stats_page = i;
				return;
			}
		}
		stats_page = (stats_page+1)%(party_count+1);
	} else {
		stats_page = params[0];
		if (stats_page < 0 ||
			stats_page >= gwin->get_party_man()->get_count())
			stats_page = 0;
	}
	
	actor = Get_party_member(stats_page);
	if (actor) gwin->get_gump_man()->add_gump(actor, game->get_shape("gumps/statsdisplay"));

	Mouse::mouse->set_speed_cursor();
}

//  { ActionCombatStats, 0, "Show Combat stats", normal_keys, SERPENT_ISLE }
void ActionCombatStats(int* params)
{
	Game_window *gwin = Game_window::get_instance();
	int cnt = gwin->get_party_man()->get_count();
	gwin->get_gump_man()->add_gump(0, game->get_shape("gumps/cstats/1") + cnt);
}

//  { ActionFaceStats, 0, "Change Face Stats State", normal_keys, NONE }
void ActionFaceStats(int* params)
{
	Face_stats::AdvanceState();
	Face_stats::save_config(config);
}


//  { ActionSIIntro, 0,  "Show SI intro", cheat_keys, SERPENT_ISLE },
void ActionSIIntro(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	game->set_jive();
	game->play_intro();
	game->clear_jive();
	gwin->clear_screen(true);
	gwin->get_pal()->set(0);
	gwin->paint();
	gwin->get_pal()->fade(50, 1, 0);
}

//  { ActionEndgame, 1, "Show endgame", cheat_keys, BLACK_GATE },
// params[0] = -1,0 = won, 1 = lost
void ActionEndgame(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	game->end_game(params[0] != 1);
	gwin->clear_screen(true);
	gwin->get_pal()->set(0);
	gwin->paint();
	gwin->get_pal()->fade(50, 1, 0);
}

//  { ActionScrollLeft, 0, "Scroll left", cheat_keys, NONE },
void ActionScrollLeft(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	for (int i = 16; i; i--)
		gwin->view_left();
}

//  { ActionScrollRight, 0, "Scroll right", cheat_keys, NONE },
void ActionScrollRight(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	for (int i = 16; i; i--)
		gwin->view_right();
}

//  { ActionScrollUp, 0, "Scroll up", cheat_keys, NONE },
void ActionScrollUp(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	for (int i = 16; i; i--)
		gwin->view_up();
}

//  { ActionScrollDown, 0, "Scroll down", cheat_keys, NONE },
void ActionScrollDown(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	for (int i = 16; i; i--)
		gwin->view_down();
}

int get_walking_speed(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int parm = params ? params[0] : 0;
	int speed;
	if (parm == 2)
		speed = Mouse::slow_speed_factor;
	else if (gwin->in_combat() || gwin->is_hostile_nearby())
		speed = Mouse::medium_combat_speed_factor;
	else
		speed = parm==1 ? Mouse::medium_speed_factor : Mouse::fast_speed_factor;
	return (200*gwin->get_std_delay()/speed);
}

//  { ActionWalkWest, 0, "Walk west", normal_keys, NONE },
void ActionWalkWest(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2-50, gwin->get_height()/2,speed);
}

//  { ActionWalkEast, 0, "Walk east", normal_keys, NONE },
void ActionWalkEast(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2+50, gwin->get_height()/2, speed);
}

//  { ActionWalkNorth, 0, "Walk north", normal_keys, NONE },
void ActionWalkNorth(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2, gwin->get_height()/2-50, speed);
}

//  { ActionWalkSouth, 0, "Walk south", normal_keys, NONE },
void ActionWalkSouth(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2, gwin->get_height()/2+50, speed);
}

//  { ActionWalkNorthEast, 0, "Walk north-east", normal_keys, NONE },
void ActionWalkNorthEast(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2+50, gwin->get_height()/2-50, speed);
}

//  { ActionWalkSouthEast, 0, "Walk south-east", normal_keys, NONE },
void ActionWalkSouthEast(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2+50, gwin->get_height()/2+50, speed);
}

//  { ActionWalkNorthWest, 0, "Walk north-west", normal_keys, NONE },
void ActionWalkNorthWest(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2-50, gwin->get_height()/2-50, speed);
}

//  { ActionWalkSouthWest, 0, "Walk south-west", normal_keys, NONE },
void ActionWalkSouthWest(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	int speed = get_walking_speed(params);
	gwin->start_actor(gwin->get_width()/2-50, gwin->get_height()/2+50, speed);
}

//  { ActionStopWalking, 0, "Stop Walking", cheat_keys, NONE },
void ActionStopWalking(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	gwin->stop_actor();
}

//  { ActionCenter, 0, "Center screen", cheat_keys, NONE },
void ActionCenter(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	gwin->center_view(gwin->get_camera_actor()->get_tile());
	gwin->paint();
}

//  { ActionShapeBrowser, 0, "Shape browser", cheat_keys, NONE },
void ActionShapeBrowser(int *params)
{
	cheat.shape_browser();
}

void ActionShapeBrowserHelp(int* params)
{
	keybinder->ShowBrowserKeys();
}

//  { ActionCreateShape, 3, "Create last shape", cheat_keys, NONE },
// params[0] = shape nr., or -1 for 'last selected shape in browser'
// params[1] = frame nr.
// params[2] = quantity
// params[3] = quality
void ActionCreateShape(int *params)
{
	if (params[0] == -1) {
		cheat.create_last_shape();
	} else {
		Game_window *gwin = Game_window::get_instance();
		if (params[1] == -1) params[1] = 0;
		if (params[2] == -1) params[2] = 1;
		if (params[3] == -1) params[3] = c_any_qual;
		gwin->get_main_actor()->add_quantity(params[2], params[0], params[3], params[1]);
		gwin->get_effects()->center_text("Object created");
	}
}

//  { ActionDeleteObject, 0, "Delete object", cheat_keys, NONE },
void ActionDeleteObject(int *params)
{
	cheat.delete_object();
}

//  { ActionDeleteSelected, "Delete selected", true, ture, NONE, false },
void ActionDeleteSelected(int *params)
{
	cheat.delete_selected();
}

//  { ActionMoveSelected, 3, "Move selected", cheat_keys, NONE },
// params[0] = deltax (tiles)
// params[1] = deltay
// params[2] = deltaz
void ActionMoveSelected(int *params)
{
	cheat.move_selected(params[0], params[1], params[2]);
}

//  { ActionToggleEggs, 0, "Toggle egg display", cheat_keys, NONE },
void ActionToggleEggs(int *params)
{
	cheat.toggle_eggs();
}

//  { ActionGodMode, 1, "Toggle god mode", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionGodMode(int *params)
{
	if (params[0] == -1)
		cheat.toggle_god();
	else
		cheat.set_god(params[0] != 0);
}

//  { ActionGender, 0, "Change gender", cheat_keys, NONE },
void ActionGender(int *params)
{
	cheat.change_gender();
}

//  { ActionCheatHelp, 0, "List cheat keys", cheat_keys, NONE },
void ActionCheatHelp(int *params)
{
	keybinder->ShowCheatHelp();
}

//  { ActionMapeditHelp, 0, "List mapedit keys", mapedit_keys, NONE },
void ActionMapeditHelp(int *params)
{
	keybinder->ShowMapeditHelp();
}

//  { ActionInfravision, 1, "Toggle infravision", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionInfravision(int *params)
{
	if (params[0] == -1)
		cheat.toggle_infravision();
	else
		cheat.set_infravision(params[0] != 0);
}

//  { ActionSkipLift, 1, "Change skiplift", cheat_keys, NONE },
// params[0] = level, or -1 to decrease one
void ActionSkipLift(int *params)
{
	if (params[0] == -1)
		cheat.dec_skip_lift();
	else
		cheat.set_skip_lift(params[0]);
}

//  { ActionLevelup, 1, "Level-up party", cheat_keys, NONE },
// params[0] = nr. of levels (or -1 for one)
void ActionLevelup(int *params)
{
	if (params[0] == -1) params[0] = 1;
	for (int i=0; i < params[0]; i++)
		cheat.levelup_party();
}

//  { ActionMapEditor, 1, "Toggle map-editor mode", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionMapEditor(int *params)
{
	if (params[0] == -1)
		cheat.toggle_map_editor();
	else
		cheat.set_map_editor(params[0] != 0);
}

// { ActionHackMover, "Toggle hack-mover mode", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionHackMover(int *params)
{
	if (params[0] == -1)
		cheat.toggle_hack_mover();
	else
		cheat.set_hack_mover(params[0] != 0);
}

//  { ActionMapTeleport, 0, "Map teleport", cheat_keys, NONE },
void ActionMapTeleport(int *params)
{
	cheat.map_teleport();
}

//  { ActionWriteMiniMap, 0, "Write minimap", cheat_keys, NONE },
void ActionWriteMiniMap(int *params)
{
	Game_map::write_minimap();
}

//  { ActionTeleport, 0, "Teleport to cursor", cheat_keys, NONE },
void ActionTeleport(int *params)
{
	cheat.cursor_teleport();
}

//  { ActionNextMapTeleport, 0, "Teleport to next map", cheat_keys, NONE },
void ActionNextMapTeleport(int *params)
{
	cheat.next_map_teleport();
}

//  { ActionTime, 0, "Next time period", cheat_keys, NONE },
void ActionTime(int *params)
{
	cheat.fake_time_period();
}

//  { ActionWizard, 1, "Toggle archwizard mode", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionWizard(int *params)
{
	if (params[0] == -1)
		cheat.toggle_wizard();
	else
		cheat.set_wizard(params[0] != 0);
}

//  { ActionHeal, 0, "Heal party", cheat_keys, NONE },
void ActionHeal(int *params)
{
	cheat.heal_party();
}

//  { ActionCheatScreen, 0, "Cheat Screen", cheat_keys, NONE },
void ActionCheatScreen(int *params)
{
	cheat.cheat_screen();
}

//  { ActionPickPocket, 1, "Toggle Pickpocket", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionPickPocket(int *params)
{
	if (params[0] == -1)
		cheat.toggle_pickpocket();
	else
		cheat.set_pickpocket(params[0] != 0);
}

//  { ActionPickPocket, 1, "Toggle Pickpocket", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionNPCNumbers(int *params)
{
	if (params[0] == -1)
		cheat.toggle_number_npcs();
	else
		cheat.set_number_npcs(params[0] != 0);
}

//  { ActionGrabActor, 1, "Grab Actor for Cheatscreen", cheat_keys, NONE },
// params[0] = -1 for toggle, 0 for off, 1 for on
void ActionGrabActor(int *params)
{
	if (params[0] == -1)
		cheat.toggle_grab_actor();
	else
		cheat.set_grab_actor(params[0] != 0);
}

//  { ActionCut, "Cut Selected Objects", normal_keys, NONE, true},
void ActionCut(int *params)
{
	cheat.cut(false);
}

//  { ActionCopy, "Copy Selected Objects", normal_keys, NONE, true},
void ActionCopy(int *params)
{
	cheat.cut(true);
}

//  { ActionPaste, "Paste Selected Objects", normal_keys, NONE, true},
void ActionPaste(int *params)
{
	cheat.paste();
}

//  { ActionPlayMusic, 1, "Play song", cheat_keys, NONE },
// params[0] = song nr., or -1 for next, -2 for previous
void ActionPlayMusic(int *params)
{
	static int mnum = 0;

	if (params[0] == -1) {
		Audio::get_ptr()->start_music(mnum++, false);
	} else if (params[0] == -2) {
		if (mnum > 0)
			Audio::get_ptr()->start_music(--mnum, false);
	} else {
		mnum = params[0];
		Audio::get_ptr()->start_music(mnum, false);
	}
}

//  { ActionNaked, 0, "Toggle naked mode", cheat_keys, SERPENT_ISLE },
void ActionNaked(int  *params)
{
	cheat.toggle_naked();
}

//  { ActionPetra, 0, "Toggle Petra mode", cheat_keys, SERPENT_ISLE },
void ActionPetra(int *params)
{
	cheat.toggle_Petra();
}

//  { ActionSkinColour, 0 "Change skin colour", cheat_keys, NONE },
void ActionSkinColour(int *params)
{
	cheat.change_skin();
}

//   { ActionNotebook, 0, "Show notebook", normal_keys, NONE, false },
void ActionNotebook(int *params)
{
	Game_window *gwin = Game_window::get_instance();
	Gump_manager *gman = gwin->get_gump_man();
	Notebook_gump *notes = Notebook_gump::get_instance();
	if (notes)
		gman->remove_gump(notes);	// Want to raise to top.
	else
		notes = Notebook_gump::create();
	gman->add_gump(notes);
	gwin->paint();
}

//  { ActionSoundTester, 0, "Sound tester", cheat_keys, NONE }
void ActionSoundTester(int *params)
{
	cheat.sound_tester();
}

void ActionTest(int *params)
{
}

