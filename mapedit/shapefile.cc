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
#include "utils.h"
#include "Flex.h"
#include "exceptions.h"
#include "combo.h"

using std::vector;
using std::string;
using std::cerr;
using std::endl;
using std::ofstream;

/*
 *	Delete file and groups.
 */

Shape_file_info::~Shape_file_info
	(
	)
	{
	delete groups;
	}

/*
 *	Cleanup.
 */

Image_file_info::~Image_file_info
	(
	)
	{
	delete ifile;
	}

/*
 *	Create a browser for our data.
 */

Object_browser *Image_file_info::create_browser
	(
	Shape_file_info *vgafile,	// THE 'shapes.vga' file.
	unsigned char *palbuf,		// Palette for displaying.
	Shape_group *g			// Group, or 0.
	)
	{
	Shape_chooser *chooser = new Shape_chooser(ifile, palbuf, 400, 64, 
								g, this);
					// Fonts?  Show 'A' as the default.
	if (strcasecmp(basename.c_str(), "fonts.vga") == 0)
		chooser->set_framenum0('A');
	if (this == vgafile)		// Main 'shapes.vga' file?
		{
		chooser->set_shapes_file(
			(Shapes_vga_file *) vgafile->get_ifile());
		}
	return chooser;
	}

/*
 *	Write out if modified.  May throw exception.
 */

void Image_file_info::flush
	(
	)
	{
	if (!modified)
		return;
	modified = false;
	int nshapes = ifile->get_num_shapes();
	int shnum;			// First read all entries.
	Shape **shapes = new Shape *[nshapes];
	for (shnum = 0; shnum < nshapes; shnum++)
		shapes[shnum] = ifile->extract_shape(shnum);
	string filestr("<PATCH>/");	// Always write to 'patch'.
	filestr += basename;
	write_file(filestr.c_str(), shapes, nshapes, false);
	delete [] shapes;
	}

/*
 *	Write a shape file.  (Note:  static method.)
 *	May print an error.
 */

void Image_file_info::write_file
	(
	const char *pathname,		// Full path.
	Shape **shapes,			// List of shapes to write.
	int nshapes,			// # shapes.
	bool single			// Don't write a FLEX file.
	)
	{
	ofstream out;
	U7open(out, pathname);		// May throw exception.
	if (single)
		{
		shapes[0]->write(out);
		out.flush();
		if (!out.good())
			throw file_write_exception(pathname);
		out.close();
		return;
		}
	Flex_writer writer(out, "Written by ExultStudio", nshapes);
					// Write all out.
	for (int shnum = 0; shnum < nshapes; shnum++)
		{
		shapes[shnum]->write(out);
		writer.mark_section_done();
		}
	if (!writer.close())
		throw file_write_exception(pathname);
	}

/*
 *	Cleanup.
 */

Chunks_file_info::~Chunks_file_info
	(
	)
	{
	delete file;
	}

/*
 *	Create a browser for our data.
 */

Object_browser *Chunks_file_info::create_browser
	(
	Shape_file_info *vgafile,	// THE 'shapes.vga' file.
	unsigned char *palbuf,		// Palette for displaying.
	Shape_group *g			// Group, or 0.
	)
	{
					// Must be 'u7chunks' (for now).
	return new Chunk_chooser(vgafile->get_ifile(), *file, palbuf, 
								400, 64, g);
	}

/*
 *	Write out if modified.  May throw exception.
 */

void Chunks_file_info::flush
	(
	)
	{
	if (!modified)
		return;
	modified = false;
	cerr << "Chunks should be stored by Exult" << endl;
	}

/*
 *	Init.
 */

Flex_file_info::Flex_file_info
	(
	const char *bnm,		// Basename,
	const char *pnm,		// Full pathname,
	Flex *fl,			// Flex file (we'll own it).
	Shape_group_file *g		// Group file (or 0).
	) : Shape_file_info(bnm, pnm, g), flex(fl)
	{
	entries.resize(flex->number_of_objects());
	lengths.resize(entries.size());
	}

/*
 *	Cleanup.
 */

Flex_file_info::~Flex_file_info
	(
	)
	{
	delete flex;
	int cnt = entries.size();
	for (int i = 0; i < cnt; i++)
		delete entries[i];
	}

/*
 *	Get i'th entry.
 */

char *Flex_file_info::get
	(
	int i,
	size_t& len
	)
	{
	if (i >= 0 && i < entries.size())
		{
		if (!entries[i])	// Read it if necessary.
			{
			entries[i] = flex->retrieve(i, len);
			lengths[i] = len;
			}
		len = lengths[i];
		return entries[i];
		}
	else
		return 0;
	}

/*
 *	Set i'th entry.
 */

void Flex_file_info::set
	(
	int i,
	char *newentry,			// Allocated data that we'll own.
	int entlen			// Length.
	)
	{
	if (i < 0 || i > entries.size())
		return;
	if (i == entries.size())	// Appending?
		{
		entries.push_back(newentry);
		lengths.push_back(entlen);
		}
	else
		{
		delete entries[i];
		entries[i] = newentry;
		lengths[i] = entlen;
		}
	}

/*
 *	Swap the i'th and i+1'th entries.
 */

