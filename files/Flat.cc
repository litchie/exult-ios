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


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#include "Flat.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif
#include <iostream>
#include "exceptions.h"
#include "utils.h"

using std::string;
// using std::vector;

using std::cout;
using std::cerr;
using std::endl;
using std::FILE;
using std::size_t;


Flat::Flat(const string &n) : U7file(n)
{
	// Make sure the file exists and is readable
	FILE *fp=U7open(filename.c_str(),"rb");
	fclose(fp);
}


char *	Flat::retrieve(uint32 objnum, size_t &len)
{ 
	FILE	*fp;
	char	*buffer;
	fp=U7open(filename.c_str(),"rb");

	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	buffer = new char[len];
	fseek(fp,0,SEEK_SET);
	fread(buffer,len,1,fp);
	fclose(fp);
	
	return buffer;
}
