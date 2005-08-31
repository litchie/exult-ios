/*
 *  Gump_model.h - A gump factory.
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

#ifndef GUMP_MODEL_INCLUDED
#define GUMP_MODEL_INCLUDED

#include <vector>
#include "exceptions.h"
#include "rect.h"
#include "shapeid.h"

class Gump;
class Game_object;
class Gump_widget;

/*
 *	A 'model' represents a top-level gump OR a widget within.
 */
class  Gump_model : public ShapeID
{
	UNREPLICATABLE_CLASS(Gump_model);
public:
	typedef enum {
		// Top-level gumps.
		audio_options = 0,
		book,
		combat_options,
		gameplay_options,
		container,
		jawbone,
		notebook,
		scroll,
		sign,
		slider,
		spellbook,
		stats,
		video_options,
		yesno,
		// Widgets.
		checkmark = 100,
		heart,
		disk,
		combat,
		halo,
		combat_mode,
		combat_stats
	} Gump_model_tag;
private:
	Gump_model_tag tag;
	int x, y;			// Location within parent.
	Rectangle object_area;		// Area within this for objects.
					// Sub-elems (like buttons).
	typedef std::vector<Gump_model *> Gump_model_elems;
	Gump_model_elems elems;
public:
	Gump_model(Gump_model_tag t, int shnum, int frnum, ShapeFile shfile,
				Rectangle area, int px = 0, int py = 0)
		: ShapeID(shnum, frnum, shfile), tag(t), object_area(area), 
		  x(px), y(py)
		{  }
	~Gump_model();
	Gump *create(Game_object *obj, int initx, int inity);
	Gump_widget *create_widget(Gump *gump);
};

#endif // GUMP_MODEL_INCLUDED
