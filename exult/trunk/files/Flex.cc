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
	fread(title,sizeof(title),1,fp);
	magic1 = Read4(fp);
	count = Read4(fp);
	magic2 = Read4(fp);
	if(magic1!=0xffff1a00UL)
		throw wrong_file_type_exception(filename,"FLEX");	// Not a flex file

	for(int i=0;i<9;i++)
		padding[i] = Read4(fp);
#if DEBUGFLEX
	cout << "Title: " << title << endl;
	cout << "Count: " << count << endl;
#endif

	// We should already be there.
	fseek(fp,128,SEEK_SET);
	for(uint32 i=0;i<count;i++)
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

char *	Flex::retrieve(uint32 objnum, size_t &len)
{
	FILE	*fp;
	char	*buffer;

	if (objnum >= object_list.size())
		throw exult_exception("objnum too large in retrieve()");

	fp = U7open(filename.c_str(), "rb");
	fseek(fp, object_list[objnum].offset, SEEK_SET);
	len = object_list[objnum].size;
	buffer = new char[len];
	fread(buffer, len, 1, fp);
	fclose(fp);
	
	return buffer;
}
