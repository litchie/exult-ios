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


#ifndef	_U7FILE_H_
#define	_U7FILE_H_

#include <map>
#include <string>

#include "common_types.h"



class	U7file
{
public:
	std::string	filename;
	U7file() {}
	U7file(const std::string &name) : filename(name) {}
	U7file(const U7file &f) : filename(f.filename) {}
	U7file &operator=(const U7file &u) { filename=u.filename; return *this; }
	virtual	~U7file() {}

	virtual	uint32	number_of_objects(void)=0;
	virtual	char *	retrieve(uint32 objnum,std::size_t &len)=0;
	virtual const char *get_archive_type()=0;
};

class	U7FileManager
{
protected:
	class	exclusive {};
#if 0
	struct ltstr
	{
	  bool operator()(const std::string &s1, const std::string &s2) const
	  {
	    return s1<s2;
	  }
	};
	std::map<const std::string,U7file *,ltstr> file_list;
#endif
	std::map<const std::string,U7file *> file_list;


	static	U7FileManager	*self;

public:
	U7FileManager();
	~U7FileManager();

	void reset();

	U7file	*get_file_object(const std::string &s);
	static U7FileManager *get_ptr(void);
};

class	U7object
{
protected:
	std::string	filename;
	int	objnumber;

public:
	U7object(const std::string &file,int objnum) : filename(file),objnumber(objnum) {}
	virtual	~U7object() {}

	uint32	number_of_objects(void);
	virtual	char *	retrieve(std::size_t &len);
	bool			retrieve(const char *fname);
					// FIX ME - this is only used in Game::play_audio and should be removed
};

#endif
