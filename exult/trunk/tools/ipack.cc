/**
 **	Ipack.cc - Create/extract image Flex files using the .png format.
 **
 **	Written: 2/19/2002 - JSF, with lots of code borrowed from Tristan's
		 *		Gimp Plugin.
 **/

/*
 *  Copyright (C) 2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <fstream>
#  include <cstdio>
#  include <cstdlib>
#endif
#include <iostream>
#include <iomanip>
#include <vector>
#include "Flex.h"
#include "utils.h"
#include "exceptions.h"
#include "vgafile.h"
#include "pngio.h"
#include "ibuf8.h"

using std::atoi;
using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using std::size_t;
using std::strdup;
using std::strlen;
using std::strncmp;
using std::strcpy;
using std::strcat;
using std::vector;

/*
 *	A shape specification:
 */
class Shape_spec
	{
public:
	char *filename;			// Should be allocated.
	int nframes;			// # frames in shape.
	bool flat;			// A 'flat' shape.
public:
	Shape_spec() : filename(0), nframes(0), flat(false)
		{  }
	~Shape_spec()
		{ delete filename; }
	};
typedef vector<Shape_spec> Shape_specs;

/*
 *	Skip white space.
 */

static char *Skip_space
	(
	char *ptr
	)
	{
	while (isspace(*ptr))
		ptr++;
	return ptr;
	}

/*
 *	Find white space (or null).
 */

static char *Find_space
	(
	char *ptr
	)
	{
	while (*ptr && !isspace(*ptr))
		ptr++;
	return ptr;
	}

/*
 *	Parse a number, and quit with an error msg. if not found.
 */

static long Get_number
	(
	int linenum,			// For printing errors.
	char *errmsg,
	char *ptr,
	char *& endptr			// ->past number and spaces returned.
	)
	{
	long num = strtol(ptr, &endptr, 0);
	if (endptr == ptr)		// No #?
		{
		cerr << "Line " << linenum << ":  " << errmsg << endl;
		exit(1);
		}
	endptr = Skip_space(endptr);
	return num;
	}

/*
 *	Return the next token, or quit with an error.
 *
 *	Output:	->copy of token.
 */

static char *Get_token
	(
	int linenum,			// For printing errors.
	char *ptr,			// Where to start looking.
	char *& endptr			// ->past token returned.
	)
	{
	ptr = Skip_space(ptr + 7);
	endptr = Find_space(ptr);
	if (endptr == ptr)
		{
		cerr << "Line #" << linenum <<
				":  Expecting a name" << endl;
		exit(1);
		}
	char sav = *endptr;
	*endptr = 0;
	char *token = strdup(ptr);
	*endptr = sav;
	return token;
	}

/*
 *	Read in script, with the following format:
 *		Max. text length is 1024.
 *		A line beginning with a '#' is a comment.
 *		'archive imgfile' specifies name of the image archive.
 *		'palette palfile' specifies the palette file (which, if a
 *			Flex, contains palette in entry 0).
 *		'nnn/fff:filename' indicates shape #nnn will consist of fff
 *			frames in files "filename-iii.png", where iii is the
 *			frame #.
 *		Filename may be followed by 'flat' to indicate 8x8 non-RLE
 *			shape.
 */

