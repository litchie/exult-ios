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

#ifndef FONT_H
#define FONT_H

#include "files/utils.h"

#ifdef MACOS
  #include <hashmap.h>
#else
  #include <hash_map>
#endif

class Image_buffer8;
class Shape_file;

/*
 *	A single font:
 */
class Font
{
private:
	int hor_lead;
	int ver_lead;
	Shape_file *font_shapes;
public:
	Font();
	Font(const char *fname, int index, int hlead=0, int vlead=1);
	~Font();
	int load(const char *fname, int index, int hlead=0, int vlead=1);
					// Text rendering:
	int paint_text_box(Image_buffer8 *win,  
		const char *text, int x, int y, int w, 
		int h, int vert_lead = 0, int pbreak = 0);
	int paint_text(Image_buffer8 *win,  
		const char *text, int xoff, int yoff);
	int paint_text(Image_buffer8 *win,  
		const char *text, int textlen, int xoff, int yoff);
					// Get text width.
	int get_text_width(const char *text);
	int get_text_width(const char *text, int textlen);
					// Get text height, baseline.
	int get_text_height();
	int get_text_baseline();

	int draw_text(Image_buffer8 *win, int x, int y, const char *s)
		{ return paint_text(win, s, x, y); }
	int draw_text_box(Image_buffer8 *win, 
				int x, int y, int w, int h, const char *s)
		{ return paint_text_box(win, s, x, y, w, h, 0, 0); }
	int center_text(Image_buffer8 *iwin, int x, int y, const char *s);
};

/*
 *	Manage a list of fonts by name.
 */
class FontManager
{
private:
	std::hash_map<const char*, Font*, hashstr, eqstr> fonts;
public:
	FontManager();
	~FontManager();
	void add_font(const char *name, const char *archive, int index, int hlead=0, int vlead=1);
	Font *get_font(const char *name);
};

extern FontManager fontManager;

#endif
