/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gumps.h - Open containers and their contents.
 **
 **	Written: 3/4/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#ifndef INCL_GUMPS
#define INCL_GUMPS

#include "objs.h"

class Actor;
class Gump_object;

/*
 *	Some gump shape numbers:
 */
const int CHECKMARK = 2;		// Shape # in gumps.vga for checkmark.
const int HORIZBAR = 4;
const int HALO = 7;
const int SLIDER = 14;
const int SLIDERDIAMOND = 15;
const int SLIDERRIGHT = 16;
const int SLIDERLEFT = 17;
const int DISK = 24;			// Diskette shape #.
const int HEART = 25;			// Stats button shape #.
const int DOVE = 46;
const int STATSDISPLAY = 47;

/*
 *	A pushable button on a gump:
 */
class Gump_button
	{
protected:
	Gump_object *parent;		// Who this is in.
	int shapenum;			// In "gumps.vga".
	short x, y;			// Coords. relative to parent.
	unsigned char pushed;		// 1 if in pushed state.
public:
	friend class Gump_object;
	Gump_button(Gump_object *par, int shnum, int px, int py)
		: parent(par), shapenum(shnum), x(px), y(py), pushed(0)
		{  }
					// Is a given point on the checkmark?
	int on_button(Game_window *gwin, int mx, int my);
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin) = 0;
	void push(Game_window *gwin);	// Redisplay as pushed.
	void unpush(Game_window *gwin);
	};

/*
 *	A checkmark for closing its parent:
 */
class Checkmark_gump_button : public Gump_button
	{
public:
	Checkmark_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, CHECKMARK, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	A 'heart' button for bringing up stats.
 */
class Heart_gump_button : public Gump_button
	{
public:
	Heart_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, HEART, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	A diskette for bringing up the 'save' box.
 */
class Disk_gump_button : public Gump_button
	{
public:
	Disk_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, DISK, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	One of the two arrow button on the slider:
 */
class Slider_gump_button : public Gump_button
	{
public:
	Slider_gump_button(Gump_object *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	A gump contains an image of an open container from "gumps.vga".
 */
class Gump_object : public ShapeID
	{
protected:
	Gump_object *next;		// ->next to draw.
	Container_game_object *container;// What this gump shows.
	int x, y;			// Location on screen.
	unsigned char shapenum;
	Rectangle object_area;		// Area to paint objects in, rel. to
					// Where the 'checkmark' goes.
	Checkmark_gump_button *check_button;
public:
	Gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum);
	~Gump_object()
		{ delete check_button; }
	int get_x()			// Get coords.
		{ return x; }
	int get_y()
		{ return y; }
	void set_pos(int newx, int newy)// Set new spot on screen.
		{
		x = newx;
		y = newy;
		}
					// Append to end of chain.
	void append_to_chain(Gump_object *& chain);
					// Remove from chain.
	void remove_from_chain(Gump_object *& chain);
	Gump_object *get_next()		// (Chain ends with ->next == 0.)
		{ return next; }
	Container_game_object *get_container()
		{ return container; }
					// Get screen rect. of obj. in here.
	Rectangle get_shape_rect(Game_object *obj);
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& ox, int& oy)
		{
		ox = x + object_area.x + obj->cx,
		oy = y + object_area.y + obj->cy;
		}
					// Find obj. containing mouse point.
	Game_object *find_object(Game_window *gwin, int mx, int my);
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Paint button.
	void paint_button(Game_window *gwin, Gump_button *btn);
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1);
	virtual void remove(Game_object *obj)
		{ container->remove(obj); }
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A spot on an Actor gump:
 */
class Actor_gump_spot
	{
	friend class Actor_gump_object;
	short x, y;			// Location rel. to gump.
	Game_object *obj;		// Object that is here.
	int usecode_id;			// ID # used in intrinsic 0x72.
	Actor_gump_spot() : x(0), y(0), obj(0)
		{  }
	};

/*
 *	A rectangular area showing a character and his/her possessions:
 */
class Actor_gump_object : public Gump_object
	{
	Heart_gump_button *heart_button;// For bringing up stats.
	Disk_gump_button *disk_button;	// For bringing up 'save' box.
//+++++++Move this info to Actor!!
	Actor_gump_spot spots[8];	// Where things can go.
	enum Spots {			// Index of each spot.
		head = 0,
		back = 1,
		lhand = 2,
		rhand = 3,
		legs = 4,
		feet = 5,
		lfinger = 6,
		rfinger = 7
		};
					// Find index of closest spot.
	int find_closest(int mx, int my, int only_empty = 0);
	void add_to_spot(Game_object *obj, int index);
	static short diskx, disky;	// Where to show 'diskette' button.
	static short heartx, hearty;	// Where to show 'stats' button.
public:
	Actor_gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum);
	~Actor_gump_object()
		{  
		delete heart_button;
		delete disk_button;
		}
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1);
	virtual void remove(Game_object *obj);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A rectangular area showing a character's statistics:
 */
class Stats_gump_object : public Gump_object
	{
	Actor *get_actor()
		{ return (Actor *) container; }
	static short textx;		// X-coord. of where to write.
	static short texty[10];		// Y-coords.
public:
	Stats_gump_object(Container_game_object *cont, int initx, int inity)
		: Gump_object(cont, initx, inity, STATSDISPLAY)
		{  }
	~Stats_gump_object()
		{  }
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1)
		{ return 0; }		// Can't drop onto it.
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A slider for choosing a number.
 */
class Slider_gump_object : public Gump_object
	{
					// The arrows at each end:
	Slider_gump_button *left_arrow, *right_arrow;
	int diamondx;			// Rel. pos. where diamond is shown.
	int min_val, max_val;		// Max., min. values to choose from.
	int step_val;			// Amount to step by.
	static int val;			// Current value.
	void set_val(int newval);	// Set to new value.
					// Coords:
	static short leftbtnx, rightbtnx, btny;
public:
	Slider_gump_object(int initx, int inity, int mival, int mxval,
					int step, int defval);
	~Slider_gump_object()
		{
		delete left_arrow;
		delete right_arrow;
		}
	static int get_val()		// Get last value set.
		{ return val; }
					// An arrow was clicked on.
	void clicked_arrow(Slider_gump_button *arrow);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

#endif	/* INCL_GUMPS */