void Flex_file_info::swap
	(
	int i
	)
	{
	assert (i >= 0 && i < entries.size() - 1);
	char *tmpent = entries[i];
	int tmplen = lengths[i];
	entries[i] = entries[i + 1];
	lengths[i] = lengths[i + 1];
	entries[i + 1] = tmpent;
	lengths[i + 1] = tmplen;
	}

/*
 *	Remove i'th entry.
 */

void Flex_file_info::remove
	(
	int i
	)
	{
	assert (i >= 0 && i < entries.size());
	delete entries[i];		// Free memory.
	entries.erase(entries.begin() + i);
	lengths.erase(lengths.begin() + i);
	}

/*
 *	Create a browser for our data.
 */

Object_browser *Flex_file_info::create_browser
	(
	Shape_file_info *vgafile,	// THE 'shapes.vga' file.
	unsigned char *palbuf,		// Palette for displaying.
	Shape_group *g			// Group, or 0.
	)
	{
	return new Combo_chooser(vgafile->get_ifile(), this, palbuf,
								400, 64, g);
	}

/*
 *	Write out if modified.  May throw exception.
 */

void Flex_file_info::flush
	(
	)
	{
	if (!modified)
		return;
	modified = false;
	int cnt = entries.size();
	size_t len;
	int i;
	for (i = 0; i < cnt; i++)	// Make sure all are read.
		{
		if (!entries[i])
			get(i, len);
		}
	ofstream out;
	string filestr("<PATCH>/");	// Always write to 'patch'.
	filestr += basename;
	U7open(out, filestr.c_str());	// May throw exception.
	Flex_writer writer(out, "Written by ExultStudio", cnt);
					// Write all out.
	for (int i = 0; i < cnt; i++)
		{
		out.write(entries[i], lengths[i]);
		writer.mark_section_done();
		}
	if (!writer.close())
		throw file_write_exception(filestr.c_str());
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
 *	This routines tries to create files that don't yet exist.
 *
 *	Output:	true if successful.
 */

static bool Create_file
	(
	const char *basename,		// Base file name.
	string& pathname		// Pathname is returned here.
	)
	{
	int namelen = strlen(basename);
	if (strcasecmp(".flx", basename + namelen - 4) == 0)
		{			// We can create an empty flx.
		ofstream out;
		pathname = "<PATCH>/";	// Always write to 'patch'.
		pathname += basename;
		U7open(out, pathname.c_str());	// May throw exception.
		Flex_writer writer(out, "Written by ExultStudio", 0);
		if (!writer.close())
			throw file_write_exception(pathname.c_str());
		return true;
		}
	return false;			// Might add more later.
	}

/*
 *	Create a new 'Shape_file_info', or return existing one.
 *
 *	Output: ->file info, or 0 if error.
 */

Shape_file_info *Shape_file_set::create
	(
	const char *basename		// Like 'shapes.vga'.
	)
	{
	string fullstr("<PATCH>/");	// First look in 'patch'.
	fullstr += basename;
	if (!U7exists(fullstr.c_str()))
		{
		fullstr = "<STATIC>/";
		fullstr += basename;
		if (!U7exists(fullstr.c_str()) && 
					// Try to create it (in some cases).
		    !Create_file(basename, fullstr))
			return 0;
		}
	const char *fullname = fullstr.c_str();
	for (vector<Shape_file_info *>::iterator it = files.begin(); 
					it != files.end(); ++it)
		if (strcasecmp((*it)->pathname.c_str(), fullname) == 0)
			return *it;	// Found it.
	string group_name(basename);	// Create groups file.
	group_name += ".grp";
	Shape_group_file *groups = new Shape_group_file(group_name.c_str());
	int u7drag_type = U7_SHAPE_UNK;
	Vga_file *ifile = 0;
	std::ifstream *file = 0;
	Flex *flex = 0;
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
		{
		file = new std::ifstream;
		U7open(*file, fullname);// Automatically does binary.
		}
	else if (strcasecmp(basename, "combos.flx") == 0)
		flex = new Flex(fullname);
	if (!ifile && !file && !flex)	// Not handled above?
					// Get image file for this path.
		ifile = new Vga_file(fullname, u7drag_type);
	if ((ifile && !ifile->is_good()) || (file && !file->good()))
		{
		cerr << "Error opening image file '" << basename << "'.\n";
		abort();
		}
	Shape_file_info *fi;
	if (ifile)
		fi = new Image_file_info(basename, fullname, ifile, groups);
	else if (file)
		fi = new Chunks_file_info(basename, fullname, file, groups);
	else if (flex)
		fi = new Flex_file_info(basename, fullname, flex, groups);
	else
		return 0;
	files.push_back(fi);
	return fi;
	}

/*
 *	Write any modified image files.
 */

void Shape_file_set::flush
	(
	)
	{
	for (vector<Shape_file_info *>::iterator it = files.begin(); 
					it != files.end(); ++it)
		(*it)->flush();
	}

/*
 *	Any files modified?
 */

bool Shape_file_set::is_modified
	(
	)
	{
	for (vector<Shape_file_info *>::iterator it = files.begin(); 
					it != files.end(); ++it)
		if ((*it)->modified)
			return true;
	return false;
	}
