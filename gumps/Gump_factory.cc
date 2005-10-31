/*
 *  Gump_factory.cc - A gump factory.
 *
 *  Copyright (C) 2001-2005  The Exult Team
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
#include "Gump_factory.h"
#include "Gump.h"
#include "contain.h"

using std::cout;
using std::endl;

/*
 *	Create.
 */
Gump_factory::Gump_factory
	(
	)
	{
	}

/*
 *	Cleanup.
 */
Gump_factory::~Gump_factory()
	{
	for (Lookup_map::iterator it = table.begin(); 
						it != table.end(); ++it)
		delete (*it).second;
	}

/*
 *	Create a gump for a given object.
 */

Gump *Gump_factory::create
	(
	Game_object *obj,
	int initx, int inity		// Initial screen pos.
	)
	{
	Gump *model = table[obj->get_shapenum()];
	if (!model)
		return 0;
	return model->clone((Container_game_object *) obj, initx, inity);
	}





