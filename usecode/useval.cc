/*
 *  useval.cc - Values used in Usecode interpreter.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <fstream>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <limits>
#include "useval.h"
#include "utils.h"
#include "ucsymtbl.h"
#include "gamewin.h"
#include "ucmachine.h"
#include "databuf.h"
#include "ios_state.hpp"

#ifndef UNDER_EMBEDDED_CE
using std::cout;
using std::endl;
using std::dec;
using std::hex;
using std::memcpy;
using std::ostream;
using std::setfill;
using std::setw;
using std::strcmp;
using std::strlen;
using std::string;
using std::vector;
#endif


/*
 *  Destructor
 *
 */

Usecode_value::~Usecode_value() {
	destroy();
}

/*
 *  Copy another to this.
 */

Usecode_value &Usecode_value::operator=(
    const Usecode_value &v2
) {
	if (&v2 == this)
		return *this;
	if (type == v2.type) {
		if (type == string_type) {
			strval = v2.strval;
			return *this;
		}
	}
	destroy();
	type = v2.type;         // Assign new values.
	switch (type) {
	case int_type:
		intval = v2.intval;
		break;
	case pointer_type:
		ptrval = v2.ptrval;
		keep_ptr = v2.keep_ptr;
		break;
	case string_type:
		new (&strval) string(v2.strval);
		break;
	case array_type:
		arrayval.cnt = v2.arrayval.cnt;
		arrayval.elems = new Usecode_value[arrayval.cnt];
		for (int i = 0; i < arrayval.cnt; ++i)
			arrayval.elems[i] = v2.arrayval.elems[i];
		break;
	case class_sym_type:
		clssym = v2.clssym;
		break;
	case class_obj_type:        // Copy ->.
		arrayval.cnt = v2.arrayval.cnt;
		arrayval.elems = v2.arrayval.elems;
		break;
	}
	undefined = v2.undefined;
	return *this;
}

/*
 *  Steals an array from another usecode value. If the other
 *  usecode value is not an array, it will be converted into
 *  one and then its data will be stolen.
 *
 *  Warning: is left undefined after this.
 */

void Usecode_value::steal_array(Usecode_value &v2) {
	destroy();
	undefined = false;
	type = array_type;
	if (v2.type == array_type) {
		// Swipe array.
		arrayval.cnt = v2.arrayval.cnt;
		arrayval.elems = v2.arrayval.elems;
	} else {
		arrayval.elems = new Usecode_value[1];
		arrayval.cnt = 1;
		if (v2.type == string_type)
			// Swipe string.
			new (&arrayval.elems[0].strval) string(std::move(v2.strval));
		else
			arrayval.elems[0] = v2;
	}
	v2.undefined = true;
	v2.type = int_type;
	v2.intval = 0;
}

/*
 *  Resize array (or turn single element into an array).  The new values
 *  are (integer) 0.
 *
 *  Output: Always true.
 */

int Usecode_value::resize(
    int new_size
) {
	if (type != array_type) { // Turn it into an array.
		Usecode_value elem(*this);
		*this = Usecode_value(new_size, &elem);
		return (1);
	}
	int size = arrayval.cnt; // Get current size.
	if (new_size == size)
		return (1);     // Nothing to do.
	Usecode_value *newvals = new Usecode_value[new_size];
	// Move old values over.
	int cnt = new_size < size ? new_size : size;
	for (int i = 0; i < cnt; i++)
		newvals[i] = arrayval.elems[i];
	delete [] arrayval.elems;
	arrayval.elems = newvals;        // Store new.
	arrayval.cnt = new_size;
	return (1);
}

void    Usecode_value::push_back(int i) {
	resize(arrayval.cnt + 1);
	arrayval.elems[arrayval.cnt - 1] = Usecode_value(i);
}

/*
 *  Comparator.
 *
 *  Output: 1 if they match, else 0.
 */

