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


//-*-Mode: C++;-*-
#ifndef _XMLEntity_h_
#define _XMLEntity_h_


#if __GNUG__ >= 2
#  pragma interface
#endif



#include <string>
#include <vector>


class	XMLEntity
	{
public:
	std::string	id;
	std::string	content;
	XMLEntity &operator =(const XMLEntity &x) { id=x.id; content=x.content; return *this; }
	XMLEntity(const XMLEntity &e) : id(e.id),content(e.content)
		{  }
	XMLEntity();
	~XMLEntity();
	};


class	XMLnode
	{
public:
	XMLEntity	entity;
	std::vector<XMLnode>	nodelist;
	std::string	&reference(std::string &,bool &);
	XMLnode *subtree(std::string &);
	XMLnode &operator=(const XMLnode &n) { entity=n.entity; nodelist=n.nodelist; return *this; }
	XMLnode(const XMLnode &n) : entity(n.entity),nodelist(n.nodelist)
		{  }
	XMLnode();
	~XMLnode();
	};

#endif
