/*
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef XMLENTITY_H
#define XMLENTITY_H

#include <string>
#include <vector>


class	XMLnode
{
protected:
	std::string				id;
	std::string				content;
	std::vector<XMLnode*>	nodelist;

public:
	XMLnode()
		{  }
	XMLnode(const std::string &i) : id(i)
		{  }
	XMLnode(const XMLnode &n) : id(n.id),content(n.content),nodelist(n.nodelist)
		{  }
	~XMLnode();
	
	XMLnode &operator=(const XMLnode &n) { id=n.id; content=n.content; nodelist=n.nodelist; return *this; }
	const std::string &reference(const std::string &,bool &);
	const XMLnode *subtree(const std::string &) const;
	
	const std::string &value(void) const
		{ return content; }
	
	std::string dump(int depth = 0);
	void	xmlassign(std::string &key,std::string &value);
	void	xmlparse(std::string &s,std::size_t &pos);

	void	listkeys(const std::string &,std::vector<std::string> &, bool longformat=true) const;
};

#endif
