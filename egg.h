/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	egg.h - Game objects.
 **
 **	Written: 10/1/98 - JSF
 **/

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

#ifndef INCL_EGG
#define INCL_EGG	1

class	Egg_object;
class	Animated_egg_object;
class	Animator;

#include "objs.h"

/*
 *	An "egg" is a special object that activates under certain
 *	circumstances.
 */
class Egg_object : public Game_object
	{
protected:
	unsigned char type;		// One of the below types.
	unsigned char probability;	// 1-100, chance of egg activating.
	unsigned char criteria:3;	// How it's activated.  See below.
	unsigned distance:6;		// Distance for activation (0-31).
	unsigned flags:4;		// Formed from below flags.
	unsigned short data1, data2;	// More data, depending on type.
	Rectangle area;			// Active area.
	unsigned char solid_area;	// 1 if area is solid, 0 if outline.
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
		button = 10
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
		something_on = 6	// Something placed on/near it??
		};
					// Create from ireg. data.
	Egg_object(unsigned char l, unsigned char h, unsigned int shapex,
		unsigned int shapey, unsigned int lft, 
		unsigned short itype,
		unsigned char prob, short d1, short d2);
	virtual ~Egg_object()
		{  }
	void set_area();		// Set up active area.
	int get_distance() const
		{ return distance; }
	int get_criteria() const
		{ return criteria; }
	void monster_died();		// Monster this created just died.
					// Can it be activated?
	int is_active(int tx, int ty, int from_tx, int from_ty);
	Rectangle get_area() const	// Get active area.
		{ return area; }
	int is_solid_area() const
		{ return solid_area; }
#if 0	/* ++++Going away. Is_active() does the test. */
	int within_distance(int abs_tx, int abs_ty) const;
#endif
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
	void activate(Usecode_machine *umachine, Game_object *obj);
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual int is_egg() const	// An egg?
		{ return 1; }
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
	};

/*
 *	An object that cycles through its frames, or wiggles if just one
 *	frame.  The base class is for non-Ireg ones.
 */
class Animated_egg_object : public Egg_object
	{
	Animator *animator;		// Controls animation.
public:
					// Create from ireg. data.
	Animated_egg_object(unsigned char l, unsigned char h, 
		unsigned int shapex,
		unsigned int shapey, unsigned int lft, 
		unsigned short itype,
		unsigned char prob, short d1, short d2);
	virtual ~Animated_egg_object();
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
	};

#endif
