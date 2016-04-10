/*
Copyright (C) 2011-2013 The Exult Team

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

#include "SDL_events.h"

#include "fnames.h"
#include "exult.h"
#include "keyactions.h"
#include "gamewin.h"
#include "game.h"
#include "shapeid.h"
#include "actors.h"
#include "Gamemenu_gump.h"
#include "Gump_manager.h"
#include "Gump_button.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "Text_button.h"
#include "cheat.h"
#include "ucmachine.h"
#include "Notebook_gump.h"
#include "party.h"
#include "ignore_unused_variable_warning.h"

using std::cout;
using std::endl;


/*
 * Get the i'th party member, with the 0'th being the Avatar.
 * Needed for opening the inventories
 */
static Actor *Get_party_member(
    int num             // 0=avatar.
) {
	int npc_num = 0;        // Default to Avatar
	if (num > 0)
		npc_num = Game_window::get_instance()->get_party_man()->get_member(num - 1);
	return Game_window::get_instance()->get_npc(npc_num);
} 

/*
 * some buttons should only be there or change appearance
 * when a certain item is in the party's inventory
 */
Game_object *ShortcutBar_gump::is_party_item(
    int shnum,          // Desired shape.
    int frnum,          // Desired frame
    int qual            // Desired quality
) {
	Actor *party[9];        // Get party.
	int cnt = gwin->get_party(party, 1);
	for (int i = 0; i < cnt; i++) {
		Actor *person = party[i];
		Game_object *obj = person->find_item(shnum, qual, frnum);
		if (obj) {
			return obj;
		}
	}
	return NULL;
}

void ShortcutBar_gump::check_for_updates(int shnum){
	if(shnum == 761 || (GAME_SI && (shnum == 485 || shnum == 555))) //spellbook, keyring, jawbone
		has_changed = true;
}

// add dirty region, if dirty
void ShortcutBar_gump::update_gump() {
	if (has_changed) {
		deleteButtons();
		createButtons();
		has_changed = false;
	}
}

/*
 * To align button shapes vertically, we need to micro-manage the shapeOffsetY
 * values to shift shapes up or down.
 */
