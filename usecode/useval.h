/*
 *  useval.h - Values used in Usecode interpreter.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2013  The Exult Team
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
#define USEVAL_H    1

#include <cassert>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>   // STL string
#include <vector>   // STL container

#include "databuf.h"
#include "objs.h"

class Usecode_class_symbol;

/*
 *  A value that we store can be an integer, string, or array.
 */
class Usecode_value {
public:
	enum Val_type {         // The types:
	    int_type = 0,
	    string_type = 1,    // Allocated string.
	    array_type = 2,
	    pointer_type = 3,
	    class_sym_type = 4, // ->Usecode_class_symbol
	    class_obj_type = 5  // An 'array_type' for a class obj.
	};

private:
	Val_type type;      // Type stored here.
	Game_object_shared keep_ptr;
    union {
		long intval;
		char *strval;
		struct {
			Usecode_value *elems;
			short cnt;
		} arrayval;
		Game_object *ptrval;
		Usecode_class_symbol *clssym;
	};	// Anonymous union member
	bool undefined;

	template <typename Op>
	Usecode_value operate(const Usecode_value &v2);


public:
	inline Usecode_value() : type(int_type), intval(0), undefined(true) {}
	inline Usecode_value(int ival) : type(int_type), intval(ival), undefined(false) {}
	Usecode_value(const char *s);
	// Create array with 1st element.
	Usecode_value(int size, Usecode_value *elem0)
		: type(array_type), undefined(false) {
		arrayval.elems = new Usecode_value[size];
		arrayval.cnt = size;
		if (elem0)
			arrayval.elems[0] = *elem0;
	}
	Usecode_value(Game_object *ptr) : type(pointer_type), undefined(false) {
	    ptrval = ptr;
	    keep_ptr = ptr ? ptr->shared_from_this() : nullptr;
	}
    Usecode_value(Game_object_shared ptr) : type(pointer_type), undefined(false) {
	    ptrval = ptr.get();
	    keep_ptr = ptr;
	}
	Usecode_value(Usecode_class_symbol *ptr) : type(class_sym_type),
		undefined(false) {
		clssym = ptr;
	}
	~Usecode_value();
	Usecode_value &operator=(const Usecode_value &v2);
	Usecode_value &operator=(const char *str);
	// Copy ctor.
	inline Usecode_value(const Usecode_value &v2)
		: type(int_type) {
		*this = v2;
	}

	Usecode_value operator+(const Usecode_value &v2);
	Usecode_value operator-(const Usecode_value &v2);
	Usecode_value operator*(const Usecode_value &v2);
	Usecode_value operator/(const Usecode_value &v2);
	Usecode_value operator%(const Usecode_value &v2);
	// Comparator.
	void push_back(int);
	bool operator==(const Usecode_value &v2) const;
	bool operator!=(const Usecode_value &v2) const {
		return !(*this == v2);
	}

	inline Val_type get_type() const {
		return type;
	}
	int get_array_size() const {    // Get size of array.
		return (type == array_type) ? arrayval.cnt : 0;
	}
	bool is_array() const {
		return (type == array_type);
	}
	bool is_int() const {
		return (type == int_type);
	}
	bool is_ptr() const {
		return (type == pointer_type);
	}
	long get_int_value() const { // Get integer value.
#ifdef DEBUG
		if (type == pointer_type || (type == int_type && (intval > 0x10000 || intval < -0x10000)))
			std::cerr << "Probable attempt at getting int value of pointer!!" << std::endl;
#endif
		return ((type == int_type) ? intval : 0);
	}
	Game_object *get_ptr_value() const { // Get pointer value.
		return ((type == pointer_type) ? ptrval : 0);
	}
	// Get string value.
	const char *get_str_value() const {
		static char const *emptystr = "";
		return ((type == string_type) ? strval :
		        ((undefined ||
		          (type == array_type && arrayval.cnt == 0)) ? emptystr : 0));
	}
	long need_int_value() const {
		// Convert strings.
		const char *str = get_str_value();
		return str ? std::atoi(str)
		       : ((type == array_type && get_array_size())
		          ? arrayval.elems[0].need_int_value()
		          // Pointer = ref.
		          : (type == pointer_type ? (reinterpret_cast<uintptr>(ptrval) & 0x7ffffff)
		             : get_int_value()));
	}
	// Add array element. (No checking!)
	void put_elem(int i, Usecode_value &val) {
		arrayval.elems[i] = val;
	}
	// Get an array element.
	inline Usecode_value &get_elem(int i) const {
		static Usecode_value zval(0);
		return (type == array_type) ? arrayval.elems[i] : zval;
	}
	inline Usecode_value &operator[](int i) {
		assert(type == array_type);
		return arrayval.elems[i];
	}
	// Get array elem. 0, or this.
	inline Usecode_value &get_elem0() {
		static Usecode_value zval(0);
		return (type == array_type) ? (get_array_size() ? arrayval.elems[0]
		                               : zval) : *this;
	}
	void steal_array(Usecode_value &v2);
	inline bool is_false() const {  // Represents a FALSE value?
		switch (type) {
		case int_type:
			return intval == 0;
		case pointer_type:
			return ptrval == NULL;
		case array_type:
			return arrayval.cnt == 0;
		default:
			return false;
		}
	}
	inline bool is_true() const {
		return !is_false();
	}

	inline bool is_undefined() const {
		return undefined;
	}

	int resize(int new_size);   // Resize array.
	// Look in array for given value.
	int find_elem(const Usecode_value &val);
	// Concat. to end of this array.
	Usecode_value &concat(Usecode_value &val2);
	void append(int *vals, int cnt);// Append integer values.
	// Add value(s) to an array.
	int add_values(int index, Usecode_value &val2);
	void print(std::ostream &out, bool shortformat = false); // Print in ASCII.
	// Save/restore.
	bool save(DataSource *out);
	bool restore(DataSource *in);
	// Class objects.
	void class_new(Usecode_class_symbol *cls, int nvars);
	void class_delete();
	Usecode_value &nth_class_var(int n) {
		// Note:  Elem. 0 is the ->class.
		static Usecode_value zval(0);
		return (type == class_obj_type && n + 1 < arrayval.cnt) ?
		       arrayval.elems[n + 1] : zval;
	}
	int get_class_var_count() {
		// Note:  Elem. 0 is the ->class.
		return type == class_obj_type ? arrayval.cnt - 1 : 0;
	}
	Usecode_class_symbol *get_class_ptr() const {
		return (type == class_obj_type) ?
		       arrayval.elems[0].clssym : 0;
	}
};


std::ostream &operator<<(std::ostream &out, Usecode_value &val);

#endif
