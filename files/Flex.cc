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


#define	DEBUGFLEX 0

#include "Flex.h"

#include <cstdio>
#include <iostream>
#include "exceptions.h"
#include "utils.h"

using std::string;

using std::cerr;
using std::endl;
using std::FILE;
using std::fread;
using std::size_t;


Flex::Flex(const string &n) : U7file(n)
{
	IndexFlexFile();
}

void	Flex::IndexFlexFile(void)
{
	FILE	*fp;
	fp=U7open(filename.c_str(),"rb");
	if(!fp)
		throw file_not_found_error(filename);
	fread(title,sizeof(title),1,fp);
	magic1 = Read4(fp);
	count = Read4(fp);
	magic2 = Read4(fp);
	if(magic1!=0xffff1a00UL)
		{
		cerr << "Magic number is not a flex file" << endl;
		throw wrong_file_type_error();	// Not a flex file
		}
	for(int i=0;i<9;i++)
		padding[i] = Read4(fp);
#if DEBUGFLEX
	cout << "Title: " << title << endl;
	cout << "Count: " << count << endl;
#endif

	// We should already be there.
	fseek(fp,128,SEEK_SET);
	for(unsigned int i=0;i<count;i++)
		{
		Flex::Reference f;
		f.offset = Read4(fp);
		f.size = Read4(fp);
#if DEBUGFLEX
		cout << "Item " << i << ": " << f.size << " bytes @ " << f.offset << endl;
#endif
		object_list.push_back(f);
		}
	fclose(fp);
}

void     Flex::retrieve(int objnum,char **buf,size_t *len)
{
	*buf=0;
	*len=0;
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		throw exult_exception("objnum too large in read_object()");
		}
	FILE	*fp=U7open(filename.c_str(),"rb");
	if(!fp)
		{
		cerr << "File open failed in read_object: " << filename << endl;
		throw exult_exception("File open failed in read_object: "+filename);
		}
	fseek(fp,object_list[objnum].offset,SEEK_SET);
	size_t length=object_list[objnum].size;
	char	*ret=new char[length];
	fread(ret,length,1,fp);
	fclose(fp);
	*buf=ret;
	*len=length;
}

#if 0
char	*Flex::read_object(int objnum,uint32 &length)
{
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		return 0;
		}
	FILE	*fp=U7open(filename.c_str(),"rb");
	if(!fp)
		{
		cerr << "File open failed in read_object: " << filename << endl;
		return 0;
		}
	fseek(fp,object_list[objnum].offset,SEEK_SET);
	length=object_list[objnum].size;
	char	*ret=new char[length];
	fread(ret,length,1,fp);
	fclose(fp);
	return ret;
}
#endif
