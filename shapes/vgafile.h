/*
 *	vgafile.h - Handle access to one of the xxx.vga files.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef VGAFILE_H
#define VGAFILE_H	1


#include <fstream>
#include <iostream>
#include <cassert>
#include "exult_types.h"
#include "exult_constants.h"
#include "imagebuf.h"
#include <vector>
#include <utility>
#include <string>
#include <map>

class DataSource;
class StreamDataSource;
class Shape;
class Image_buffer8;
class GL_texshape;
class GL_manager;
class Palette;

#ifdef HAVE_OPENGL
#include "glshape.h"

#define GLRENDER	if (glman) glman->paint(this, px, py); else
#define GLRENDERTRANS	if (glman) glman->paint(this, px, py, xforms, xfcnt); \
								else
#define GLOUTLINE	if (glman) ; else
#else
#define GLRENDER
#define GLRENDERTRANS
#define GLOUTLINE
#endif

/*
 *	A shape from "shapes.vga":
 */
class Shape_frame
	{
	bool rle;			// Run-length encoded.
	unsigned char *data;		// The actual data.
	int datalen;
	short xleft;			// Extent to left of origin.
	short xright;			// Extent to right.
	short yabove;			// Extent above origin.
	short ybelow;			// Extent below origin.
#ifdef HAVE_OPENGL
	GL_texshape *glshape;		// OpenGL texture for painting this.
#endif
	static GL_manager *glman;	// One to rule them all.
	static Image_buffer8 *scrwin;	// Screen window to render to.

	Shape_frame *reflect();		// Create new frame, reflected.
					// Create RLE data & store in frame.
	void create_rle(unsigned char *pixels, int w, int h);
					// Create from RLE entry.
	void get_rle_shape(DataSource* shapes, long filepos, long len);
	
public:
	friend class Game_window;
	friend class Shape_manager;
	friend class Shape;
	friend class Shape_file;
	friend class GL_texshape;
	friend class GL_manager;
	Shape_frame() : data(0), datalen(0)
#ifdef HAVE_OPENGL
		, glshape(0)
#endif
		{  }
					// Create frame from data.
	Shape_frame(unsigned char *pixels, int w, int h, int xoff, int yoff,
								bool setrle);
	static void set_to_render(Image_buffer8 *w, GL_manager *gl = 0)
		{ scrwin = w; glman = gl; }
	unsigned char *get_data() { return data; }
	bool is_rle() const { return rle; }
					// Convert raw image to RLE.
	static unsigned char *encode_rle(unsigned char *pixels, int w, int h,
					int xoff, int yoff, int& datalen);
					// Read in shape/frame.
	unsigned int read(DataSource* shapes, uint32 shapeoff,
					uint32 shapelen, int frnum);
					// Paint into given buffer.
	void paint_rle(Image_buffer8 *win, int px, int py);
	void paint_rle_remapped(Image_buffer8 *win, int px, int py, unsigned char *trans);
	void paint(Image_buffer8 *win, int px, int py);
	void paint_rle_translucent(Image_buffer8 *win, int px, int py,
					Xform_palette *xforms, int xfcnt);
	void paint_rle_transformed(Image_buffer8 *win, int px, int py,
					Xform_palette& xform);
	void paint_rle_outline(Image_buffer8 *win, int px, int py,
							unsigned char color);
					// Paint to screen.
	void paint_rle(int px, int py)
		{ GLRENDER  paint_rle(scrwin, px, py); }
	void paint_rle_remapped(int px, int py, unsigned char *trans)
		{ GLRENDER  paint_rle_remapped(scrwin, px, py, trans); }
	void paint(int px, int py)
		{ GLRENDER  paint(scrwin, px, py); }
					// ++++++GL versions of these needed:
	void paint_rle_translucent(int px, int py,
					Xform_palette *xforms, int xfcnt)
		{ GLRENDERTRANS  paint_rle_translucent(
					scrwin, px, py, xforms, xfcnt); }
	void paint_rle_transformed(int px, int py, Xform_palette& xform)
		{ GLRENDER  paint_rle_transformed(scrwin, px, py, xform); }
	void paint_rle_outline(int px, int py, unsigned char color)
		{ GLOUTLINE  paint_rle_outline(scrwin, px, py, color); }

	int has_point(int x, int y);	// Is a point within the shape?
	int get_width() const		// Get dimensions.
		{ return xleft + xright + 1; }
	int get_height() const
		{ return yabove + ybelow + 1; }
	int get_xleft()
		{ return xleft; }
	int get_xright()
		{ return xright; }
	int get_yabove()
		{ return yabove; }
	int get_ybelow()
		{ return ybelow; }
	void set_offset(int new_xright, int new_ybelow);
	int get_size()
		{ return datalen; }
	int is_empty()
		{ return data[0] == 0 && data[1] == 0; }
	virtual ~Shape_frame()
		{ delete [] data; }
	};