void ShortcutBar_gump::createButtons()
{
	startx = gwin->get_win()->get_start_x();
	resx = gwin->get_win()->get_full_width();
	gamex = gwin->get_game_width();
	starty = gwin->get_win()->get_start_y();
	resy = gwin->get_win()->get_full_height();
	gamey = gwin->get_game_height();

	int x = (gamex - 320)/2;
	int y = starty + 20;

	memset(buttonItems, 0, sizeof(buttonItems));
	bool trlucent = gwin->get_shortcutbar_type() == 1 && starty >= 0;
	// disk
	buttonItems[0].shapeId = new ShapeID(EXULT_FLX_SB_DISK_SHP, trlucent ? 1 : 0, SF_EXULT_FLX);
	buttonItems[0].name = "disk";
	buttonItems[0].type = SB_ITEM_DISK;
	buttonItems[0].shapeOffsetY = -4;

	// peace/combat
	if (gwin->in_combat())
		buttonItems[1].shapeId = new ShapeID(EXULT_FLX_SB_COMBAT_SHP, trlucent ? 3 : 2, SF_EXULT_FLX);
	else
		buttonItems[1].shapeId = new ShapeID(EXULT_FLX_SB_COMBAT_SHP, trlucent ? 1 : 0, SF_EXULT_FLX);
	buttonItems[1].name = "toggle combat";
	buttonItems[1].type = SB_ITEM_TOGGLE_COMBAT;
	buttonItems[1].shapeOffsetY = 5;

	// map
	if(trlucent) {
		if (GAME_SI) 
			buttonItems[2].shapeId = new ShapeID(EXULT_FLX_SB_MAPS_SHP, 1, SF_EXULT_FLX);
		else
			buttonItems[2].shapeId = new ShapeID(EXULT_FLX_SB_MAPS_SHP, 0, SF_EXULT_FLX);
	} else {
		buttonItems[2].shapeId = new ShapeID(178, 0, SF_SHAPES_VGA);
	}
	buttonItems[2].name = "map";
	buttonItems[2].type = SB_ITEM_MAP;
	buttonItems[2].shapeOffsetY = -1;

	// spellbook
	if (GAME_SI) {
		if (is_party_item(761))
			if(trlucent)
				buttonItems[3].shapeId = new ShapeID(EXULT_FLX_SB_SPELLBOOK_SHP, 2, SF_EXULT_FLX);
			else
				buttonItems[3].shapeId = new ShapeID(761, 0, SF_SHAPES_VGA);
		else {
			if (gwin->sb_hide_missing_items()) {
				buttonItems[3].shapeId = new ShapeID(EXULT_FLX_TRANSPARENTMENU_SHP, 0, SF_EXULT_FLX);
			} else {
				buttonItems[3].shapeId = new ShapeID(EXULT_FLX_SB_SPELLBOOK_SHP, 3, SF_EXULT_FLX);
				buttonItems[3].translucent = 1;
			}
		}
	} else {
		if (is_party_item(761)) {
			if(trlucent)
				buttonItems[3].shapeId = new ShapeID(EXULT_FLX_SB_SPELLBOOK_SHP, 0, SF_EXULT_FLX);
			else
				buttonItems[3].shapeId = new ShapeID(761, 0, SF_SHAPES_VGA);
		} else {
			if (gwin->sb_hide_missing_items()) {
				buttonItems[3].shapeId = new ShapeID(EXULT_FLX_TRANSPARENTMENU_SHP, 0, SF_EXULT_FLX);
			} else {
				buttonItems[3].shapeId = new ShapeID(EXULT_FLX_SB_SPELLBOOK_SHP, 1, SF_EXULT_FLX);
				buttonItems[3].translucent = 1;
			}
		}
	}
	buttonItems[3].name = "spellbook";
	buttonItems[3].type = SB_ITEM_SPELLBOOK;
	buttonItems[3].shapeOffsetY = -4;

	// backpack
	if(trlucent)
		buttonItems[4].shapeId = new ShapeID(EXULT_FLX_SB_BACKPACK_SHP, 0, SF_EXULT_FLX);
	else
		buttonItems[4].shapeId = new ShapeID(801, 0, SF_SHAPES_VGA);
	buttonItems[4].name = "backpack";
	buttonItems[4].type = SB_ITEM_BACKPACK;
	buttonItems[4].shapeOffsetY = -2;

	// key/keyring
	if (is_party_item(485) && GAME_SI) {
		buttonItems[5].shapeId = new ShapeID(EXULT_FLX_SB_KEYRING_SHP, trlucent ? 1 : 0, SF_EXULT_FLX);
		buttonItems[5].name = "keyring";
		buttonItems[5].type = SB_ITEM_KEYRING;
		buttonItems[5].shapeOffsetY = -2;
	} else {
	if(trlucent)
		buttonItems[5].shapeId = new ShapeID(EXULT_FLX_SB_KEY_SHP, 0, SF_EXULT_FLX);
	else
		buttonItems[5].shapeId = new ShapeID(641, 28, SF_SHAPES_VGA);
		buttonItems[5].name = "key";
		buttonItems[5].type = SB_ITEM_KEY;
		buttonItems[5].shapeOffsetY = -1;
	}

	// notebook
	if(trlucent)
		buttonItems[6].shapeId = new ShapeID(EXULT_FLX_SB_NOTEBOOK_SHP, 0, SF_EXULT_FLX);
	else
		buttonItems[6].shapeId = new ShapeID(642, 7, SF_SHAPES_VGA);
	buttonItems[6].name = "notebook";
	buttonItems[6].type = SB_ITEM_NOTEBOOK;
	buttonItems[6].shapeOffsetY = -3;

	// target
	buttonItems[7].shapeId = new ShapeID(EXULT_FLX_SB_TARGET_SHP, trlucent ? 1 : 0, SF_EXULT_FLX);
	buttonItems[7].name = "target";
	buttonItems[7].type = SB_ITEM_TARGET;
	buttonItems[7].shapeOffsetY = -8;

	// feed
	if(trlucent)
		buttonItems[8].shapeId = new ShapeID(EXULT_FLX_SB_FOOD_SHP, 0, SF_EXULT_FLX);
	else if (GAME_SI)
		buttonItems[8].shapeId = new ShapeID(23, 3, SF_GUMPS_VGA);
	else
		buttonItems[8].shapeId = new ShapeID(28, 3, SF_GUMPS_VGA);
	buttonItems[8].name = "feed";
	buttonItems[8].type = SB_ITEM_FEED;
	buttonItems[8].shapeOffsetY = -1;

	// jawbone
	if (GAME_SI) {
		Game_object *jawbone;
		if ((jawbone = is_party_item(555))) {
			if(trlucent)
				buttonItems[9].shapeId = new ShapeID(EXULT_FLX_SB_JAWBONE_SHP, 0, SF_EXULT_FLX);
			else
				buttonItems[9].shapeId = new ShapeID(555, jawbone->get_framenum(), SF_SHAPES_VGA);
		} else {
			if (gwin->sb_hide_missing_items()) {
				buttonItems[9].shapeId = new ShapeID(EXULT_FLX_TRANSPARENTMENU_SHP, 0, SF_EXULT_FLX);
			} else {
				buttonItems[9].shapeId = new ShapeID(EXULT_FLX_SB_JAWBONE_SHP, 1, SF_EXULT_FLX);
				buttonItems[9].translucent = 1;
			}
		}
		buttonItems[9].name = "jawbone";
		buttonItems[9].type = SB_ITEM_JAWBONE;
		buttonItems[9].shapeOffsetY = -1;

		numButtons = 10;
	} else
		numButtons = 9;

	int barItemWidth = (320/numButtons);

	for (int i = 0; i < numButtons; i++, x += barItemWidth) {
		Shape_frame*frame = buttonItems[i].shapeId->get_shape();
		
		int left = (barItemWidth-frame->get_width())/2;
		int top = (height-frame->get_height())/2;
		buttonItems[i].shapeOffsetX += frame->get_xleft() + left;
		buttonItems[i].shapeOffsetY += -5 + top;
		buttonItems[i].rect = new Rectangle(x, y, barItemWidth, height);
		// this is safe to do since it only effects certain palette colors
		// which will be color cycling otherwise
		if (trlucent)
			buttonItems[i].translucent = 1;
	}
}

