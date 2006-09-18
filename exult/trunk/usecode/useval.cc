/*
 *	useval.cc - Values used in Usecode interpreter.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2002  The Exult Team
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

#ifndef ALPHA_LINUX_CXX
  #include <fstream>
  #include <cstring>
  #include <iomanip>
  #include <iostream>
  #include <cstdlib>
  #include <cstdio>
#endif

#include "useval.h"
#include "utils.h"
#include "ucsymtbl.h"

#ifndef UNDER_CE
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
#endif


/*
 *	Destructor
 *
 */

Usecode_value::~Usecode_value()
{
	if (type == array_type)
		delete [] value.array.elems;
	else if (type == string_type)
		delete [] value.str;
}

/*
 *	Copy another to this.
 */

Usecode_value& Usecode_value::operator=
	(
	const Usecode_value& v2
	)
	{
	if (&v2 == this)
		return *this;
	if (type == array_type)
		delete [] value.array.elems;
	else if (type == string_type)
		delete [] value.str;
	type = v2.type;			// Assign new values.
	switch (type) {
	case int_type:
		value.intval = v2.value.intval;
		break;
	case pointer_type:
		value.ptr = v2.value.ptr;
		break;
	case string_type:
		value.str = v2.value.str ? newstrdup(v2.value.str) : 0;
		break;
	case array_type:
                value.array.cnt = v2.value.array.cnt;
		value.array.elems = new Usecode_value[value.array.cnt];
		for (int i = 0; i < value.array.cnt; ++i)
			value.array.elems[i] = v2.value.array.elems[i];
		break;
	case class_sym_type:
		value.cptr = v2.value.cptr;
		break;
	case class_obj_type:		// Copy ->.
		value.array.cnt = v2.value.array.cnt;
		value.array.elems = v2.value.array.elems;
		break;
	}
	undefined = v2.undefined;
	return *this;
	}

/*
 *	Set this to (a copy of) a string.
 */


Usecode_value& Usecode_value::operator=
	(
	const char *str
	)
	{
	if (type == array_type)
		delete [] value.array.elems;
	else if (type == string_type)
		delete [] value.str;
	type = string_type;
	value.str = str ? newstrdup(str) : 0;
	return *this;
	}

/*
 *	Create a string value.
 */

Usecode_value::Usecode_value
	(
	const char *s
	) : type(string_type), undefined(false)
	{
	value.str = s ? newstrdup(s) : 0; 
	}

/*
 *	Resize array (or turn single element into an array).  The new values
 *	are (integer) 0.
 *
 *	Output:	Always true.
 */

int Usecode_value::resize
	(
	int new_size
	)
	{
	if (type != array_type)	// Turn it into an array.
		{
		Usecode_value elem(*this);
		*this = Usecode_value(new_size, &elem);
		return (1);
		}
	int size = value.array.cnt;	// Get current size.
	if (new_size == size)
		return (1);		// Nothing to do.
	Usecode_value *newvals = new Usecode_value[new_size];
					// Move old values over.
	int cnt = new_size < size ? new_size : size;
	for (int i = 0; i < cnt; i++)
		newvals[i] = value.array.elems[i];
	delete [] value.array.elems;
	value.array.elems = newvals;		// Store new.
	value.array.cnt = new_size;
	return (1);
	}

void	Usecode_value::push_back(int i)
{
	resize(value.array.cnt + 1);
	value.array.elems[value.array.cnt-1]=Usecode_value(i);
}

/*
 *	Comparator.
 *
 *	Output:	1 if they match, else 0.
 */

