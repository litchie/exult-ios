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

#include <ctype.h>
#include "files/U7file.h"
#include "files/databuf.h"
#include "font.h"
#include "gamewin.h"
#include "imagewin.h"
#include "vgafile.h"

FontManager fontManager;

/*
 *	Pass space.
 */

static const char *Pass_space
	(
	const char *text
	)
	{
	while (isspace(*text))
		text++;
	return (text);
	}

/*
 *	Pass a word.
 */

static const char *Pass_word
	(
	const char *text
	)
	{
	while (*text && !isspace(*text))
		text++;
	return (text);
	}


Font::Font(): font_shapes(0)
{
}

Font::Font(const char *fname, int index, int hlead, int vlead): font_shapes(0)
{
	load(fname, index, hlead, vlead);
}

Font::~Font()
{
	if(font_shapes)
		delete font_shapes;
}

int Font::load(const char *fname, int index, int hlead, int vlead)
{
	if(font_shapes)
		delete font_shapes;
	char *font_buf;
	size_t len;
	U7object font_obj(fname, index);
	font_obj.retrieve(&font_buf, len);
	if(!strncmp(font_buf,"font",4))	// If it's an IFF archive...
		font_buf += 8;		// Skip first 8 bytes
	BufferDataSource *font_data = new BufferDataSource(font_buf, len);
	font_shapes = new Shape_file(*font_data);
	hor_lead = hlead;
	ver_lead = vlead;
	return 0;
}

int Font::draw_text(Game_window *gwin, int x, int y, const char *s)
{
	int xoff = x;
	int chr;
	int yoff = y+get_text_baseline();
	while ((chr = *s++) != 0) {
		Shape_frame *shape = font_shapes->get_frame(chr);
		if (!shape)
			continue;
		shape->paint_rle(gwin->get_win()->get_ib8(), x, yoff);
		x += shape->get_width() + hor_lead;
		}
	return (x - xoff);
}

int Font::center_text(Game_window *gwin, int x, int y, const char *s)
{
	return 0;
}

int Font::get_text_width(const char *s)
{
	int width = 0;
	short chr;
	while ((chr = *s++) != 0)
		width += font_shapes->get_frame(chr)->get_width() + hor_lead;
	return width;
}

int Font::get_text_baseline() 
{
	return font_shapes->get_frame('A')->get_yabove();
}

int Font::get_text_height()
{
	return get_text_baseline()+font_shapes->get_frame('y')->get_ybelow();
}

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
	// FIXME: free all fonts
}

void FontManager::add_font(const char *name, const char *archive, int index, int hlead, int vlead)
{
#if DEBUG
	if(fonts[name]!=0)
		cout << "font " << name << " already here" << endl;
	else
		cout << "adding font " << name << endl;
#endif
	Font *font = new Font(archive, index, hlead, vlead);
	
	fonts[name] = font;
}

Font *FontManager::get_font(const char *name)
{
	return fonts[name];
}
