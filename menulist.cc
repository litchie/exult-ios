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
#include "mouse.h"
#include "rect.h"

MenuEntry::MenuEntry(Shape_frame *on, Shape_frame *off, int xpos, int ypos)
{
	frame_on = on;
	frame_off = off;
	int max_width = on->get_width()>off->get_width()?on->get_width():off->get_width();
	int max_height = on->get_height()>off->get_height()?on->get_height():off->get_height();
	x = xpos;
	y1 = y = ypos;
	x1 = xpos-max_width/2;
	x2 = x1+max_width;
	y2 = y1+max_height;
	selected = false;
	dirty = true;
}	

void MenuEntry::paint(Game_window *gwin)
{
        if (!dirty) return;
	dirty = false;

//	Rectangle d(x1,y1,x2-x1+1,y2-y1+1);
//	gwin->add_dirty(d);

	Shape_frame *shape;
	if(selected)
		shape = frame_on;
	else
		shape = frame_off;
	gwin->paint_shape(x-shape->get_width()/2, y, shape);
	gwin->get_win()->show(x1,y1,x2-x1+1,y2-y1+1);	
}

bool MenuEntry::handle_event(SDL_Event& event)
{
	return((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) ||
	       event.type == SDL_MOUSEBUTTONUP);
}


MenuChoice::MenuChoice(Shape_frame *on, Shape_frame *off, int xpos, int ypos, Font *fnt)
{
	frame_on = on;
	frame_off = off;
	int max_width = on->get_width()>off->get_width()?on->get_width():off->get_width();
	int max_height = on->get_height()>off->get_height()?on->get_height():off->get_height();
	x = xpos;
	x1 = x-max_width;
	y1 = y = ypos;
	x2 = x1 + max_width;
	y2 = y1 + max_height;
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
	x2 = x+32+max_choice_width;
}

void MenuChoice::paint(Game_window *gwin)
{
        if (!dirty) return;
	dirty = false;

	//	Rectangle d(x1,y1,x2-x1+1,y2-y1+1);
	//	gwin->add_dirty(d);

	Shape_frame *shape;
	if(selected)
		shape = frame_on;
	else
		shape = frame_off;

	gwin->paint_shape(x-shape->get_width(), y, shape);
	gwin->get_win()->show(x1,y1,x2-x1+1,y2-y1+1);
	if(choice>=0) {
		gwin->get_win()->fill8(0, max_choice_width, font->get_text_height(), x+32, y);
		font->draw_text(gwin, x+32, y, (*choices)[choice].c_str());
		gwin->get_win()->show(x+32,y, x+32+max_choice_width, y+font->get_text_height());
	}
}

bool MenuChoice::handle_event(SDL_Event& event)
{
	if(event.type==SDL_MOUSEBUTTONUP) {
		choice++;
		if(choice>=choices->size())
			choice = 0;
	} else if(event.type==SDL_KEYDOWN) {
		switch(event.key.keysym.sym) {
		case SDLK_LEFT:
			choice--;
			dirty = true;
			if(choice<0)
				choice = choices->size()-1;
			break;
		case SDLK_RIGHT:
		        choice++;
			dirty = true;
			if(choice>=choices->size())
				choice = 0;
			break;
		default:
			break;
		}
	}
	return false;
}

MenuList::~MenuList()
{
	MenuObject *entry;
	for(int i=0; i<entries->size(); i++) {
		entry = (*entries)[i];
		delete entry;
	}
	delete entries;
}

void MenuList::set_selection(int sel)
{
	MenuObject *entry;
	// deselect the previous entry
	if (selected) {
		entry = (*entries)[selection];
		entry->set_selected(false);
	}
	// select the new one
	selected = true;
	selection = sel;
	entry = (*entries)[selection];
	entry->set_selected(true);
}

void MenuList::set_selection(int x, int y)
{
	MenuObject *entry;
	// Skip everything if it's the same
	if (selected) {
	          entry = (*entries)[selection];
	          if (entry->is_mouse_over(x, y))
	                return;
		  else
  	                entry->set_selected(false);
	}
	
	for(int i=0; i<entries->size(); i++) {
		entry = (*entries)[i];
		if(entry->is_mouse_over(x, y)) {
			entry->set_selected(true);
			selected = true;
			selection = i;
			return;
		}
	}

	selected = false;
}

int MenuList::handle_events(Game_window *gwin, Mouse *mouse)
{
	int count = entries->size();
	bool exit_loop = false;
	int scale = gwin->get_win()->get_scale() == 2 ? 1 : 0;
	SDL_Event event;

	for(int i=0; i<count; i++)
	  (*entries)[i]->dirty = true;

	mouse->show();
	gwin->show(1);
//	gwin->clear_dirty();
	do {
        	mouse->hide();
		for(int i=0; i<count; i++) {
			MenuObject *entry = (*entries)[i];
			entry->paint(gwin);
		}
//		gwin->show_dirty();
		mouse->show();
		mouse->blit_dirty();
//		gwin->clear_dirty();

		SDL_WaitEvent(&event);
		if(event.type==SDL_MOUSEMOTION) {
			mouse->hide();
			mouse->move(event.motion.x >> scale, 
					event.motion.y >> scale);
			set_selection(event.motion.x >> scale, 
					event.motion.y >> scale); 
			mouse->show();
			mouse->blit_dirty();
//		} else if(event.type==SDL_MOUSEBUTTONDOWN) {
//		        if (!mouse->is_onscreen()){
//			        mouse->show();
//				mouse->blit_dirty();
//			}
		} else if(event.type==SDL_MOUSEBUTTONUP) {
		        MenuObject *entry = (*entries)[selection];
			if (entry->is_mouse_over(
					   event.button.x >> scale, 
					   event.button.y >> scale)) {
			        exit_loop = entry->handle_event(event);
			}			
		} else if(event.type==SDL_KEYDOWN) {
		        set_selection(selection);
		        mouse->hide();
			switch(event.key.keysym.sym) {
			case SDLK_x:
				if(event.key.keysym.mod & KMOD_ALT) {
					return -1;
				}
				break;
#ifdef MACOS
			case SDLK_q:
				if(event.key.keysym.mod & KMOD_META) {
					return -1;
				}
				break;
#endif
			case SDLK_UP:
				if(selection==0)
					set_selection(count-1);
				else
					set_selection(selection-1);
				continue;
			case SDLK_DOWN:
				if(selection==(count-1))
					set_selection(0);
				else
					set_selection(selection+1);
				continue;
			default:
				{
					if(selected) {
						MenuObject *entry = (*entries)[selection];
						exit_loop = entry->handle_event(event);
					}
				}
				break;
			}
			mouse->show();
			mouse->blit_dirty();
		}
	} while(!exit_loop);
	mouse->hide();
	return selection;
}

