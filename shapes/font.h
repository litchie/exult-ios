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

#include "hash_utils.h"

class Image_buffer8;
class Shape_file;
class DataSource;

/*
 *	A single font:
 */
class Font
{
private:
	int hor_lead;
	int ver_lead;
	Shape_file *font_shapes;
	DataSource *font_data;
	char *font_buf;
	char *orig_font_buf;
	int  highest, lowest;

	void calc_highlow();
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
	int paint_text_box_fixedwidth(Image_buffer8 *win,  
		const char *text, int x, int y, int w, 
		int h, int char_width, int vert_lead = 0, int pbreak = 0);
	int paint_text_fixedwidth(Image_buffer8 *win,  
		const char *text, int xoff, int yoff, int width);
	int paint_text_fixedwidth(Image_buffer8 *win,  
		const char *text, int textlen, int xoff, int yoff, int width);
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
#ifndef DONT_HAVE_HASH_MAP
	hash_map<const char*, Font*, hashstr, eqstr> fonts;
#else
	std::map<const char*, Font*, ltstr> fonts;
#endif
public:
	FontManager();
	~FontManager();
	void add_font(const char *name, const char *archive, int index, int hlead=0, int vlead=1);
	void remove_font(const char *name);
	Font *get_font(const char *name);

	void reset();
};

extern FontManager fontManager;

#endif
