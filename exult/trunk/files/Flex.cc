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


#if __GNUG__ >= 2
#  pragma implementation
#endif


#define	DEBUGFLEX 1

#include "Flex.h"

#include <cstdio>
#include <iostream>

Flex::Flex(const char *n) : U7file(n)
{
	IndexFlexFile();
}

Flex::Flex(const string &n) : U7file(n.c_str())
{
	IndexFlexFile();
}

void	Flex::IndexFlexFile(void)
{
	Flex	&ret=*this;
	FILE	*fp;
	const 	char *name=ret.filename.c_str();
	fp=fopen(name,"rb");
	if(!fp)
		{
		throw 0;
		return;
		}
	fread(ret.title,sizeof(ret.title),1,fp);
	fread(&ret.magic1,sizeof(uint32),1,fp);
	fread(&ret.count,sizeof(uint32),1,fp);
	fread(&ret.magic2,sizeof(uint32),1,fp);
	if(magic1!=0xffff1a00UL)
		{
		cerr << "Magic number is not a flex file" << endl;
		throw 0;	// Not a flex file
		}
	for(int i=0;i<9;i++)
		fread(&ret.padding[i],sizeof(uint32),1,fp);
#if DEBUGFLEX
	cout << "Title: " << ret.title << endl;
	cout << "Count: " << ret.count << endl;
#endif

	// We should already be there.
	fseek(fp,128,SEEK_SET);
	for(unsigned int i=0;i<ret.count;i++)
		{
		Flex::Reference f;
		fread(&f.offset,sizeof(uint32),1,fp);
		fread(&f.size,sizeof(uint32),1,fp);
#if DEBUGFLEX
		cout << "Item " << i << ": " << f.size << " bytes @ " << f.offset << endl;
#endif
		ret.object_list.push_back(f);
		}
	fclose(fp);
}

int     Flex::retrieve(int objnum,char **buf,size_t *len)
{
	*buf=0;
	*len=0;
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		return 0;
		}
	FILE	*fp=fopen(filename.c_str(),"rb");
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

int     Flex::retrieve(int objnum,const char *)
{ return 0; }


Flex::~Flex() {}

#if 0
char	*Flex::read_object(int objnum,uint32 &length)
{
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		return 0;
		}
	FILE	*fp=fopen(filename.c_str(),"rb");
	if(!fp)
		{
		cerr << "File open failed in read_object" << endl;
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
