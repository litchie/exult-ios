/*
Copyright (C) 2000-2001 The Exult Team

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

// Header for gump manager

#ifndef GUMP_INCLUDED
#define GUMP_INCLUDED

class Gump;
class Game_object;
class Game_window;

class  Gump_manager
{
	struct Gump_list
	{
		Gump		*gump;
		Gump_list	*next;

		Gump_list() : gump(0), next(0) { }
		Gump_list(Gump *g) : gump(g), next(0) { }
	};

	Gump_list	*open_gumps;
public:
	void add_gump(Gump *gump);			// Add a single gump to screen
	void add_gump(Game_object *obj, int shapenum);	// Show a gump for object obj

	bool remove_gump(Gump *gump);			// Detatch a gump from the list
	bool close_gump(Gump *gump);			// Close a gump
	void close_all_gumps(bool pers = false);	// Close all gumps

	bool showing_gumps(bool no_pers = false) const;	// Are gumps showing?

	Gump *find_gump(int x, int y);			// Find gump x,y is in
	Gump *find_gump(Game_object *obj);		// Find gump that object is in

	void update_gumps(Game_window *gwin);
	void paint(Game_window *gwin);

	bool double_clicked(int x, int y, Game_object *&obj);

	Gump_manager() :  open_gumps(0) { }
	~Gump_manager() { close_all_gumps(true); }
};

#endif //CONTAINER_GUMP_INCLUDED