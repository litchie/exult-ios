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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstring> 
#endif
#include "files/U7file.h" 
#include "gamewin.h"
#include "palette.h"
#include "ibuf8.h"
#include "utils.h"

#include "SDL_timer.h"

using std::memcpy;
using std::memset;
using std::size_t;

Palette::Palette()
	{
		win = Game_window::get_game_window()->get_win();
		brightness = 100;
		max_val = 63;
	}

Palette::~Palette()
	{
	}
	
void Palette::apply(bool repaint)
{
	win->set_palette(pal1, max_val, brightness);
	if (repaint)
		win->show();
}

/*
 *	Returns 0 if file not found.
 */
void Palette::load(const char *fname, int index, const char *xfname, int xindex)
	{
	size_t len;
	char *buf = 0;
	if (strncmp(fname, STATICDAT, sizeof(STATICDAT) - 1) == 0 &&
					is_system_path_defined("<PATCH>"))
		{			// Check in "patch" dir. first.
		string pname(PATCHDAT);
		pname += fname + sizeof(STATICDAT) - 1;
		U7object pal(pname.c_str(), index);
		try {
			buf = pal.retrieve(len);
		}
		catch (exult_exception& e) {
			buf = 0;
		}
		}
	if (!buf)			// Not in patch.
		{
		U7object pal(fname, index);
		buf = pal.retrieve(len);// this may throw an exception
		}
	if(len==768) {	// Simple palette
		if (xindex >= 0) {	// Get xform table.
			U7object xform(xfname, xindex);
			unsigned char *xbuf = 0;
			size_t xlen;
			try {
#if 0	/* +++++TESTING */
				xbuf = new unsigned char[256];
				Game_window *gwin = 
					Game_window::get_game_window();
				memcpy(xbuf, gwin->get_xform(11 - xindex - 1),
									256);
#else
				xbuf = (unsigned char *) xform.retrieve( xlen);
#endif
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
			unsigned char *xbuf; size_t xlen;
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
	if (Game_window::get_game_window()->get_fades_enabled())
	{
		unsigned char fade_pal[768];
		unsigned int ticks = SDL_GetTicks() + 20;
		for (int i = 0; i <= cycles; i++)
		{
			for(int c=0; c < 768; c++)
				fade_pal[c] = ((pal1[c]-pal2[c])*i)/cycles+pal2[c];
			win->set_palette(fade_pal, max_val, brightness);
			// Frame skipping on slow systems
			if (i == cycles || ticks >= SDL_GetTicks() ||
						!Game_window::get_game_window()->get_frame_skipping())
				win->show();
			while (ticks >= SDL_GetTicks())
				;
			ticks += 20;
		}
	}
	else
	{
		win->set_palette(pal1, max_val, brightness);
		win->show();
	}
}

void Palette::fade_out(int cycles)
{
	if (Game_window::get_game_window()->get_fades_enabled())
	{
		unsigned char fade_pal[768];
		unsigned int ticks = SDL_GetTicks() + 20;
		for (int i = cycles; i >= 0; i--)
		{
			for(int c=0; c < 768; c++)
				fade_pal[c] = ((pal1[c]-pal2[c])*i)/cycles+pal2[c];
			win->set_palette(fade_pal, max_val, brightness);
			// Frame skipping on slow systems
			if (i == 0 || ticks >= SDL_GetTicks() ||
						!Game_window::get_game_window()->get_frame_skipping())
				win->show();
			while (ticks >= SDL_GetTicks())
				;
			ticks += 20;
		}
	}
	else
	{
		win->set_palette(pal2, max_val, brightness);
		win->show();
	}
//Messes up sleep.	        win->set_palette(pal1, max_val, brightness);
}

//	Find index (0-255) of closest color (r,g,b < 64).
int Palette::find_color(int r, int g, int b) {
	int best_index = -1;
	long best_distance = 0xfffffff;
					// But don't search rotating colors.
	for (int i = 0; i < 0xe0; i++) {
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

/*
 *	Create a translucency table for this palette seen through a given
 *	color.  (Based on a www.gamedev.net article by Jesse Towner.)
 */

void Palette::create_trans_table
	(
					// Color to blend with:
	unsigned char br, unsigned bg, unsigned bb,
	int alpha,			// 0-255, applied to 'blend' color.
	unsigned char *table		// 256 indices are stored here.
	)
	{
	for (int i = 0; i < 256; i++)
		{
		int newr = ((int) br * alpha)/255 + 
				((int) pal1[i*3] * (255 - alpha))/255;
		int newg = ((int) bg * alpha)/255 + 
				((int) pal1[i*3 + 1] * (255 - alpha))/255;
		int newb = ((int) bb * alpha)/255 + 
				((int) pal1[i*3 + 2] * (255 - alpha))/255;
		table[i] = find_color(newr, newg, newb);
		}
	}

void Palette::show() {
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			win->fill8(y*16+x, 8, 8, x*8, y*8);
		}
	} 
}

void Palette::set_color(int nr, int r, int g, int b) {
	pal1[nr*3] = r;
	pal1[nr*3+1] = g;
	pal1[nr*3+2] = b;
}

void Palette::set_palette(unsigned char palnew[768]) {
	memcpy(pal1,palnew,768);
	memset(pal2,0,768);
}

void Palette::set_max_val(int max)
{
	max_val = max;
}

int Palette::get_max_val()
{
	return max_val;
}
void Palette::update()
{
	win->set_palette(pal1, max_val, brightness);
}	
