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

#ifndef	_TABLE_H_
#define	_TABLE_H_

#include <vector>
#include <string>
#include "U7file.h"
#include "common_types.h"
#include "exceptions.h"

class DataSource;

/**
 *	The Table class is an data reader which reads data in the table
 *	file format. The actual data need not be in a file, however.
 */
class Table : public U7file
	{
protected:
	struct Reference
		{
		uint32 offset;
		uint16 size;
		Reference()
			: offset(0),size(0)
			{  };
		};
	/// List of objects in the table file.
	std::vector<Reference> object_list;

	virtual void index_file();
public:
	///	Basic constructor.
	///	@param spec	File name and object index pair.
	Table(const File_spec &spec)
		: U7file(spec)
		{  }
	virtual ~Table()
		{	}

		virtual size_t number_of_objects(void)
			{ return object_list.size(); };
		virtual char *retrieve(uint32 objnum,std::size_t &len);
		virtual const char *get_archive_type()
			{ return "TABLE"; };

	static bool is_table(DataSource *in);
	static bool is_table(const char *fname);
private:
	/// No default constructor.
	Table();
	UNREPLICATABLE_CLASS_I(Table, U7file(""));
	};

typedef U7DataFile<Table> TableFile;
typedef U7DataBuffer<Table> TableBuffer;

#endif
