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

#ifndef __IFF_h_
#define __IFF_h_

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <vector>
#include <string>
#include "common.h"



struct	IFF
	{
	struct  IFFhdr
		{
		char    form_magic[4];
		uint32  size;
		char    data_type[4];
		};
	struct  IFFobject
		{
		char    type[4];
		uint32  size;
		char    even;
		};
	struct  u7IFFobj
		{
		char    name[8];
		// char    data[]; // Variable
		};
	string	filename;
	IFFhdr	header;
	vector<IFFobject> index;
	};

extern IFF AccessIFFFile(const char *);


#endif
