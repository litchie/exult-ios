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

#ifndef _Configuration_h_
#define _Configuration_h_


#include "XMLEntity.h"

class	Configuration
{
public:
	Configuration()
	             : xmltree(new XMLnode("config")), rootname("config"), filename(), is_file(false)
		{ }
	Configuration(const std::string fname, const std::string root)
	             : xmltree(new XMLnode(root)), rootname(root), filename(), is_file(false)
		{ if(fname.size()) read_config_file(fname); }

	~Configuration() { if(xmltree!=0) delete xmltree; };
	
	bool	read_config_file(const std::string input_filename, const std::string root=std::string());
	
	bool	read_config_string(const std::string &);
	
	void	value(const std::string key, std::string &ret, const char *defaultvalue="") const;
	void	value(const std::string key, bool &ret, bool defaultvalue=false) const;
	void	value(const std::string key, int &ret, int defaultvalue=0) const;
	
	void	value(const char *key, std::string &ret, const char *defaultvalue="") const
			{ value(std::string(key), ret, defaultvalue); };
	void	value(const char *key, bool &ret, bool defaultvalue=false) const
			{ value(std::string(key), ret, defaultvalue); };
	void	value(const char *key, int &ret, int defaultvalue=0) const
			{ value(std::string(key), ret, defaultvalue); };
	
	void    set(const char *key,const char *value,bool write_to_file);
	void    set(const char *key,const std::string &value,bool write_to_file);
	void    set(const char *key,int,bool write_to_file);

	// Return a list of keys that are subsidiary to the supplied
	// key
	std::vector<std::string>	listkeys(const std::string &key,bool longformat=true);
	std::vector<std::string>	listkeys(const char *key,bool longformat=true);

	std::string	dump(void); // Assembles a readable representation
	std::ostream &dump(std::ostream &o, std::string indentstr);

	void	write_back(void);

	void clear(std::string new_root=std::string());
	
	typedef XMLnode::KeyType     KeyType;
	typedef XMLnode::KeyTypeList KeyTypeList;
	
	void getsubkeys(KeyTypeList &ktl, const std::string basekey);
	
private:
	void    set(std::string &key,std::string &value,bool write_to_file);
	XMLnode *xmltree;
	std::string	rootname;
	std::string	filename;
	bool	is_file;
};

// Global Config
extern Configuration *config;

#endif
