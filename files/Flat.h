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

#ifndef	FLAT_H
#define	FLAT_H

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <vector>
#include <string>
#include "U7file.h"
#include "common_types.h"


class	Flat : public	virtual	U7file
	{
public:
	Flat(const std::string &name);
	Flat(const Flat &t) {  }
	Flat &operator=(const Flat &t)
		{
			return *this;
		}

        virtual int     number_of_objects(const char *) { return 1; };
        virtual void     retrieve(int objnum,char **,std::size_t *len); // To a memory block
	virtual const char *get_archive_type() { return "FLAT"; };
	Flat();
	};
#endif
