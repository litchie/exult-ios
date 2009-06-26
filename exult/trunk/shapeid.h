/*
 *  Copyright (C) 2000-2003  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SHAPEID_H
#define SHAPEID_H	1

#include "exult_constants.h"
#include "shapevga.h"
#include "singles.h"
#include <vector>

class Shape_frame;
class Shape_info;
class Fonts_vga_file;
class Font;
class Image_buffer8;
struct Cursor_info;

enum ShapeFile {
	SF_SHAPES_VGA = 0,	// <STATIC>/shapes.vga.  MUST be first.
	SF_GUMPS_VGA,		// <STATIC>/gumps.vga
	SF_PAPERDOL_VGA,	// <STATIC>/paperdol.vga
	SF_SPRITES_VGA,		// <STATIC>/sprites.vga
	SF_FACES_VGA,		// <STATIC>/faces.vga
	SF_EXULT_FLX,		// <DATA>/exult.flx
	SF_GAME_FLX,		// <DATA>/bg_data.flx or <DATA>/si_data.flx
	// Not yet
	//SF_FONTS_VGA,		// <STATIC>/fonts.vga
#ifdef UNDER_CE
	SF_POCKETPC_FLX,
#endif
	SF_OTHER,		// Other unknown FLX
	SF_COUNT		// # of preceding entries.
};

					// Special pixels.
enum Pixel_colors {POISON_PIXEL = 0, PROTECT_PIXEL, CURSED_PIXEL,
			CHARMED_PIXEL, HIT_PIXEL, PARALYZE_PIXEL, NPIXCOLORS};

/*
 *	Manage the set of shape files.
 */
class Shape_manager : public Game_singletons
	{
	static Shape_manager *instance;	// There shall be only one.
	Shapes_vga_file shapes;		// Main 'shapes.vga' file.
	Vga_file files[(int) SF_COUNT];	// The files we manage.
	Fonts_vga_file *fonts;		// "fonts.vga" file.
	std::vector<Xform_palette> xforms;	// Transforms translucent colors
					//   0xf4 through 0xfe.
	Xform_palette *invis_xform;	// For showing invisible NPC's.
	unsigned char special_pixels[NPIXCOLORS];	// Special colors.
	bool can_have_paperdolls;	// Set true if the SI paperdoll file 
					//   is found when playing BG
	bool paperdolls_enabled;	// True if paperdolls are on.
	bool got_si_shapes;	// Set true if the SI shapes file 
					//   is found when playing BG
	void read_shape_info();
public:
	friend class ShapeID;
	Shape_manager();
	~Shape_manager();
	static Shape_manager *get_instance()
		{ return instance; }
	void load();			// Read in files.
	void reload_shapes(int dragtype);	// Reload a shape file.
	void reload_shape_info();
	Vga_file& get_file(enum ShapeFile f)
		{ return files[(int) f]; };
	Shapes_vga_file& get_shapes()
		{ return shapes; }
	inline Xform_palette& get_xform(int i)
		{ return xforms[i]; }
	// BG Only
	inline bool can_use_paperdolls() const
	{ return can_have_paperdolls; }

	inline bool are_paperdolls_enabled() const
	{ return paperdolls_enabled; }

	inline void set_paperdoll_status(bool p)
	{ paperdolls_enabled = p; }

	inline bool have_si_shapes() const
	{ return got_si_shapes; }

					// Paint shape in window.
	void paint_shape(int xoff, int yoff, Shape_frame *shape,
						int translucent = 0, unsigned char *trans = 0)
		{
		if (!shape || !shape->data)
			CERR("NULL SHAPE!!!");
		else if (!shape->rle)
			shape->paint(xoff, yoff);
		else if (trans)
			shape->paint_rle_remapped(xoff, yoff, trans);
		else if (!translucent)
			shape->paint_rle(xoff, yoff);
		else
			shape->paint_rle_translucent(xoff, yoff, &xforms[0], xforms.size());
		}

	inline void paint_invisible(int xoff, int yoff, Shape_frame *shape)
		{
		if (shape) shape->paint_rle_transformed(
						xoff, yoff, *invis_xform);
		}
					// Paint outline around a shape.
	inline void paint_outline(int xoff, int yoff, Shape_frame *shape, 
							Pixel_colors pix)
		{
		if (shape) shape->paint_rle_outline(
					xoff, yoff, special_pixels[(int) pix]);
		}
	unsigned char get_special_pixel(Pixel_colors pix)
		{ return special_pixels[(int) pix]; }

					// Paint text using "fonts.vga".
	int paint_text_box(int fontnum, const char *text, int x, int y, int w, 
		int h, int vert_lead = 0, bool pbreak = false,
		bool center = false, int shading = -1,Cursor_info *cursor = 0);
	int paint_text(int fontnum, const char *text, int xoff, int yoff);
	int paint_text(int fontnum, const char *text, int textlen, 
							int xoff, int yoff);
					// Get text width.
	int get_text_width(int fontnum, const char *text);
	int get_text_width(int fontnum, const char *text, int textlen);
					// Get text height, baseline.
	int get_text_height(int fontnum);
	int get_text_baseline(int fontnum);
	int find_cursor(int fontnum, const char *text, int x, int y, 
				int w, int h, int cx, int cy, int vert_lead);
	Font *get_font(int fontnum);
	};

