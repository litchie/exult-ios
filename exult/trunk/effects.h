/*
 *	effects.h - Special effects.
 *
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

#ifndef EFFECTS_H
#define EFFECTS_H	1

#include <string>

#include "tqueue.h"
#include "tiles.h"
#include "singles.h"

class PathFinder;
class Game_object;
class Game_window;
class Image_window8;
class Shape_frame;
class Actor;
class Special_effect;

/*
 *	Manage special effects.
 */
class Effects_manager
	{
	Game_window *gwin;		// Handy pointer.
	Special_effect *effects;	// Text snippets, sprite effects.
public:
	Effects_manager(Game_window *g) : gwin(g), effects(0)
		{  }
	~Effects_manager();
					// Add text item.
	void add_text(const char *msg, Game_object *item);
	void add_text(const char *msg, int x, int y);
	void center_text(const char *msg);
	void add_effect(Special_effect *effect);
	void remove_text_effect(Game_object *item);
					// Remove text item & delete it.
	void remove_effect(Special_effect *txt);
	void remove_all_effects(bool repaint=false);
	void remove_text_effects();
					// Remove just the weather.
	void remove_weather_effects(int dist = 0);
	int get_weather();		// Get # of last weather added.
	void paint();			// Draw all effects.
	};

/*
 *	Base class for special-effects:
 */
class Special_effect : public Time_sensitive, public Game_singletons
	{
	Special_effect *next, *prev;	// All of them are chained together in
					//   Game_window.
public:
	friend class Effects_manager;
	Special_effect() : next(0), prev(0)
		{  }
	virtual ~Special_effect()
		{  }
					// Render.
	virtual void paint();
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
	ShapeID sprite;
	//int sprite_num;		// Which one.
	//int frame_num;		// Current frame.
	int frames;			// # frames.
	Game_object *item;		// Follows this around if not null.
	Tile_coord pos;			// Position within world.
	int xoff, yoff;			// Offset from position in pixels.
	int deltax, deltay;		// Add to xoff, yoff on each frame.
	void add_dirty(int frnum);
public:
	Sprites_effect(int num, Tile_coord p, int dx = 0, int dy = 0);
	Sprites_effect(int num, Game_object *it, 
					int xf, int yf, int dx, int dy);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
					// Render.
	virtual void paint();
	};

/*
 *	An explosion.
 */
class Explosion_effect : public Sprites_effect
	{
	Game_object *explode;		// What's exploding, or 0.
public:
	Explosion_effect(Tile_coord p, Game_object *exp);
	virtual void handle_event(unsigned long time, long udata);
	};

/*
 *	A moving animation, followed by an 'attack' at the end, to
 *	implement Usecode intrinsic 0x41:
 */
class Projectile_effect : public Special_effect
	{
	Actor *attacker;		// Source of attack/spell.
	Game_object *target;		// Target of path.
	int projectile_shape;		// Shape # of projectile/spell.
	ShapeID sprite;			// Sprite shape to display.
	int weapon;			// Shape # of firing weapon, or 0.
	int frames;			// # frames.
	PathFinder *path;		// Determines path.
	Tile_coord pos;			// Current position.
	bool return_path;		// Returning a boomerang.
					// Add dirty rectangle.
	void add_dirty();
	void init(Tile_coord s, Tile_coord t);
public:
	Projectile_effect(Actor *att, Game_object *to, int shnum,
								int weap = 0);
					// For missile traps:
	Projectile_effect(Tile_coord s, Tile_coord d, int shnum, int weap);
	Projectile_effect(Tile_coord s, Game_object *to, int shnum, int weap,
							bool retpath = false);
	~Projectile_effect();
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
					// Render.
	virtual void paint();
	};

/*
 *	'Death Vortex', from the spell.  (Maybe this could be a
 *	Sprites_effect.)
 */
class Death_vortex : public Special_effect
	{
	ShapeID vortex;
	Actor *target;			// We'll follow this around if not 0.
	Tile_coord pos;			// Current position.
	int frames;			// # frames.
	uint32 stop_time;		// Time in 1/1000 secs. to stop.
	uint32 next_damage_time;	// When to check for NPC's beneath us.
	int add_dirty();
public:
	Death_vortex(Game_object *trg, Tile_coord tp);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
					// Render.
	virtual void paint();
	};

/*
 *	A text object is a message that stays on the screen for just a couple
 *	of seconds.  These are all kept in a single list, and managed by
 *	Game_window.
 */
class Text_effect : public Special_effect
	{
	std::string msg;		// What to print.
	Game_object *item;		// Item text is on.  May be null.
	Tile_coord pos;			// Position to display it at.
	short width, height;		// Dimensions of rectangle.
	int num_ticks;			// # ticks passed.
	void add_dirty();
	void init();
public:
	friend class Game_window;
	Text_effect(const std::string &m, Game_object *it);
	Text_effect(const std::string &m, int t_x, int t_y);
					// At timeout, remove from screen.
	virtual void handle_event(unsigned long curtime, long udata);
					// Render.
	virtual void paint();
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
	Tile_coord eggloc;		// Location of egg that started this.
public:
	Weather_effect(int duration, int delay, int n, Game_object *egg = 0);
	virtual ~Weather_effect()
		{  }
					// Avatar out of range?
	int out_of_range(Tile_coord& avpos, int dist);
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
		int ndrops = MAXDROPS, int n = -1, Game_object *egg = 0)
		: Weather_effect(duration, delay, n, egg),
		  num_drops(ndrops)
		{  }
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
					// Render.
	virtual void paint();
	};

/*
 *	Lightning.
 */
class Lightning_effect : public Weather_effect
	{
	static int active;		// Just want one at a time.
	bool flashing;			// Lightning palette is set.
	friend class Storm_effect;
public:
	Lightning_effect(int duration, int delay = 0) 
		: Weather_effect(duration, delay, -1), flashing(false)
		{ }
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
	Storm_effect(int duration, int delay = 0, Game_object *egg = 0);
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
	Sparkle_effect(int duration, int delay = 0, Game_object *egg = 0) 
					// Weather #3.
		: Rain_effect(duration, delay, 50, 3, egg)
		{  }
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
	};

/*
 *	A single cloud (sprite shape 2):
 */
class Cloud
	{
	ShapeID cloud;
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
	void paint();
	};

/*
 *	Clouds.
 */
class Clouds_effect : public Weather_effect
	{
	int num_clouds;
	Cloud **clouds;			// ->clouds.
public:
	Clouds_effect(int duration, int delay = 0, Game_object *egg = 0);
					// Execute when due.
	virtual void handle_event(unsigned long curtime, long udata);
					// Render.
	virtual void paint();
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

