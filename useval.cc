/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Useval.cc - Usecode-interpreter values.
 **
 **	Written: 8/12/99 - JSF
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

#include <fstream>
#include <string.h>
#include <iomanip>
#include <iostream>
#include "useval.h"


/*
 *	Get array size.
 */

int Usecode_value::count_array
	(
	const Usecode_value& val
	)
	{
	int i;
	for (i = 0; val.value.array[i].type != (int) end_of_array_type; i++)
		;
	return (i);
	}

/*
 *	Destructor
 *
 */

Usecode_value::~Usecode_value()
{
	if (type == (unsigned char) array_type)
		delete [] value.array;
	else if (type == (unsigned char) string_type)
		delete value.str;
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
	if (type == (int) array_type)
		delete [] value.array;
	else if (type == (int) string_type)
		delete value.str;
	type = v2.type;			// Assign new values.
	if (type == (int) int_type)
		value.intval = v2.value.intval;
	else if (type == (int) string_type)
		value.str = v2.value.str ? strdup(v2.value.str) : 0;
	else if (type == (int) array_type)
		{
                int tempsize = 1+count_array(v2);
		value.array = new Usecode_value[tempsize];
		int i = 0;
		do
			value.array[i] = v2.value.array[i];
		while (value.array[i++].type != 
					(int) end_of_array_type);
		}
	return *this;
	}

/*
 *	Create a string value.
 */

Usecode_value::Usecode_value
	(
	const char *s
	) : type((unsigned char) string_type)
	{
	value.str = s ? strdup(s) : 0; 
	}

/*
 *	Resize array (or turn single element into an array).
 *
 *	Output:	Always true.
 */

int Usecode_value::resize
	(
	int new_size
	)
	{
	if (type != (int) array_type)	// Turn it into an array.
		{
		Usecode_value elem(*this);
		*this = Usecode_value(new_size, &elem);
		return (1);
		}
	int size = count_array(*this);	// Get current size.
	if (new_size == size)
		return (1);		// Nothing to do.
	Usecode_value *newvals = new Usecode_value[new_size + 1];
	newvals[new_size].type = (unsigned char) end_of_array_type;
					// Move old values over.
	int cnt = new_size < size ? new_size : size;
	for (int i = 0; i < cnt; i++)
		newvals[i] = value.array[i];
	delete [] value.array;		// Delete old list.
	value.array = newvals;		// Store new.
	return (1);
	}

void	Usecode_value::push_back(int i)
{
	resize(count_array(*this)+1);
	value.array[count_array(*this)-1]=Usecode_value(i);
}

/*
 *	Comparator.
 *
 *	Output:	1 if they match, else 0.
 */

int Usecode_value::operator==
	(
	const Usecode_value& v2
	)
	{
	if (&v2 == this)
		return (1);		// Same object.
	if (v2.type != type)
		return (0);		// Wrong type.
	if (type == (int) int_type)
		return (value.intval == v2.value.intval);
	else if (type == (int) string_type)
		return (strcmp(value.str, v2.value.str) == 0);
	else
		return (0);
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
	for (i = 0; value.array[i].type != (int) end_of_array_type; i++)
		if (value.array[i] == val)
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
	ostream& out
	)
	{
	switch ((Val_type) type)
		{
	case int_type:
		cout << hex << setfill((char)0x30) << setw(4);
		out << (value.intval&0xffff);
		cout << dec;
		break;
	case string_type:
		out << '"' << value.str << '"'; break;
	case array_type:
		{
		out << "[ ";
		for (int i = 0; value.array[i].type != (int) end_of_array_type;
									i++)
			{
			if (i)
				out << ", ";
			value.array[i].print(out);
			}
		out << " ]";
		}
		break;
	default:
		break;
		}
	}

