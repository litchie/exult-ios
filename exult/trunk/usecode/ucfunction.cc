/*
 *	ucfunction.cc - Usecode function
 *
 *  Copyright (C) 2002  The Exult Team
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

#include <iostream>
using std::istream;

#include "ucfunction.h"
#include "utils.h"

/*
 *	Read in a function.
 */

Usecode_function::Usecode_function
	(
	istream& file
	)
{
	id = Read2(file);

	// support for our extended usecode format. (32 bit lengths)
	if (id == 0xFFFF) {
		id = Read2(file);
		len = Read4(file);
		extended = true;
	} else {
		len = Read2(file);
		extended = false;
	}

	code = new unsigned char[len];	// Allocate buffer & read it in.
	file.read((char*)code, len);
}

