/**
 **	Handle access to one of the xxx.vga files.
 **
 **	Written: 4/29/99 - JSF
 **/

#ifndef INCL_VGAFILE
#define INCL_VGAFILE	1

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

#include <fstream.h>

class Shape;

/*
 *	A shape from "shapes.vga":
 */
class Shape_frame
	{
	unsigned char rle;		// 1 if run-length encoded.
#if 0
	unsigned char framenum;		// Frame #.
	unsigned short shapenum;	// Shape #.
#endif
	unsigned char *data;		// The actual data.
	short xleft;			// Extent to left of origin.
	short xright;			// Extent to right.
	short yabove;			// Extent above origin.
	short ybelow;			// Extent below origin.
					// Create from RLE entry.
	void get_rle_shape(ifstream& shapes, long filepos);
public:
	friend class Game_window;
	Shape_frame() /* : shapenum((unsigned short) -1) */
		{  }
					// Read in shape/frame.
	unsigned char read(ifstream& shapes, int shnum, int frnum);
	int get_width()			// Get dimensions.
		{ return xleft + xright; }
	int get_height()
		{ return yabove + ybelow; }
	~Shape_frame()
		{ delete data; }
	};

/*
 *	A shape from a .vga file consists of one of more frames.
 */
class Shape
	{
	Shape_frame **frames;		// List of ->'s to frames.
	unsigned char num_frames;	// # of frames.
	unsigned char dim;		// Each nibble is # of 8x8 shape boxes.
	unsigned char obstacle;		// 1 if an obstacle.
					// Read in shape/frame.
	Shape_frame *read(ifstream& shapes, int shnum, int frnum);
public:
	friend class Vga_file;
	Shape() : frames(0), dim(0), obstacle(0)
		{  }
					// Get dim. (in 8x8 shape boxes).
	void get_dim(int& w, int& h)
		{
		w = dim >> 4;
		h = dim & 0xf;
		}
	Shape_frame *get(ifstream& shapes, int shnum, int frnum)
		{ return (frames && frames[frnum]) ? frames[frnum] 
					: read(shapes, shnum, frnum); }
	};

/*
 *	A class for accessing any .vga file:
 */
class Vga_file
	{
	ifstream file;			// For reading.
	int num_shapes;			// Total # of shapes.
	Shape *shapes;			// List of ->'s to shapes' lists
public:
	Vga_file(char *nm);
					// Read in dimensions file.
	int read_dims(char *nm, int first, int num);
					// Get dim. (in 8x8 shape boxes).
	void get_dim(int shapenum, int& w, int& h)
		{ shapes[shapenum].get_dim(w, h); }
	int is_obstacle(int shapenum)	// Can't walk through it?
		{ return shapes[shapenum].obstacle != 0; }
	int get_num_shapes()
		{ return num_shapes; }
	int is_good()
		{ return (num_shapes != 0); }
					// Get shape.
	Shape_frame *get_shape(int shapenum, int framenum = 0)
		{
		return (shapes[shapenum].get(file, shapenum, framenum));
		}
	unsigned char *dims;	//+++++++++++++Debugging.
	};

#endif
