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

#include "Table.h"

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
using std::size_t;


Table::Table(const string &n) : U7file(n)
{
	IndexTableFile();
}


void	Table::IndexTableFile(void)
{
	Table	&ret=*this;
	FILE	*fp;
	try {
		fp=U7open(ret.filename.c_str(),"rb");
	} catch (const file_open_exception &e)
	{
		cerr << e.what() << ". exiting." << endl;
		exit(1);
	}
	fseek(fp,0,SEEK_END);
	size_t file_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	unsigned int i=0;
	while(1)
		{
		Table::Reference f;
		f.size = Read2(fp);
//		fread(&f.size,sizeof(uint16),1,fp);
		if(f.size==65535)
			break;
		f.offset = Read4(fp);
//		fread(&f.offset,sizeof(uint32),1,fp);
		if(f.size>file_size||f.offset>file_size)
			throw wrong_file_type_exception(filename,"Table");
#if 0
		cout << "Item " << i << ": " << f.size << " @ " << f.offset << endl;
#endif
		ret.object_list.push_back(f);
		i++;
		}
	fclose(fp);
	return;
}


char*	Table::retrieve(uint32 objnum,size_t &len)
{ throw exult_exception("Illegal call to Table::retrieve()"); }

#if 0
char	*Table::read_object(int objnum,uint32 &length)
{
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		return 0;
		}
	FILE *fp;
	try {
		fp=U7open(filename.c_str(),"rb");
	} catch (const file_open_exception &e)
	{
		cerr << e.what() << ". exiting." << endl;
		exit(1);
	}
	if(!fp)
		{
		cerr << "File open failed in read_object: " << filename << endl;
		return 0;
		}
	fseek(fp,object_list[objnum].offset,SEEK_SET);
	// length=object_list[objnum].size;
	uint16	sz;
	sz = Read2(fp);
//	fread(&sz,sizeof(sz),1,fp);
	length=sz-2;
	char	*ret=new char[length];
	fread(ret,length,1,fp);
	fclose(fp);
	return ret;
}
#endif
