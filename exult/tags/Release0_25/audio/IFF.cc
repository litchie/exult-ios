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



#include "IFF.h"

#include <cstdio>
#include <iostream>
#include <netinet/in.h>

IFF	AccessFlexFile(const char *name)
{
	IFF	ret;
	FILE	*fp;
	ret.filename=name;
	fp=fopen(name,"rb");
	if(!fp)
		{
		return ret;
		}
	fread(&ret.header,sizeof(ret.header),1,fp);
	ret.header.size=ntohl(ret.header.size);
#if 0

	for(unsigned int i=0;i<ret.count;i++)
		{
		IFF::Reference f;
		fread(&f.offset,sizeof(uint32),1,fp);
		fread(&f.size,sizeof(uint32),1,fp);
#if DEBUG
		cout << "Item " << i << ": " << f.size << " bytes @ " << f.offset << endl;
#endif
		ret.object_list.push_back(f);
		}
#endif
	fclose(fp);
	return ret;
}

#if 0
char	*IFF::read_object(int objnum,uint32 &length)
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
