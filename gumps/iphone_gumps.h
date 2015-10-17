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

#ifndef _IPHONE_GUMPS_H
#define _IPHONE_GUMPS_H

#ifdef __IPHONEOS__

#include "SDL.h"
#include "gamewin.h"
#include "../objs/objs.h"
#include "misc_buttons.h"
#include "Modal_gump.h"
#include <string>
#include "Gamemenu_gump.h"

using std::string;

extern "C" int SDLCALL SDL_iPhoneKeyboardShow(SDL_Window *window);
extern "C" int SDLCALL SDL_iPhoneKeyboardHide(SDL_Window *window);
extern "C" SDL_bool SDLCALL SDL_iPhoneKeyboardIsShown(SDL_Window *window);
extern "C" int SDLCALL SDL_iPhoneKeyboardToggle(SDL_Window *window);

#define SHORTCUT_BAR_USER_EVENT 0x53425545 // "SBUE"

/* -------------------------------------------- */

typedef enum {
	SB_ITEM_DISK,
	SB_ITEM_TOGGLE_COMBAT,
	SB_ITEM_MAP,
	SB_ITEM_SPELLBOOK,
	SB_ITEM_BACKPACK,
	SB_ITEM_KEY,
	SB_ITEM_KEYRING,
	SB_ITEM_NOTEBOOK,
	SB_ITEM_TARGET,
	SB_ITEM_JAWBONE,
	SB_ITEM_FEED,
} ShortcutBarButtonItemType;

struct ShortcutBarButtonItem {
	const char *name;
	ShortcutBarButtonItemType type;
	ShapeID *shapeId;
	Rectangle *rect; /* coordinate values related to shortcut bar */
	int shapeOffsetX;
	int shapeOffsetY;
	bool pushed;
};

#define MAX_SHORTCUT_BAR_ITEMS 10

class ShortcutBar_gump: public Gump {
public:
	ShortcutBar_gump(int placex = 0, int placey = 0);
	~ShortcutBar_gump();
	int handle_event(SDL_Event *event);
	void paint();

	// Don't close on end_gump_mode
	virtual bool is_persistent() const {
		return true;
	}
	// Can't be dragged with mouse
	virtual bool is_draggable() const {
		return false;
	}
	// Show the hand cursor
	virtual bool no_handcursor() const {
		return true;
	}
	int gamex = gwin->get_game_width();
	int starty = gwin->get_win()->get_start_y();
	int resy = gwin->get_win()->get_full_height();
	int gamey = gwin->get_game_height();
	void onUserEvent(SDL_Event *event);
	
private:
	ShortcutBarButtonItem buttonItems[MAX_SHORTCUT_BAR_ITEMS];
	int numButtons;
	int lastClickedButton;
	SDL_TimerID timerId;
	
	bool is_party_item(int shnum, int frnum = c_any_framenum,
	                   int qual = c_any_qual);
	void createButtons();
	void deleteButtons();
	void onItemClicked(int index, bool doubleClicked);
	void mouse_down(SDL_Event *event, int mx, int my);
	void mouse_up(SDL_Event *event, int mx, int my);

	int locx;
	int locy;
	int width;
	int height;
};

class Gump_button;
typedef std::vector<Gump_button *> Gump_button_vector;
typedef std::map<Game_object *, int *> Game_object_map_xy;


enum ITEMMENU_ACTIONS { ITEMMENU_ACTION_NONE, ITEMMENU_ACTION_MENU, ITEMMENU_ACTION_USE, ITEMMENU_ACTION_PICKUP, ITEMMENU_ACTION_MOVE, ITEMMENU_ACTION_COUNT };
class Itemmenu_gump : public Modal_gump {
	UNREPLICATABLE_CLASS_I(Itemmenu_gump, Modal_gump(0, 0, 0, 0));
public:
	Gump_button_vector buttons;
	Game_object_map_xy objects;
	Game_object *objectSelected;
	int objectSelectedClickXY[2];
	int objectAction;

	Itemmenu_gump(Game_object_map_xy *mobjxy, int cx, int cy);
	Itemmenu_gump(Game_object *obj, int ox, int oy, int cx, int cy);
	virtual ~Itemmenu_gump();

	// Paint it and its contents.
	virtual void paint();
	virtual void close() {
		done = 1;
	}
	// Handle events:
	virtual bool mouse_down(int mx, int my, int button);
	virtual bool mouse_up(int mx, int my, int button);

	void postCloseActions();

};
#endif
#endif
