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

class Game_window;
class Shape_file;

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
	int draw_text(Game_window *gwin, int x, int y, const char *s);
	int draw_text_box(Game_window *gwin, int x, int y, int w, int h, const char *s);
	int center_text(Game_window *gwin, int x, int y, const char *s);
	int get_text_width(const char *s);
	int get_text_baseline();
	int get_text_height();
};

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