bool Usecode_value::operator==
	(
	const Usecode_value& v2
	) const
	{
	if (&v2 == this)
		return true;		// Same object.
	if (type == int_type)		// Might be ptr==0.
		return (v2.type == int_type || v2.type == pointer_type) ?
					(value.intval == v2.value.intval)
					// Okay if 0==empty array.
			: v2.type == array_type &&
					// Happens in blacksword usecode:
					(v2.get_array_size() ?
					v2.get_elem(0).value.intval == value.intval:
					!value.intval);
	else if (type == pointer_type)	// Might be ptr==0.
		{			// Or ptr == array.  Test elem. 0.
		if (v2.type == pointer_type || v2.type == int_type)
			return (value.ptr == v2.value.ptr);
		else if (v2.type == array_type && v2.get_array_size())
			{
			const Usecode_value& val2 = v2.value.array.elems[0];
			return (value.ptr == val2.value.ptr);
			}
		else
			return false;
		}
	else if (type == array_type)
		{
		if (v2.type == int_type)
			// Making it symetric just in case:
			return (get_array_size() ?
				get_elem(0).value.intval == v2.value.intval :
				!v2.value.intval);
		else if (v2.type == pointer_type && get_array_size())
			{
			Usecode_value& v = get_elem(0);
			return v2.value.ptr == v.value.ptr;
			}
		if (v2.type != array_type)
			return false;
		int cnt = get_array_size();
		if (cnt != v2.get_array_size())
			return false;
		for (int i = 0; i < cnt; i++)
			{
			Usecode_value& e1 = get_elem(i);
			const Usecode_value& e2 = v2.get_elem(i);
			if (!(e1 == e2))
				return false;
			}
		return true;		// Arrays matched.
		}
	else if (type == string_type)
		return (v2.type == string_type &&
					strcmp(value.str, v2.value.str) == 0);
	else
		return false;
	}

/*
 *	Search an array for a given value.
 *
 *	Output:	Index if found, else -1.
 */

int Usecode_value::find_elem
	(
	const Usecode_value& val
	)
	{
	if (type != array_type)
		return (-1);		// Not an array.
	int i;
	for (i = 0; i < value.array.cnt; i++)
		if (value.array.elems[i] == val)
			return (i);
	return (-1);
	}

/*
 *	Concatenate another value onto this.
 *
 *	Output:	This.
 */

Usecode_value& Usecode_value::concat
	(
	Usecode_value& val2		// Concat. val2 onto end.
	)
	{
	int size;			// Size of result.
	if (type != array_type)		// Not an array?  Create one.
		{			// Current value becomes 1st elem.
		Usecode_value tmp(1, this);
		*this = tmp;
		size = 1;
		}
	else
		size = get_array_size();
	if (val2.type != array_type)	// Appending a single value?
		{
		resize(size + 1);
		put_elem(size, val2);
		}
	else				// Appending an array.
		{
		int size2 = val2.get_array_size();
		resize(size + size2);
		for (int i = 0; i < size2; i++)
			put_elem(size + i, val2.get_elem(i));
		}
	return (*this);
	}

/*
 *	Append a list of integer values.
 */

void Usecode_value::append
	(
	int *vals,
	int cnt
	)
	{
	assert(type == array_type);
	int sz = get_array_size();
	resize(sz + cnt);
	for (int i = 0; i < cnt; i++)
		value.array.elems[sz + i].value.intval = vals[i];
	}

/*
 *	Given an array and an index, and a 2nd value, add the new value at that
 *	index, or if the new value is an array itself, add its values.
 *
 *	Output:	# elements added.
 */