/*
 *	A shape ID contains a shape # and a frame # within the shape encoded
 *	as a 2-byte quantity.
 */
class ShapeID : public Game_singletons
	{
	short shapenum;			// Shape #.
	signed char framenum;		// Frame # within shape.
	char has_trans;
	ShapeFile shapefile;
	Shape_frame *shape;
	Shape_info *info;

	Shape_frame* cache_shape();

public:
					// Read from buffer & incr. ptr.
	ShapeID(unsigned char *& data)
		: has_trans(0), shapefile(SF_SHAPES_VGA), shape(0), info(0)
		{
		unsigned char l = *data++;
		unsigned char h = *data++;
		shapenum = l + 256*(h&0x3);
		framenum = h >> 2;
		}
					// Create "end-of-list"/invalid entry.
	ShapeID() : shapenum(-1), framenum(-1), has_trans(0),
			shapefile(SF_SHAPES_VGA), shape(0), info(0)
		{  }

    virtual ~ShapeID()
        {  }
					// End-of-list or invalid?
	int is_invalid() const
		{ return shapenum == -1; }
	int is_eol() const
		{ return is_invalid(); }

	inline int get_shapenum() const
		{ return shapenum; }
	inline int get_framenum() const
		{ return framenum; }
	inline ShapeFile get_shapefile() const
		{ return shapefile; }
	inline Shape_frame *get_shape()
		{ return (shape!=0)?shape:cache_shape(); }
	inline void set_translucent(int trans)
		{ has_trans=trans; }
	inline bool is_translucent()
		{ if (shape==0) cache_shape(); return has_trans!=0; }
					// Set to given shape.
	void set_shape(int shnum, int frnum)
		{
		shapenum = shnum;
		framenum = frnum;
		shape = 0;
		info = 0;
		}
	ShapeID(int shnum, int frnum, ShapeFile shfile = SF_SHAPES_VGA) :
		shapenum(shnum), framenum(frnum), has_trans(0),
		shapefile(shfile), shape(0), info(0)
		{  }

	void set_shape(int shnum)	// Set shape, but keep old frame #.
		{ shapenum = shnum; shape = 0; info = 0; }
	void set_frame(int frnum)	// Set to new frame.
		{ framenum = frnum; shape = 0; }
	void set_file(ShapeFile shfile)	// Set to new flex
		{ shapefile = shfile; shape = 0; }

	void paint_shape(int xoff, int yoff, bool force_trans = false)
		{
		sman->paint_shape(xoff, yoff, get_shape(), 
						has_trans || force_trans);
		}
	void paint_invisible(int xoff, int yoff)
		{ sman->paint_invisible(xoff, yoff, get_shape()); }
					// Paint outline around a shape.
	inline void paint_outline(int xoff, int yoff, Pixel_colors pix)
		{ sman->paint_outline(xoff, yoff, get_shape(), pix); }
	int get_num_frames() const;
#if 0
	//This would be very bad form, but who knows?
	Shape_info& get_info() const	// Get info. about shape.
		{ return *(info ? info :
			const_cast<ShapeID *>(this)->info = 
				&Shape_manager::instance->shapes.get_info(shapenum)); }
#endif
	Shape_info& get_info() const	// Get info. about shape.
		{ return info ? *info :
			Shape_manager::instance->shapes.get_info(shapenum); }
	Shape_info& get_info()	// Get info. about shape.
		{ return *(info ? info : info =
			&Shape_manager::instance->shapes.get_info(shapenum)); }
	static Shape_info& get_info(int shnum)	// Get info. about shape.
		{ return Shape_manager::instance->shapes.get_info(shnum); }
	};

/*
 *	An interface used in Get_click():
 */
class Paintable
	{
public:
	virtual void paint() = 0;
	virtual ~Paintable() {  };
	};

#endif
