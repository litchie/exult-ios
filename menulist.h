/*
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

#ifndef MENU_LIST_H
#define MENU_LIST_H

#include "vec.h"

class Game_window;
class Shape_frame;

class MenuEntry {
private:
	Shape_frame *frame_on, *frame_off;
	int x, y, width, height;
	bool selected;
public:
	MenuEntry(Shape_frame *on, Shape_frame *off, int xpos, int ypos) :
		frame_on(on), frame_off(off), x(xpos), y(ypos), selected(false) { }
	void paint(Game_window *gwin);
	void set_selected(bool sel) { selected = sel; }
	bool is_selected() { return selected; }
};

class MenuList {
private:
	Vector *entries;
	int selected;
public:
	MenuList(): selected(-1) { entries = new Vector(); }
	~MenuList();
	void add_entry(MenuEntry *entry) { entries->append(entry); }
	void paint(Game_window *gwin);
	int handle_events(Game_window *gwin);
	int get_selected() { return selected; }
	void set_selected(int sel);
};

#endif
