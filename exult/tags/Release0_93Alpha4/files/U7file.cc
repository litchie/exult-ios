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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif
#include <iostream>

#include "U7file.h"
#include "Flex.h"
#include "IFF.h"
#include "Table.h"
#include "Flat.h"
#include "exceptions.h"
#include "utils.h"

using std::cerr;
using std::endl;
using std::size_t;
using std::FILE;
using std::fclose;
using std::fwrite;
using std::string;


#define	TRY_FILE_TYPE(uf,CLASS_NAME)	\
	if(!uf) \
	try {	\
		uf=new CLASS_NAME(s);	\
	} catch(const wrong_file_type_exception &e)	\
		{	\
		;	\
		}

U7file  *U7FileManager::get_file_object(const string &s)
{
	U7file	*uf=0;
	if(file_list.count(s))
	{
		return file_list[s];
	}
	// Not in our cache. Attempt to figure it out.
	
	TRY_FILE_TYPE(uf,IFF);
	TRY_FILE_TYPE(uf,Flex);
	TRY_FILE_TYPE(uf,Table);
	TRY_FILE_TYPE(uf,Flat);

	// Failed
	if (!uf)
		throw file_open_exception(s);

	file_list[s]=uf;
	return uf;
}

U7FileManager	*U7FileManager::get_ptr(void)
{
	if(!self)
		new U7FileManager();	// self gets the pointer, so it's okay
					// This might look like it creates a
					// leak, but this is a singleton object
	return self;
}
	

U7FileManager::~U7FileManager() {}

U7FileManager   *U7FileManager::self=0;

U7FileManager::U7FileManager()
{
	if(self)
		throw exclusive();
	else
		self=this;
}


char*	U7object::retrieve(size_t &len)
{
	U7file *uf=U7FileManager::get_ptr()->get_file_object(filename);
	return uf->retrieve(objnumber,len);
}

void	U7object::retrieve(const char *fname)
{
	FILE	*fp=U7open(fname,"wb");

	char	*n;
	size_t	l;

	try
	{
		n = retrieve(l);
	}
	catch( const std::exception & err )
	{
		fclose(fp);
		throw err;
	}
	fwrite(n,l,1,fp);	// &&&& Should check return value
	fclose(fp);
	delete [] n;
}
