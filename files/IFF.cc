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


#define	DEBUGFLEX 1

#include "IFF.h"

#include <cstdio>
#include <iostream>
#include "utils.h"

using std::string;
using std::vector;

using std::cout;
using std::cerr;
using std::endl;
using std::FILE;
using std::fread;
using std::memcmp;
using std::memset;
using std::size_t;


//Own implementation of ntohl.
//convert MSB -> LSB
long ntohl(long x) {
  static unsigned long _test_=0x01020304;
  if(*(char *)&_test_==0x01)
	{
	// We are big-endian
	return x;
	}
  else
	{
	  return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) |
	    ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24);
	}
}



IFF::IFF(const char *n) : U7file(std::string(n))
{
	IndexIFFFile();
}

IFF::IFF(const string &n) : U7file(n)
{
	IndexIFFFile();
}

void	IFF::IndexIFFFile(void)
{
	FILE	*fp;
	char	ckid[4];
	fp=U7open(filename.c_str(),"rb");
	if(!fp)
		{
		throw 0;
		return;
		}
	fread(ckid,4,1,fp);
	if(memcmp(ckid,"FORM",4))
		{
		// Not an IFF file we recognise
		throw 0;
		}
	cout << "Okay. It looks like an IFF file chunk" << endl;
	long	full_length;
	fread(&full_length,sizeof(full_length),1,fp);
	full_length=ntohl(full_length);
	cout << "length looks like: " << full_length << endl;
	fseek(fp,4,SEEK_CUR);	// We don't really need to know what the general data type is


#if 0
-the objects entries
  entry   = type, size, object, [even]
  type    = 4 chars representing the type of this object
  size    = reversed longint (size of the entry excluding the first 8 bytes)
  even    = 1 byte (set to 0) present only to get an even number of bytes
  (the objects found in U7 IFF files have the following format:)
  object  = name, data
  name    = 8 chars (filled with 0s)
  data    = the data of the object


#endif

	while(ftell(fp)<full_length)
		{
		Reference	r;
		size_t	len;
		char	type[5],name[9];
		memset(type,0,sizeof(type));
		memset(name,0,sizeof(name));
//		bool	even=false;

		fread(type,4,1,fp);	// 4 bytes of type
		if(type[0]<32)
			{
			// We've missed the target. Try to correct
			fseek(fp,-3,SEEK_CUR);
			continue;
			}
		fread(&len,4,1,fp);	// 4 bytes of length
		size_t	n=ftell(fp);	// If it's not an even fileposition, advance one byte
		if(n%2)
			fseek(fp,1,SEEK_CUR);	// Advance one byte
		fread(name,4,1,fp);	// data name is 4 bytes
		r.offset=ftell(fp);
		r.size=ntohl(len);
		if(r.size==0||r.offset==0)
			break;
		cout << "Object type: " << type << " (" << name <<") at position " << r.offset << " with length of " << r.size << endl;
		object_list.push_back(r);
		fseek(fp,r.offset+r.size-(2+object_list.size()),SEEK_SET);
		}

	fclose(fp);
}

int     IFF::retrieve(int objnum,char **buf,size_t *len)
{
	*buf=0;
	*len=0;
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		return 0;
		}
	FILE	*fp=U7open(filename.c_str(),"rb");
	if(!fp)
		{
		cerr << "File open failed in read_object" << endl;
		return 0;
		}
	fseek(fp,object_list[objnum].offset,SEEK_SET);
	size_t length=object_list[objnum].size;
	char	*ret=new char[length];
	fread(ret,length,1,fp);
	fclose(fp);
	*buf=ret;
	*len=length;
	return 1;
}

int     IFF::retrieve(int objnum,const char *)
{ return 0; }


IFF::~IFF() {}