int Usecode_value::add_values
	(
	int index,
	Usecode_value& val2
	)
	{
	int size = get_array_size();
	if (!val2.is_array())		// Simple case?
		{
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
	return (size2);			// Return # added.
	}

/*
 *	Print in ASCII.
 */

void Usecode_value::print
	(
	 ostream& out, bool shortformat
	)
	{
	switch ((Val_type) type)
		{
	case int_type:
		out << hex << setfill((char)0x30) << setw(4);
		out << (value.intval&0xffff);
		out << dec;
		break;
	case pointer_type:
		out << hex << setfill((char)0x30) << setw(8);
		out << (long)value.ptr;
		out << dec;
		break;
	case string_type:
		out << '"' << value.str << '"'; break;
	case array_type:
		{
		out << "[ ";
		int i;
		for (i = 0; i < value.array.cnt; i++) {
			if (!shortformat || i < 2) {
				if (i)
					out << ", ";
				
				value.array.elems[i].print(out);
			}
		}
		if (shortformat && i > 2)
			out << ", ... (size " << i << ")";
		out << " ]";
		}
		break;
	case class_obj_type:
		{
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

Usecode_value Usecode_value::operator+(const Usecode_value& v2)
{
	Usecode_value& v1 = *this;
	Usecode_value sum(0);

	if (v1.is_undefined())
	{
		sum = v2;
	}
	else if (v2.is_undefined())
	{
		sum = v1;
	} 
	else if (v1.get_type() == Usecode_value::int_type)
	{
		if (v2.get_type() == Usecode_value::int_type) {
			sum = Usecode_value(v1.get_int_value()
					+ v2.get_int_value());
		} else if (v2.get_type() == Usecode_value::string_type) {
			unsigned int newlen = strlen(v2.get_str_value()) + 32;
			char* buf = new char[newlen];
			snprintf(buf, newlen, "%ld%s",
					 v1.get_int_value(),
					 v2.get_str_value());
			sum = Usecode_value(buf);
			delete[] buf;
		} else {
			sum = v1;
		}
	}
	else if (v1.get_type() == Usecode_value::string_type)
	{
		if (v2.get_type() == Usecode_value::int_type) {
			unsigned int newlen = strlen(v1.get_str_value()) + 32;
			char* buf = new char[newlen];
			snprintf(buf, newlen, "%s%ld", 
				v1.get_str_value(),
				v2.get_int_value());
			sum = Usecode_value(buf);
			delete[] buf;
		} else if (v2.get_type() == Usecode_value::string_type) {
			unsigned int newlen = strlen(v1.get_str_value()) +
				strlen(v2.get_str_value()) + 32;
			char* buf = new char[newlen];
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

/*
 *	Serialize out.
 *
 *	Output:	# bytes stored, or -1 if error.
 */

int Usecode_value::save
	(
	unsigned char *buf,
	int buflen
	)
	{
	uint8 *ptr = buf;
	switch ((Val_type) type)
		{
	case int_type:
		if (buflen < 5)
			return -1;
		*ptr++ = type;
		Write4(ptr, value.intval);
		break;
	case pointer_type:
		if (buflen < 5)
			return -1;
		*ptr++ = type;
		Write4(ptr, 0);
		break;
	case string_type:
		{
		int len = std::strlen(value.str);
		if (buflen < len + 3)
			return -1;
		*ptr++ = type;
		Write2(ptr, len);
		std::memcpy(ptr, value.str, len);
		ptr += len;
		break;
		}
	case array_type:
		{
		if (buflen < 3)
			return -1;
		*ptr++ = type;
		int len = value.array.cnt;
		Write2(ptr, len); // first length, then length Usecode_values
		int remaining = buflen - 3;
		for (int i=0; i < len; i++) {
			int retval = value.array.elems[i].save(ptr, remaining);
			if (retval == -1)
				return -1;

			ptr += retval;
			remaining -= retval;
		}
		break;
		}
	default:
		return -1;
		}
	return (ptr - buf);
	}

/*
 *	Serialize in.  Assumes 'this' contains no data yet.
 *
 *	Output:	False if error.
 */

bool Usecode_value::restore
	(
	unsigned char *& ptr,		// Ptr. to data.  Updated past it.
	int buflen
	)
	{
	undefined = false;
	type = (Val_type) *ptr++;
	switch (type)
		{
	case int_type:
		if (buflen < 5)
			return false;
		value.intval = Read4(ptr);
		return true;
	case pointer_type:
		if (buflen < 5)
			return false;
		Read4(ptr);
		value.ptr = 0; //DON'T dereference this pointer!
		// Maybe add a new type "serialized_pointer" to prevent "accidents"?
		return true;
	case string_type:
		{
		int len = Read2(ptr);
		if (buflen < len + 3)
			return false;
		value.str = new char[len + 1];
		std::memcpy(value.str, ptr, len);
		value.str[len] = 0;
		ptr += len;
		return true;
		}
	case array_type:
		{
		if (buflen < 3)
			return false;
		int len = Read2(ptr);
		int remaining = buflen - 3; // already read one byte
		*this = Usecode_value(len, 0); // create array
		for (int i=0; i < len; i++) {
			uint8* t = ptr;
			bool retval = value.array.elems[i].restore(
							ptr, remaining);
			remaining -= (ptr - t);
			if (!retval)
				return false;
		}
		return true;
		}
	default:
		return false;
		}
	}


ostream& operator<<(ostream& out, Usecode_value& val)
{
	val.print(out, true);
	return out;
}

/*
 *	Create/delete class objects.
 */
void Usecode_value::class_new(Usecode_class_symbol *cls, int nvars)
{
	assert(type == int_type);
	type = class_obj_type;
	value.array.cnt = nvars + 1;	// Stores class, class vars.
	value.array.elems = new Usecode_value[value.array.cnt];
	value.array.elems[0] = Usecode_value(cls);
}

void Usecode_value::class_delete()
{
	assert(type == class_obj_type);
	delete [] value.array.elems;
	*this = Usecode_value(0);
}
