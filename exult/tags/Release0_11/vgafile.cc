/**
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
	int shnum,			// Shape #.
	int frnum			// Frame #.
	)
	{
	int shapenum = shnum;
	int framenum = frnum;
	rle = 0;
					// Figure offset in "shapes.vga".
	unsigned long shapeoff = 0x80 + shapenum*8;
	shapes.seekg(shapeoff);
					// Get location, length.
	shapeoff = Read4(shapes);
	unsigned long shapelen = Read4(shapes);
					// Get to actual shape.
	shapes.seekg(shapeoff);
	unsigned long datalen = Read4(shapes);
	unsigned long hdrlen = Read4(shapes);
	if (datalen == shapelen)
		{
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
		return ((hdrlen - 4)/4);
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
	unsigned char bbuf[12000];
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
					// Read it in and get frame count.
	num_frames = frame->read(shapes, shapenum, framenum);
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
 *	Read in a "dimensions" file.  (There's currently only one.)
 *
 *	Output:	0 if error.
 */

int Vga_file::read_dims
	(
	char *nm,			// Filename.
	int first,			// First shape # in file.
	int num				// Num. shapes in file.
	)
	{
	ifstream in;
	if (!U7open(in, nm))
		return (0);
	int size = 2*num;		// 2 bytes per shape.
	unsigned char *buf = new unsigned char[size + 1];
	dims = buf;			// +++++++++++++Debugging.
	in.read(buf, size);
	if (in.good())
		{
		for (int i = 0; i < num; i++)
			{
			Shape& shape = shapes[first + i];
					// (Each dim. is shifted over 1.)
			shape.dim =	((buf[2*i] << 3)&0xf0) |
					((buf[2*i + 1] >> 1) & 0xf);
					// This is my guess:
			shape.obstacle = (buf[2*i] & 1) != 0 ||
					(buf[2*i + 1] & 1) != 0;
			}
		}
//++++++++Debugging	delete buf;
	return (in.good());
	}
