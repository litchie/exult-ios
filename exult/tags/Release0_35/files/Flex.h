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

#ifndef	__Flex_h_
#define	__Flex_h_

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <vector>
#include <string>
#include "common_types.h"
#include "U7file.h"


class	Flex	:	public virtual U7file
	{
protected:
	char	title[80];
	uint32	magic1;
	uint32	count;
	uint32	magic2;
	uint32	padding[9];
	struct Reference
		{
		uint32 offset;
		uint32 size;
		Reference() : offset(0),size(0) {};
		};
	std::vector<Reference> object_list;
public:
	Flex(const char *fname);
	Flex(const std::string &fname);
	Flex(const Flex &f) : magic1(f.magic1),count(f.count),magic2(f.magic2),object_list(f.object_list)
		{ std::memcpy(title,f.title,sizeof(title));
		  std::memcpy(padding,f.padding,sizeof(padding)); }
	Flex &operator=(const Flex &f)
		{
		magic1=f.magic1;
		count=f.count;
		magic2=f.magic2;
		object_list=f.object_list;
		std::memcpy(title,f.title,sizeof(title));
		std::memcpy(padding,f.padding,sizeof(padding));
		return *this;
		}
		
	virtual ~Flex();
	
	// char *read_object(int objnum,uint32 &length);
        virtual int     number_of_objects(const char *) { return object_list.size(); };
        virtual int     retrieve(int objnum,char **,std::size_t *len); // To a memory block
        virtual int     retrieve(int objnum,const char *);       // To a file
private:
	Flex();	// No default constructor
	void IndexFlexFile(void);
	};


#endif
