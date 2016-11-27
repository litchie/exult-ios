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

#ifdef __IPHONEOS__

#include "SDL_events.h"

#include "fnames.h"
#include "iphone_gumps.h"
#include "exult.h"
#include "keyactions.h"
#include "gamewin.h"
#include "shapeid.h"
#include "actors.h"
#include <string>

#include "Gump_manager.h"
#include "Gump_button.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "Text_button.h"

using std::string;
using std::cout;
using std::endl;


class Itemmenu_button : public Text_button {
public:
	Itemmenu_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 20)
	{ }
	// What to do when 'clicked':
	virtual bool activate(int button) {
		if (button != 1) return false;
		Itemmenu_gump *par = (Itemmenu_gump *)parent;
		if (text == "Cancel") {
			par->close();
		} else if (par->objectAction == ITEMMENU_ACTION_NONE) {
			Gump_button_vector::const_iterator bit = par->buttons.begin();
			Game_object_map_xy::const_iterator oit = par->objects.begin();

			for (; bit < par->buttons.end(); bit++, oit++) {
				if (*bit == this) {
					par->objectSelected = ((Game_object *)((*oit).first));
					int *arrXY = (*oit).second;
					par->objectSelectedClickXY[0] = arrXY[0];
					par->objectSelectedClickXY[1] = arrXY[1];
					par->close();
					break;
				}
			}
		} else { // if (par->objectAction == ITEMMENU_ACTION_MENU
			if (text == "Use") {
				par->objectAction = ITEMMENU_ACTION_USE;
			} else if (text == "Pickup") {
				par->objectAction = ITEMMENU_ACTION_PICKUP;
			} else if (text == "Move") {
				par->objectAction = ITEMMENU_ACTION_MOVE;
			} else if (text == "Cancel") {
				par->objectSelected = NULL;
				par->objectSelectedClickXY[0] = -1;
				par->objectSelectedClickXY[1] = -1;
			}
			par->close();
		}
		return true;
	}
};

Itemmenu_gump::Itemmenu_gump(Game_object_map_xy *mobjxy, int cx, int cy)
	: Modal_gump(0, cx, cy, EXULT_FLX_TRANSPARENTMENU_SHP, SF_EXULT_FLX) {
	objectSelected = NULL;
	objectSelectedClickXY[0] = -1;
	objectSelectedClickXY[1] = -1;
	objectAction = ITEMMENU_ACTION_NONE;
	int btop = 0;
	int byspace = 22;
	//set_object_area(Rectangle(0, 0, 0, 0), -1, -1);//++++++ ???
	Itemmenu_button *b;
	int i = 0;
	for (Game_object_map_xy::const_iterator it = mobjxy->begin(); it != mobjxy->end(); it++, i++) {
		Game_object *o = (*it).first;
		int *sArrXY = (*it).second;
		int *arrXY = new int[2];
		arrXY[0] = sArrXY[0];
		arrXY[1] = sArrXY[1];
		objects.insert(std::pair<Game_object *, int *>(o, arrXY));
		b = new Itemmenu_button(this, o->get_name().c_str(), 10, btop + (i * byspace));
		buttons.push_back((Gump_button *)b);
	}
	b = new Itemmenu_button(this, "Cancel", 10, btop + (i * byspace));
	buttons.push_back((Gump_button *)b);
}

Itemmenu_gump::Itemmenu_gump(Game_object *obj, int ox, int oy, int cx, int cy)
	: Modal_gump(0, cx, cy, EXULT_FLX_TRANSPARENTMENU_SHP, SF_EXULT_FLX) {
	objectSelected = obj;
	objectAction = ITEMMENU_ACTION_MENU;
	int btop = 0;
	int byspace = 22;
	Itemmenu_button *b;
	objectSelectedClickXY[0] = ox;
	objectSelectedClickXY[1] = oy;
	int i = 0;
	b = new Itemmenu_button(this, "Use", 10, btop + (i * byspace));
	buttons.push_back((Gump_button *)b);
	i++;
	b = new Itemmenu_button(this, "Pickup", 10, btop + (i * byspace));
	buttons.push_back((Gump_button *)b);
	i++;
	b = new Itemmenu_button(this, "Move", 10, btop + (i * byspace));
	buttons.push_back((Gump_button *)b);
	i++;
	b = new Itemmenu_button(this, "Cancel", 10, btop + (i * byspace));
	buttons.push_back((Gump_button *)b);
}