void ShortcutBar_gump::deleteButtons()
{
	for (int i = 0; i < numButtons; i++) {
		delete buttonItems[i].shapeId;
		delete buttonItems[i].rect;
		buttonItems[i].shapeId = NULL;
		buttonItems[i].rect = NULL;
	}
	startx = 0;
	resx = 0;
	gamex = 0;
	starty = 0;
	resy = 0;
	gamey = 0;
}

/*
 * Construct a shortcut bar gump at the top of screen.
 * Also register it to gump manager.
 * This gump is persistent, not draggable.
 * There must be only one shortcut bar in the game.
 */
ShortcutBar_gump::ShortcutBar_gump(int placex, int placey)
:	Gump(0, placex, placey,/*shape number=*/ EXULT_FLX_TRANSPARENTMENU_SHP, SF_EXULT_FLX)
{
	/*static bool init = false;
	assert(init == 0); // Protect against re-entry
	init = true;*/

	resx = gwin->get_win()->get_full_width();
	width = resx;
	height = 25;
	locx = placex;
	locy = placey;

	createButtons();
	gumpman->add_gump(this);
	has_changed = true;
}

ShortcutBar_gump::~ShortcutBar_gump()
{
	deleteButtons();
	gwin->set_all_dirty();
}

void ShortcutBar_gump::paint()
{
	Game_window *gwin = Game_window::get_instance();
	Shape_manager *sman = Shape_manager::get_instance();

	Gump::paint();

	for (int i = 0; i < numButtons; i++) {
		int x = locx + buttonItems[i].rect->x + buttonItems[i].shapeOffsetX;
		int y = locy + buttonItems[i].rect->y + buttonItems[i].shapeOffsetY;
		sman->paint_shape(x, y, buttonItems[i].shapeId->get_shape(), buttonItems[i].translucent);
		// when the bar is on the game screen it may need an outline
		if (gwin->get_outline_color() < NPIXCOLORS && starty >= 0)
			sman->paint_outline(x, y, buttonItems[i].shapeId->get_shape(), gwin->get_outline_color());
	}

	gwin->set_painted();
}

