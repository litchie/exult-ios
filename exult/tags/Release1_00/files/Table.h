/*
Copyright (C) 2000  Dancer A.L Vesperman

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

#ifndef	_TABLE_H_
#define	_TABLE_H_

#include <vector>
#include <string>
#include "U7file.h"
#include "common_types.h"


class Table : public U7file
{
protected:
	struct Reference
		{
		uint32 offset;
		uint16 size;
		Reference() : offset(0),size(0) {};
		};
	std::vector<Reference> object_list;
public:
	Table(const std::string &name);
	Table(const Table &t) : object_list(t.object_list)
		{  }
	Table &operator=(const Table &t)
		{
		object_list=t.object_list;
		return *this;
		}

		virtual uint32	number_of_objects(void) { return object_list.size(); };
		virtual char *	retrieve(uint32 objnum,std::size_t &len); // To a memory block
		virtual const char *get_archive_type() { return "TABLE"; };
private:
	void IndexTableFile(void);
	Table();
};

#endif
