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

#include <string.h> 
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
	
/*
 *	Returns 0 if file not found.
 */
void Palette::load(const char *fname, int index, const char *xfname, int xindex)
	{
	U7object pal(fname, index);
	size_t len;
	char *buf;
	pal.retrieve(&buf, len);	// this may throw an exception
	if(len==768) {	// Simple palette
		if (xindex >= 0) {	// Get xform table.
			U7object xform(xfname, xindex);
			char *xbuf; size_t xlen;
			try {
				xform.retrieve(&xbuf, xlen);
				for (int i = 0; i < 256; i++) {
					int ix = xbuf[i];
					pal1[3*i] = buf[3*ix];
					pal1[3*i+1] = buf[3*ix+1];
					pal1[3*i+2] = buf[3*ix+2];
				}
			}
			catch( const std::exception & err ) {
				xindex = -1;
			}
			delete [] xbuf;
		}
		if (xindex < 0)		// Set the first palette
			memcpy(pal1,buf,768);
		memset(pal2,0,768);	// The second one is black
	} else {			// Double palette
		for(int i=0; i<768; i++) {
			pal1[i]=buf[i*2];
			pal2[i]=buf[i*2+1];
		}
	}
	delete [] buf;
	}
/*void Palette::load(const char *fname, int index, const char *xfname, int xindex)
	{
	U7object pal(fname, index);
	size_t len;
	char *buf;
	pal.retrieve(&buf, len);	// this may throw an exception
	if(len==768) {	// Simple palette
		if (xindex >= 0)	// Get xform table.
			{
			U7object xform(xfname, xindex);
			char *xbuf; size_t xlen;
			if (!xform.retrieve(&xbuf, xlen))
				xindex = -1;
			else
				for (int i = 0; i < 256; i++)
					{
					int ix = xbuf[i];
					pal1[3*i] = buf[3*ix];
					pal1[3*i+1] = buf[3*ix+1];
					pal1[3*i+2] = buf[3*ix+2];
					}
			delete [] xbuf;
			}
		if (xindex < 0)		// Set the first palette
			memcpy(pal1,buf,768);
		memset(pal2,0,768);	// The second one is black
	} else {			// Double palette
		for(int i=0; i<768; i++) {
			pal1[i]=buf[i*2];
			pal2[i]=buf[i*2+1];
		}
	}
	delete [] buf;
	}
*/
	
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
 	        if (Game_window::get_game_window()->get_fades_enabled()) {
		        unsigned char fade_pal[768];
		        unsigned int ticks = SDL_GetTicks() + 20;
			for (int i = 0; i <= cycles; i++) {
			        for(int c=0; c < 768; c++)
				        fade_pal[c] = ((pal1[c]-pal2[c])*i)/cycles+pal2[c];
				win->set_palette(fade_pal, 63);
				win->show();
				while (ticks >= SDL_GetTicks())
					;
				ticks+= 20;
			}
		} else {
		        win->set_palette(pal1, 63);
		}
	}

void Palette::fade_out(int cycles)
	{
	        if (Game_window::get_game_window()->get_fades_enabled()) {
		        unsigned char fade_pal[768];
		        unsigned int ticks = SDL_GetTicks() + 20;
		        for (int i = cycles; i >= 0; i--) {
			        for(int c=0; c < 768; c++)
				        fade_pal[c] = ((pal1[c]-pal2[c])*i)/cycles+pal2[c];
				win->set_palette(fade_pal, 63);
				win->show();
				while (ticks >= SDL_GetTicks())
				  ;
				ticks+= 20;
			}
		} else {
		        win->set_palette(pal1, 63);
		}
	}

//	Find index (0-255) of closest color (r,g,b < 64).
int Palette::find_color(int r, int g, int b) {
	int best_index = -1;
	long best_distance = 0xfffffff;
	for (int i = 0; i < 256; i++) {
					// Get deltas.
		long dr = r - pal1[3*i], dg = g - pal1[3*i + 1], 
							db = b - pal1[3*i + 2];
					// Figure distance-squared.
		long dist = dr*dr + dg*dg + db*db;
		if (dist < best_distance) {	// Better than prev?
			best_index = i;
			best_distance = dist;
		}
	}
	return best_index;
}