/*
 *	A shape from a .vga file consists of one of more frames.
 */
class Shape
	{
protected:
	Shape_frame **frames;		// List of ->'s to frames.
	unsigned int frames_size;	// Size of 'frames' (incl. reflects).
	unsigned int num_frames;	// # of frames (not counting reflects).
	bool modified;
	bool from_patch;
					// Create reflected frame.
	Shape_frame *reflect(std::vector<std::pair<DataSource *,bool> > shapes, int shnum,
		int frnum, std::vector<int> counts);
	void enlarge(int newsize);	// Increase 'frames'.
	void create_frames_list(int nframes);
					// Read in shape/frame.
	Shape_frame *read(std::vector<std::pair<DataSource *,bool> > shapes, int shnum,
		int frnum, std::vector<int> counts, int src=-1);
					// Store shape that was read.
	Shape_frame *store_frame(Shape_frame *frame, int framenum);
public:
	friend class Vga_file;
	
	Shape() : frames(0), frames_size(0), num_frames(0),
			modified(false), from_patch(false)
		{  }
	Shape(Shape_frame* fr);
	Shape(int n);			// Create with given #frames.	
	virtual ~Shape();
	void reset();
	void take(Shape *s2);		// Take frames from another shape.
	void load(DataSource* shape_source);
	void write(std::ostream& out);	// Write out.
	void set_modified()
		{ modified = true; }
	bool get_modified()
		{ return modified; }
	void set_from_patch()
		{ from_patch = true; }
	bool get_from_patch()
		{ return from_patch; }
	Shape_frame *get(std::vector<std::pair<DataSource *,bool> > shapes, int shnum,
		int frnum, std::vector<int> counts, int src=-1)
		{ 
		return (frames && frnum < frames_size && frames[frnum]) ? 
			frames[frnum] : 
			read(shapes, shnum, frnum, counts, src); 
		}
	int get_num_frames()
		{ return num_frames; }
	Shape_frame *get_frame(int framenum)
		{ return 0 <= framenum && framenum < frames_size 
						? frames[framenum] : 0L; }
	void resize(int newsize);	// Modify #frames.
					// Set frame.
	void set_frame(Shape_frame *f, int framenum);
					// Add/insert frame.
	void add_frame(Shape_frame *f, int framenum);
	void del_frame(int framenum);
	};

/*
 *	A shape file just has one shape with multiple frames.  They're all
 *	read in during construction.
 */
class Shape_file : public Shape
	{
public:
	Shape_file(const char *nm);
	Shape_file(Shape_frame *fr): Shape(fr) {}
	Shape_file(DataSource* shape_source);
	Shape_file();
	virtual ~Shape_file() {}
	void load(const char *nm);
	void load(DataSource* shape_source)
		{ Shape::load(shape_source); }
	int get_size();
	void save(DataSource* shape_source);
	};

/*
 *	A class for accessing any .vga file:
 */
