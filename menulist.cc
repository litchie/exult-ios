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

#include "font.h"
#include "gamewin.h"
#include "menulist.h"

MenuEntry::MenuEntry(Shape_frame *on, Shape_frame *off, int xpos, int ypos)
{
	frame_on = on;
	frame_off = off;
	x = xpos;
	y = ypos;
	selected = false;
}	

void MenuEntry::paint(Game_window *gwin)
{
	Shape_frame *shape;
	if(selected)
		shape = frame_on;
	else
		shape = frame_off;
	gwin->paint_shape(x-shape->get_width()/2, y, shape);
}

bool MenuEntry::handle_event(SDL_Event& event)
{
	return(event.key.keysym.sym == SDLK_RETURN);
}


MenuChoice::MenuChoice(Shape_frame *on, Shape_frame *off, int xpos, int ypos, Font *fnt)
{
	frame_on = on;
	frame_off = off;
	x = xpos;
	y = ypos;
	selected = false;
	choice = -1;
	font = fnt;
	max_choice_width = 0;
	choices = new std::vector<std::string>();
}

void MenuChoice::add_choice(char *s)
{
	choices->push_back(std::string(s));
	int len = font->get_text_width(s);
	max_choice_width = (len>max_choice_width)?len:max_choice_width;
}

void MenuChoice::paint(Game_window *gwin)
{
	Shape_frame *shape;
	if(selected)
		shape = frame_on;
	else
		shape = frame_off;
	gwin->paint_shape(x-shape->get_width(), y, shape);
	if(choice>=0) {
		gwin->get_win()->fill8(0, x+32+max_choice_width, y+font->get_text_height(), x+32, y);
		font->draw_text(gwin, x+32, y, choices->at(choice).c_str());
	}
}

bool MenuChoice::handle_event(SDL_Event& event)
{
	switch(event.key.keysym.sym) {
	case SDLK_LEFT:
		choice--;
		if(choice<0)
			choice = choices->size()-1;
		break;
	case SDLK_RIGHT:
		choice++;
		if(choice>=choices->size())
			choice = 0;
		break;
	default:
		break;
	}
	return false;
}

MenuList::~MenuList()
{
	MenuObject *entry;
	for(int i=0; i<entries->size(); i++) {
		entry = entries->at(i);
		delete entry;
	}
	delete entries;
}

void MenuList::set_selected(int sel)
{
	MenuObject *entry;
	// deselect the previous entry
	if(selected>=0) {
		entry = entries->at(selected);
		entry->set_selected(false);
	}
	// select the new one
	selected = sel;
	entry = entries->at(selected);
	entry->set_selected(true);
}

int MenuList::handle_events(Game_window *gwin)
{
	int count = entries->size();
	bool exit_loop = false;
	bool redraw = true;
	SDL_Event event;
	do {
		if (redraw) {
			for(int i=0; i<count; i++) {
				MenuObject *entry = entries->at(i);
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
			default:
				{
					MenuObject *entry = entries->at(selected);
					exit_loop = entry->handle_event(event);
				}
				break;
			}
		}
	} while(!exit_loop);
	return selected;
}
