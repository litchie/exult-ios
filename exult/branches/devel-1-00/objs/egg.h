/*
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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

#ifndef INCL_EGG
#define INCL_EGG	1

class	Egg_object;
class	Animated_egg_object;
class	Animator;
class   Monster_actor;
class 	Missile_launcher;

#include "iregobjs.h"

/*
 *	Here's a class for eggs and paths; i.e., objects that generally aren't
 *	visible.
 */
class Egglike_game_object : public Ireg_game_object
	{
public:
	Egglike_game_object(int shapenum, int framenum,
				unsigned int tilex,
				unsigned int tiley, unsigned int lft = 0)
		: Ireg_game_object(shapenum, framenum, tilex, tiley, lft)
		{  }
					// Render.
	virtual void paint(Game_window *gwin);
					// Can this be clicked on?
	virtual int is_findable(Game_window *gwin);
	};

/*
 *	An "egg" is a special object that activates under certain
 *	circumstances.
 */
class Egg_object : public Egglike_game_object
	{
	static Egg_object *editing;	// Egg being edited by ExultStudio.
protected:
	unsigned char type;		// One of the below types.
	unsigned char probability;	// 1-100, chance of egg activating.
	unsigned char criteria:3;	// How it's activated.  See below.
	unsigned distance:6;		// Distance for activation (0-31).
	unsigned flags:4;		// Formed from below flags.
	unsigned short data1, data2;	// More data, depending on type.
	Rectangle area;			// Active area.
	unsigned char solid_area;	// 1 if area is solid, 0 if outline.
	Missile_launcher *launcher;	// For missile eggs.
	void init_field(unsigned char ty);
	void activate_teleport(Game_object *obj);	// Handle teleport egg.
public:
	enum Egg_types {		// Types of eggs:
		monster = 1,
		jukebox = 2,
		soundsfx = 3,
		voice = 4,
		usecode = 5,
		missile = 6,
		teleport = 7,
		weather = 8,
		path = 9,
		button = 10,
					// Our own:
		fire_field = 128,
		sleep_field = 129,
		poison_field = 130,
		caltrops_field = 131,
		mirror_object = 132
		};
	enum Egg_flag_shifts {
		nocturnal = 0,
		once = 1,
		hatched = 2,
		auto_reset = 3
		};
	enum Egg_criteria {
		cached_in = 0,		// Activated when chunk read in?
		party_near = 1,
		avatar_near = 2,	// Avatar steps into area.
		avatar_far = 3,		// Avatar steps outside area.
		avatar_footpad = 4,	// Avatar must step on it.
		party_footpad = 5,
		something_on = 6,	// Something placed on/near it.
		external_criteria = 7	// Appears on Isle of Avatar.  Guessing
					//   these set off all nearby.
		};
					// Create normal eggs.
	Egg_object(int shapenum, int framenum, unsigned int tilex,
		unsigned int tiley, unsigned int lft, 
		unsigned short itype,
		unsigned char prob, short d1, short d2);
					// Ctor. for fields:
	Egg_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft,
				unsigned char ty);
	virtual ~Egg_object();
	virtual void set_area();		// Set up active area.
	int get_distance() const
		{ return distance; }
	int get_criteria() const
		{ return criteria; }
	int get_type() const
		{ return type; }
					// Can it be activated?
	virtual int is_active(Game_object *obj,
			int tx, int ty, int tz, int from_tx, int from_ty);

	Rectangle get_area() const	// Get active area.
		{ return area; }
	int is_solid_area() const
		{ return solid_area; }
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
					// Saved from ExultStudio.
	static void update_from_studio(unsigned char *data, int datalen);
	virtual void activate(Usecode_machine *umachine, Game_object *obj,
							bool must = false);
	void print_debug();
	static void set_weather(Game_window *gwin, int weather, int len = 15,
						Game_object *egg = 0);
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual int is_egg() const	// An egg?
		{ return 1; }
					// Write out to IREG file.
	virtual void write_ireg(std::ostream& out);

	virtual void reset() 
		{ flags &= ~(1 << hatched); }

	};

/*
 *	An object that cycles through its frames, or wiggles if just one
 *	frame.
 */
class Animated_egg_object : public Egg_object
	{
protected:
	Animator *animator;		// Controls animation.
public:
	Animated_egg_object(int shapenum, int framenum,
		unsigned int tilex,
		unsigned int tiley, unsigned int lft, 
		unsigned short itype,
		unsigned char prob, short d1, short d2);
	Animated_egg_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft,
				unsigned char ty);
	virtual ~Animated_egg_object();
					// Render.
	virtual void paint(Game_window *gwin);
					// Can this be clicked on?
	virtual int is_findable(Game_window *gwin)
		{ return Ireg_game_object::is_findable(gwin); }
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
	};

/*
 *	Fields are activated like eggs.
 */

class Field_object : public Animated_egg_object
	{
	bool field_effect(Actor *actor);// Apply field.
public:
	Field_object(int shapenum, int framenum, unsigned int tilex, 
		unsigned int tiley, unsigned int lft, unsigned char ty)
		: Animated_egg_object(shapenum, framenum, tilex, tiley,
							lft, ty)
		{  }
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
	virtual void activate(Usecode_machine *umachine, Game_object *obj,
							bool must = false);
					// Write out to IREG file.
	virtual void write_ireg(std::ostream& out);
	};

/*
 *	Mirrors are handled like eggs.
 */

class Mirror_object : public Egg_object
{
public:
	Mirror_object(int shapenum, int framenum, unsigned int tilex, 
		unsigned int tiley, unsigned int lft);

					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);

	virtual void activate(Usecode_machine *umachine, Game_object *obj,
							bool must = false);

					// Can it be activated?
	virtual int is_active(Game_object *obj,
			int tx, int ty, int tz, int from_tx, int from_ty);

	virtual void set_area();		// Set up active area.

					// Render.
	virtual void paint(Game_window *gwin);
					// Can this be clicked on?
	virtual int is_findable(Game_window *gwin)
		{ return Ireg_game_object::is_findable(gwin); }

	virtual void write_ireg(std::ostream& out);
};
#endif
