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

#include "files/U7file.h" 
#include "gamewin.h"
#include "palette.h"

Palette::Palette()
	{
		win = Game_window::get_game_window()->get_win();
		brightness = 100;
	}

Palette::~Palette()
	{
	}
	
void Palette::apply()
	{
		win->set_palette(pal1, 63, brightness);
		win->show();
	}
	
void Palette::load(const char *fname, int index)
	{
		U7object pal(fname, index);
		size_t len;
		char *buf;
		pal.retrieve(&buf, len);
		if(len==768) {	// Simple palette
			memcpy(pal1,buf,768);	// Set the first palette
			memset(pal2,0,768);		// The second one is black
		} else {			// Double palette
			for(int i=0; i<768; i++) {
				pal1[i]=buf[i*2];
				pal2[i]=buf[i*2+1];
			}
		}
		delete [] buf;
	}
	
void Palette::set_brightness(int bright)
	{
		brightness = bright;
	}
	
int Palette::get_brightness()
	{
		return brightness;
	}
	
void Palette::brighten(int percent)
	{
		brightness += percent;
		if(brightness>100)
			brightness = 100;
		if(brightness<20)
			brightness = 20;		
	}
	
void Palette::fade_in(int cycles)
	{
		unsigned char fade_pal[768];
		for (int i = 0; i <= cycles; i++) {
			for(int c=0; c < 768; c++)
				fade_pal[c] = ((pal1[c]-pal2[c])*i)/cycles+pal2[c];
				win->set_palette(fade_pal, 63);
				win->show();
				SDL_Delay(20);
		}
	}

void Palette::fade_out(int cycles)
	{
		unsigned char fade_pal[768];
		for (int i = cycles; i >= 0; i--) {
			for(int c=0; c < 768; c++)
				fade_pal[c] = ((pal1[c]-pal2[c])*i)/cycles+pal2[c];
				win->set_palette(fade_pal, 63);
				win->show();
				SDL_Delay(20);
		}
	}
