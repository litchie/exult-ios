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

#ifndef PALETTE_H
#define PALETTE_H

#include "iwin8.h"

class Palette
	{
		Image_window8 *win;
		unsigned char pal1[768];
		unsigned char pal2[768];
		int brightness;
		
public:
		Palette();
		~Palette();
		void apply();
		void load(const char *fname, int index,
				const char *xfname = 0, int xindex = -1);
		void set_brightness(int bright);
		int get_brightness();
		void brighten(int percent);
		void fade_in(int cycles);
		void fade_out(int cycles);
		int find_color(int r, int g, int b);
		void show();
	};
	

#endif
