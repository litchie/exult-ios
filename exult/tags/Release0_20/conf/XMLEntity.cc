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



#include "XMLEntity.h"

XMLEntity::XMLEntity() : id(""),content("") {}
XMLEntity::~XMLEntity() {}


XMLnode::XMLnode() {}
XMLnode::~XMLnode() {}


string	&XMLnode::reference(string &h,bool &exists)
{
	static string dummy("");
	if(h.find('/')==string::npos)
		{
		// Must refer to me.
		if(entity.id==h)
			{
			exists=true;
			return entity.content;
			}
		else
			{
			exists=false;
			return dummy;
			}
		}
	// Otherwise we want to split the string at the first /
	// then locate the branch to walk, and pass the rest
	// down.

	string k;
	k=h.substr(h.find('/')+1);
	string k2=k.substr(0,k.find('/'));
	for(vector<XMLnode>::iterator it=nodelist.begin();
		it!=nodelist.end();++it)
		{
		if(it->entity.id==k2)
			return it->reference(k,exists);
		}

	return dummy;
}

