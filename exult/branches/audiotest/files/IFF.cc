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

#include "IFF.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif
#include <iostream>
#include "exceptions.h"
#include "utils.h"

using std::string;
using std::vector;

using std::cout;
using std::cerr;
using std::endl;
using std::FILE;
using std::memcmp;
using std::memset;
using std::size_t;

IFF::IFF(const string &n) : U7file(n)
{
	IndexIFFFile();
}

void	IFF::IndexIFFFile(void)
{
	FILE	*fp;
	char	ckid[4];
	fp=U7open(filename.c_str(),"rb");

	std::fread(ckid,4,1,fp);
	if(memcmp(ckid,"FORM",4))
		throw wrong_file_type_exception(filename,"IFF");	// Not an IFF file we recognise

#ifdef DEBUG
	cout << "Okay. It looks like an IFF file chunk" << endl;
#endif
	long	full_length;
	full_length = Read4high(fp);
#ifdef DEBUG
	cout << "length looks like: " << full_length << endl;
#endif
	fseek(fp,4,SEEK_CUR);	// We don't really need to know what the general data type is


/*
-the objects entries
  entry   = type, size, object, [even]
  type    = 4 chars representing the type of this object
  size    = reversed longint (size of the entry excluding the first 8 bytes)
  even    = 1 byte (set to 0) present only to get an even number of bytes
  (the objects found in U7 IFF files have the following format:)
  object  = name, data
  name    = 8 chars (filled with 0s)
  data    = the data of the object
*/

	while(ftell(fp) < full_length)
	{
		Reference	r;
		char	type[5];
		memset(type,0,sizeof(type));

		std::fread(type,4,1,fp);	// 4 bytes of type
		if(type[0]<32)
		{
			// We've missed the target. Try to correct
			fseek(fp,-3,SEEK_CUR);
			continue;
		}
			
		r.size=Read4high(fp);	// 4 bytes for len
		r.offset=ftell(fp);
		
		if(r.size==0||r.offset==0)
			break;
		object_list.push_back(r);
		fseek(fp,r.offset+r.size,SEEK_SET);
	}

	fclose(fp);
}

char *	IFF::retrieve(uint32 objnum, size_t &len)
{
	FILE	*fp;
	char	*buffer;

	if (objnum >= object_list.size())
		throw exult_exception("objnum too large in IFF::retrieve()");
	
	fp = U7open(filename.c_str(), "rb");
	fseek(fp, object_list[objnum].offset, SEEK_SET);
	len = object_list[objnum].size;
	buffer = new char[len];
	std::fread(buffer, len, 1, fp);
	fclose(fp);

	return buffer;
}

