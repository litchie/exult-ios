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

#ifndef	__Table_h_
#define	__Table_h_

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <vector>
#include <string>
#include "U7file.h"
#include "common_types.h"


class	Table : public	virtual	U7file
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
	Table(const char *name);
	Table(const std::string &name);
	Table(const Table &t) : object_list(t.object_list)
		{  }
	Table &operator=(const Table &t)
		{
		object_list=t.object_list;
		return *this;
		}
	virtual ~Table();

        virtual int     number_of_objects(const char *) { return object_list.size(); };
        virtual int     retrieve(int objnum,char **,std::size_t *len); // To a memory block
        virtual int     retrieve(int objnum,const char *);       // To a file
private:
	void IndexTableFile(void);
	Table();
	};

extern Table AccessTableFile(const char *);

#endif
