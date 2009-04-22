/*
 *  Copyright (C) 2000-2002  The Exult Team
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

#include "menulist.h"
#include "exult.h"
#include "font.h"
#include "gamewin.h"
#include "mouse.h"
#include "rect.h"
#include "shapeid.h"
#include "gump_utils.h"

#ifdef UNDER_CE
#include <string.h>
#endif
// MenuEntry: a selectable menu entry (a button)
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
	if (!dirty && !GL_manager::get_instance()) return;
	dirty = false;

	Shape_frame *shape;
	if(selected)
		shape = frame_on;
	else
		shape = frame_off;
	Shape_manager::get_instance()->paint_shape(
					x-shape->get_width()/2, y, shape);
	if (!GL_manager::get_instance())
		gwin->get_win()->show(x1,y1,x2-x1+1,y2-y1+1);	
}

bool MenuEntry::handle_event(SDL_Event& event)
{
	SDL_keysym& key = event.key.keysym;
	return((event.type == SDL_KEYDOWN &&
				(key.sym == SDLK_RETURN || key.sym == SDLK_KP_ENTER)) ||
			event.type == SDL_MOUSEBUTTONUP);
}

// MenuTextEntry: a selectable menu entry (a button)
MenuTextEntry::MenuTextEntry(Font *fnton, Font *fnt, const char *txt, int xpos, int ypos)
	: enabled(true)
{
	font_on = fnton;
	font = fnt;
	text = txt;
	int max_width = font->get_text_width(text);
	int max_height = font->get_text_height();
	x = xpos;
	y1 = y = ypos;
	x1 = xpos-max_width/2+1;
	x2 = x1+max_width+1;
	y2 = y1+max_height+1;
	selected = false;
	dirty = true;
}	

void MenuTextEntry::paint(Game_window *gwin)
{
	if (!dirty && !GL_manager::get_instance()) return;
	dirty = false;

	Font *fnt;
	if (selected && enabled)
		fnt = font_on;
	else
		fnt = font;
	fnt->paint_text_box(gwin->get_win()->get_ib8(), text,
		 x1, y1, x2-x1, y2-y1, 0, false, true, 0);
	if (!GL_manager::get_instance())
		gwin->get_win()->show(x1,y1,x2-x1,y2-y1);	
}

bool MenuTextEntry::handle_event(SDL_Event& event)
{
	SDL_keysym& key = event.key.keysym;
	return (((event.type == SDL_KEYDOWN &&
				(key.sym == SDLK_RETURN || key.sym == SDLK_KP_ENTER)) ||
			event.type == SDL_MOUSEBUTTONUP)) && enabled;
}

// MenuGameEntry: a selectable menu entry (a button)
MenuGameEntry::MenuGameEntry(
	 Font *fnton,
	 Font *fnt,
	 const char *txt,
	 Shape_frame *sfx,
	 int xpos, int ypos
	 )
	 : MenuTextEntry(fnton, fnt, txt, xpos, ypos), enabled(true)
{
	sfxicon = sfx;
	int max_width = 0, width;
	int max_height = font->get_text_height();
	//For game titles, which may have more than one line:
	char *ptr = strdup(txt);
	char *lineptr = ptr;
	int lines = 1;
	while (*ptr != 0)
	{
		if (*ptr == '\n')
		{
			lines++;
			*ptr = 0;
			width = font->get_text_width(lineptr);
			if (width > max_width)
				max_width = width;
			*ptr = '\n';
			lineptr = ptr + 1;
		}
		ptr++;
	}
	width = font->get_text_width(lineptr);
	if (width > max_width)
		max_width = width;
	max_height *= lines;
	x = xpos;
	y1 = y = ypos;
	x1 = xpos-max_width/2+1;
	x2 = x1+max_width+1;
	y2 = y1+max_height+lines;
	selected = false;
	dirty = true;
}	

void MenuGameEntry::paint(Game_window *gwin)
{
	if (!dirty && !GL_manager::get_instance()) return;
	dirty = false;

	if (sfxicon)
	{
		Shape_manager::get_instance()->paint_shape(x1-sfxicon->get_width()-3,y, sfxicon);
		if (!GL_manager::get_instance())
			gwin->get_win()->show(x1-sfxicon->get_width()-3,y,sfxicon->get_width(),sfxicon->get_height());
	}

	Font *fnt;
	if (selected && enabled)
		fnt = font_on;
	else
		fnt = font;
	fnt->paint_text_box(gwin->get_win()->get_ib8(), text,
		 x1, y1, x2-x1, y2-y1, 0, false, true, 0);
	if (!GL_manager::get_instance())
		gwin->get_win()->show(x1,y1,x2-x1,y2-y1);	
}

bool MenuGameEntry::handle_event(SDL_Event& event)
{
	SDL_keysym& key = event.key.keysym;
	return (((event.type == SDL_KEYDOWN &&
				(key.sym == SDLK_RETURN || key.sym == SDLK_KP_ENTER)) ||
			event.type == SDL_MOUSEBUTTONUP)) && enabled;
}

// MenuTextChoice: a multiple-choice menu entry
MenuTextChoice::MenuTextChoice(Font *fnton, Font *fnt, const char *txt, int xpos, int ypos)
{
	font_on = fnton;
	font = fnt;
	text = txt;
	int max_width = font->get_text_width(text);
	int max_height = font->get_text_height();
	x = xpos;
	x1 = x-max_width;
	y1 = y = ypos;
	x2 = x1 + max_width;
	y2 = y1 + max_height;
	selected = false;
	choice = -1;
	max_choice_width = 0;
	choices = new std::vector<std::string>();
}

void MenuTextChoice::add_choice(const char *s)
{
	choices->push_back(std::string(s));
	int len = font->get_text_width(s);
	max_choice_width = (len>max_choice_width)?len:max_choice_width;
	x2 = x+32+max_choice_width;
}

void MenuTextChoice::paint(Game_window *gwin)
{
	if (!dirty && !GL_manager::get_instance()) return;
	dirty = false;

	Font *fnt;
	if (selected)
		fnt = font_on;
	else
		fnt = font;
	fnt->draw_text(gwin->get_win()->get_ib8(), x1, y1, text);
	if (!GL_manager::get_instance())
		gwin->get_win()->show(x1,y1, x1+fnt->get_text_width(text), y2);

	if(choice>=0) {
		gwin->get_win()->fill8(0, max_choice_width, font->get_text_height(), x+32, y);
		font->draw_text(gwin->get_win()->get_ib8(), 
					x+32, y, (*choices)[choice].c_str());
		if (!GL_manager::get_instance())
			gwin->get_win()->show(x+32,y, x+32+max_choice_width, y2);
	}
}

bool MenuTextChoice::handle_event(SDL_Event& event)
{
	if(event.type==SDL_MOUSEBUTTONUP) {
		dirty = true;
		choice++;
		if(choice>=choices->size())
			choice = 0;
	} else if(event.type==SDL_KEYDOWN) {
		switch(event.key.keysym.sym) {
		case SDLK_LEFT:
			dirty = true;
			choice--;
			if(choice<0)
				choice = choices->size()-1;
			break;
		case SDLK_RIGHT:
			dirty = true;
			choice++;
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

	// deselect the previous one, unless nothing changed
	if (selected) {
		entry = (*entries)[selection];
		if (entry->is_mouse_over(x, y))
			return;

		entry->set_selected(false);
	}
	
	// select the new one, and return when found
	for(int i=0; i<entries->size(); i++) {
		entry = (*entries)[i];
		if(entry->is_mouse_over(x, y)) {
			entry->set_selected(true);
			selected = true;
			selection = i;
			return;
		}
	}

	// nothing has been selected
	selected = false;
}

int MenuList::handle_events(Game_window *gwin, Mouse *mouse)
{
	unsigned char mouse_visible;
	int count = entries->size();
	bool exit_loop = false;
	int scale = gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();
	SDL_Event event;
	for(int i=0; i<count; i++)
	  (*entries)[i]->dirty = true;

	gwin->show(1);
	mouse->show();
	do {
		Delay();
		mouse_visible = mouse->is_onscreen();
		if (mouse_visible) mouse->hide();
		if (GL_manager::get_instance()) {
			if (bg) {
				int cx = gwin->get_width()/2, cy = gwin->get_height()/2;
				cx -= bg->get_width()/2;
				cy -= bg->get_height()/2;
				Shape_manager::get_instance()->paint_shape(cx, cy, bg);
			}
		}
		// redraw items if they're dirty
		for(int i=0; i<count; i++) {
			MenuObject *entry = (*entries)[i];
			entry->paint(gwin);
		}
		// redraw mouse if visible
		if (mouse_visible) {
			mouse->show();
			mouse->blit_dirty();
		}
		if (bg && GL_manager::get_instance())
			gwin->get_win()->show();
		while (SDL_PollEvent(&event))
		{
			if(event.type==SDL_MOUSEMOTION) {
				mouse->hide();
				mouse->move(event.motion.x / scale, 
						event.motion.y / scale);
				set_selection(event.motion.x / scale, 
						event.motion.y / scale); 
				mouse->show();
				mouse->blit_dirty();
			} else if(event.type==SDL_MOUSEBUTTONDOWN) {
				if (!mouse_visible) {
					// if invisible, redraw mouse
					set_selection(event.button.x / scale, 
								event.button.y / scale); 
					mouse->show();
					mouse->blit_dirty();
				}
			} else if(event.type==SDL_MOUSEBUTTONUP) {
				MenuObject *entry = (*entries)[selection];
				if (entry->is_mouse_over(
							event.button.x / scale, 
							event.button.y / scale)) {
					exit_loop = entry->handle_event(event);
				}
			} else if(event.type==SDL_KEYDOWN) {
				mouse->hide();
				mouse->blit_dirty();
				switch(event.key.keysym.sym) {
				case SDLK_x:
					if(event.key.keysym.mod & KMOD_ALT) {
						return -1;
					}
					break;
#if defined(MACOS) || defined(MACOSX)
				case SDLK_q:
					if(event.key.keysym.mod & KMOD_META) {
						return -1;
					}
					break;
#endif
				case SDLK_UP:
					if (!selected)
					{
						// if unselected (by 'MouseOut' event), just re-select
						set_selection(selection);
						continue;
					}
					if(selection<=0)
						set_selection(count-1);
					else
						set_selection(selection-1);
					continue;
				case SDLK_DOWN:
					if (!selected)
					{
						// if unselected (by 'MouseOut' event), just re-select
						set_selection(selection);
						continue;
					}
					if(selection>=(count-1))
						set_selection(0);
					else
						set_selection(selection+1);
					continue;
				case SDLK_s:
					if ((event.key.keysym.mod & KMOD_ALT) &&
						(event.key.keysym.mod & KMOD_CTRL))
						make_screenshot(true);
				default:
					{
						// let key be processed by selected menu-item
						if(selected) {
							MenuObject *entry = (*entries)[selection];
							exit_loop = entry->handle_event(event);
						}
					}
					break;
				}
			} else if(event.type==SDL_QUIT) {
				return -1;
			}
		}
	} while(!exit_loop);
	mouse->hide();
	if ((*entries)[selection]->get_has_id())
		return (*entries)[selection]->get_id();
	else
		return selection;
}

