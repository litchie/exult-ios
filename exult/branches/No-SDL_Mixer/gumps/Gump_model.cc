++++++++++GOING AWAY (jsf)

/*
 *  Gump_model.cc - A gump factory.
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
#include "Gump_model.h"
#include "Gump.h"
#include "misc_buttons.h"

using std::cout;
using std::endl;

/*
 *	Cleanup.
 */
Gump_model::~Gump_model()
	{
	for (Gump_model_elems::iterator it = elems.begin(); 
						it != elems.end(); ++it)
		delete *it;
	}

/*
 *	Create a gump for a given object.
 */

Gump *Gump_model::create
	(
	Game_object *obj,
	int initx, int inity		// Initial screen pos.
	)
	{
	Gump *gump = 0;
	int shnum = get_shapenum();
	ShapeFile shfile = get_shapefile();
	switch (tag)
		{
	case container:
		gump = new Container_gump((Container_game_object *)obj,
			initx, inity, shnum, shfile, object_area);
		break;
	default:
		cout << "Gump_model type not implemented: " << tag << endl;
		assert(0);
		break;
		}
	for (Gump_model_elems::iterator it = elems.begin(); 
						it != elems.end(); ++it)
		{
		Gump_model *wmodel = *it;
		wmodel->create_widget(gump);
		}
	return gump;
	}

/*
 *	Create a widget.
 */

Gump_widget *Gump_model::create_widget
	(
	Gump *gump			// Parent.
	)
	{
	Gump_widget *w = 0;
	switch (tag)
		{
	case checkmark:
		w = new Checkmark_button(gump, x, y);
		break;
	default:
		cout << "Gump_model widget type not implemented: " << tag << 
									endl;
		assert(0);
		break;
		}
	gump->add_elem(w);
	return w;
	}




