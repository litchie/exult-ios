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
#ifndef _Configuration_h_
#define _Configuration_h_

#if __GNUG__ >= 2
#  pragma interface
#endif



#include "XMLEntity.h"

class	Configuration
{
public:
	Configuration();
	Configuration(const char *);
	~Configuration();

	bool	read_config_file(const char *);
	bool	read_config_string(const string &);
	void	value(const char *key,string &ret,const char *defaultvalue="");
	void	value(const char *key,int &ret,int defaultvalue=0);
	void    set(const char *key,const char *value,bool write_to_file);
	void    set(const char *key,const string &value,bool write_to_file);
	void    set(const char *key,int,bool write_to_file);

	// Return a list of keys that are subsidiary to the supplied
	// key
	vector<string>	listkeys(string &key,bool longformat=true);
	vector<string>	listkeys(const char *key,bool longformat=true);

	string	dump(void); // Assembles a readable representation

	void	write_back(void);


private:
	string	&value(const char *key,bool &exists);
	void    set(string &key,string &value,bool write_to_file);
	XMLnode xmltree;
	string	filename;
	bool	is_file;
};



#endif
