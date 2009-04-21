/*
 *	U7obj.h - Generic file reader object.
 *
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

#ifndef _U7OBJ_H_
#define _U7OBJ_H_

#include <string>
#include <vector>
#include <cstring>
#include "common_types.h"
#include "utils.h"

/**
 *	This structure condenses a file name and an object number in
 *	one place. It is meant to unify opening files and memory buffers.
 */
struct File_spec
	{
	/// This is a file name, always.
	const char *name;
	///	If -1, this indicates that we are reading from the file itself.
	///	If >= 0, this means we want to read from the object with the
	///	given index inside the file.
	int index;
	bool ownstr;
	File_spec()
		: name(""), index(-1), ownstr(false)
		{  }
	///	Constructs a File_spec from a c-string.
	///	Note that it performs implicit conversion from the c-string.
	File_spec(const char *n)
		: name(n), index(-1), ownstr(false)
		{  }
	File_spec(const char *n, int i)
		: name(n), index(i), ownstr(false)
		{  }
	///	Constructs a File_spec from a string.
	///	Note that it does NOT perform implicit conversions from std::string,
	///	and that it owns a copy of the string.
	explicit File_spec(std::string n)
		: index(-1), ownstr(true)
		{ name = newstrdup(n.c_str()); }
	explicit File_spec(std::string n, int i)
		: index(i), ownstr(true)
		{ name = newstrdup(n.c_str()); }
	File_spec(const File_spec& other)
		: index(other.index), ownstr(other.ownstr)
		{
		if (ownstr)
			name = newstrdup(other.name);
		else
			name = other.name;
		}
	~File_spec()
		{ if (ownstr) delete [] name; }
	const File_spec& operator=(const File_spec& other)
		{
		if (this != &other)
			{
			index = other.index;
			ownstr = other.ownstr;
			if (ownstr)
				name = newstrdup(other.name);
			else
				name = other.name;
			}
		return *this;
		}
	bool operator<(const File_spec& other) const
		{
		//int cmp = name.compare(other.name);
		int cmp = std::strcmp(name, other.name);
		return cmp < 0 || (cmp == 0 && index < other.index);
		}
	};

/**
 *	This class loads a file of undefined type and retrieves
 *	a given object contained in said file.
 */
class U7object
	{
	friend class U7multiobject;
protected:
	/// Name of desired file.
	File_spec identifier;
	/// Number of object to retrieve.
	int objnumber;
public:
	///	Setups to load an object from a buffer or file.
	///	@param spec	Specification of the data source.
	///	@param objnum	Index of object in the data object.
	///	@param p	Optional, defaults to false. If true, indicates
	///	the object comes from the patch dir.
	U7object(const File_spec& spec, int objnum)
		: identifier(spec), objnumber(objnum)
		{  }
	///	Copy constructor.
	///	@param other	What we are copying.
	U7object(const U7object& other)
		: identifier(other.identifier), objnumber(other.objnumber)
		{  }
	virtual	~U7object()
		{  }

	size_t number_of_objects();
	virtual	char *retrieve(std::size_t &len) const;
	};

/**
 *	This class loads an object from several data sources, going
 *	from last to first source when determining the correct object.
 *	The intended use is to reduce code duplication for something
 *	which should be patchable but is not.
 *	To make it clear: it is a multi-source, EXCLUSIVE object loader;
 *	it does NOT "merge" the objects from the different sources,
 *	it picks a single source from which the object is loaded.
 *	An 'object' it loads may be an object in a file or an object
 *	in a buffer.
 */
class U7multiobject : public U7object
	{
protected:
	char *buffer;
	size_t length;
	void set_object(const std::vector<U7object>& objects);
public:
	U7multiobject(const File_spec& file0, int objnum);
	U7multiobject(const File_spec& file0, const File_spec& file1, int objnum);
	U7multiobject(const File_spec& file0, const File_spec& file1,
			const File_spec& file2, int objnum);
	U7multiobject(const File_spec& file0, const File_spec& file1,
			const File_spec& file2, const File_spec& file3, int objnum);
	U7multiobject(const std::vector<File_spec>& files, int objnum);
	virtual	~U7multiobject()
		{ if (buffer) delete [] buffer; }

	virtual	char *retrieve(std::size_t &len) const;
	};

#endif
