/*
 *	useval.h - Values used in Usecode interpreter.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef USEVAL_H
#define USEVAL_H	1

#include <cassert>
#include <iostream>

#include <vector>	// STL container
#include <deque>	// STL container
#include <string>	// STL string

class Game_object;

/*
 *	A value that we store can be an integer, string, or array.
 */
class Usecode_value
	{
public:
	enum Val_type			// The types:
		{
		int_type = 0,
		string_type = 1,	// Allocated string.
		array_type = 2,
		end_of_array_type = 3,	// Marks end of array.
		pointer_type = 4
		};
private:
	Val_type type;		// Type stored here.
	union
		{
		long intval;
		char *str;
		Usecode_value *array;
		Game_object *ptr;
		} value;

	bool undefined;
					// Count array elements.
	static int count_array(const Usecode_value& val);
public:
	inline Usecode_value() : type(int_type), undefined(true)
		{ value.intval = 0; }
	inline Usecode_value(int ival) : type(int_type), undefined(false)
		{ value.intval = ival; }
	Usecode_value(const char *s);
					// Create array with 1st element.
	Usecode_value(int size, Usecode_value *elem0) 
			: type(array_type), undefined(false)
		{
		value.array = new Usecode_value[size + 1];
		value.array[size].type = end_of_array_type;
		if (elem0)
			value.array[0] = *elem0;
		}
	Usecode_value(Game_object *ptr) : type(pointer_type), undefined(false)
		{ value.ptr = ptr; }
	~Usecode_value();
	Usecode_value& operator=(const Usecode_value& v2);
	Usecode_value& operator=(const char *str);
					// Copy ctor.
	inline Usecode_value(const Usecode_value& v2)
		: type(int_type)
		{ *this = v2; }

	Usecode_value operator+(const Usecode_value& v2);
					// Comparator.
	void	push_back(int);
	bool operator==(const Usecode_value& v2) const;
	bool operator!=(const Usecode_value& v2) const { return !(*this == v2); }

	inline Val_type get_type() const
		{ return type; }
	int get_array_size() const		// Get size of array.
		{ return (type == array_type) ? count_array(*this) : 0; }
	bool is_array() const
		{ return (type == array_type); }
	bool is_int() const
		{ return (type == int_type); }
	bool is_ptr() const
		{ return (type == pointer_type); }
	long get_int_value() const	// Get integer value.
		{ 
#ifdef DEBUG
		if (type == pointer_type || (type == int_type && (value.intval > 0x10000 || value.intval < -0x10000)))
			std::cerr << "Probable attempt at getting int value of pointer!!" << std::endl; 
#endif
		return ((type == int_type) ? value.intval : 0);
		}
	Game_object* get_ptr_value() const	// Get pointer value.
		{ return ((type == pointer_type) ? value.ptr : 0); }
					// Get string value.
	const char *get_str_value() const
		{ return ((type == string_type) ? value.str : 0); }
	long need_int_value() const
		{
					// Convert strings.
		const char *str = get_str_value();
		return str ? std::atoi(str) 
			: ((type == array_type && get_array_size())
			? value.array[0].need_int_value() 
					// Pointer = ref.
			: (type == pointer_type ? (value.intval&0x7ffffff)
					: get_int_value()));
		}
					// Add array element. (No checking!)
	void put_elem(int i, Usecode_value& val)
		{ value.array[i] = val; }
					// Get an array element.
	inline Usecode_value& get_elem(int i) const
		{
		static Usecode_value zval(0);
//		assert(type == array_type);//+++++Testing.
		return (type == array_type) ? value.array[i] : zval;
		}
	inline Usecode_value& operator[](int i)
		{
		assert(type == array_type);
		return value.array[i];
		}
					// Get array elem. 0, or this.
	inline Usecode_value& get_elem0()
		{ return (type == array_type) ? value.array[0] : *this; }
	inline bool is_false() const	// Represents a FALSE value?
		{
		switch(type)
			{
			case int_type:
				return value.intval == 0;
			case pointer_type:
				return value.ptr == NULL;
			case array_type:
				return value.array[0].type == end_of_array_type;
			default:
				return false;
			}
		}
	inline bool is_true() const
		{ return !is_false(); }

	inline bool is_undefined() const
		{ return undefined; }

	int resize(int new_size);	// Resize array.
					// Look in array for given value.
	int find_elem(const Usecode_value& val);
					// Concat. to end of this array.
	Usecode_value& concat(Usecode_value& val2);
	void append(int *vals, int cnt);// Append integer values.
					// Add value(s) to an array.
	int add_values(int index, Usecode_value& val2);
	void print(std::ostream& out, bool shortformat=false); // Print in ASCII.
					// Save/restore.
	int save(unsigned char *buf, int len);
	bool restore(unsigned char *& ptr, int len);
	};


std::ostream& operator<<(std::ostream& out, Usecode_value& val);

#endif
