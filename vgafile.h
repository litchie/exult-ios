/**	-*-mode: Fundamental; tab-width: 8; -*-
**
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

#include <fstream>
#include <iostream>
#include "fnames.h"
#include "autoarray.h"

class Shape;
class Image_buffer8;

/*
 *	A shape from "shapes.vga":
 */
class Shape_frame
	{
	unsigned char rle;		// 1 if run-length encoded.
	unsigned char *data;		// The actual data.
	short xleft;			// Extent to left of origin.
	short xright;			// Extent to right.
	short yabove;			// Extent above origin.
	short ybelow;			// Extent below origin.
	Shape_frame *reflect();		// Create new frame, reflected.
					// Create RLE data & store in frame.
	void create_rle(unsigned char *pixels, int w, int h);
					// Create from RLE entry.
	void get_rle_shape(ifstream& shapes, long filepos, long len);
public:
	friend class Game_window;
	friend class Shape;
	Shape_frame() : data(0)
		{  }
					// Read in shape/frame.
	unsigned char read(ifstream& shapes, unsigned long shapeoff,
					unsigned long shapelen, int frnum);
					// Paint.
	void paint_rle(Image_buffer8 *win, int xoff, int yoff);
	void paint_rle(Image_window8 *win, int xoff, int yoff)
		{ paint_rle(win->get_ib8(), xoff, yoff); }
	void paint_rle_translucent(Image_buffer8 *win, int xoff, int yoff,
					Xform_palette *xforms, int xfcnt);
	int has_point(int x, int y);	// Is a point within the shape?
	int get_width() const		// Get dimensions.
		{ return xleft + xright; }
	int get_height() const
		{ return yabove + ybelow; }
	int get_xleft()
		{ return xleft; }
	int get_xright()
		{ return xright; }
	int get_yabove()
		{ return yabove; }
	int get_ybelow()
		{ return ybelow; }
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
	unsigned char num_frames;	// # of frames.
					// Create reflected frame.
	Shape_frame *reflect(ifstream& shapes, int shnum, int frnum);
					// Read in shape/frame.
	Shape_frame *read(ifstream& shapes, int shnum, int frnum);
					// Store shape that was read.
	Shape_frame *store_frame(Shape_frame *frame, int framenum);
public:
	friend class Vga_file;
	Shape() : frames(0), num_frames(0)
		{  }
	virtual ~Shape();
	Shape_frame *get(ifstream& shapes, int shnum, int frnum)
		{ 
		return (frames && frnum < num_frames && frames[frnum]) ? 
			frames[frnum] : read(shapes, shnum, frnum); 
		}
	};

/*
 *	A shape file just has one shape with multiple frames.  They're all
 *	read in during construction.
 */
class Shape_file : private Shape
	{
public:
	Shape_file(const char *nm);
	int get_num_frames()
		{ return num_frames; }
	Shape_frame *get_frame(int framenum)
		{ return framenum < num_frames ? frames[framenum] : 0; }
	virtual ~Shape_file()
		{  }
	};

/*
 *	A class for accessing any .vga file:
 */
class Vga_file
	{
	ifstream file;			// For reading.
protected:
	int num_shapes;			// Total # of shapes.
	Shape *shapes;			// List of ->'s to shapes' lists
public:
	Vga_file(const char *nm);
	virtual ~Vga_file();
	int get_num_shapes()
		{ return num_shapes; }
	int is_good()
		{ return (num_shapes != 0); }
					// Get shape.
	Shape_frame *get_shape(int shapenum, int framenum = 0)
		{
		Shape_frame *r=(shapes[shapenum].get(file, shapenum, framenum));
		if(!r)
			{
#if DEBUG
				cerr << "get_shape(" <<
					shapenum << "," <<
					framenum << ") -> NULL" << endl;
#endif
			}
		return r;
		}
					// Get # frames for a shape.
	int get_num_frames(int shapenum)
		{
		get_shape(shapenum, 0);	// Force it into memory.
		return shapes[shapenum].num_frames;
		}
	};

/*
 *	Specific information about weapons from 'weapons.dat':
 */
class Weapon_info
	{
	short damage;			// Damage points (positive).
	short ammo;			// Shape # of ammo., or 0.
public:
	Weapon_info(short d, short am) : damage(d), ammo(am)
		{  }
	int get_damage()
		{ return damage; }
	int get_ammo()
		{ return ammo; }
	};

/*
 *	This class contains information only about shapes from "shapes.vga".
 */
