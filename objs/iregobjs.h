/*
 *	iregobjs.h - Ireg (moveable) game objects.
 *
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

#ifndef IREGOBJS_H
#define IREGOBJS_H	1

#include "exult_types.h"
#include "objs.h"

/*
 *	A moveable game object (from 'ireg' files):
 */
class Ireg_game_object : public Game_object
	{
	Container_game_object *owner;	// Container this is in, or 0.
protected:
	unsigned flags:32;		// 32 flags used in 'usecode'.
	unsigned flags2:32;		// Another 32 flags used in 'usecode'.
public:
	Ireg_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft),
				owner(0), flags(0), flags2(0), lowlift(-1), 
				highshape (-1)
		{  }
					// Copy constructor.
	Ireg_game_object(const Ireg_game_object& obj2)
		: Game_object(obj2), owner(0), flags(0), flags2(0),
					lowlift(-1), highshape (-1)
		{  }
					// Create fake entry.
	Ireg_game_object() : owner(0), flags(0), flags2(0), lowlift(-1), highshape (-1)
		{  }
	virtual ~Ireg_game_object()
		{  }
	void set_flags(uint32 f)	// For initialization.
		{ flags = f; }
					// Render.
	virtual void paint(Game_window *gwin);
	virtual void paint_terrain(Game_window *gwin) {  }
					// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift);
	void move(Tile_coord t)
		{ move(t.tx, t.ty, t.tz); }
					// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual Container_game_object *get_owner()
		{ return owner; }
	virtual void set_owner(Container_game_object *o)
		{ owner = o; }
	virtual int is_dragable() const;// Can this be dragged?
	virtual void set_flag(int flag)
		{
		if (flag >= 0 && flag < 32)
			flags |= ((uint32) 1 << flag);
		else if (flag >= 32 && flag < 64)
			flags2 |= ((uint32) 1 << (flag-32));
		}
	virtual void clear_flag(int flag)
		{
		if (flag >= 0 && flag < 32)
			flags &= ~((uint32) 1 << flag);
		else if (flag >= 32 && flag < 64)
			flags2 &= ~((uint32) 1 << (flag-32));
		}
	virtual int get_flag(int flag) const
		{
		if (flag >= 0 && flag < 32)
			return flags & ((uint32) 1 << flag);
		else if (flag >= 32 && flag < 64)
			return flags2 & ((uint32) 1 << (flag-32));
		return 0;
		}
	virtual void set_flag_recursively(int flag)
		{ set_flag(flag); }
	uint32 get_flags() const { return flags; }
	uint32 get_flags2() const { return flags2; }

					// Write out to IREG file.
	virtual void write_ireg(DataSource* out);
	int	lowlift;
	int highshape;
	virtual int get_high_shape() const { return highshape; };
	virtual void set_high_shape(int s) { highshape = s;};
	virtual int get_low_lift() const { return lowlift; };
	virtual void set_low_lift(int l) { lowlift = l;};
	};

#endif