static void Read_script
	(
	istream& in,
	char *& imagename,		// Archive name returned.
	char *& palname,		// Palette name returned.
	Shape_specs& specs		// Shape specs. returned here.
	)
	{
	imagename = 0;
	specs.resize(0);		// Initialize.
	specs.reserve(1200);
	char buf[1024];
	int linenum = 0;
	while (!in.eof())
		{
		++linenum;
		in.get(buf, sizeof(buf));
		char delim;		// Check for end-of-line.
		in.get(delim);
		if (delim != '\n' && !in.eof())
			{
			cerr << "Line #" << linenum << " is too long" << endl;
			exit(1);
			}
		if (!buf[0])
			continue;	// Empty line.
		char *ptr = Skip_space(&buf[0]);
		if (*ptr == '#')
			continue;	// Comment.
		char *endptr;
		if (strncmp(ptr, "archive", 7) == 0)
			{		// Archive name.
			imagename = Get_token(linenum, ptr, endptr);
			continue;
			}
		if (strncmp(ptr, "palette", 7) == 0)
			{
			palname = Get_token(linenum, ptr, endptr);
			continue;
			}
					// Get shape# in decimal, hex, or oct.
		long shnum = Get_number(linenum, "Shape # missing",
								ptr, endptr);
		if (*endptr != '/')
			{
			cerr << "Line #" << linenum << 
				":  Missing '/' after shape number" << endl;
			exit(1);
			}
		ptr = endptr + 1;
		long nframes = Get_number(linenum, "Frame count missing",
								ptr, endptr);
		if (*endptr != ':')
			{
			cerr << "Line #" << linenum << 
				":  Missing ':' after frame count" << endl;
			exit(1);
			}
		ptr = Skip_space(endptr + 1);
		endptr = Find_space(ptr);
		if (endptr == ptr)
			{
			cerr << "Line #" << linenum <<
				":  Missing filename" << endl;
			exit(1);
			}
					// Get ->past filename.
		char *past_end = *endptr ? Skip_space(endptr + 1) : endptr;
		*endptr = 0;
		if (shnum >= specs.size())
			specs.resize(shnum + 1);
		specs[shnum].filename = strdup(ptr);
		specs[shnum].nframes = nframes;
		specs[shnum].flat = (strncmp(past_end, "flat", 4) == 0);
		}
	}

/*
 *	Modify a palette by fixed amounts and/or percentages.
 */

static void Modify_palette
	(
	unsigned char *from,		// Rgb values to start with,
					//   each range 0-255.
	unsigned char *to,		// Result stored here.
	int palsize,			// 0-256.
	int roff, int goff, int boff,	// Add these offsets first.
	int r256, int g256, int b256	// Modify by x/256.
	)
	{
	int cnt = 3*palsize;
	for (int i = 0; i < 3*palsize; )
		{
		int r = from[i] + roff;	// First the offsets.
					// Then percentage.
		r += (r*r256)/256;
		if (r < 0)
			r = 0;
		if (r > 255)
			r = 255;
		to[i++] = r;
		int g = from[i] + goff;
		g += (g*g256)/256;
		if (g < 0)
			g = 0;
		if (g > 255)
			g = 255;
		to[i++] = g;
		int b = from[i] + boff;
		b += (b*b256)/256;
		if (b < 0)
			b = 0;
		if (b > 255)
			b = 255;
		to[i++] = b;
		}
	}

/*
 *	Modify a palette by removing all color.
 */

static void Greyify_palette
	(
	unsigned char *from,		// Rgb values to start with,
					//   each range 0-255.
	unsigned char *to,		// Result stored here.
	int palsize			// 0-256.
	)
	{
	for (int i = 0; i < palsize; i++)
		{
		int ind = i*3;
					// Take average.
		int ave = (from[ind] + from[ind + 1] + from[ind + 2])/3;
		to[ind] = to[ind + 1] = to[ind + 2] = ave;
		}
	}

/*
 *	Convert a palette to values 0-63.
 */

static void Convert_palette63
	(
	unsigned char *from,		// 3*palsize, values 0-255.
	unsigned char *to,		// 3*256.  Values 0-63 returned, with
					//   colors > palsize 0-filled.
	int palsize			// # entries.
	)
	{
	int i;				// Convert 0-255 to 0-63 for Exult.
	for (i = 0; i < 3*palsize; i++)
		to[i] = from[i]/4;
	memset(to + i, 0, 3*256 - i);	// 0-fill the rest.

	}

/*
 *	Write out a palette as text.
 */

