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

#include "gamewin.h"
#include "menulist.h"

void MenuEntry::paint(Game_window *gwin)
{
	Shape_frame *shape;
	if(selected)
		shape = frame_on;
	else
		shape = frame_off;
	gwin->paint_shape(x-shape->get_width()/2, y, shape);
}

MenuList::~MenuList()
{
	MenuEntry *entry;
	for(int i=0; i<entries->get_cnt(); i++) {
		MenuEntry *entry = (MenuEntry *)entries->get(i);
		delete entry;
	}
	delete entries;
}

void MenuList::set_selected(int sel)
{
	MenuEntry *entry;
	// deselect the previous entry
	if(selected>=0) {
		entry = (MenuEntry *)entries->get(selected);
		entry->set_selected(false);
	}
	// select the new one
	selected = sel;
	entry = (MenuEntry *)entries->get(selected);
	entry->set_selected(true);
}

int MenuList::handle_events(Game_window *gwin)
{
	int count = entries->get_cnt();
	bool exit_loop = false;
	bool redraw = true;
	SDL_Event event;
	do {
		if (redraw) {
			for(int i=0; i<count; i++) {
				MenuEntry *entry = (MenuEntry *)entries->get(i);
				entry->paint(gwin);
			}
			gwin->get_win()->show();
			redraw = false;
		}
		SDL_WaitEvent(&event);
		if(event.type==SDL_KEYDOWN) {
		        redraw = true;
			switch(event.key.keysym.sym) {
			case SDLK_x:
				if(event.key.keysym.mod & KMOD_ALT) {
					return -1;
					
				}
				break;
			case SDLK_UP:
				if(selected==0)
					set_selected(count-1);
				else
					set_selected(selected-1);
				continue;
			case SDLK_DOWN:
				if(selected==(count-1))
					set_selected(0);
				else
					set_selected(selected+1);
				continue;
			case SDLK_RETURN:
				exit_loop = true;
				break;
			default:
				break;
			}
		}
	} while(!exit_loop);
	return selected;
}