int ShortcutBar_gump::handle_event(SDL_Event *event)
{
	Game_window *gwin = Game_window::get_instance();
	// When the Save/Load menu is open, don't handle events
	if (gumpman->modal_gump_mode() || gwin->get_usecode()->in_usecode() || g_waiting_for_click)
		return 0;

	for (int i = 0; i < numButtons; i++) {
		if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
			int x, y;
			gwin->get_win()->screen_to_game(event->button.x, event->button.y, gwin->get_fastmouse(), x, y);

#if 0
			std::cout << "clicks:" << (int)event->button.clicks << ", x,y: "
				<< x << "," << y << " locx,locy: " << locx << "," << locy
				<< " widthXheight: " << width << "X" << height << std::endl;
#endif

			if (x >= startx && x <= (locx + width) && y >= starty && y <= (starty + height)) {
				if (event->type == SDL_MOUSEBUTTONDOWN) {
					mouse_down(event, x, buttonItems[i].rect->y);
				} else if (event->type == SDL_MOUSEBUTTONUP) {
					mouse_up(event, x, buttonItems[i].rect->y);
				}
				return 1;
			}
		}
		return 0;
	}
}

void ShortcutBar_gump::mouse_down(SDL_Event *event, int mx, int my)
{
	ignore_unused_variable_warning(event);
	int i;
	for (i = 0; i < numButtons; i++) {
		if (buttonItems[i].rect->has_point(mx, my))
			break;
	}
	if (i == numButtons)
		return;

	ShortcutBarButtonItem *item = buttonItems + i;
	item->pushed = true;

}

#define DID_MOUSE_UP 1

/*
 * Runs on timer thread. Should never directly access anything in main thread.
 * Just push an event to main thread so that our global shortcut bar instance
 * can catch it.
 */
static Uint32 didMouseUp(Uint32 interval, void *param)
{
	ignore_unused_variable_warning(interval);
	SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = SHORTCUT_BAR_USER_EVENT;
	userevent.data1 = param;
	userevent.data2 = (void*)DID_MOUSE_UP;

	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user = userevent;
	SDL_PushEvent(&event);
	return 0;
}

/*
 * Runs on main thread.
 */
void ShortcutBar_gump::onUserEvent(SDL_Event *event)
{
	ignore_unused_variable_warning(event);
	switch ((intptr_t)(event->user.data2)) {
		case DID_MOUSE_UP:
			if (lastClickedButton >= 0 && lastClickedButton < numButtons) {
				onItemClicked(lastClickedButton, false);
				lastClickedButton = -1;
				if (timerId > 0) {
					SDL_RemoveTimer(timerId);
					timerId = 0;
				}
			}
			break;
		default:
			break;
	}
}