class Shape_info
	{
	unsigned char tfa[3];		// From "tfa.dat".+++++Keep for
					//   debugging, for now.
					// 3D dimensions in tiles:
	unsigned char xtiles, ytiles, ztiles;
	unsigned char weight, volume;	// From "wgtvol.dat".
	unsigned char shpdims[2];	// From "shpdims.dat".
	unsigned char ready_type;	// From "ready.dat":  where item can
					//   be worn.
	unsigned char *weapon_offsets;	// From "wihh.dat": pixel offsets
					//   for drawing weapon in hand
	unsigned char armor;		// Armor, from armor.dat.
	Weapon_info *weapon;		// From weapon.dat, if a weapon.
	void set_tfa_data()		// Set fields from tfa.
		{
		xtiles = 1 + (tfa[2]&7);
		ytiles = 1 + ((tfa[2]>>3)&7);
		ztiles = (tfa[0] >> 5);
		}
	// This private copy constructor and assignment operator are never
	// defined so copying will cause a link error (intentional)
	Shape_info(const Shape_info & other);
	const Shape_info & operator = (const Shape_info & other);
public:
	friend class Shapes_vga_file;	// Class that reads in data.
	Shape_info() : xtiles(0), ytiles(0), ztiles(0), weight(0), volume(0),
		ready_type(0), weapon_offsets(0), armor(0), weapon(0)
		{ tfa[0] = tfa[1] = tfa[2] = shpdims[0] = shpdims[1] = 0; }
	virtual ~Shape_info();
	int get_weight()		// Get weight, volume.
		{ return weight; }
	int get_volume()
		{ return volume; }
	int get_armor()			// Get armor protection.
		{ return armor; }
	Weapon_info *get_weapon_info()
		{ return weapon; }
	int get_weapon_damage()
		{ return weapon ? weapon->get_damage() : 0; }
	int get_3d_height()		// Height (in lifts?).
		{ return ztiles; }
	int get_3d_xtiles()		// Dimension in tiles - X.
		{ return xtiles; }
	int get_3d_ytiles()		// Dimension in tiles - Y.
		{ return ytiles; }
	unsigned char get_tfa(int i)	// For debugging:
		{ return tfa[i]; }
	int is_animated()
		{ return (tfa[0] & (1<<2)) != 0; }
	int is_solid()			// Guessing.  Means can't walk through.
		{ return (tfa[0] & (1<<3)) != 0; }
	int is_water()			// Guessing.
		{ return (tfa[0] & (1<<4)) != 0; }
	int is_poisonous()		// Swamps.
		{ return (tfa[1] & (1<<4)) != 0; }
	int is_door()
		{ return (tfa[1] & (1<<5)) != 0; }
	int is_barge_part()
		{ return (tfa[1] & (1<<6)) != 0; }
	int is_transparent()		// ??
		{ return (tfa[1] & (1<<7)) != 0; }
	int is_light_source()
		{ return (tfa[2] & (1<<6)) != 0; }
	int has_translucency()
		{ return (tfa[2] & (1<<7)) != 0; }
	int is_xobstacle()		// Obstacle in x-dir.???
		{ return (shpdims[1] & 1) != 0; }
	int is_yobstacle()		// Obstacle in y-dir.???
		{ return (shpdims[0] & 1) != 0; }
	/*
	 *	TFA[1][b0-b3] seems to indicate object types:
	 *	+++++++Just guessing for now.
	 */
	enum Shape_class {
		unusable = 0,		// Trees.
		has_quality = 2,
		has_quantity = 3,	// Can have more than 1:  coins, arrs.
//		shutters = 4,		// Also mirrors.
//		wearable = 5,		// Includes wieldable weapons, food,
					//   table, curtain??
		container = 6,		// Includes NPC's.
		hatchable = 7,		// Eggs, traps, moongates.
		spellbook = 8,
		barge = 9,
		virtue_stone = 11,
					// 12, 13 for some NPC's.
		building = 14		// Roof, window, mountain.
		};
	Shape_class get_shape_class()
		{ return (Shape_class) (tfa[1]&15); }
	unsigned char get_ready_type()
		{ return ready_type; }
	// Sets x to 255 if there is no weapon offset
	void get_weapon_offset(int frame, unsigned char& x, unsigned char& y)
		{
		if(!weapon_offsets)
			x = 255;
		else
			{
			// x could be 255 (see read_info())
			x = weapon_offsets[frame * 2];
			y = weapon_offsets[frame * 2 + 1];
			}
		}
	};

/*
 *	The "shapes.vga" file:
 */
class Shapes_vga_file : public Vga_file
	{
	autoarray<Shape_info> info;		// Extra info. about each shape.
	Shape_info zinfo;		// A fake one (all 0's).
public:
	Shapes_vga_file() : Vga_file(SHAPES_VGA),info(num_shapes)
		{  }
	virtual ~Shapes_vga_file();
	int read_info();		// Read additional data files.
	Shape_info& get_info(int shapenum)
		{ return info[shapenum]; }
	};

#endif
