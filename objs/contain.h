/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Contain.h - Container objects.
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

#ifndef INCL_CONTAIN
#define INCL_CONTAIN	1

#include "iregobjs.h"

/*
 *	A container object:
 */
class Container_game_object : public Ireg_game_object
	{
	int volume_used;		// Amount of volume occupied.
	unsigned char resistance;	// Resistance to attack.
protected:
	Object_list objects;		// ->first object.
	int get_max_volume() const	// Max. we'll hold. (0 == infinite).
		{ return 4*get_volume(); }
public:
	Container_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft,
				unsigned char res = 0)
		: Ireg_game_object(shapenum, framenum, tilex, tiley, lft),
		  volume_used(0), resistance(res), objects(0)
		{  }
	Container_game_object() : volume_used(0), resistance(0),
		objects(0) {  }
	virtual ~Container_game_object();
	Object_list& get_objects()
		{ return objects; }
					// For when an obj's quantity changes:
	void modify_volume_used(int delta)
		{ volume_used += delta; }
					// Room for this object?
	int has_room(Game_object *obj) const
		{ return obj->get_volume() + volume_used <= get_max_volume(); }
					// Remove an object.
	virtual void remove(Game_object *obj);
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Add to NPC 'ready' spot.
	virtual int add_readied(Game_object *obj, int index)
		{ return add(obj); }
					// Change member shape.
	virtual void change_member_shape(Game_object *obj, int newshape);
					// Find object's spot.
	virtual int find_readied(Game_object *obj)
		{ return -1; }
	virtual Game_object *get_readied(int index) const
		{ return 0; }
					// Add/remove quantities of objs.
	virtual int add_quantity(int delta, int shapenum, int qual = c_any_qual,
				int framenum = c_any_framenum, int dontcreate = 0);
	virtual int create_quantity(int delta, int shapenum, int qual,
							int framenum);
	virtual int remove_quantity(int delta, int shapenum, int qual,
								int framenum);
	virtual Game_object *find_item(int shapenum, int qual, int framenum);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine, int event = 1);
	virtual int get_weight();
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Count contained objs.
	virtual int count_objects(int shapenum, int qual = c_any_qual,
							int framenum = c_any_framenum);
					// Get contained objs.
	virtual int get_objects(Game_object_vector& vec, int shapenum, int qual,
						int framenum);
					// Under attack.
	virtual Game_object *attacked(Actor *attacker, int weapon_shape = 0,
					int ammo_shape = 0);
	virtual void set_flag_recursively(int flag);
					// Write out to IREG file.
	virtual void write_ireg(std::ostream& out);
					// Write contents in IREG format.
	void write_contents(std::ostream& out);
	};

#endif
