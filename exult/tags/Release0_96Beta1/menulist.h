/*
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifndef MENULIST_H
#define MENULIST_H

#include <string>
#include <vector>
#include "SDL_events.h"

class Game_window;
class Shape_frame;
class Font;
class Mouse;

class MenuObject {
public:
	Shape_frame *frame_on, *frame_off;
	int x, y, x1, y1, x2, y2;
	bool selected;
	bool dirty;

	MenuObject() { };
	virtual ~MenuObject() { }
	
	void set_selected(bool sel) { 
  	        dirty |= (selected != sel);
 	        selected = sel;
	}
	bool is_selected() { return selected; }
	bool is_mouse_over(int mx, int my) 
		{ return ((mx>=x1)&&(mx<=x2)&&(my>=y1)&&(my<=y2)); };
	
	virtual void paint(Game_window *gwin) =0;
	virtual bool handle_event(SDL_Event& event) =0;
};

class MenuEntry: public MenuObject {
public:
	MenuEntry(Shape_frame *on, Shape_frame *off, int xpos, int ypos);
	virtual ~MenuEntry() { }

	virtual void paint(Game_window *gwin);
	virtual bool handle_event(SDL_Event& event);
};

class MenuChoice: public MenuObject {
private:
	std::vector<std::string> *choices;
	int choice;
	Font *font;
	int max_choice_width;
public:
	MenuChoice(Shape_frame *on, Shape_frame *off, int xpos, int ypos, Font *fnt);
	virtual ~MenuChoice() { delete choices; }
	void add_choice(const char *s);
	int get_choice() { return choice; }
	void set_choice(int c) { choice = c; }
	
	virtual void paint(Game_window *gwin);
	virtual bool handle_event(SDL_Event& event);
};

class MenuList {
private:
	std::vector<MenuObject*> *entries;
	bool selected;
	int selection;
public:
	MenuList(): selected(false), selection(0) { entries = new std::vector<MenuObject*>(); }
	~MenuList();
	int add_entry(MenuObject *entry) { entries->push_back(entry); return (entries->size()-1);}
	void paint(Game_window *gwin);
	int handle_events(Game_window *gwin, Mouse *mouse);
	int get_selection() { return selection; }
	void set_selection(int sel);
	void set_selection(int x, int y);
};

#endif
