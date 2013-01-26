/*
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *  Original file by Dancer A.L Vesperman
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

#ifndef __IFF_H_
#define __IFF_H_

#include <vector>
#include <string>
#include <cstring>
#include "common_types.h"
#include "U7file.h"
#include "exceptions.h"

class DataSource;

/**
 *  The IFF class is an data reader which reads data in the IFF
 *  file format. The actual data need not be in a file, however.
 */
class IFF : public U7file {
public:
	struct  IFFhdr {
		char    form_magic[4];
		uint32  size;
		char    data_type[4];
		IFFhdr()
		{  }
	};
	struct  IFFobject {
		char    type[4];
		uint32  size;
		char    even;
	};
	struct  u7IFFobj {
		char    name[8];
		// char    data[]; // Variable
	};
	struct Reference {
		size_t  offset;
		uint32  size;
		Reference() : offset(0), size(0) {};
	};

protected:
	//  The IFF's header. ++++ Unused????
	//IFFhdr    header;
	/// List of objects in the IFF file.
	std::vector<Reference> object_list;

	virtual void index_file();
public:
	/// Basic constructor.
	/// @param spec File name and object index pair.
	IFF(const File_spec &spec)
		: U7file(spec)
	{  }
	virtual ~IFF()
	{   }

	virtual size_t number_of_objects(void) {
		return object_list.size();
	};
	virtual char *retrieve(uint32 objnum, std::size_t &len);
	virtual const char *get_archive_type() {
		return "IFF";
	};

	static bool is_iff(DataSource *in);
	static bool is_iff(const char *fname);
private:
	/// No default constructor
	IFF();
	UNREPLICATABLE_CLASS_I(IFF, U7file(""));
};

typedef U7DataFile<IFF> IFFFile;
typedef U7DataBuffer<IFF> IFFBuffer;

#endif
