/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Effects.h - Special effects.
 **
 **	Written: 5/25/2000 - JSF
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

#ifndef INCL_EFFECTS
#define INCL_EFFECTS	1

#include <string>

#include "tqueue.h"
#include "tiles.h"

class PathFinder;
class Game_object;
class Game_window;
class Image_window8;
class Shape_frame;
class Actor;

/*
 *	Base class for special-effects:
 */
class Special_effect : public Time_sensitive
	{
	Special_effect *next, *prev;	// All of them are chained together in
					//   Game_window.
public:
	friend class Game_window;
	Special_effect() : next(0), prev(0)
		{  }
	virtual ~Special_effect()
		{  }
					// Render.
	virtual void paint(Game_window *gwin);
	virtual int is_weather()	// Need to distinguish weather.
		{ return 0; }
	virtual int is_text(Game_object *it = 0)
		{ return 0; }
	};

/*
 *	An animation from 'sprites.vga':
 */
class Sprites_effect : public Special_effect
	{
protected:
	int sprite_num;			// Which one.
	int frame_num;			// Current frame.
	int frames;			// # frames.
	Tile_coord pos;			// Position within world.
	void add_dirty(Game_window *gwin, int frnum);
public:
	Sprites_effect(int num, Tile_coord p);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
					// Render.
	virtual void paint(Game_window *gwin);
	};

/*
 *	An explosion.
 */
class Explosion_effect : public Sprites_effect
	{
	Game_object *explode;		// What's exploding, or 0.
public:
	Explosion_effect(Tile_coord p, Game_object *exp);
	virtual ~Explosion_effect();
	};

/*
 *	A moving animation, followed by an 'attack' at the end, to
 *	implement Usecode intrinsic 0x41:
 */
class Projectile_effect : public Special_effect
	{
	Actor *attacker;		// Source of attack/spell.
	Game_object *target;		// Target of path.
	int shape_num;			// Shape # in 'shapes.vga' of projec.
					//   or spell to 'attack' with.
	int weapon;			// Shape # of firing weapon, or 0.
	int frame_num;			// Current frame.
	int frames;			// # frames.
	PathFinder *path;		// Determines path.
	Tile_coord pos;			// Current position.
					// Add dirty rectangle.
	void add_dirty(Game_window *gwin);
	void init(Tile_coord s, Tile_coord t);
public:
	Projectile_effect(Actor *att, Game_object *to, int shnum,
							int weap = 0);
					// For missile traps:
	Projectile_effect(Tile_coord s, Tile_coord d, int shnum, int weap);
	Projectile_effect(Tile_coord s, Game_object *to, int shnum, int weap);
	~Projectile_effect();
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
					// Render.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A text object is a message that stays on the screen for just a couple
 *	of seconds.  These are all kept in a single list, and managed by
 *	Game_window.
 */
class Text_effect : public Special_effect
	{
	std::string msg;			// What to print.
	Game_object *item;		// Item text is on.  May be null.
	short tx, ty;			// Tile coords. within world of upper-
					//   left corner.
	short width, height;		// Dimensions of rectangle.
public:
	friend class Game_window;
	Text_effect(const std::string &m, Game_object *it,
					int t_x, int t_y, int w, int h);
					// At timeout, remove from screen.
	virtual void handle_event(unsigned long curtime, long udata);
					// Render.
	virtual void paint(Game_window *gwin);
					// Check for matching item if !null.
	virtual int is_text(Game_object *it = 0)
		{ return !it || it == item; }
	};

/*
 *	Weather.
 */
class Weather_effect : public Special_effect
	{
protected:
	uint32 stop_time;		// Time in 1/1000 secs. to stop.
	int num;			// Weather ID (0-6), or -1.
public:
	Weather_effect(int duration, int delay, int n);
	virtual ~Weather_effect()
		{  }
	virtual int is_weather()
		{ return 1; }
	int get_num() { return num; }
	};

/*
 *	A raindrop:
 */
class Raindrop
	{
	unsigned char oldpix;		// Pixel originally on screen.
	unsigned char yperx;		// Move this many y-pixels for each x.
	long ax, ay;			// Coords. where drawn in abs. pixels.
public:
	Raindrop() : oldpix(0xff), yperx(1), ax(-1), ay(-1)
		{  }
	void paint(Image_window8 *iwin, int scrolltx, int scrollty,
						unsigned char *xform);
					// Move to next position.
	void next(Image_window8 *iwin, int scrolltx, int scrollty,
					unsigned char *xform, int w, int h);
	void next_random(Image_window8 *iwin, int scrolltx, int scrollty,
					unsigned char *xform, int w, int h);
	};	

/*
 *	Raining.
 */
class Rain_effect : public Weather_effect
	{
protected:
#define MAXDROPS 200
	Raindrop drops[MAXDROPS];	// Drops moving down the screen.
	int num_drops;			// # to actually use.
public:
	Rain_effect(int duration, int delay = 0, 
			int ndrops = MAXDROPS, int n = -1)
		: Weather_effect(duration, delay, n),
		  num_drops(ndrops)
		{  }
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
					// Render.
	virtual void paint(Game_window *gwin);
	};

/*
 *	Lightning.
 */
class Lightning_effect : public Weather_effect
	{
	static int active;		// Just want one at a time.
	int save_brightness;		// Palette brightness.
	friend class Storm_effect;
	Lightning_effect(int duration, int delay = 0) 
		: Weather_effect(duration, delay, -1), save_brightness(-1)
		{ }
public:
	~Lightning_effect();
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Storm.
 */
class Storm_effect : public Weather_effect
	{
	int start;			// 1 to start storm.
public:
	Storm_effect(int duration, int delay = 0);
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
	virtual ~Storm_effect();
	};

/*
 *	Ambrosia's 'twinkling rain'.
 */
class Sparkle_effect : public Rain_effect
	{
public:
	Sparkle_effect(int duration, int delay = 0) 
					// Weather #3.
		: Rain_effect(duration, delay, 50, 3)
		{  }
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
	};

/*
 *	A single cloud (sprite shape 2):
 */
class Cloud
	{
	int frame;			// Frame #.
	long wx, wy;			// Position within world.
	short deltax, deltay;		// How to move.
	int count;			// Counts down to 0.
	int max_count;
	uint32 start_time;	// When to start.
	static int randcnt;		// For generating random times.
	void set_start_pos(Shape_frame *shape, int w, int h, int& x, int& y);
public:
	Cloud(short dx, short dy);
					// Move to next position & paint.
	void next(Game_window *gwin, unsigned long curtime, int w, int h);
	void paint(Game_window *gwin);
	};

/*
 *	Clouds.
 */
class Clouds_effect : public Weather_effect
	{
	int num_clouds;
	Cloud **clouds;			// ->clouds.
public:
	Clouds_effect(int duration, int delay = 0);
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
					// Render.
	virtual void paint(Game_window *gwin);
	virtual ~Clouds_effect()
		{ delete [] clouds; }
	};

/*
 *	Earthquakes.  +++++Might make this a Weather_effect.
 */
class Earthquake : public Time_sensitive
	{
	int len;			// From Usecode intrinsic.
	int i;				// Current index.
public:
	Earthquake(int l) : len(l), i(0)
		{
		}
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
	};

#endif

