/*
 *  Copyright (C) 2008  The Exult Team
 *
 *	Based on code by Dancer A.L Vesperman
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

#ifndef _U7FILEMAN_H_
#define _U7FILEMAN_H_

#include <map>
#include <string>
#include "U7obj.h"
#include "exceptions.h"

class U7file;

/**
 *	This class manages "files". These "files" are managed by an
 *	unique identifier which is the actual file name for real files.
 */
class U7FileManager
	{
	UNREPLICATABLE_CLASS(U7FileManager);
protected:
	/// The actual "file" list.
	std::map<File_spec, U7file *> file_list;
	/// Static pointer to self.
	static U7FileManager *self;
public:
	U7FileManager()
		{  }
	~U7FileManager()
		{ reset(); }

	void reset();

	U7file *get_file_object(const File_spec &s, bool allow_errors = false);
	static U7FileManager *get_ptr()
		{ return self; }
	};

#endif
