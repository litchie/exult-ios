/*
 *  Gump_factory.h - A gump factory.
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

#ifndef GUMP_FACTORY_INCLUDED
#define GUMP_FACTORY_INCLUDED

#include <map>

class Gump;
class Game_object;

/*
 *	Create gumps.
 */
class  Gump_factory
{
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
	} Gump_factory_tag;
private:
	typedef std::map<int,Gump*> Lookup_map;
	Lookup_map table;		// Lookup by shape.
public:
	Gump_factory();
	~Gump_factory();
	Gump *create(Game_object *obj, int initx, int inity);
};

#endif
