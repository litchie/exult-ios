/**
 **	Information about a shapes file.
 **
 **	Written: 1/23/02 - JSF
 **/

/*
Copyright (C) 2002 The Exult Team

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

#include "shapefile.h"
#include "u7drag.h"
#include "shapegroup.h"
#include "shapevga.h"

using std::vector;
using std::string;
using std::cerr;
using std::endl;

/*
 *	Delete file and groups.
 */

Shape_file_info::~Shape_file_info
	(
	)
	{
	delete ifile;
	delete groups;
	}

/*
 *	Delete set's entries.
 */

Shape_file_set::~Shape_file_set
	(
	)
	{
	for (vector<Shape_file_info *>::iterator it = files.begin(); 
					it != files.end(); ++it)
		delete (*it);
	}

/*
 *	Create a new 'Shape_file_info', or return existing one.
 */

Shape_file_info *Shape_file_set::create
	(
	const char *basename,		// Like 'shapes.vga'.
	const char *fullname		// Like '.../static/shapes.vga'.
	)
	{
	for (vector<Shape_file_info *>::iterator it = files.begin(); 
					it != files.end(); ++it)
		if ((*it)->pathname == fullname)
			return *it;	// Found it.
	int u7drag_type = U7_SHAPE_UNK;
	Vga_file *ifile = 0;
	if (strcasecmp(basename, "shapes.vga") == 0)
		{			// Special case.
		u7drag_type = U7_SHAPE_SHAPES;
		ifile = new Shapes_vga_file(fullname, U7_SHAPE_SHAPES);
		}
	else if (strcasecmp(basename, "gumps.vga") == 0)
		u7drag_type = U7_SHAPE_GUMPS;
	else if (strcasecmp(basename, "faces.vga") == 0)
		u7drag_type = U7_SHAPE_FACES;
	else if (strcasecmp(basename, "sprites.vga") == 0)
		u7drag_type = U7_SHAPE_SPRITES;
	if (!ifile)			// Not assigned to vgafile?
					// Get image file for this path.
		ifile = new Vga_file(fullname, u7drag_type);
	if (!ifile->is_good())
		{
		cerr << "Error opening image file '" << basename << "'.\n";
		abort();
		}
	string group_name(basename);	// Create groups file.
	group_name += ".grp";
	Shape_file_info *fi = new Shape_file_info(fullname, ifile,
				new Shape_group_file(group_name.c_str()));
	files.push_back(fi);
	return fi;
	}