Itemmenu_gump::~Itemmenu_gump() {
	for (Gump_button_vector::const_iterator it = buttons.begin(); it < buttons.end(); it++) {
		Gump_button *b = *it;
		delete b;
	}
}

void Itemmenu_gump::paint() {
	Gump::paint();
	for (Gump_button_vector::const_iterator it = buttons.begin(); it < buttons.end(); it++)
		(*it)->paint();
	gwin->set_painted();
}

bool Itemmenu_gump::mouse_down(int mx, int my, int button) {
	// Only left and right buttons
	if (button != 1 && button != 3) return false;

	// We'll eat the mouse down if we've already got a button down
	if (pushed) return true;

	// First try checkmark
	pushed = Gump::on_button(mx, my);

	// Try buttons at bottom.
	if (!pushed) {
		for (Gump_button_vector::const_iterator it = buttons.begin(); it < buttons.end(); it++) {
			if ((*it)->on_button(mx, my)) {
				pushed = (Gump_button *)*it;
				break;
			}
		}
	}

	if (pushed && !pushed->push(button))                    // On a button?
		pushed = 0;

	return button == 1 || pushed != 0;
}

bool Itemmenu_gump::mouse_up(int mx, int my, int button) {
	// Not Pushing a button?
	if (!pushed) {
		close();
		return false;
	}

	if (pushed->get_pushed() != button) return button == 1;

	bool res = false;
	pushed->unpush(button);
	if (pushed->on_button(mx, my))
		res = ((Gump_button *)pushed)->activate(button);
	pushed = 0;
	return res;
}

void Itemmenu_gump::postCloseActions() {
	if (!objectSelected)
		return;

	Itemmenu_gump *itemgump;
	Game_window *gwin = Game_window::get_instance();
	Main_actor *ava = gwin->get_main_actor();
	Tile_coord avaLoc = ava->get_tile();
	int avaX = (avaLoc.tx - gwin->get_scrolltx()) * c_tilesize;
	int avaY = (avaLoc.ty - gwin->get_scrollty()) * c_tilesize;
	Game_object *tmpObj;
	int tmpX, tmpY;

	switch (objectAction) {
	case ITEMMENU_ACTION_USE:
		objectSelected->activate();
		break;
	case ITEMMENU_ACTION_PICKUP:
		tmpObj = gwin->find_object(avaX, avaY);
		if (tmpObj != (Game_object *)ava) {
			// Avatar isn't in a good spot...
			// Let's give up for now :(
			break;
		}

		if (gwin->start_dragging(objectSelectedClickXY[0], objectSelectedClickXY[1]))
			if (gwin->drag(avaX, avaY))
				gwin->drop_dragged(avaX, avaY, true);
		break;
	case ITEMMENU_ACTION_MOVE:
		if (Get_click(tmpX, tmpY, Mouse::greenselect, 0, true))
			if (gwin->start_dragging(objectSelectedClickXY[0], objectSelectedClickXY[1]))
				if (gwin->drag(tmpX, tmpY))
					gwin->drop_dragged(tmpX, tmpY, true);
		break;
	case ITEMMENU_ACTION_NONE:
	{
		// Make sure menu is visible on the screen
		int w = Game_window::get_instance()->get_width();
		int h = Game_window::get_instance()->get_height();
		int left = w - 100;
		int top = h - 100;
		if (left > x) left = x;
		if (top > y) top = y;
	
		// This will draw a selection menu for the object
		itemgump = new Itemmenu_gump(objectSelected, objectSelectedClickXY[0], objectSelectedClickXY[1], left, top);
		Game_window::get_instance()->get_gump_man()->do_modal_gump(itemgump, Mouse::hand);
		itemgump->postCloseActions();
		delete itemgump;
		break;
	}
	
	case ITEMMENU_ACTION_MENU:
		break;
	}
}
#endif
