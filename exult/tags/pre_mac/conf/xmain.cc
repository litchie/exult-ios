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
#include "Configuration.h"
#include <string>

Configuration	config;

void	dump_stringvec(vector<string> &vs)
{
	size_t	n;
	cout << "vs is " << vs.size() << " entries" << endl;
	for(n=0;n<vs.size();n++)
		cout << n << " : " << vs[n] << endl;



}

int	main(int argc,char **argv)
{
	config.read_config_file("config.xml");

	int	n;
	config.value("config/audio/midi/device",n,-1);
	cout << "Returned from reference. Got '" << n << "'" << endl;
	string	r;
	config.value("config/audio/midi/enabled",r,"--nil--");
	cout << "Returned from reference. Got '" << r << "'" << endl;

	config.set("config/something/something/else","wibble",false);

	string	out=config.dump();
	cout << out << endl;

	vector<string> vs;

	vs=config.listkeys("config");
	dump_stringvec(vs);

	vs=config.listkeys("config/audio");
	dump_stringvec(vs);

	vs=config.listkeys("config/something",false);
	dump_stringvec(vs);

	vs=config.listkeys("config/somenonexistantthing");
	dump_stringvec(vs);

	return 0;
}
