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
const int BOOK = 32;
const int SPELLBOOK = 43;
const int DOVE = 46;
const int STATSDISPLAY = 47;
const int SCROLL = 55;
const int YESNOBOX = 69;
const int YESBTN = 70, NOBTN = 71;

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
	void initialize();		// Initialize object_area.
public:
	Gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum);
					// Create centered.
	Gump_object(Container_game_object *cont, int shnum);
	virtual ~Gump_object()
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
	Game_object *find_object(int mx, int my);
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
					// Close (and delete).
	virtual void close(Game_window *gwin);
	};

/*
 *	A spot on an Actor gump:
 */
class Actor_gump_spot
	{
	friend class Actor_gump_object;
	Game_object *obj;		// Object that is here.
	Actor_gump_spot() : obj(0)
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
	Actor_gump_spot spots[12];	// Where things can go.
	static short coords[24];	// Coords. of where to draw things.
	static int spotx(int i) { return coords[2*i]; }
	static int spoty(int i) { return coords[2*i + 1]; }
	enum Spots {			// Index of each spot, starting at
					//   upper, rt., going clkwise.
		head = 0,
		chest = 1,
		belt = 2,
		lhand = 3,
		lfinger = 4,
		legs = 5,
		feet = 6,
		rfinger = 7,
		rhand = 8,
		arms = 9,
		neck = 10,
		back = 11
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
 *	A sign showing runes.
 */
class Sign_gump : public Gump_object
	{
	char **lines;			// Lines of text.
	int num_lines;
public:
	Sign_gump(int shapenum, int nlines);
	~Sign_gump();
					// Set a line of text.
	void add_text(int line, const char *txt);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A text gump is the base class for books and scrolls.
 */
class Text_gump : public Gump_object
	{
	char *text;			// The text.
	int textlen;			// Length of text.
protected:
	int curtop;			// Offset of top of current page.
	int curend;			// Offset past end of current page(s).
public:
	Text_gump(int shapenum) : Gump_object(0, shapenum),
				text(0), textlen(0), curtop(0), curend(0)
		{  }
	~Text_gump()
		{ delete text; }
	void add_text(char *str);	// Append text.
	int paint_page(Game_window *gwin, Rectangle box, int start);
					// Next page of book/scroll.
	int show_next_page(Game_window *gwin);
	};

/*
 *	A book shows text side-by-side.
 */
class Book_gump : public Text_gump
	{
public:
	Book_gump() : Text_gump(BOOK)
		{  }
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A scroll:
 */
class Scroll_gump : public Text_gump
	{
public:
	Scroll_gump() : Text_gump(SCROLL)
		{  }
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A modal gump object represents a 'dialog' that grabs the mouse until
 *	the user clicks okay.
 */
class Modal_gump_object : public Gump_object
	{
protected:
	int done;			// 1 when user clicks checkmark.
	Gump_button *pushed;		// Button currently being pushed.
public:
	Modal_gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum)
		: Gump_object(cont, initx, inity, shnum), done(0), pushed(0)
		{  }
					// Create centered.
	Modal_gump_object(Container_game_object *cont, int shnum)
		: Gump_object(cont, shnum), done(0), pushed(0)
		{  }
	int is_done()
		{ return done; }
					// Handle events:
	virtual void mouse_down(int mx, int my) = 0;
	virtual void mouse_up(int mx, int my) = 0;
	virtual void mouse_drag(int mx, int my) = 0;
	};

/*
 *	A slider for choosing a number.
 */
class Slider_gump_object : public Modal_gump_object
	{
					// The arrows at each end:
	Slider_gump_button *left_arrow, *right_arrow;
	int diamondx;			// Rel. pos. where diamond is shown.
	static short diamondy;
	int min_val, max_val;		// Max., min. values to choose from.
	int step_val;			// Amount to step by.
	int val;			// Current value.
	unsigned char dragging;		// 1 if dragging the diamond.
	int prev_dragx;			// Prev. x-coord. of mouse.
	void set_val(int newval);	// Set to new value.
					// Coords:
	static short leftbtnx, rightbtnx, btny;
	static short xmin, xmax;
public:
	Slider_gump_object(int mival, int mxval, int step, int defval);
	~Slider_gump_object()
		{
		delete left_arrow;
		delete right_arrow;
		}
	int get_val()			// Get last value set.
		{ return val; }
					// An arrow was clicked on.
	void clicked_arrow(Slider_gump_button *arrow);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	virtual void close(Game_window *gwin)
		{ done = 1; }
					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);
	virtual void mouse_drag(int mx, int my);
	};

#endif	/* INCL_GUMPS */