class Vga_file
	{
	struct imported_map
		{
		int realshape;
		int pointer_offset;
		int source_offset;
		};
	std::vector<std::ifstream *> files;
	std::vector<std::pair<DataSource *,bool> > shape_sources;
	std::vector<std::ifstream *> imported_files;
	std::vector<std::pair<DataSource *,bool> > imported_sources;
	std::map<int, imported_map> imported_shape_table;
	int u7drag_type;		// # from u7drag.h, or -1.
	bool flex;			// This is the normal case (all .vga
					//   files).  If false, file is a
					//   single shape, like 'pointers.shp'.
					// In this case, all frames are pre-
					//   loaded.
protected:
	int num_shapes;			// Total # of shapes.
	std::vector<int> shape_cnts;	// Total # of shapes in each file.
	std::vector<int> imported_cnts;	// Total # of shapes in each file.
	Shape *shapes;				// List of ->'s to shapes' lists
	std::vector<Shape *> imported_shapes;	// List of ->'s to shapes' lists
public:
	Vga_file(const char *nm, int u7drag = -1, const char *nm2 = 0);
	Vga_file(std::vector<std::pair<std::string, int> > sources, int u7drag = -1);
	Vga_file();
	int get_u7drag_type() const
		{ return u7drag_type; }
	DataSource *U7load(std::pair<std::string, int> resource,
		std::vector<std::ifstream *> &fs, std::vector<std::pair<DataSource *,bool> > &shps);
	bool load(const char *nm, const char *nm2 = 0);
	bool load(std::vector<std::pair<std::string, int> > sources);
	bool import_shapes(std::pair<std::string, int> source,
		std::vector<std::pair<int, int> > imports);
	bool is_shape_imported(int shnum);
	bool get_imported_shape_data(int shnum, imported_map& data);
	void reset();
	void reset_imports();
	virtual ~Vga_file();
	int get_num_shapes() const
	{ return num_shapes; }
	int is_good() const
		{ return shapes != 0; }
	bool is_flex() const
		{ return flex; }
					// Get shape.
	Shape_frame *get_shape(int shapenum, int framenum = 0)
		{
		Shape_frame *r;
		imported_map data;
		if (get_imported_shape_data(shapenum, data))
			{
			// The shape is imported from another file.
			// Import table was set but the source could not be opened.
			if (data.pointer_offset < 0)
				return 0;
			r = (imported_shapes[data.pointer_offset]->get(imported_sources,
					data.realshape, framenum, imported_cnts, data.source_offset));
			}
		else
			{
			assert(shapes!=0);	// Because if shapes is NULL
						// here, we won't die on the deref
						// but we will return rubbish.
			// I've put this assert in _before_ you know...
			// So this isn't the first time we've had trouble here
			r = (shapes[shapenum].get(shape_sources, shapenum, 
				framenum, shape_cnts, -1));
			}
		return r;
		}
		
	Shape *extract_shape(int shapenum)
		{
		imported_map data;
		bool is_imported = get_imported_shape_data(shapenum, data);
		if (is_imported)
			assert(imported_shapes.size());	// For safety.
		else
			assert(shapes!=0);
		// Load all frames into memory
		int count = get_num_frames(shapenum);
		for(int i=1; i<count; i++)
			get_shape(shapenum, i);
		if (is_imported)
			return imported_shapes[data.pointer_offset];
		else
			return shapes+shapenum;
		}
					// Get # frames for a shape.
	int get_num_frames(int shapenum)
		{
		get_shape(shapenum, 0);	// Force it into memory.
		imported_map data;
		if (get_imported_shape_data(shapenum, data))
			return imported_shapes[data.pointer_offset]->num_frames;
		else
			return shapes[shapenum].num_frames;
		}
					// Create new shape (or del old).
	virtual Shape *new_shape(int shapenum);	
	};
	
#endif	/* VGAFILE_H */
