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

#include <cstdio>
#include "XMLEntity.h"
#include <string>

extern	void	xmlparse(string &,size_t &,XMLnode *);
extern	void	xmldump(string &,XMLnode *);

int	main(int argc,char **argv)
{
	char	buf[4096];
	string	sbuf;

	while(fgets(buf,sizeof(buf),stdin))
		{
		sbuf+=buf;
		}

	XMLnode	xmltree;
	size_t	n=1;
	xmlparse(sbuf,n,&xmltree);

	sbuf="";
	xmldump(sbuf,&xmltree);
	cout << sbuf;

	string ss2="config/audio/midi_device";
	string	sss=xmltree.reference(ss2);
	cout << "Returned from reference. Got '" << sss << "'" << endl;

	return 0;
}
