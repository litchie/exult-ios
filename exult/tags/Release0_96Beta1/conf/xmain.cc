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


#if __GNUG__ >= 2
#  pragma implementation
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif
#include "Configuration.h"
#include <iostream>
#include <string>
#include "exult_constants.h"

const std::string c_empty_string;

Configuration	config;
using std::cout;
using std::cerr;
using std::endl;

void	dump_stringvec(std::vector<std::string> &vs,int expect=-2)
{
	size_t	n;
	cout << "vs is " << vs.size() << " entries" << endl;
	for(n=0;n<vs.size();n++)
		cout << n << " : " << vs[n] << endl;
	if(expect==-2)
		return;
	assert(vs.size()==expect);
}

void	test1(void)
{
	config.read_config_file("./config.xml");

	int	n;
	config.value("config/audio/midi/device",n,-1);
	cout << "Returned from reference. Got '" << n << "'" << endl;
	assert(n==5);
	std::string	r;
	config.value("config/audio/midi/enabled",r,"--nil--");
	cout << "Returned from reference. Got '" << r << "'" << endl;
	assert(r=="yes");

	config.set("config/something/something/else","wibble",false);

	std::string	out=config.dump();
	cout << out << endl;

	std::vector<std::string> vs;

	vs=config.listkeys("config");
	dump_stringvec(vs,4);

	vs=config.listkeys("config/audio");
	dump_stringvec(vs,3);

	vs=config.listkeys("config/something",false);
	dump_stringvec(vs,1);

	vs=config.listkeys("config/somenonexistantthing");
	dump_stringvec(vs,0);

}

int	main(void)
{
	test1();
	return 0;
}