static void Write_text_palette
	(
	char *palname,			// Base name.  '.txt' is appended.
	unsigned char *palette,		// RGB's, 0-255.
	int palsize			// # colors in palette
	)
	{
					// Write out as (Gimp) text.
	char *txtpal = new char[strlen(palname) + 10];
	strcpy(txtpal, palname);
	strcat(txtpal, ".txt");
	cout << "Creating text (Gimp) palette '" << txtpal << "'" << endl;
	ofstream pout(txtpal);		// OKAY that it's a 'text' file.
	pout << "GIMP palette" << endl;	// MUST be this for Gimp to use.
	pout << "Palette from Exult's Ipack" << endl;
	int i;				// Skip 0's at end.
	for (i = palsize - 1; i > 0; i--)
		if (palette[3*i] != 0 || palette[3*i+1] != 0 || 
							palette[3*i+2] != 0)
			break;
	int last_color = i;
	for (i = 0; i <= last_color; i++)
		{
		int r = palette[3*i],
		    g = palette[3*i + 1],
		    b = palette[3*i + 2];
		pout << setw(3) << r << ' ' << setw(3) << g << ' ' << 
						setw(3) << b << endl;
		}
	pout.close();
	delete txtpal;
	}

const unsigned char transp = 255;	// Transparent pixel.

/*
 *	Write out one frame as a .png.
 */

static void Write_frame
	(
	char *basename,			// Base filename to write.
	int frnum,			// Frame #.
	Shape_frame *frame,		// What to write.
	unsigned char *palette		// 3*256 bytes.
	)
	{
	assert(frame != 0);
	char *fullname = new char[strlen(basename) + 30];
	sprintf(fullname, "%s%02d.png", basename, frnum);
	cout << "Writing " << fullname << endl;
	int w = frame->get_width(), h = frame->get_height();
	Image_buffer8 img(w, h);	// Render into a buffer.
	img.fill8(transp);		// Fill with transparent pixel.
	frame->paint(&img, frame->get_xleft(), frame->get_yabove());
	int xoff = 0, yoff = 0;
	if (frame->is_rle())
		{
		xoff = -frame->get_xright();
		yoff = -frame->get_ybelow();
		}
					// Write out to the .png.
	if (!Export_png8(fullname, transp, w, h, w, xoff, yoff, img.get_bits(),
					palette, 256))
		throw file_write_exception(fullname);
	delete fullname;
	}

/*
 *	Write out all (8x8 flat) frames by tiling them into one file.
 */

static void Write_tiled_frames
	(
	char *filename,			// Filename to write.
	Shape *shape,			// What to write.
	int shnum,			// For printing errors.
	bool bycol,			// If true, go down each column first,
					//   else go across each row first.
	int dim0_cnt,			// If bycol, #rows; else #cols.
	unsigned char *palette		// 3*256 bytes.
	)
	{
	assert(shape != 0);
	cout << "Writing " << filename << " tiled" 
		<< (bycol ? ", by cols" : ", by rows") << " first" << endl;
	int nframes = shape->get_num_frames();
					// Figure #tiles in other dim.
	int dim1_cnt = (nframes + dim0_cnt - 1)/dim0_cnt;
	int w, h;
	if (bycol)
		{ h = dim0_cnt*8; w = dim1_cnt*8; }
	else
		{ w = dim0_cnt*8; h = dim1_cnt*8; }
	Image_buffer8 img(w, h);
	img.fill8(transp);		// Fill with transparent pixel.
	for (int f = 0; f < nframes; f++)
		{
		Shape_frame *frame = shape->get_frame(f);
		if (!frame)
			continue;	// We'll just leave empty ones blank.
		if (!frame->is_rle() || frame->get_width() != 8 ||
					frame->get_height() != 8)
			{
			cerr << "Can only tile 8x8 flat shapes, but shape "
				<< shnum << " doesn't qualify" << endl;
			exit(1);
			}
		int x, y;
		if (bycol)
			{ y = f%dim0_cnt; x = f/dim0_cnt; }
		else
			{ x = f%dim0_cnt; y = f/dim0_cnt; }
		frame->paint(&img, x*8 + frame->get_xleft(), 
						y*8 + frame->get_yabove());
		}
					// Write out to the .png.
	if (!Export_png8(filename, transp, w, h, w, 0, 0, img.get_bits(),
					palette, 256))
		throw file_write_exception(filename);
	}

/*
 *	Write out palettes.  The first is the one given, and the rest are
 *	automatically generated to work with the Exult engine (see
 *	palettes.h for definitions).
 */