bool Usecode_value::operator==(
    const Usecode_value &v2
) const {
	if (&v2 == this)
		return true;        // Same object.
	switch (type) {
	case int_type:
		switch (v2.type) {
		case int_type:
			return intval == v2.intval;
		case pointer_type:
			// Allow 0 == ptr
			return intval == 0 && v2.ptrval == nullptr;
		case array_type:
			// Okay if 0 == empty array.
			// Otherwise, compare first element of array versus value
			// (happens in blacksword usecode).
			return v2.get_array_size()
			       ? *this == v2.get_elem(0)
				   : intval == 0;
		default:
			return false;
		}
	case pointer_type:
		switch (v2.type) {
		case int_type:
			// Allow ptr == 0
			return ptrval == nullptr && v2.intval == 0;
		case pointer_type:
			return ptrval == v2.ptrval;
		case array_type:
			// On pointer == array, compare pointer to first element of array.
			return v2.get_array_size() && *this == v2.get_elem(0);
		default:
			return false;
		}
	case array_type:
		switch (v2.type) {
		case int_type:
			// Allow empty array == 0
			// Otherwise, compare first element of array versus value
			// (makes it symmetric).
			return get_array_size()
			       ? get_elem(0) == v2
				   : v2.intval == 0;
		case pointer_type:
			// On array == pointer, compare first element of array to pointer.
			return get_array_size() && get_elem(0) == v2;
		case array_type: {
			// On array == array, arrays must be equal.
			int cnt = get_array_size();
			if (cnt != v2.get_array_size()) {
				return false;
			}
			for (int i = 0; i < cnt; i++) {
				const Usecode_value &e1 = get_elem(i);
				const Usecode_value &e2 = v2.get_elem(i);
				if (!(e1 == e2)) {
					return false;
				}
			}
			// Arrays matched.
			return true;
		}
		default:
			return false;
		}
	case string_type:
		return v2.type == string_type && strval == v2.strval;
	case class_sym_type:
		return v2.type == class_sym_type && clssym == v2.clssym;
	case class_obj_type:
		return v2.type == class_obj_type && ptrval == v2.ptrval;
	default:
		return false;
	}
}

/*
 *  Search an array for a given value.
 *
 *  Output: Index if found, else -1.
 */

int Usecode_value::find_elem(
    const Usecode_value &val
) {
	if (type != array_type)
		return (-1);        // Not an array.
	int i;
	for (i = 0; i < arrayval.cnt; i++)
		if (arrayval.elems[i] == val)
			return (i);
	return (-1);
}

/*
 *  Concatenate another value onto this.
 *
 *  Output: This.
 */

Usecode_value &Usecode_value::concat(
    Usecode_value &val2     // Concat. val2 onto end.
) {
	int size;           // Size of result.
	if (type != array_type) {   // Not an array?  Create one.
		// Current value becomes 1st elem.
		Usecode_value tmp(1, this);
		*this = tmp;
		size = 1;
	} else
		size = get_array_size();
	if (val2.type != array_type) {  // Appending a single value?
		resize(size + 1);
		put_elem(size, val2);
	} else {            // Appending an array.
		int size2 = val2.get_array_size();
		resize(size + size2);
		for (int i = 0; i < size2; i++)
			put_elem(size + i, val2.get_elem(i));
	}
	return (*this);
}

/*
 *  Append a list of integer values.
 */

void Usecode_value::append(
    int *vals,
    int cnt
) {
	assert(type == array_type);
	int sz = get_array_size();
	resize(sz + cnt);
	for (int i = 0; i < cnt; i++)
		arrayval.elems[sz + i].intval = vals[i];
}

/*
 *  Given an array and an index, and a 2nd value, add the new value at that
 *  index, or if the new value is an array itself, add its values.
 *
 *  Output: # elements added.
 */

int Usecode_value::add_values(
    int index,
    Usecode_value &val2
) {
	int size = get_array_size();
	if (!val2.is_array()) {     // Simple case?
		if (index >= size)
			resize(index + 1);
		put_elem(index, val2);
		return (1);
	}
	// Add each element.
	int size2 = val2.get_array_size();
	if (index + size2 > size)
		resize(index + size2);
	for (int i = 0; i < size2; i++)
		put_elem(index++, val2.get_elem(i));
	return (size2);         // Return # added.
}

/*
 *  Print in ASCII.
 */

