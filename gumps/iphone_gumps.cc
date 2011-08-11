/*
Copyright (C) 2001-2011 The Exult Team

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

#ifdef __IPHONEOS__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_events.h"

#include "fnames.h"
#include "iphone_gumps.h"
#include "exult.h"
#include "exult_iphone_flx.h"
#include "keyactions.h"
#include "gamewin.h"
#include "shapeid.h"
#include <string>

#include "Gump_manager.h"
#include "Gump_button.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "Text_button.h"

using std::string;
using std::cout;
using std::endl;

KeyboardButton_gump::KeyboardButton_gump(int placex, int placey)
{
	autopaint = true;
	iphone_vga.load(IPHONE_FLX);
	width = iphone_vga.get_shape(EXULT_IPHONE_FLX_KEYBOARDBUTTON_SHP, 0)->get_width();
	height = iphone_vga.get_shape(EXULT_IPHONE_FLX_KEYBOARDBUTTON_SHP, 0)->get_height();
	locx = placex;
	locy = placey;
}

KeyboardButton_gump::~KeyboardButton_gump()
{

}


void KeyboardButton_gump::paint()
{
	Game_window *gwin = Game_window::get_instance();
	Shape_manager *sman = Shape_manager::get_instance();
	sman->paint_shape(locx, locy,
 		iphone_vga.get_shape(EXULT_IPHONE_FLX_KEYBOARDBUTTON_SHP, 0), 0);
	gwin->add_dirty(Rectangle(locx, locy, width+4, height+4));
	gwin->set_painted();
}

int KeyboardButton_gump::handle_event(SDL_Event *event)
{
	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
	{
		void (KeyboardButton_gump::*mouseFunc)(int, int) = &KeyboardButton_gump::mouse_down; // Mouse down by default
		if (event->type == SDL_MOUSEBUTTONUP)
			mouseFunc = &KeyboardButton_gump::mouse_up;
		Game_window *gwin = Game_window::get_instance();
		int scale = gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale_factor();
		int x = event->button.x/scale, y = event->button.y/scale;
		//std::cout << "x,y: " << x << "," << y << " locx,locy: " << locx << "," << locy << " widthXheight: " << width << "X" << height << std::endl;
                if (x >= locx && x <= (locx + width)
                        && y >= locy && y <= (locy + height))
                {
                        (*this.*mouseFunc)(x - locx, y - locy);
                        return 1;
                }
	}
	
	return 0;
}

void KeyboardButton_gump::mouse_down(int mx, int my)
{
	// Find the SDL window...
        SDL_Window *window = NULL;
        unsigned int idWin = -1;
        while (window == NULL)
        {
                idWin++;
                window = SDL_GetWindowFromID(idWin);
        }
        SDL_iPhoneKeyboardToggle(window);
}

void KeyboardButton_gump::mouse_up(int mx, int my)
{

}

class Itemmenu_button : public Text_button {
public:
        Itemmenu_button(Gump *par, string text, int px, int py)
                : Text_button(par, text, px, py, 59, 20)
                { }
                                        // What to do when 'clicked':
        virtual bool activate(int button)
        {
                if (button != 1) return false;
		Itemmenu_gump *par = (Itemmenu_gump *)parent;	
		if (text == "Cancel")
		{
			par->close();
		}
		else if (par->objectAction == ITEMMENU_ACTION_NONE)
		{
			Gump_button_vector::const_iterator bit = par->buttons.begin();
			Game_object_vector::const_iterator oit = par->objects.begin();

		        for (;bit < par->buttons.end(); bit++, oit++)
			{
			   if (*bit == this)
			   {
			      par->objectSelected = ((Game_object *)*oit);
			      par->close();
			      break;
			   }
			}
		}
		else // if (par->objectAction == ITEMMENU_ACTION_MENU
		{
			if (text == "Use")
			{
			   par->objectAction = ITEMMENU_ACTION_USE;
			}
			else if (text == "Cancel")
			{
			   par->objectSelected = NULL;
			}
			par->close();
		}
                return true;
        }
};

Itemmenu_gump::Itemmenu_gump(Game_object_vector *vobjs, int cx, int cy) 
        : Modal_gump(0, cx, cy, EXULT_IPHONE_FLX_TRANSPARENTMENU_SHP, SF_IPHONE_FLX)
{
	objectSelected = NULL;
	objectAction = ITEMMENU_ACTION_NONE;
	int btop = 0;
	int byspace = 22;
        //set_object_area(Rectangle(0, 0, 0, 0), -1, -1);//++++++ ???
	Itemmenu_button *b;
	int i = 0;
	for (Game_object_vector::const_iterator it = vobjs->begin(); it < vobjs->end(); it++, i++)
	{
           Game_object *o = *it;
	   objects.push_back(o);
	   b = new Itemmenu_button(this, o->get_name().c_str(), 10, btop+(i*byspace));
	   buttons.push_back((Gump_button *)b);
	}
	b = new Itemmenu_button(this, "Cancel", 10, btop+(i*byspace));
	buttons.push_back((Gump_button *)b);
}

Itemmenu_gump::Itemmenu_gump(Game_object *obj, int cx, int cy)
        : Modal_gump(0, cx, cy, EXULT_IPHONE_FLX_TRANSPARENTMENU_SHP, SF_IPHONE_FLX)
{
	objectSelected = obj;
	objectAction = ITEMMENU_ACTION_MENU;
	int btop = 0;
	int byspace = 22;
	Itemmenu_button *b;
	int i = 0;
	b = new Itemmenu_button(this, "Use", 10, btop+(i*byspace));
	buttons.push_back((Gump_button *)b);
	i++;
	b = new Itemmenu_button(this, "Cancel", 10, btop+(i*byspace));
	buttons.push_back((Gump_button *)b);
}

Itemmenu_gump::~Itemmenu_gump()
{
        for (Gump_button_vector::const_iterator it = buttons.begin(); it < buttons.end(); it++)
	{
           Gump_button *b = *it;
	   delete b;
	}
}
	   
void Itemmenu_gump::paint()
{
	Gump::paint();
        for (Gump_button_vector::const_iterator it = buttons.begin(); it < buttons.end(); it++)
		(*it)->paint();
	gwin->set_painted();
}

bool Itemmenu_gump::mouse_down(int mx, int my, int button)
{
        // Only left and right buttons
        if (button != 1 && button != 3) return false;

        // We'll eat the mouse down if we've already got a button down
        if (pushed) return true;

        // First try checkmark
        pushed = Gump::on_button(mx, my);
                                        
        // Try buttons at bottom.
        if (!pushed) {
        	for (Gump_button_vector::const_iterator it = buttons.begin(); it < buttons.end(); it++)
		{
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

bool Itemmenu_gump::mouse_up(int mx, int my, int button)
{
        // Not Pushing a button?
        if (!pushed) return false;

        if (pushed->get_pushed() != button) return button == 1;

        bool res = false;
        pushed->unpush(button);
        if (pushed->on_button(mx, my))
                res = ((Gump_button*)pushed)->activate(button);
        pushed = 0;
        return res;
}

void Itemmenu_gump::postCloseActions()
{
	if (!objectSelected)
		return;

	Itemmenu_gump *itemgump;

	switch (objectAction)
	{
	   case ITEMMENU_ACTION_USE:
		objectSelected->activate();
		break;
	   case ITEMMENU_ACTION_NONE:
		// This will draw a selection menu for the object
                itemgump = new Itemmenu_gump(objectSelected, x, y);
                Game_window::get_instance()->get_gump_man()->do_modal_gump(itemgump, Mouse::hand);
                itemgump->postCloseActions();
                delete itemgump;
		break;
	   case ITEMMENU_ACTION_MENU:
		break;
	}	
}
#endif
