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



#include "Table.h"

#include <cstdio>
#include <iostream>

Table::Table(const char *n)
{
        filename=n;
        IndexTableFile();
}

Table::Table(const string &n)
{
        filename=n;
        IndexTableFile();
}


void	Table::IndexTableFile(void)
{
	Table	&ret=*this;
	FILE	*fp;
	fp=fopen(ret.filename.c_str(),"rb");
	if(!fp)
		{
		throw 0;
		return;
		}
	unsigned int i=0;
	while(1)
		{
		Table::Reference f;
		fread(&f.size,sizeof(uint16),1,fp);
		if(f.size==65535)
			break;
		fread(&f.offset,sizeof(uint32),1,fp);
#if 0
		cout << "Item " << i << ": " << f.size << " @ " << f.offset << endl;
#endif
		ret.object_list.push_back(f);
		i++;
		}
	fclose(fp);
	return;
}


int     Table::retrieve(int objnum,char **,size_t *len)
{ return 0; }

int     Table::retrieve(int objnum,const char *)
{ return 0; }


Table::~Table() {}

#if 0
char	*Table::read_object(int objnum,uint32 &length)
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
	// length=object_list[objnum].size;
	uint16	sz;
	fread(&sz,sizeof(sz),1,fp);
	length=sz-2;
	char	*ret=new char[length];
	fread(ret,length,1,fp);
	fclose(fp);
	return ret;
}
#endif