void Usecode_value::print(
    ostream &out, bool shortformat
) {
	boost::io::ios_flags_saver flags(out);
	boost::io::ios_fill_saver fill(out);
	switch (type) {
	case int_type:
		out << hex << setfill('0') << setw(4);
		out << (intval & 0xffff);
		break;
	case pointer_type:
		out << hex << setfill('0') << setw(8);
		out << reinterpret_cast<uintptr>(ptrval);
		break;
	case string_type:
		out << '"' << strval << '"';
		break;
	case array_type: {
		out << "[ ";
		int i;
		for (i = 0; i < arrayval.cnt; i++) {
			if (!shortformat || i < 2) {
				if (i)
					out << ", ";

				arrayval.elems[i].print(out);
			}
		}
		if (shortformat && i > 2)
			out << ", ... (size " << i << ")";
		out << " ]";
	}
	break;
	case class_obj_type: {
		Usecode_class_symbol *c = get_class_ptr();
		out << "->";
		if (c)
			out << c->get_name();
		else
			out << "obj?";
		break;
	}
	default:
		break;
	}
}

Usecode_value Usecode_value::operator+(const Usecode_value &v2) {
	Usecode_value &v1 = *this;
	Usecode_value sum(0);

	if (v1.is_undefined()) {
		sum = v2;
	} else if (v2.is_undefined()) {
		sum = v1;
	} else if (v1.get_type() == Usecode_value::int_type) {
		if (v2.get_type() == Usecode_value::int_type) {
			sum = v1.get_int_value() + v2.get_int_value();
		} else if (v2.get_type() == Usecode_value::string_type) {
#if 0
			// This seems to be how addition of int + string was done in the
			// original, but I won't do it here.
			sum = v1.get_int_value() + v2.need_int_value();
#else
			// Note: this actually seems wrong compared to the originals,
			// but I am leaving this the way it is unless it causes a bug.
			unsigned int newlen = strlen(v2.get_str_value()) + 32;
			char *buf = new char[newlen];
			snprintf(buf, newlen, "%ld%s",
			         v1.get_int_value(),
			         v2.get_str_value());
			sum = Usecode_value(buf);
			delete[] buf;
#endif
		} else {
			sum = v1;
		}
	} else if (v1.get_type() == Usecode_value::string_type) {
		if (v2.get_type() == Usecode_value::int_type) {
			unsigned int newlen = strlen(v1.get_str_value()) + 32;
			char *buf = new char[newlen];
			snprintf(buf, newlen, "%s%ld",
			         v1.get_str_value(),
			         v2.get_int_value());
			sum = Usecode_value(buf);
			delete[] buf;
		} else if (v2.get_type() == Usecode_value::string_type) {
			unsigned int newlen = strlen(v1.get_str_value()) +
			                      strlen(v2.get_str_value()) + 32;
			char *buf = new char[newlen];
			snprintf(buf, newlen, "%s%s",
			         v1.get_str_value(),
			         v2.get_str_value());
			sum = Usecode_value(buf);
			delete[] buf;
		} else {
			sum = v1;
		}
	}
	return sum;
}

template <typename T, template <typename> class Op>
struct safe_divide : Op<T> {
	T operator()(const T &x, const T &y) const {
		// Watch for division by zero. Originals do this, but they return
		// 32768 in all cases.
		if (y == 0)
			return x < 0 ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
		else
			return Op<T>::operator()(x, y);
	}
};

