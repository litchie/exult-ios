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
 *  GNU Library General Public License for more details.
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
#include "imagebuf.h"

class DataSource;
class StreamDataSource;
class Shape;
class Image_buffer8;

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
	Shape_frame *reflect();		// Create new frame, reflected.
					// Create RLE data & store in frame.
	void create_rle(unsigned char *pixels, int w, int h);
					// Create from RLE entry.
	void get_rle_shape(DataSource* shapes, long filepos, long len);
	
public:
	friend class Game_window;
	friend class Shape;
	friend class Shape_file;
	Shape_frame() : data(0), datalen(0)
		{  }
					// Create frame from data.
	Shape_frame(unsigned char *pixels, int w, int h, int xoff, int yoff,
								bool setrle);
	unsigned char *get_data() { return data; }
	bool is_rle() const { return rle; }
					// Convert raw image to RLE.
	static unsigned char *encode_rle(unsigned char *pixels, int w, int h,
					int xoff, int yoff, int& datalen);
					// Read in shape/frame.
	unsigned char read(DataSource* shapes, uint32 shapeoff,
					uint32 shapelen, int frnum);
					// Paint.
	void paint_rle(Image_buffer8 *win, int xoff, int yoff);
	void paint(Image_buffer8 *win, int xoff, int yoff);
	void paint_rle_translucent(Image_buffer8 *win, int xoff, int yoff,
					Xform_palette *xforms, int xfcnt);
	void paint_rle_transformed(Image_buffer8 *win, int xoff, int yoff,
					Xform_palette xform);
	void paint_rle_outline(Image_buffer8 *win, int xoff, int yoff,
							unsigned char color);
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
	unsigned char frames_size;	// Size of 'frames' (incl. reflects).
	unsigned char num_frames;	// # of frames (not counting reflects).
					// Create reflected frame.
	Shape_frame *reflect(DataSource* shapes, int shnum, int frnum);
	void enlarge(int newsize);	// Increase 'frames'.
	void create_frames_list(int nframes);
					// Read in shape/frame.
	Shape_frame *read(DataSource* shapes, int shnum, int frnum, 
		DataSource *shapes2 = 0, int count1 = -1, int count2 = -1);
					// Store shape that was read.
	Shape_frame *store_frame(Shape_frame *frame, int framenum);
public:
	friend class Vga_file;
	
	Shape() : frames(0), frames_size(0), num_frames(0)
		{  }
	Shape(Shape_frame* fr);
	Shape(int n);			// Create with given #frames.	
	virtual ~Shape();
	void reset();
	void take(Shape *s2);		// Take frames from another shape.
	void load(DataSource* shape_source);
	void write(std::ostream& out);	// Write out.
	Shape_frame *get(DataSource* shapes, int shnum, int frnum, 
		DataSource *shapes2 = 0, int count1 = -1, int count2 = -1)
		{ 
		return (frames && frnum < frames_size && frames[frnum]) ? 
			frames[frnum] : 
			read(shapes, shnum, frnum, shapes2, count1, count2); 
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
	std::ifstream file;
	DataSource *shape_source;
	std::ifstream file2;
	DataSource *shape_source2;
	int u7drag_type;		// # from u7drag.h, or -1.
	bool flex;			// This is the normal case (all .vga
					//   files).  If false, file is a
					//   single shape, like 'pointers.shp'.
					// In this case, all frames are pre-
					//   loaded.
protected:
	int num_shapes;			// Total # of shapes.
	int num_shapes1;		// Total # of shapes in file 1.
	int num_shapes2;		// Total # of shapes in file 2.
	Shape *shapes;			// List of ->'s to shapes' lists
public:
	Vga_file(const char *nm, int u7drag = -1, const char *nm2 = 0);
	Vga_file();
	int get_u7drag_type() const
		{ return u7drag_type; }
	void load(const char *nm, const char *nm2 = 0);
	void reset();
	virtual ~Vga_file();
	int get_num_shapes() const
	{ return num_shapes>num_shapes2?num_shapes:num_shapes2; }
	int is_good() const
		{ return shapes != 0; }
	bool is_flex() const
		{ return flex; }
					// Get shape.
	Shape_frame *get_shape(int shapenum, int framenum = 0)
		{
		assert(shapes!=0);	// Because if shapes is NULL
					// here, we won't die on the deref
					// but we will return rubbish.
		// I've put this assert in _before_ you know...
		// So this isn't the first time we've had trouble here
		Shape_frame *r=(shapes[shapenum].get(shape_source, shapenum, 
			framenum, shape_source2, num_shapes1, num_shapes2));
#if 0	/* Occurs normally in ExultStudio. */
		if(!r)
			{
#ifdef DEBUG
				std::cerr << "get_shape(" <<
					shapenum << "," <<
					framenum << ") -> NULL" << std::endl;
#endif
			}
#endif
		return r;
		}
		
	Shape *extract_shape(int shapenum)
		{
		assert(shapes!=0);
		// Load all frames into memory
		int count = get_num_frames(shapenum);
		for(int i=1; i<count; i++)
			get_shape(shapenum, i);
		return shapes+shapenum;
		}
					// Get # frames for a shape.
	int get_num_frames(int shapenum)
		{
		get_shape(shapenum, 0);	// Force it into memory.
		return shapes[shapenum].num_frames;
		}
	Shape *new_shape(int shapenum);	// Create new shape (or del old).
	};
	
#endif	/* VGAFILE_H */
