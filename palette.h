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

#ifndef PALETTE_H
#define PALETTE_H

class Image_window8;

/*
 *	Palette #'s in 'palettes.flx':
 */
const int PALETTE_DAY = 0;
const int PALETTE_DUSK = 1;
const int PALETTE_DAWN = 1;		// Think this is it.
const int PALETTE_NIGHT = 2;
const int PALETTE_INVISIBLE = 3;	// When Avatar is invisible.
					// 4 looks just like #1.
const int PALETTE_HAZE = 5;
					// 6 looks a little brighter than #2.
					// 7 is somewhat warmer.  Torch?
const int PALETTE_RED = 8;		// Used when hit in combat.
					// 9 has lots of black.
const int PALETTE_LIGHTNING = 10;



class Palette
	{
		Image_window8 *win;
		unsigned char pal1[768];
		unsigned char pal2[768];
		int brightness;
		int max_val;
		
public:
		Palette();
		~Palette();
		void apply(bool repaint=true);
		void load(const char *fname, int index,
				const char *xfname = 0, int xindex = -1);
		void set_brightness(int bright);
		int get_brightness();
		void brighten(int percent);
		void set_max_val(int max);
		int get_max_val();
		void fade_in(int cycles);
		void fade_out(int cycles);
		int find_color(int r, int g, int b);
		void create_trans_table(unsigned char br, unsigned bg,
			unsigned bb, int alpha, unsigned char *table);
		void show();

		void set_color(int nr, int r, int g, int b);
		unsigned char get_red(int nr) { return pal1[3*nr]; }
		unsigned char get_green(int nr) { return pal1[3*nr + 1]; }
		unsigned char get_blue(int nr) { return pal1[3*nr + 2]; }
		void set_palette (unsigned char palnew[768]);
		void update();
	};
	

#endif