template <typename Op>
Usecode_value Usecode_value::operate(const Usecode_value &v2) {
	Op op;
	Usecode_value &v1 = *this;
	Usecode_value result(0);

	if (v1.is_undefined())
		// Seems correct.
		result = op(0, v2.need_int_value());
	else if (v2.is_undefined()) {
		// Just return zero
	} else if (v1.get_type() == Usecode_value::int_type) {
		if (v2.get_type() == Usecode_value::int_type)
			result = op(v1.get_int_value(), v2.get_int_value());
		else if (v2.get_type() == Usecode_value::string_type)
			result = op(v1.get_int_value(), v2.need_int_value());
		else
			result = v1;
	} else if (v1.get_type() == Usecode_value::string_type) {
		if (v2.get_type() == Usecode_value::string_type) {
			// Note: SI usecode seems to assume this in two cases: selling
			// leather stuff to Krayg in Monitor and party members when
			// you ask Boydon to leave. However, the engine behaves unlike
			// what the usecode expects.
			// I decided to go the way usecode expects, adding a space along
			// the way, to "fix" this as it seems to be the only places in
			// the originals where this matters.
			unsigned int newlen = strlen(v1.get_str_value()) +
			                      strlen(v2.get_str_value()) + 32;
			char *buf = new char[newlen];
			snprintf(buf, newlen, "%s %s",
			         v1.get_str_value(),
			         v2.get_str_value());
			result = Usecode_value(buf);
			delete[] buf;
		} else
			// This seems right.
			result = v1;
	}
	return result;
}

Usecode_value Usecode_value::operator-(const Usecode_value &v2) {
	return operate<std::minus<long> >(v2);
}

Usecode_value Usecode_value::operator*(const Usecode_value &v2) {
	return operate<std::multiplies<long> >(v2);
}

Usecode_value Usecode_value::operator/(const Usecode_value &v2) {
	return operate<safe_divide<long, std::divides> >(v2);
}

Usecode_value Usecode_value::operator%(const Usecode_value &v2) {
	return operate<safe_divide<long, std::modulus> >(v2);
}

/*
 *  Serialize out.
 *
 *  Output: # bytes stored, or -1 if error.
 */

bool Usecode_value::save(
    DataSource *out
) {
	out->write1(static_cast<int>(type));
	switch (type) {
	case int_type:
		out->write4(intval);
		break;
	case pointer_type:
		out->write4(0);
		break;
	case class_sym_type: {
		const char *classname = clssym->get_name();
		int len = std::strlen(classname);
		out->write2(len);
		out->write(classname, len);
		break;
	}
	case string_type: {
		out->write2(strval.size());
		out->write(strval);
		break;
	}
	case array_type:
	case class_obj_type: {
		int len = arrayval.cnt;
		out->write2(len); // first length, then length Usecode_values
		for (int i = 0; i < len; i++) {
			if (!arrayval.elems[i].save(out))
				return false;
		}
		break;
	}
	default:
		return false;
	}
	return true;
}

/*
 *  Serialize in.  Assumes 'this' contains no data yet.
 *
 *  Output: False if error.
 */

bool Usecode_value::restore(
    DataSource *in
) {
	undefined = false;
	type = static_cast<Val_type>(in->read1());
	switch (type) {
	case int_type:
		intval = in->read4();
		return true;
	case pointer_type:
		in->read4();
		ptrval = 0; //DON'T dereference this pointer!
		// Maybe add a new type "serialized_pointer" to prevent "accidents"?
		return true;
	case class_sym_type: {
		int len = in->read2();
		char *nm = new char[len + 1];
		in->read(nm, len);
		nm[len] = 0;
		Game_window *gwin = Game_window::get_instance();
		clssym = gwin->get_usecode()->get_class(nm);
		return true;
	}
	case string_type: {
		int len = in->read2();
		new (&strval) string;
		in->read(strval, len);
		return true;
	}
	case array_type:
	case class_obj_type: {
		int len = in->read2();
		arrayval.cnt = len;  // Stores class, class vars.
		arrayval.elems = new Usecode_value[arrayval.cnt];
		for (int i = 0; i < len; i++)
			if (!arrayval.elems[i].restore(in))
				return false;
		return true;
	}
	default:
		return false;
	}
}


ostream &operator<<(ostream &out, Usecode_value &val) {
	val.print(out, true);
	return out;
}

/*
 *  Create/delete class objects.
 */
void Usecode_value::class_new(Usecode_class_symbol *cls, int nvars) {
	assert(type == int_type);
	type = class_obj_type;
	arrayval.cnt = nvars + 1;    // Stores class, class vars.
	arrayval.elems = new Usecode_value[arrayval.cnt];
	arrayval.elems[0] = Usecode_value(cls);
}

void Usecode_value::class_delete() {
	assert(type == class_obj_type);
	delete [] arrayval.elems;
	*this = Usecode_value(0);
}
