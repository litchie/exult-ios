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


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif



#include "Flat.h"

#include <cstdio>
#include <iostream>
#include "utils.h"

using std::string;
using std::vector;

using std::cout;
using std::cerr;
using std::endl;
using std::FILE;
using std::size_t;


Flat::Flat(const string &n) : U7file(n)
{
	// Making it safe
	FILE *fp=U7open(filename.c_str(),"rb");
	if(!fp)
		throw 0;

	fclose (fp);
}


int     Flat::retrieve(int objnum,char **c,size_t *len)
{ 
	FILE	*fp;
	fp=U7open(filename.c_str(),"rb");
	if(!fp)
		{
		throw 0;
		return 0;
		}
	fseek(fp,0,SEEK_END);
	*len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	char * buf = new char[*len];
	fread(buf,*len,1,fp);
	*c = buf;
	fclose(fp);
	return 1;
}

int     Flat::retrieve(int objnum,const char *)
{ return 0; }


Flat::~Flat() {}
