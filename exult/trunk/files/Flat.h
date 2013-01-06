/*
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *	Original file by Dancer A.L Vesperman
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

#ifndef	_FLAT_H_
#define	_FLAT_H_

#include <string>
#include "U7file.h"
#include "common_types.h"
#include "exceptions.h"

class DataSource;

/**
 *	This is a "flat" file, which contains but a single object.
 *	Strictly speaking, the Flat class, as its base, has an
 *	undefined data origin, and may well be a buffer in memory.
 */
class Flat : public	U7file
	{
private:
	///	No default constructor.
	Flat();
	UNREPLICATABLE_CLASS_I(Flat, U7file(""));
public:
	///	Basic constructor.
	///	@param spec	File name and object index pair.
	Flat(const File_spec &spec)
		: U7file(spec)
		{  }

	virtual size_t number_of_objects(void)
		{ return 1; };
	virtual char *retrieve(uint32 objnum, std::size_t &len);
	virtual const char *get_archive_type()
		{ return "FLAT"; };

	static bool is_flat(DataSource *in);
	static bool is_flat(const char *fname);
	};

typedef U7DataFile<Flat> FlatFile;
typedef U7DataBuffer<Flat> FlatBuffer;

#endif
