/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Handle access to one of the xxx.vga files.
 **
 **	Written: 4/29/99 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include <string.h>
#include "vgafile.h"
#include "utils.h"
#include "objs.h"

/*
 *	+++++Debugging
 */
inline void Check_file
	(
	ifstream& shapes
	)
	{
	if (!shapes.good())
		{
		cout << "VGA file is bad!\n";
		shapes.clear();
		}
	}

/*
 *	Read in a desired shape.
 *
 *	Output:	# of frames.
 */

unsigned char Shape_frame::read
	(
	ifstream& shapes,		// "Shapes.vga" file to read.
	unsigned long shapeoff,		// Offset of shape in file.
	unsigned long shapelen,		// Length expected for detecting RLE.
	int frnum			// Frame #.
	)
	{
	int framenum = frnum;
	rle = 0;
					// Get to actual shape.
	shapes.seekg(shapeoff);
	Check_file(shapes);
	unsigned long datalen = Read4(shapes);
	unsigned long hdrlen = Read4(shapes);
	if (datalen == shapelen)
		{			// Figure # frames.
		int nframes = (hdrlen - 4)/4;
		if (framenum >= nframes)// Bug out if bad frame #.
			return (nframes);
					// Get frame offset, lengeth.
		unsigned long frameoff, framelen;
		if (framenum == 0)
			{
			frameoff = hdrlen;
			framelen = nframes > 1 ? Read4(shapes) - frameoff :
						datalen - frameoff;
			}
		else
			{
			shapes.seekg((framenum - 1) * 4, ios::cur);
			frameoff = Read4(shapes);
					// Last frame?
			if (framenum == nframes - 1)
				framelen = datalen - frameoff;
			else
				framelen = Read4(shapes) - frameoff;
			}
		Check_file(shapes);
					// Get compressed data.
		get_rle_shape(shapes, shapeoff + frameoff, framelen);
					// Return # frames.
		return (nframes);
		}
	xleft = yabove = 8;
	xright= ybelow = 0;
	shapes.seekg(shapeoff + framenum*64);
	data = new unsigned char[64];	// Read in 8x8 pixels.
	shapes.read((char *) data, 64);
	Check_file(shapes);
	return (shapelen/64);		// That's how many frames.
	}

/*
 *	Read in a Run-Length_Encoded shape.
 */

void Shape_frame::get_rle_shape
	(
	ifstream& shapes,		// "Shapes.vga" file to read.
	long filepos,			// Position in file.
	long len			// Length of frame data.
	)
	{
	shapes.seekg(filepos);		// Get to extents.
	Check_file(shapes);
	xright = Read2(shapes);
	xleft = Read2(shapes);
	yabove = Read2(shapes);
	ybelow = Read2(shapes);
	data = new unsigned char[len + 2];	// Allocate and read data.
	shapes.read(data, len);
	Check_file(shapes);
	data[len] = 0;			// 0-delimit.
	data[len + 1] = 0;
	rle = 1;
	}

/*
 *	See if a point, relative to the shape's 'origin', actually within the
 *	shape.
 */

int Shape_frame::has_point
	(
	int x, int y			// Relative to origin of shape.
	)
	{
	unsigned char *in = data; 	// Point to data.
	int scanlen;
	while ((scanlen = Read2(in)) != 0)
		{
					// Get length of scan line.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		short scanx = Read2(in);
		short scany = Read2(in);
					// Be liberal by 1 pixel.
		if (y == scany && x >= scanx - 1 && x <= scanx + scanlen)
			return (1);
		if (!encoded)		// Raw data?
			{
			in += scanlen;
			continue;
			}
		for (int b = 0; b < scanlen; )
			{
			unsigned char bcnt = *in++;
					// Repeat next char. if odd.
			int repeat = bcnt&1;
			bcnt = bcnt>>1; // Get count.
			if (repeat)
				in++;	// Skip pixel to repeat.
			else		// Skip that # of bytes.
				in += bcnt;
			b += bcnt;
			}
		}
	return (0);			// Never found it.
	}

/*
 *	Read in a frame.
 *
 *	Output:	->frame.
 */
Shape_frame *Shape::read
	(
	ifstream& shapes,		// "Shapes.vga" file to read.
	int shapenum,			// Shape #.
	int framenum			// Frame # within shape.
	)
	{
	Shape_frame *frame = new Shape_frame();
					// Figure offset in "shapes.vga".
	unsigned long shapeoff = 0x80 + shapenum*8;
	shapes.seekg(shapeoff);
					// Get location, length.
	shapeoff = Read4(shapes);
	unsigned long shapelen = Read4(shapes);
					// Read it in and get frame count.
	int nframes = frame->read(shapes, shapeoff, shapelen, framenum);
	if (!num_frames)		// 1st time?
		num_frames = nframes;
#if 0
	else if (nframes != num_frames)	// DEBUGGING.
		cout << "New num_frames??\n";
#endif
	return store_frame(frame, framenum);
	}

/*
 *	Store frame that was read.
 *
 *	Output:	->frame, or 0 if not valid.
 */

Shape_frame *Shape::store_frame
	(
	Shape_frame *frame,		// Frame that was read.
	int framenum			// It's frame #.
	)
	{
	if (framenum >= num_frames)	// Something fishy?
		{
		delete frame;
		return (0);
		}
	if (!frames)	// First one?
		{
		frames = new Shape_frame *[num_frames];
		memset((char *) frames, 0, num_frames * sizeof(Shape *));
		}
	frames[framenum] = frame;
	return (frame);
	}

/*
 *	Read in all shapes from a single-shape file.
 */

Shape_file::Shape_file
	(
	char *nm			// Path to file.
	) : Shape()
	{
	ifstream file;
	if (!U7open(file, nm))
		return;
	Shape_frame *frame = new Shape_frame();
	unsigned long shapelen = Read4(file);
					// Read frame 0 & get frame count.
	num_frames = frame->read(file, 0L, shapelen, 0);
	store_frame(frame, 0);
					// Get the rest.
	for (int i = 1; i < num_frames; i++)
		{
		frame = new Shape_frame();
		frame->read(file, 0L, shapelen, i);
		store_frame(frame, i);
		}
	}

/*
 *	Open file.
 */

Vga_file::Vga_file
	(
	char *nm			// Path to file.
	) : num_shapes(0), shapes(0)
	{
	if (!U7open(file, nm))
		return;
	file.seekg(0x54);		// Get # of shapes.
	num_shapes = Read4(file);
					// Set up lists of pointers.
	shapes = new Shape[num_shapes];
#if 0
	memset((char *) shapes, 0, num_shapes * sizeof(Shape **));
#endif
	}

/*
 *	Read in data files about shapes.
 *
 *	Output:	0 if error.
 */

int Shapes_vga_file::read_info
	(
	)
	{
	ifstream shpdims;
	if (!U7open(shpdims, SHPDIMS))
		return (0);
					// Starts at 0x96'th shape.
	for (int i = 0x96; i < num_shapes; i++)
		{
		shpdims.get((char&) info[i].shpdims[0]);
		shpdims.get((char&) info[i].shpdims[1]);
		}
	ifstream wgtvol;
	if (!U7open(wgtvol, WGTVOL))
		return (0);
	for (int i = 0; i < num_shapes; i++)
		{
		wgtvol.get((char&) info[i].weight);
		wgtvol.get((char&) info[i].volume);
		}
	ifstream tfa;
	if (!U7open(tfa, TFA))
		return (0);
	for (int i = 0; i < num_shapes; i++)
		{
		tfa.read(&info[i].tfa[0], 3);
		info[i].set_tfa_data();
		}
	return (1);
	}

