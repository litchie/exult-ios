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
#include "shapelst.h"
#include "chunklst.h"

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
	delete file;
	delete groups;
	}

/*
 *	Create a browser for our data.
 */

Object_browser *Shape_file_info::create_browser
	(
	Shape_file_info *vgafile,	// THE 'shapes.vga' file.
	char **names,			// Names for shapes.vga entries.
	unsigned char *palbuf,		// Palette for displaying.
	Shape_group *g			// Group, or 0.
	)
	{
	if (file)			// Must be 'u7chunks' (for now).
		return new Chunk_chooser(vgafile->get_ifile(), *file, palbuf, 
								400, 64, g);
	Shape_chooser *chooser = new Shape_chooser(ifile, palbuf, 400, 64, g);
	int len = pathname.length();	// Fonts?  Show 'A' as the default.
	if (len >= 9 && strcasecmp(pathname.c_str() - 9, "fonts.vga") == 0)
		chooser->set_framenum0('A');
	if (this == vgafile)		// Main 'shapes.vga' file?
		{
		chooser->set_shape_names(names);
		chooser->set_shapes_file(
			(Shapes_vga_file *) vgafile->get_ifile());
		}
	return chooser;
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
	string group_name(basename);	// Create groups file.
	group_name += ".grp";
	Shape_group_file *groups = new Shape_group_file(group_name.c_str());
	int u7drag_type = U7_SHAPE_UNK;
	Vga_file *ifile = 0;
	std::ifstream *file = 0;
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
	else if (strcasecmp(basename, "u7chunks") == 0)
		file = new std::ifstream(fullname, 
						std::ios::in|std::ios::binary);
	if (!ifile && !file)		// Not handled above?
					// Get image file for this path.
		ifile = new Vga_file(fullname, u7drag_type);
	if ((ifile && !ifile->is_good()) || (file && !file->good()))
		{
		cerr << "Error opening image file '" << basename << "'.\n";
		abort();
		}
	Shape_file_info *fi = new Shape_file_info(fullname, ifile, file,
								groups);
	files.push_back(fi);
	return fi;
	}
