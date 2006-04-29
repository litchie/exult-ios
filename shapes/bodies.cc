/*
 *  bodies.cc - Associate bodies with 'live' shapes.
 *
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "bodies.h"
#include "game.h"
#include "utils.h"
#include "msgfile.h"
#include "U7file.h"
#include "databuf.h"
#include <fstream>
#include <map>
#include <vector>
#include <iostream>

using std::vector;
using std::ifstream;
using std::map;
using std::pair;
using std::size_t;

static std::map<int, long> *bodies_table = 0;
static vector<int> body_shape_table;

/*
 *	Setup table.
 */
void Body_lookup::setup
	(
	)
	{
	int i = -1, j = -1;
	ifstream in;
	vector<char *> bodies_strings;
	vector<char *> shapes_strings;
	try {
		ifstream in;
		U7open(in, "<PATCH>/bodies.txt", true);
		i = Read_text_msg_file(in, shapes_strings, "bodyshapes");
		j = Read_text_msg_file(in, bodies_strings, "bodylist");
		in.close();
	} catch (std::exception &) {
		if (GAME_BG || GAME_SI) {

			str_int_pair resource = game->get_resource("config/bodies");

			U7object txtobj(resource.str, resource.num);
			size_t len;
			char *txt = txtobj.retrieve(len);
			BufferDataSource ds(txt, len);
			i = Read_text_msg_file(&ds, shapes_strings, "bodyshapes");
			j = Read_text_msg_file(&ds, bodies_strings, "bodylist");
		} else {
			ifstream in;
			U7open(in, "<STATIC>/bodies.txt", true);
			i = Read_text_msg_file(in, shapes_strings, "bodyshapes");
			j = Read_text_msg_file(in, bodies_strings, "bodylist");
			in.close();
		}
	}
	bodies_table = new std::map<int, long>;
	int cnt = shapes_strings.size();
	if (i >= 0)
		{
		body_shape_table.resize(cnt);
		for ( ; i < cnt; ++i)
			{
			char *ptr = shapes_strings[i], *eptr;
			if (!ptr)
				continue;
			body_shape_table[i] = strtol(ptr, &eptr, 0);
			}
		}
	for (i = 0; i < cnt; ++i)
		delete[] shapes_strings[i];
	
	cnt = bodies_strings.size();
	if (j >= 0)
		{
		for ( ; j < cnt; ++j)
			{
			char *ptr = bodies_strings[j], *eptr;
			if (!ptr)
				continue;
			int bshape = strtol(ptr, &eptr, 0);
			int bframe = strtol(eptr + 1, 0, 0);
			(*bodies_table)[j] = (((long)bshape)<<16)|bframe;
			}
		}
	for (j = 0; j < cnt; ++j)
		delete[] bodies_strings[j];
	}

/*
 *	Lookup a shape's body.
 *
 *	Output:	0 if not found.
 */

int Body_lookup::find
	(
	int liveshape,			// Live actor's shape.
	int& deadshape,			// Dead shape returned.
	int& deadframe			// Dead frame returned.
	)
	{
	if (!bodies_table)		// First time?
		setup();
	std::map<int, long>::iterator it = bodies_table->find(liveshape);
	if (it != bodies_table->end())
		{
		deadshape = ((*it).second>>16)&0xffff;
		deadframe = (*it).second&0xffff;
		return 1;
		}
	else
		return 0;
	}


/*
 *	Recognize dead body shapes.
 */

int Body_lookup::Is_body_shape
	(
	int shapeid
	)
	{
	if (body_shape_table.empty())		// First time?
		setup();
	for (vector<int>::iterator it=body_shape_table.begin();
			it!=body_shape_table.end(); ++it)
		if ((*it) == shapeid)
			return 1;

	return 0;

	}