void ShortcutBar_gump::mouse_up(SDL_Event *event, int mx, int my)
{
	ignore_unused_variable_warning(event);
	int i;
	
	for (i = 0; i < numButtons; i++) {
		if (buttonItems[i].rect->has_point(mx, my))
			break;
	}

	if (i < numButtons) {
		/*
		 * Button i is hit.
		 * Cancel the previous mouse up timer
		 */
		if (timerId > 0) {
			SDL_RemoveTimer(timerId);
			timerId = 0;
		}
		lastClickedButton = -1;

		/*
		 * For every double click,
		 * there are usually two clicks:
		 *    MOUSEDOWN MOUSEUP MOUSEDOWN MOUSEUP
		 * Therefore when we get the first MOUSEUP, we
		 * have no idea if we are going to get another one.
		 * So we delay the handler.
		 */
#if SDL_VERSION_ATLEAST(2,0,2)
		if (event->button.clicks >= 2) {
#else
		if(0) { // FIXME add a way to doubleclick
#endif
			onItemClicked(i, true);
		} else {
			lastClickedButton = i;
			timerId = SDL_AddTimer(500/*ms delay*/, didMouseUp, this);
		}
	}

	for (i = 0; i < numButtons; i++) {
		buttonItems[i].pushed = false;
	}
}

void ShortcutBar_gump::onItemClicked(int index, bool doubleClicked)
{
	printf("Item %s is %sclicked\n", buttonItems[index].name, doubleClicked? "double " : "");

	switch (buttonItems[index].type) {
		case SB_ITEM_DISK:
		{
			Gamemenu_gump *menu;

			if (doubleClicked) {
				menu = new Gamemenu_gump();
				menu->loadsave();
				delete menu;
			} else {
				menu = new Gamemenu_gump();
				gumpman->do_modal_gump(menu, Mouse::hand);
				delete menu;
			}
			break;
		}

		case SB_ITEM_BACKPACK:
		{
			Game_window *gwin = Game_window::get_instance();
			static int inventory_page = -1;
			Actor *actor;
			Gump_manager *gump_man = gwin->get_gump_man();
			int party_count = gwin->get_party_man()->get_count();
			int shapenum;
			for (int i = 0; i <= party_count; ++i) {
				actor = Get_party_member(i);
				if (!actor)
					continue;
				shapenum = actor->inventory_shapenum();
				// Check if this actor's inventory page is open or not
				if (!gump_man->find_gump(actor, shapenum) && actor->can_act_charmed()) {
					gump_man->add_gump(actor, shapenum, true); //force showing inv.
					inventory_page = i;
					return;
				}
			}
			inventory_page = (inventory_page + 1) % (party_count + 1);
			actor = Get_party_member(inventory_page);
			if (actor && actor->can_act_charmed()) {
				actor->show_inventory(); //force showing inv.
			}
			Mouse::mouse->set_speed_cursor();
			break;
		}

		case SB_ITEM_SPELLBOOK:
		{
			gwin->activate_item(761);
			break;
		}

		case SB_ITEM_NOTEBOOK:
		{
			if (doubleClicked && cheat())
				cheat.cheat_screen();
			
			if (!doubleClicked) {
				Game_window *gwin = Game_window::get_instance();
				Gump_manager *gman = gwin->get_gump_man();
				Notebook_gump *notes = Notebook_gump::get_instance();
				if (notes)
					gman->remove_gump(notes);   // Want to raise to top.
				else
					notes = Notebook_gump::create();
				gman->add_gump(notes);
				gwin->paint();
			} 
			break;
		}

		case SB_ITEM_KEY:
		{
			if (doubleClicked) { // Lockpicks
				gwin->activate_item(627);
			} else {
				Game_window *gwin = Game_window::get_instance();
				int x, y;           // Allow dragging.
				if (!Get_click(x, y, Mouse::greenselect, 0, true))
					return;
				// Look for obj. in open gump.
				Gump *gump = gwin->get_gump_man()->find_gump(x, y);
				Game_object *obj;
				if (gump)
					obj = gump->find_object(x, y);
				else                // Search rest of world.
					obj = gwin->find_object(x, y);
				if (!obj)
					return;
				int qual = obj->get_quality();  // Key quality should match.
				Actor *party[10];       // Get ->party members.
				int party_cnt = gwin->get_party(&party[0], 1);
				for (int i = 0; i < party_cnt; i++) {
					Actor *act = party[i];
					Game_object_vector keys;        // Get keys.
					if (act->get_objects(keys, 641, qual, c_any_framenum)) {
						for (size_t i = 0; i < keys.size(); i++)
							if (!keys[i]->inside_locked()) {
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
				Mouse::mouse->flash_shape(Mouse::redx);
			}
			break;
		}

		case SB_ITEM_KEYRING:
		{
			if (doubleClicked) // Lockpicks
				gwin->activate_item(627);
			else
				gwin->activate_item(485);
			break;
		}

		case SB_ITEM_MAP:
		{
			if (doubleClicked && cheat())
				cheat.map_teleport();

			if (!doubleClicked)
				gwin->activate_item(178);
			break;
		}

		case SB_ITEM_TOGGLE_COMBAT:
		{
			Game_window *gwin = Game_window::get_instance();
			gwin->toggle_combat();
			gwin->paint();
			Mouse::mouse->set_speed_cursor();
			break;
		}

		case SB_ITEM_TARGET:
		{
			int x, y;

			if (doubleClicked && cheat()){
				if (!Get_click(x, y, Mouse::redx))
					return;
				cheat.cursor_teleport();
			}
			
			if (!doubleClicked) {
				if (!Get_click(x, y, Mouse::greenselect))
					return;
				Game_window::get_instance()->double_clicked(x, y);
			}
			break;
		}

		case SB_ITEM_JAWBONE:
		{
			gwin->activate_item(555);
			break;
		}

		case SB_ITEM_FEED:
		{
			if (GAME_SI) {
				Usecode_machine *usecode = Game_window::get_instance()->get_usecode();
				usecode->call_usecode(1557,static_cast<Game_object *>(0), static_cast<Usecode_machine::Usecode_events>(0));
				Mouse::mouse->set_speed_cursor();
			} else {
				Game_window *gwin = Game_window::get_instance();
					if (gwin->activate_item(377) || gwin->activate_item(616))
						Mouse::mouse->set_speed_cursor();
			}
			break;
		}

		default:
		{
			break;
		}
	}
}
