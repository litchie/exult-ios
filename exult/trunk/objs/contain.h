/*
 *	contain.h - Container objects.
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

#ifndef INCL_CONTAIN
#define INCL_CONTAIN	1

#include "iregobjs.h"

/*
 *	A container object:
 */
class Container_game_object : public Ireg_game_object
	{
	int volume_used;		// Amount of volume occupied.
	char resistance;	// Resistance to attack.
protected:
	Object_list objects;		// ->first object.
	int get_max_volume() const	// Max. we'll hold.  (Guessing).
		{ int v = get_volume(); return v; }
public:
	Container_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft,
				char res = 0)
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
		{ return get_max_volume() <= 0 || 
			obj->get_volume() + volume_used <= get_max_volume(); }
					// Remove an object.
	virtual void remove(Game_object *obj);
					// Add an object.
	virtual bool add(Game_object *obj, bool dont_check = false,
							bool combine = false);
					// Change member shape.
	virtual void change_member_shape(Game_object *obj, int newshape);
					// Find object's spot.
	virtual int find_readied(Game_object *obj)
		{ return -1; }
	virtual Game_object *get_readied(int index) const
		{ return 0; }
	virtual void call_readied_usecode(int index,
					Game_object *obj, int eventid)
		{  }
					// Add/remove quantities of objs.
	virtual int add_quantity(int delta, int shapenum, 
			int qual = c_any_qual,
			int framenum = c_any_framenum, int dontcreate = 0);
	virtual int create_quantity(int delta, int shapenum, int qual,
					int framenum, bool temporary = false);
	virtual int remove_quantity(int delta, int shapenum, int qual,
								int framenum);
	virtual Game_object *find_item(int shapenum, int qual, int framenum);
					// Run usecode function.
	virtual void activate(int event = 1);
	virtual int get_weight();
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Count contained objs.
	virtual int count_objects(int shapenum, int qual = c_any_qual,
						int framenum = c_any_framenum);
					// Get contained objs.
	virtual int get_objects(Game_object_vector& vec, int shapenum, 
					int qual, int framenum);
	virtual void set_flag_recursively(int flag);
					// Write out to IREG file.
	virtual void write_ireg(DataSource* out);
				// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size();
					// Write contents in IREG format.
	virtual void write_contents(DataSource* out);

	virtual int get_obj_hp() const { return resistance; }
	virtual void set_obj_hp(int hp) { resistance = (char)hp; }

	bool extract_contents();

	virtual void delete_contents();
	
	virtual void remove_this(int nodel = 0);

	virtual Container_game_object *as_container() { return this; }

	};

#endif