void Write_palettes
	(
	char *palname, 
	unsigned char *palette, 	// 3 bytes for each color.
	int palsize			// # entries.
	)
	{
	cout << "Creating new palette file '" << palname <<
			"' using first file's palette" << endl;
	unsigned char palbuf[3*256];	// We always write 256 colors.
	if (palsize > 256)		// Shouldn't happen.
		palsize = 256;
	ofstream out;
	U7open(out, palname);		// May throw exception.
	Flex_writer writer(out, "Exult palette by Ipack", 11);
					// Entry 0 (DAY):
	Convert_palette63(palette, &palbuf[0], palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 1 (DUSK):
	Modify_palette(palette, palbuf, palsize, 0, 0, 0, -64, -64, -64);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 2 (NIGHT):
	Modify_palette(palette, palbuf, palsize, 0, 0, 0, -128, -128, -128);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 3 (INVISIBLE):
	Greyify_palette(palette, palbuf, palsize);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 4 (unknown):
	Convert_palette63(palette, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 5 (HAZE):
	Modify_palette(palette, palbuf, palsize, 184, 184, 184, -32, -32, -32);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 6 (a bit brighter than 2):
	Modify_palette(palette, palbuf, palsize, 0, 0, 0, -96, -96, -96);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 7 (a bit warmer):
	Modify_palette(palette, palbuf, palsize, 30, 0, 0, -96, -96, -96);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 8 (hit in combat):
	Modify_palette(palette, palbuf, palsize, 64, 0, 0, 384, -20, -20);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 9 (unknown):
	Convert_palette63(palette, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
					// Entry 10 (LIGHTNING):
	Modify_palette(palette, palbuf, palsize, 32, 32, 0, 256, 256, -20);
	Convert_palette63(palbuf, palbuf, palsize);
	out.write(&palbuf[0], sizeof(palbuf));
	writer.mark_section_done();
	if (!writer.close())
		throw file_write_exception(palname);
					// Write out in Gimp format.
	Write_text_palette(palname, palette, palsize);
	}

/*
 *	Write tiles from a single input file to and Exult image file.
 */

static void Write_exult_from_tiles
	(
	ostream& out,			// What to write to.
	char *filename,			// Filename to read.
	int nframes,			// # frames.
	bool bycol,			// If true, go down each column first,
					//   else go across each row first.
	int dim0_cnt,			// If bycol, #rows; else #cols.
	char *palname			// Store palette here if !0.
	)
	{
	cout << "Reading " << filename << " tiled" 
		<< (bycol ? ", by cols" : ", by rows") << " first" << endl;
					// Figure #tiles in other dim.
	int dim1_cnt = (nframes + dim0_cnt - 1)/dim0_cnt;
	int needw, needh;		// Figure min. image dims.
	if (bycol)
		{ needh = dim0_cnt*8; needw = dim1_cnt*8; }
	else
		{ needw = dim0_cnt*8; needh = dim1_cnt*8; }
					// Save starting position.
	unsigned long startpos = out.tellp();
	int w, h, rowsize, xoff, yoff, palsize;
	unsigned char *pixels, *palette;
					// Import, with 255 = transp. index.
	if (!Import_png8(filename, 255, w, h, rowsize, xoff, yoff,
						pixels, palette, palsize))
		throw file_read_exception(filename);
	if (w < needw || h < needh)
		{
		cerr << "File " << filename << " image is too small.  " <<
			needw << 'x' << needh << " required" << endl;
		exit(1);
		}
	for (int frnum = 0; frnum < nframes; frnum++)
		{
		int x, y;
		if (bycol)
			{ y = frnum%dim0_cnt; x = frnum/dim0_cnt; }
		else
			{ x = frnum%dim0_cnt; y = frnum/dim0_cnt; }
		unsigned char *src = pixels + w*8*y + 8*x;
		for (int row = 0; row < 8; row++)
			{		// Write it out.
			out.write(src, 8);
			src += w;
			}
		}
	delete pixels;
	if (palname)
		Write_palettes(palname, palette, palsize);
	delete palette;
	}

/*
 *	Write a shape's frames to an Exult image file.
 */

static void Write_exult
	(
	ostream& out,			// What to write to.
	char *basename,			// Base filename for files to read.
	int nframes,			// # frames.
	bool flat,			// Store as 8x8 flats.
	char *palname			// Store palette with here if !0.
	)
	{
	char *fullname = new char[strlen(basename) + 30];
	int frnum;
					// Save starting position.
	unsigned long startpos = out.tellp();
	if (!flat)
		{
		Write4(out, 0);		// Place-holder for total length.
					// Also for frame locations.
		for (frnum = 0; frnum < nframes; frnum++)
			Write4(out, 0);
		}
	for (frnum = 0; frnum < nframes; frnum++)
		{
		sprintf(fullname, "%s%02d.png", basename, frnum);
		cout << "Reading " << fullname << endl;
		int w, h, rowsize, xoff, yoff, palsize;
		unsigned char *pixels, *palette;
					// Import, with 255 = transp. index.
		if (!Import_png8(fullname, 255, w, h, rowsize, xoff, yoff,
						pixels, palette, palsize))
			throw file_read_exception(fullname);
		int datalen = h*rowsize;// Figure total #bytes.
		if (flat)
			{
			if (w != 8 || h != 8 || rowsize != 8)
				{
				cerr << "Image in '" << fullname <<
					"' is not flat" << endl;
				exit(1);
				}
			}
		else			// Encode.
			{
			int xright = -xoff, ybelow = -yoff;
			int xleft = w - xright - 1,
			    yabove = h - ybelow - 1;
			unsigned char *rle = Shape_frame::encode_rle(pixels,
						w, h, xleft, yabove, datalen);
			delete pixels;
			pixels = rle;
					// Get position of frame.
			unsigned long pos = out.tellp();
			out.seekp(startpos + (frnum + 1)*4);
			Write4(out, pos - startpos);	// Store pos.
			out.seekp(pos);			// Get back.
			Write2(out, xright);
			Write2(out, xleft);
			Write2(out, yabove);
			Write2(out, ybelow);
			}
		out.write(pixels, datalen);	// The frame data.
		delete pixels;
		if (palname)
			{
			Write_palettes(palname, palette, palsize);
			palname = 0;
			}
		delete palette;
		}
	if (!flat)
		{
		unsigned long pos = out.tellp();// Ending position.
		out.seekp(startpos);		// Store total length.
		Write4(out, pos - startpos);
		out.seekp(pos);			// And get back to end.
		}
	delete fullname;
	}

/*
 *	Create an archive from a set of image files.  May throw an exception.
 */

static void Create
	(
	char *imagename,		// Image archive name.
	char *palname,			// Palettes file (palettes.flx).
	Shape_specs& specs,		// List of things to extract.
	const char *title		// For storing in Flex file.
	)
	{
	if (palname && U7exists(palname))		// Palette?
		{
		cout << "Palette file '" << palname << 
			"' exists, so we won't overwrite it" << endl;
		palname = 0;
		}
	ofstream out;
	U7open(out, imagename);		// May throw exception.
	Flex_writer writer(out, title, specs.size());
	for (Shape_specs::const_iterator it = specs.begin();
						it != specs.end();  ++it)
		{
		int shnum = it - specs.begin();
		char *basename = (*it).filename;
		if (basename)		// Not empty?
			{
			Write_exult(out, basename, (*it).nframes, (*it).flat,
								palname);
			palname = 0;	// Only write 1st palette.
			}
		writer.mark_section_done();
		}
	if (!writer.close())
		throw file_write_exception(imagename);
	}

/*
 *	Update an archive with a set of image files.  May throw an exception.
 *	The archive is rewritten with its original shapes, plus those
 *	specified in the script.
 */

static void Update
	(
	char *imagename,		// Image archive name.
	char *palname,			// Palettes file (palettes.flx).
	Shape_specs& specs,		// List of things to extract.
	const char *title		// For storing in Flex file.
	)
	{
	if (palname && U7exists(palname))		// Palette?
		{
		cout << "Palette file '" << palname << 
			"' exists, so we won't overwrite it" << endl;
		palname = 0;
		}
	Flex in(imagename);		// May throw exception.
	int oldcnt = in.number_of_objects();
	vector<char *> data(oldcnt);	// Read in all the entries.
	vector<int> lengths(oldcnt);
	int i;
	for (i = 0; i < oldcnt; i++)
		{
		size_t len;
		data[i] = in.retrieve(i, len);
		lengths[i] = len;
		if (!len)		// Empty?
			{
			delete data[i];
			data[i] = 0;
			}
		}
	ofstream out;
	U7open(out, imagename);		// May throw exception.
	int newcnt = oldcnt > specs.size() ? oldcnt : specs.size();
	Flex_writer writer(out, title, specs.size());
	for (i = 0; i < newcnt; i++)	// Write out new entries.
		{
					// New entry for this shape?
		if (i < specs.size() && specs[i].filename != 0)
			{
			Write_exult(out, specs[i].filename,
				specs[i].nframes, specs[i].flat, palname);
			palname = 0;	// Only write 1st palette.
			}
					// Write old entry.
		else if (i < oldcnt && data[i])
			out.write(data[i], lengths[i]);
		writer.mark_section_done();
		}
	if (!writer.close())
		throw file_write_exception(imagename);
	for (i = 0; i < oldcnt; i++)	// Clean up.
		delete data[i];
	}

/*
 *	Extract from the archive.  May throw an exception.
 */

static void Extract
	(
	char *imagename,		// Image archive name.
	char *palname,			// Palettes file (palettes.flx).
	Shape_specs& specs		// List of things to extract.
	)
	{
	if (!palname)
		{
		cerr << "No palette name (i.e., 'palettes.flx') given" << endl;
		exit(1);
		}
	U7object pal(palname, 0);	// Get palette 0.
	size_t len;
	unsigned char *palbuf = (unsigned char *)
		pal.retrieve(len);	// This may throw an exception
	for (int i = 0; i < len; i++)	// Turn into full bytes.
		palbuf[i] *= 4;		// Exult palette vals are 0-63.
	Vga_file ifile(imagename);	// May throw an exception.
	for (Shape_specs::const_iterator it = specs.begin();
						it != specs.end();  ++it)
		{
		char *basename = (*it).filename;
		if (!basename)		// Empty?
			continue;
		int shnum = it - specs.begin();
		if (shnum >= ifile.get_num_shapes())
			{
			cerr << "Shape #" << shnum << " > #shapes in file" <<
								endl;
			continue;
			}
					// Read in all frames.
		Shape *shape = ifile.extract_shape(shnum);
		int nframes = shape->get_num_frames();
		if (nframes != (*it).nframes)
			cerr << "Warning: # frames (" << (*it).nframes <<
				") given for shape " << shnum <<
				" doesn't match actual count (" << nframes <<
				")" << endl;
		for (int f = 0; f < nframes; f++)
			Write_frame(basename, f, shape->get_frame(f), palbuf);
		}
	delete palbuf;
	}

/*
 *	Print usage and exit.
 */

static void Usage()
	{
	cerr << "Usage: ipack -[x|c|u] script" << endl;
	exit(1);
	}

/*
 *	Create or extract from Flex files consisting of shapes.
 */

int main
	(
	int argc,
	char **argv
	)
	{
	if (argc < 3 || argv[1][0] != '-')
		Usage();		// (Exits.)
	char *scriptname = argv[2];
	char *imagename = 0, *palname = 0;
	Shape_specs specs;		// Shape specs. stored here.
	ifstream specin;
	try {
		U7open(specin, scriptname, true);
	} catch (exult_exception& e) {
		cerr << e.what() << endl;
		exit(1);
	}
	Read_script(specin, imagename, palname, specs);
	if (!imagename)
		{
		cerr << "No archive name (i.e., 'shapes.vga') given" << endl;
		exit(1);
		}
	specin.close();
	switch (argv[1][1])		// Which function?
		{
	case 'c':			// Create.
		try {
			Create(imagename, palname, specs, "Exult image file");
		} catch (exult_exception& e){
			cerr << e.what() << endl;
			exit(1);
		}
		break;
	case 'x':			// Extract .png files.
		try {
			Extract(imagename, palname, specs);
		} catch (exult_exception& e){
			cerr << e.what() << endl;
			exit(1);
		}
		break;
	case 'u':
		try {
			Update(imagename, palname, specs, "Exult image file");
		} catch (exult_exception& e){
			cerr << e.what() << endl;
			exit(1);
		}
		break;
	default:
		Usage();
		}
	return 0;
	}
