/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Animate.h - Animated game objects.
 **
 **	Written: 7/27/2000 - JSF
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

#ifndef INCL_ANIMATE
#define INCL_ANIMATE	1

#include "objs.h"

/*
 *	An animator:
 */
class Animator : public Time_sensitive
	{
protected:
	Game_object *obj;		// Object we're controlling.
	unsigned char deltax, deltay;	// If wiggling, deltas from
					//   original position.
	unsigned char animating;	// 1 if animation turned on.
	void start_animation();
public:
	Animator(Game_object *o) : obj(o), deltax(0), deltay(0), animating(0)
		{  }
	static Animator *create(Game_object *ob);
	~Animator();
	void want_animation()		// Want animation on.
		{
		if (!animating)
			start_animation();
		}
	int get_deltax()
		{ return deltax; }
	int get_deltay()
		{ return deltay; }
	};

/*
 *	Animate by going through frames.
 */
class Frame_animator : public Animator
	{
	unsigned char frames;		// # of frames.
	unsigned char ireg;		// 1 if from an IREG file.
public:
	Frame_animator(Game_object *o, int ir);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
	};

/*
 *	Animate by wiggling.
 */
class Wiggle_animator : public Animator
	{
public:
	Wiggle_animator(Game_object *o) : Animator(o)
		{  }
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
	};

/*
 *	An object that cycles through its frames, or wiggles if just one
 *	frame.  The base class is for non-Ireg ones.
 */
class Animated_object : public Game_object
	{
	Animator *animator;		// Controls animation.
public:
	Animated_object(unsigned char l, unsigned char h, 
				unsigned int shapex, unsigned int shapey, 
				unsigned int lft = 0);
	Animated_object(int shapenum, int framenum, unsigned int tilex, 
	       unsigned int tiley, unsigned int lft = 0);
	virtual ~Animated_object();
					// Render.
	virtual void paint(Game_window *gwin);
					// +++++Needed on this one:
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord() const
		{ return get_abs_tile_coord() + 
			Tile_coord(-animator->get_deltax(), 
				   -animator->get_deltay(), 0); }
	};

/*
 *	An object that cycles through its frames, or wiggles if just one
 *	frame.  This is the IREG version.
 */
class Animated_ireg_object : public Ireg_game_object
	{
	Animator *animator;		// Controls animation.
public:
	Animated_ireg_object(unsigned char l, unsigned char h, 
				unsigned int shapex, unsigned int shapey, 
				unsigned int lft = 0);
	Animated_ireg_object(int shapenum, int framenum, unsigned int tilex, 
	       unsigned int tiley, unsigned int lft = 0);
	virtual ~Animated_ireg_object();
					// Render.
	virtual void paint(Game_window *gwin);
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord() const
		{ return get_abs_tile_coord() + 
			Tile_coord(-animator->get_deltax(), 
				   -animator->get_deltay(), 0); }
					// Write out to IREG file.
	virtual void write_ireg(ostream& out);
	};
#endif


