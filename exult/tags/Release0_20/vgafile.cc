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
	unsigned long datalen = Read4(shapes);
	unsigned long hdrlen = Read4(shapes);
	if (datalen == shapelen)
		{			// Figure # frames.
		int nframes = (hdrlen - 4)/4;
		if (framenum >= nframes)// Bug out if bad frame #.
			return (nframes);
					// Get frame offset.
		unsigned long frameoff;
		if (framenum == 0)
			frameoff = hdrlen + 8;
		else
			{
			shapes.seekg((framenum - 1) * 4, ios::cur);
			frameoff = 8 + Read4(shapes);
			}
					// Get compressed data.
		get_rle_shape(shapes, shapeoff + frameoff);
					// Return # frames.
		return (nframes);
		}
	xleft = yabove = 8;
	xright= ybelow = 0;
	shapes.seekg(shapeoff + framenum*64);
	data = new unsigned char[64];	// Read in 8x8 pixels.
	shapes.read((char *) data, 64);
	return (shapelen/64);		// That's how many frames.
	}

/*
 *	Read in a Run-Length_Encoded shape.
 */

void Shape_frame::get_rle_shape
	(
	ifstream& shapes,		// "Shapes.vga" file to read.
	long filepos			// Position in file.
	)
	{
	shapes.seekg(filepos - 8);	// Get to extents.
	xright = Read2(shapes);
	xleft = Read2(shapes);
	yabove = Read2(shapes);
	ybelow = Read2(shapes);
	unsigned char bbuf[16000];
	unsigned char *out = &bbuf[0];	// ->where to store.
	unsigned char *end = &bbuf[sizeof(bbuf)];
	int scanlen;
#if 0
	int minx = 20000, miny = 20000;	// ++++++++DEBUGGING.
	int maxx = -20000, maxy = -20000;
#endif
	do
		{
		shapes.read(out, 2);	// Get length of scan line.
		scanlen = out[0] + (out[1] << 8);
		out += 2;
		if (!scanlen)
			break;		// All done.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		shapes.read(out, 4);	// Get x, y offsets.
		short scanx = out[0] + (out[1] << 8);
		short scany = out[2] + (out[3] << 8);
		out += 4;
		if (!encoded)		// Raw data?
			{
			if (out + scanlen > end)
				break;	// Shouldn't happen.
			shapes.read(out, scanlen);
			out += scanlen;
			continue;
			}
		for (int b = 0; b < scanlen; )
			{
			unsigned char bcnt;
			shapes.get((char&) bcnt);
			*out++ = bcnt;
					// Repeat next char. if odd.
			int repeat = bcnt&1;
			bcnt = bcnt>>1;	// Get count.
			if (repeat)
				{
				unsigned char pix;
				shapes.get((char&)pix);
				*out++ = pix;
				}
			else		// Get that # of bytes.
				{
				shapes.read(out, bcnt);
				out += bcnt;
				}
			b += bcnt;
			}
		}
	while (out + 64 <= end);	// Leave space for next.
	int len = out - &bbuf[0];
	data = new unsigned char[len];	// Store data.
	memcpy(data, bbuf, len);
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
	num_frames = frame->read(shapes, shapeoff, shapelen, framenum);
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
		tfa.read(&info[i].tfa[0], 3);
	return (1);
	}

