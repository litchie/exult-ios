/**	-*-mode: Fundamental; tab-width: 8; -*-
**
 **	Fontvga.h - Handle the 'fonts.vga' file and text rendering.
 **
 **	Written: 4/29/99 - JSF
 **/

#ifndef INCL_FONTVGA
#define INCL_FONTVGA	1

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include <fstream>
#include <iostream>
#ifdef MACOS
  #include <cassert>
#endif
#include "fnames.h"
#include "vgafile.h"

/*
 *	The "fonts.vga" file:
 */
class Fonts_vga_file : public Vga_file
	{
public:
	Fonts_vga_file()
		{  }
	void init() { load(FONTS_VGA); }
					// Text rendering:
	int paint_text_box(Image_buffer8 *win, int fontnum, 
		const char *text, int x, int y, int w, 
		int h, int vert_lead = 0, int pbreak = 0);
	int paint_text(Image_buffer8 *win, int fontnum, 
		const char *text, int xoff, int yoff);
	int paint_text(Image_buffer8 *win, int fontnum, 
		const char *text, int textlen, int xoff, int yoff);
					// Get text width.
	int get_text_width(int fontnum, const char *text);
	int get_text_width(int fontnum, const char *text, int textlen);
					// Get text height, baseline.
	int get_text_height(int fontnum);
	int get_text_baseline(int fontnum);
	Shape_frame *font_get_shape (int fontnum, int framenum)
		{ return get_shape(fontnum, framenum); }
	};

#endif
