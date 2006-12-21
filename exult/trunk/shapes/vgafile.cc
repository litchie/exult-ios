/*
 *	vgafile.cc - Handle access to one of the xxx.vga files.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif
#include "utils.h"
#include "rect.h"
#include "ibuf8.h"
#include "vgafile.h"
#include "databuf.h"
#include "Flex.h"
#include "U7file.h"
#include "exceptions.h"
#include "palette.h"

#include <cassert>
#include <vector>
#include <string>
#include <utility>
#include <map>

#ifndef UNDER_CE
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::memcpy;
using std::memset;
using std::ostream;
using std::vector;
using std::string;
using std::pair;
#endif

GL_manager *Shape_frame::glman = 0;
Image_buffer8 *Shape_frame::scrwin = 0;

#if 1	/* For debugging. */
#include <iomanip>
#include "items.h"
#endif

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
		cout << "VGA file is bad!" << endl;
		shapes.clear();
		}
	}

/*
 *	Create a new frame by reflecting across a line running NW to SE.
 *
 *	May return 0.
 */

Shape_frame *Shape_frame::reflect
	(
	)
	{
	if (!data)
		return 0;
	int w = get_width(), h = get_height();
	if (w < h)
		w = h;
	else
		h = w;			// Use max. dim.
	Shape_frame *reflected = new Shape_frame();
	reflected->rle = true;		// Set data.
	reflected->xleft = yabove;
	reflected->yabove = xleft;
	reflected->xright = ybelow;
	reflected->ybelow = xright;
					// Create drawing area.
	Image_buffer8 *ibuf = new Image_buffer8(h, w);
	ibuf->fill8(255);		// Fill with 'transparent' pixel.
					// Figure origin.
	int xoff = reflected->xleft, yoff = reflected->yabove;
	uint8 *in = data; 	// Point to data, and draw.
	int scanlen;
	while ((scanlen = Read2(in)) != 0)
		{
					// Get length of scan line.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		short scanx = Read2(in);
		short scany = Read2(in);
		if (!encoded)		// Raw data?
			{
			ibuf->copy8(in, 1, scanlen,
					xoff + scany, yoff + scanx);
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
				{
				unsigned char pix = *in++;
				ibuf->fill8(pix, 1, bcnt,
					xoff + scany, yoff + scanx + b);
				}
			else		// Get that # of bytes.
				{
				ibuf->copy8(in, 1, bcnt,
					xoff + scany, yoff + scanx + b);
				in += bcnt;
				}
			b += bcnt;
			}
		}
	reflected->create_rle(ibuf->get_bits(), w, h);
	delete ibuf;			// Done with this.
	return (reflected);
	}
	
/*
 *	Skip transparent pixels.
 *
 *	Output:	Index of first non-transparent pixel (w if no more).
 *		Pixels is incremented by (delta x).
 */

static int Skip_transparent
	(
	unsigned char *& pixels,	// 8-bit pixel scan line.
	int x,				// X-coord. of pixel to start with.
	int w				// Remaining width of pixels.
	)
	{
	while (x < w && *pixels == 255)
		{
		x++;
		pixels++;
		}
	return (x);
	}

/*
 *	Split a line of pixels into runs, where a run
 *	consists of different pixels, or a repeated pixel.
 *
 *	Output:	Index of end of scan line.
 */

static int Find_runs
	(
	short *runs,			// Each run's length is returned.
					// For each byte, bit0==repeat.
					// List ends with a 0.
	unsigned char *pixels,		// Scan line (8-bit color).
	int x,				// X-coord. of pixel to start with.
	int w				// Remaining width of pixels.
	)
	{
	int runcnt = 0;			// Counts runs.
	while (x < w && *pixels != 255)	// Stop at first transparent pixel.
		{
		int run = 0;		// Look for repeat.
		while (x < w - 1 && pixels[0] == pixels[1])
			{
			x++;
			pixels++;
			run++;
			}
		if (run)		// Repeated?  Count 1st, shift, flag.
			{
			run = ((run + 1)<<1)|1;
			x++;		// Also pass the last one.
			pixels++;
			}
		else do			// Pass non-repeated run of
			{		//   andy length.
			x++;
			pixels++;
			run += 2;	// So we don't have to shift.
			}
		while (x < w && *pixels != 255 &&
			(x == w - 1 || pixels[0] != pixels[1]));
					// Store run length.
		runs[runcnt++] = run;
		}
	runs[runcnt] = 0;		// 0-delimit list.
	return (x);
	}

/*
 *	Encode an 8-bit image into an RLE frame.
 *
 *	Output:	Data is set to compressed image.
 */

void Shape_frame::create_rle
	(
	unsigned char *pixels,		// 8-bit uncompressed data.
	int w, int h			// Width, height.
	)
	{
	delete [] data;			// Delete old data if there.
	data = encode_rle(pixels, w, h, xleft, yabove, datalen);
	}

/*
 *	Encode an 8-bit image into an RLE frame.
 *
 *	Output:	->allocated RLE data.
 */

unsigned char *Shape_frame::encode_rle
	(
	unsigned char *pixels,		// 8-bit uncompressed data.
	int w, int h,			// Width, height.
	int xoff, int yoff,		// Origin (xleft, yabove).
	int& datalen			// Length of RLE data returned.
	)
	{
					// Create an oversized buffer.
	uint8 *buf = new uint8[w*h*2 + 16*h];
	uint8 *out = buf;
	int newx;			// Gets new x at end of a scan line.
	for (int y = 0; y < h; y++)	// Go through rows.
		for (int x = 0; (x = Skip_transparent(pixels, x, w)) < w; 
								x = newx)
			{
			short runs[100];// Get runs.
			newx = Find_runs(runs, pixels, x, w);
					// Just 1 non-repeated run?
			if (!runs[1] && !(runs[0]&1))
				{
				int len = runs[0] >> 1;
				Write2(out, runs[0]);
					// Write position.
				Write2(out, x - xoff);
				Write2(out, y - yoff);
				memcpy(out, pixels, len);
				pixels += len;
				out += len;
				continue;
				}
					// Encoded, so write it with bit0==1.
			Write2(out, ((newx - x)<<1)|1);
					// Write position.
			Write2(out, x - xoff);
			Write2(out, y - yoff);
					// Go through runs.
			for (int i = 0; runs[i]; i++)
				{
				int len = runs[i]>>1;
					// Check for repeated run.
				if (runs[i]&1)
					{
					while (len)
						{
						int c = len > 127
							? 127 : len;
						*out++ = (c<<1)|1;
						*out++ = *pixels;
						pixels += c;
						len -= c;
						}
					}
				else while (len > 0)
					{
					int c = len > 127 ? 127 : len;
					*out++ = c<<1;
					memcpy(out, pixels, c);
					out += c;
					pixels += c;
					len -= c;
					}
				}
			}
	Write2(out, 0);			// End with 0 length.
	datalen = out - buf;		// Create buffer of correct size.
#ifdef DEBUG
	if(datalen > w*h*2 + 16*h)
		cout << "create_rle: datalen: " << datalen << " w: " << w
			<< " h: " << h << endl;
#endif
	unsigned char *data = new unsigned char[datalen];
	memcpy(data, buf, datalen);
	delete [] buf;
	return data;
	}

/*
 *	Create from data.
 */

Shape_frame::Shape_frame
	(
	unsigned char *pixels, 		// (A copy is made.)
	int w, int h,			// Dimensions.
	int xoff, int yoff,		// Xleft, yabove.
	bool setrle			// Run-length-encode.
	) : xleft(xoff), yabove(yoff), xright(w - xoff - 1),
	    ybelow(h - yoff - 1), rle(setrle)
#ifdef HAVE_OPENGL
		, glshape(0)
#endif
	{
	if (!rle)
		{
		assert(w == 8 && h == 8);
		datalen = 64;
		data = new unsigned char[64];
		memcpy(data, pixels, 64);
		}
	else
		data = encode_rle(pixels, w, h, xleft, yabove, datalen);
	}

/*
 *	Read in a desired shape.
 *
 *	Output:	# of frames.
 */

unsigned int Shape_frame::read
	(
	DataSource* shapes,		// Shapes data source to read.
	uint32 shapeoff,		// Offset of shape in file.
	uint32 shapelen,		// Length expected for detecting RLE.
	int frnum			// Frame #.
	)
	{
	int framenum = frnum;
	rle = false;
	if (!shapelen && !shapeoff) return 0;
					// Get to actual shape.
	shapes->seek(shapeoff);
	uint32 dlen = shapes->read4();
	uint32 hdrlen = shapes->read4();
	if (dlen == shapelen)
		{
		rle = true;		// It's run-length-encoded.
					// Figure # frames.
		int nframes = (hdrlen - 4)/4;
		if (framenum >= nframes)// Bug out if bad frame #.
			return (nframes);
					// Get frame offset, lengeth.
		uint32 frameoff, framelen;
		if (framenum == 0)
			{
			frameoff = hdrlen;
			framelen = nframes > 1 ? shapes->read4() - frameoff :
						dlen - frameoff;
			}
		else
			{
			shapes->skip((framenum - 1) * 4);
			frameoff = shapes->read4();
					// Last frame?
			if (framenum == nframes - 1)
				framelen = dlen - frameoff;
			else
				framelen = shapes->read4() - frameoff;
			}
					// Get compressed data.
		get_rle_shape(shapes, shapeoff + frameoff, framelen);
					// Return # frames.
		return (nframes);
		}
	framenum &= 31;			// !!!Guessing here.
	xleft = yabove = 8;		// Just an 8x8 bitmap.
	xright= ybelow = -1;
	shapes->seek(shapeoff + framenum*64);
	data = new unsigned char[64];	// Read in 8x8 pixels.
	datalen = 64;
	shapes->read((char *) data, 64);
	return (shapelen/64);		// That's how many frames.
	}
	
/*
 *	Read in a Run-Length_Encoded shape.
 */

void Shape_frame::get_rle_shape
	(
	DataSource* shapes,		// Shapes data source to read.
	long filepos,			// Position in file.
	long len			// Length of entire frame data.
	)
	{
	shapes->seek(filepos);		// Get to extents.
	xright = shapes->read2();
	xleft = shapes->read2();
	yabove = shapes->read2();
	ybelow = shapes->read2();
	len -= 8;			// Subtract what we just read.
	data = new unsigned char[len + 2];	// Allocate and read data.
	datalen = len+2;
	shapes->read((char*)data, len);
	data[len] = 0;			// 0-delimit.
	data[len + 1] = 0;
	rle = true;
	}
	
/*
 *	Show a Run-Length_Encoded shape.
 */

void Shape_frame::paint_rle
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int xoff, int yoff		// Where to show in iwin.
	)
	{
	assert(rle);

	int w = get_width(), h = get_height();
	if (w >= 8 || h >= 8)		// Big enough to check?  Off screen?
		if (!win->is_visible(xoff - xleft, yoff - yabove, w, h))
			return;

	win->paint_rle (xoff, yoff, data);
	return;
	}

/*
 *	Show a Run-Length_Encoded shape mapped to a different palette.
 */

void Shape_frame::paint_rle_remapped
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int xoff, int yoff,		// Where to show in iwin.
	unsigned char *trans
	)
	{
	assert(rle);

	int w = get_width(), h = get_height();
	if (w >= 8 || h >= 8)		// Big enough to check?  Off screen?
		if (!win->is_visible(xoff - xleft, yoff - yabove, w, h))
			return;

	win->paint_rle_remapped (xoff, yoff, data, trans);
	return;
	}

/*
 *	Paint either type of shape.
 */

void Shape_frame::paint
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int xoff, int yoff		// Where to show in iwin.
	)
	{
	if (rle)
		paint_rle(win, xoff, yoff);
	else
		win->copy8(data, 8, 8, xoff - 8, yoff - 8);
	}

/*
 *	Show a Run-Length_Encoded shape with translucency.
 */

void Shape_frame::paint_rle_translucent
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int xoff, int yoff,		// Where to show in iwin.
	Xform_palette *xforms,		// Transforms translucent colors
	int xfcnt			// Number of xforms.
	)
	{
	assert(rle);

	int w = get_width(), h = get_height();
	if (w >= 8 || h >= 8)		// Big enough to check?  Off screen?
		if (!win->is_visible(xoff - xleft, 
						yoff - yabove, w, h))
			return;
					// First pix. value to transform.
	const int xfstart = 0xff - xfcnt;
	uint8 *in = data;
	int scanlen;
	while ((scanlen = Read2(in)) != 0)
		{
					// Get length of scan line.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		short scanx = Read2(in);
		short scany = Read2(in);
		if (!encoded)		// Raw data?
			{
			win->copy_line_translucent8(in, scanlen,
					xoff + scanx, yoff + scany,
					xfstart, 0xfe, xforms);
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
				{
				unsigned char pix = *in++;
				if (pix >= xfstart && pix <= 0xfe)
					win->fill_line_translucent8(pix, bcnt,
						xoff + scanx + b, yoff + scany,
						xforms[pix - xfstart]);
				else
					win->fill_line8(pix, bcnt,
					      xoff + scanx + b, yoff + scany);
				}
			else		// Get that # of bytes.
				{
				win->copy_line_translucent8(in, bcnt,
					xoff + scanx + b, yoff + scany,
					xfstart, 0xfe, xforms);
				in += bcnt;
				}
			b += bcnt;
			}
		}
	}

/*
 *	Paint a shape purely by translating the pixels it occupies.  This is
 *	used for invisible NPC's.
 */

void Shape_frame::paint_rle_transformed
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int xoff, int yoff,		// Where to show in iwin.
	Xform_palette& xform		// Use to transform pixels.
	)
	{
	assert(rle);

	int w = get_width(), h = get_height();
	if (w >= 8 || h >= 8)		// Big enough to check?  Off screen?
		if (!win->is_visible(xoff - xleft, 
						yoff - yabove, w, h))
			return;
	uint8 * in = data;
	int scanlen;
	while ((scanlen = Read2(in)) != 0)
		{
					// Get length of scan line.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		short scanx = Read2(in);
		short scany = Read2(in);
		if (!encoded)		// Raw data?
			{		// (Note: 1st parm is ignored).
			win->fill_line_translucent8(0, scanlen,
					xoff + scanx, yoff + scany, xform);
			in += scanlen;
			continue;
			}
		for (int b = 0; b < scanlen; )
			{
			unsigned char bcnt = *in++;
					// Repeat next char. if odd.
			int repeat = bcnt&1;
			bcnt = bcnt>>1; // Get count.
			in += repeat ? 1 : bcnt;
			win->fill_line_translucent8(0, bcnt,
				xoff + scanx + b, yoff + scany, xform);
			b += bcnt;
			}
		}
	}

/*
 *	Paint outline around a shape.
 */

void Shape_frame::paint_rle_outline
	(
	Image_buffer8 *win,		// Buffer to paint in.
	int xoff, int yoff,		// Where to show in win.
	unsigned char color		// Color to use.
	)
	{
	assert(rle);

	int w = get_width(), h = get_height();
	if (w >= 8 || h >= 8)		// Big enough to check?  Off screen?
		if (!win->is_visible(xoff - xleft, 
						yoff - yabove, w, h))
			return;
	int firsty = -10000;		// Finds first line.
	int lasty;
	uint8 * in = data;
	int scanlen;
	while ((scanlen = Read2(in)) != 0)
		{
					// Get length of scan line.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		short scanx = Read2(in);
		short scany = Read2(in);
		int x = xoff + scanx;
		int y = yoff + scany;
		if (firsty == -10000)
			{
			firsty = y;
			lasty = y + h - 1;
			}
					// Put pixel at both ends.
		win->put_pixel8(color, x, y);
		win->put_pixel8(color, x + scanlen - 1, y);

		if (!encoded)		// Raw data?
			{
			if (y == firsty ||	// First line?
			    y == lasty)		// Last line?
				win->fill_line8(color, scanlen, x, y);
			in += scanlen;
			continue;
			}
		for (int b = 0; b < scanlen; )
			{
			unsigned char bcnt = *in++;
					// Repeat next char. if odd.
			int repeat = bcnt&1;
			bcnt = bcnt>>1; // Get count.
			if (repeat)	// Pass repetition byte.
				in++;
			else		// Skip that # of bytes.
				in += bcnt;
			if (y == firsty || 	// First line?
			    y == lasty)		// Last line?
				win->fill_line8(color, bcnt, x + b, y);
			b += bcnt;
			}
		}
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
	if (!rle)			// 8x8 flat?
		{
		return x >= -xleft && x < xright &&
		       y >= -yabove && y < ybelow;
		}
	uint8 *in = data; 		// Point to data.
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
 *	Set new offset, assuming dimensions are unchanged.
 */

void Shape_frame::set_offset
	(
	int new_xright, int new_ybelow
	)
	{
	if (!rle)
		return;			// Can do it for 8x8 tiles.
	int w = get_width(), h = get_height();
	if (new_xright > w)		// Limit to left edge.
		new_xright = w;
	if (new_ybelow > h)
		new_ybelow = h;
	int deltax = new_xright - xright;	// Get changes.
	int deltay = new_ybelow - ybelow;
	xright = new_xright;
	ybelow = new_ybelow;
	xleft = w - xright - 1;		// Update other dims.
	yabove = h - ybelow - 1;
	uint8 *in = data;		// Got to update all scan lines!
	int scanlen;
	while ((scanlen = Read2(in)) != 0)
		{
					// Get length of scan line.
		int encoded = scanlen&1;// Is it encoded?
		scanlen = scanlen>>1;
		short scanx = Read2(in);
		in -= 2;
		Write2(in, scanx + deltax);
		short scany = Read2(in);
		in -= 2;
		Write2(in, scany + deltay);
					// Just need to scan past EOL.
		if (!encoded)		// Raw data?
			in += scanlen;
		else for (int b = 0; b < scanlen; )
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
	}

/*
 *	Create the reflection of a shape.
 */

Shape_frame *Shape::reflect
	(
	vector<DataSource *> shapes,		// shapes data source to read.
	int shapenum,			// Shape #.
	int framenum,			// Frame # without the 'reflect' bit.
	vector<int> counts
	)
	{
					// Get normal frame.
	Shape_frame *normal = get(shapes, shapenum, framenum, counts);
	if (!normal)
		return (0);
					// Reflect it.
	Shape_frame *reflected = normal->reflect();
	if (!reflected)
		return (0);
	framenum |= 32;			// Put back 'reflect' flag.
	if (framenum >= frames_size - 1)// Expand list if necessary.
		enlarge(framenum + 1);
	frames[framenum] = reflected;	// Store new frame.
	return reflected;
	}

/*
 *	Resize list upwards.
 */

void Shape::enlarge
	(
	int newsize
	)
	{
	Shape_frame **newframes = new Shape_frame *[newsize];
	int i;
	for (i = 0; i < frames_size; i++)
		newframes[i] = frames[i];
	frames_size = newsize;
	for ( ; i < frames_size; i++)
		newframes[i] = 0;
	delete [] frames;
	frames = newframes;
	}

/*
 *	Resize list.  This is for outside clients, and it sets num_frames.
 */

void Shape::resize
	(
	int newsize
	)
	{
	if (newsize == frames_size)
		return;
	if (newsize > frames_size)
		enlarge(newsize);	// Growing.
	else
		{			// Shrinking.
		Shape_frame **newframes = new Shape_frame *[newsize];
		int i;
		for (i = 0; i < newsize; i++)
			newframes[i] = frames[i];
					// Delete past new end.
		for ( ; i < frames_size; i++)
			delete frames[i];
		frames_size = newsize;
		delete [] frames;
		frames = newframes;
		}
	num_frames = newsize;
	}

/*
 *	Call this to set num_frames, frames_size and create 'frames' list.
 */

inline void Shape::create_frames_list
	(
	int nframes
	)
	{
	num_frames = frames_size = nframes;
	frames = new Shape_frame *[frames_size];
	memset((char *) frames, 0, frames_size * sizeof(Shape_frame *));
	}

/*
 *	Read in a frame, or convert an existing one if reflection is
 *	desired.
 *
 *	Output:	->frame, or 0 if failed.
 */

Shape_frame *Shape::read
	(
	vector<DataSource *> shapes,	// Shapes data source to read.
	int shapenum,			// Shape #.
	int framenum,			// Frame # within shape.
	vector<int> counts,		// Number of shapes in files.
	int src
	)
	{
	DataSource *shp = 0;
	Shape_frame *frame = new Shape_frame();
					// Figure offset in "shapes.vga".
	uint32 shapeoff = 0x80 + shapenum*8;
	uint32 shapelen = 0;
	
	// Check backwards for the shape file to use.
	int i = counts.size();
	if (src < 0)
		{
		vector<DataSource *>::reverse_iterator it = shapes.rbegin();
		for ( ; it != shapes.rend(); ++it)
			{
			i--;
			if (shapenum < counts[i])
				{
				(*it)->seek(shapeoff);
								// Get location, length.
				int s = (*it)->read4();
				shapelen = (*it)->read4();

				if (s && shapelen)
					{
					shapeoff = s;
					shp = *it;
					break;
					}
				}
			}
		}
	else if (shapenum < counts[src])
		{
		shapes[src]->seek(shapeoff);
						// Get location, length.
		int s = shapes[src]->read4();
		shapelen = shapes[src]->read4();

		if (s && shapelen)
			{
			shapeoff = s;
			shp = shapes[src];
			}
		}
	// The shape was not found anywhere, so leave.
	if (shp == 0)
	{
		std::cerr << "Shape num out of range: " << shapenum << std::endl;
		return 0;
	}
	if (!shapelen)
		return 0;		// Empty shape.
					// Read it in and get frame count.
	int nframes = frame->read(shp, shapeoff, shapelen, framenum);
	if (!num_frames)		// 1st time?
		create_frames_list(nframes);
	if (!frame->rle)
		framenum &= 31;		// !!Guessing.
	if (framenum >= nframes &&	// Compare against #frames in file.
	    (framenum&32))		// Reflection desired?
		{
		delete frame;
		return (reflect(shapes, shapenum, framenum&0x1f, counts));
		}
	return store_frame(frame, framenum);
	}

/*
 *	Write a shape's frames as an entry in an Exult .vga file.  Note that
 *	a .vga file is a .FLX file with one shape/entry.
 *
 *	NOTE:  This should only be called if all frames have been read.
 */

void Shape::write
	(
	ostream& out			// What to write to.
	)
	{
	int frnum;
	if (!num_frames)
		return;			// Empty.
	assert(frames != 0 && *frames != 0);
	bool flat = !frames[0]->is_rle();
					// Save starting position.
	unsigned long startpos = out.tellp();
	
	if (!flat)
		{
		Write4(out, 0);		// Place-holder for total length.
					// Also for frame locations.
		for (frnum = 0; frnum < num_frames; frnum++)
			Write4(out, 0);
		}
	for (frnum = 0; frnum < num_frames; frnum++)
		{
		Shape_frame *frame = frames[frnum];
		assert(frame != 0);	// Better all be the same type.
		assert(flat == !frame->is_rle());
		if (frame->is_rle())
			{		
					// Get position of frame.
			unsigned long pos = out.tellp();
			out.seekp(startpos + (frnum + 1)*4);
			Write4(out, pos - startpos);	// Store pos.
			out.seekp(pos);			// Get back.
			Write2(out, frame->xright);
			Write2(out, frame->xleft);
			Write2(out, frame->yabove);
			Write2(out, frame->ybelow);
			}
		out.write(reinterpret_cast<char *>(frame->data), frame->datalen);	// The frame data.
		}
	if (!flat)
		{
		unsigned long pos = out.tellp();// Ending position.
		out.seekp(startpos);		// Store total length.
		Write4(out, pos - startpos);
		out.seekp(pos);			// And get back to end.
		}
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
	if (framenum >= frames_size)	// Something fishy?
		{
		delete frame;
		cerr << "Shape::store_frame:  framenum >= frames_size ("
			 << framenum << " >= " << (unsigned int)(frames_size)
			 << ")" << endl;
		return (0);
		}
	if (!frames)	// First one?
		{
		frames = new Shape_frame *[num_frames];
		memset((char *) frames, 0, num_frames * sizeof(Shape_frame *));
		}
	frames[framenum] = frame;
	return (frame);
	}

/*
 *	Create with a single frame.
 */

Shape::Shape(Shape_frame* fr)
{
	num_frames = frames_size = 1;
	frames = new Shape_frame*[1];
	frames[0] = fr;
}

/*
 *	Create with space for a given number of frames.
 */

Shape::Shape
	(
	int n				// # frames.
	)
	{
	create_frames_list(n);
	}

void Shape::reset()
	{
	if (frames)
		{
		for(int i = 0; i < frames_size; i++)
			delete frames[i];
		delete [] frames;
		frames = 0;
		}
	else if (frames_size)
		cerr << "Shape::~Shape(): 'frames' is null, while frames_size="
				<< (int) frames_size << endl;
	}

/*
 *	Take frames from another shape and set that shape to empty.
 */

void Shape::take
	(
	Shape *sh2
	)
	{
	reset();			// Clear ourself.
	frames = sh2->frames;
	sh2->frames = 0;
	frames_size = sh2->frames_size;
	num_frames = sh2->num_frames;	
	sh2->num_frames = sh2->frames_size = 0;
	}

/*
 *	Load all frames for a single shape.  (Assumes RLE-type shape.)
 */

void Shape::load
	(
	DataSource* shape_source		// datasource.
	)
	{
	reset();
	Shape_frame *frame = new Shape_frame();
	uint32 shapelen = shape_source->read4();
					// Read frame 0 & get frame count.
	create_frames_list(frame->read(shape_source, 0L, shapelen, 0));
	store_frame(frame, 0);
					// Get the rest.
	for (int i = 1; i < num_frames; i++)
		{
		frame = new Shape_frame();
		frame->read(shape_source, 0L, shapelen, i);
		store_frame(frame, i);
		}
	}

Shape::~Shape()
	{
	reset();
	}

/*
 *	Set desired frame.
 */

void Shape::set_frame
	(
	Shape_frame *frame,		// Must be allocated.
	int framenum
	)
	{
	assert (framenum < num_frames);
	delete frames[framenum];	// Delete existing.
	frames[framenum] = frame;
	}

/*
 *	Add/insert a frame.
 */

void Shape::add_frame
	(
	Shape_frame *frame,		// Must be allocated.
	int framenum			// Insert here.
	)
	{
	assert (framenum <= num_frames);// Can append.
	enlarge(frames_size + 1);	// Make room.
	for (int i = frames_size - 1; i > framenum; i--)
		frames[i] = frames[i - 1];
	frames[framenum] = frame;
	num_frames++;
	}

/*
 *	Delete a frame.
 */

void Shape::del_frame
	(
	int framenum
	)
	{
	assert (framenum < num_frames);
	delete frames[framenum];
					// Shift down.
	for (int i = framenum + 1; i < frames_size; i++)
		frames[i - 1] = frames[i];
	frames[frames_size - 1] = 0;	// Last spot is now free.
	num_frames--;
	}

/*
 * Empty constructor
 */

Shape_file::Shape_file() : Shape()
	{
		// Nothing to see here
	}

/*
 *	Read in all shapes from a single-shape file.
 */

Shape_file::Shape_file
	(
	const char *nm			// Path to file.
	) : Shape()
	{
		load(nm);
	}

/*
 *	Read in all shapes from a single-shape file.
 */

void Shape_file::load
	(
	const char *nm			// Path to file.
	)
	{
	ifstream file;
	U7open(file, nm);
	StreamDataSource shape_source(&file);
	Shape::load(&shape_source);
	}

/*
 *	Read in all shapes from a single-shape file.
 */

Shape_file::Shape_file
	(
	DataSource* shape_source		// datasource.
	) : Shape()
	{
		Shape::load(shape_source);
	}



// NOTE: Only works on shapes other than the special 8x8 tile-shapes
int Shape_file::get_size()
{
	int size = 4;
	for (int i=0; i<num_frames; i++)
		size += frames[i]->get_size() + 4 + 8;
	return size;
}

// NOTE: Only works on shapes other than the special 8x8 tile-shapes
void Shape_file::save(DataSource* shape_source)
{
	int* offsets = new int[num_frames];
	int size;
	offsets[0] = 4 + num_frames * 4;
	int i;	// Blame MSVC
	for (i=1; i<num_frames; i++)
		offsets[i] = offsets[i-1] + frames[i-1]->get_size() + 8;
	size = offsets[num_frames-1] + frames[num_frames-1]->get_size() + 8;
	shape_source->write4(size);
	for (i=0; i<num_frames; i++)
		shape_source->write4(offsets[i]);
	for (i=0; i<num_frames; i++) {
		shape_source->write2(frames[i]->xright);
		shape_source->write2(frames[i]->xleft);
		shape_source->write2(frames[i]->yabove);
		shape_source->write2(frames[i]->ybelow);
		shape_source->write((char*)(frames[i]->data), frames[i]->get_size());
	}
	delete [] offsets;
}

/*
 *	Open file.
 */

Vga_file::Vga_file
	(
	const char *nm,			// Path to file.
	int u7drag,			// # from u7drag.h, or -1.
	const char *nm2			// Patch file, or null.
	) : num_shapes(0), shapes(0), u7drag_type(u7drag), flex(true)
	{
	load(nm, nm2);
	}

Vga_file::Vga_file
	(
	) : num_shapes(0), shapes(0), u7drag_type(-1), flex(true)
	{
		// Nothing to see here !!!
	}

Vga_file::Vga_file
	(
	vector<pair<string, int> > sources,
	int u7drag		// # from u7drag.h, or -1
	) : num_shapes(0), u7drag_type(u7drag), flex(true)
	{
	load(sources);
	}

/*
 *	Open file.
 */

bool Vga_file::load
	(
	const char *nm,
	const char *nm2
	)
	{
	vector<pair<string, int> > src;
	src.push_back(pair<string, int>(string(nm), -1));
	if (nm2 != 0)
		src.push_back(pair<string, int>(string(nm2), -1));
	return load(src);
	}

DataSource *Vga_file::U7load
	(
	pair<string, int> resource,
	vector<ifstream *> &fs,
	vector<DataSource *> &shps
	)
	{
	DataSource *source = 0;
	if (resource.second < 0)
		{
		// It is a file.
		if (U7exists(resource.first))
			{
			ifstream *file = new ifstream();
			U7open(*file, resource.first.c_str());
			source = new StreamDataSource(file);
			fs.push_back(file);
			shps.push_back(source);
			}
		}
	else
		{
		// It is a resource.
		U7object vgaobj(resource.first, resource.second);
		std::size_t len;
		try
			{
			char *buf = vgaobj.retrieve(len);
			source = new BufferDataSource(buf, len);
			}
		catch (std::exception &e)
			{
			CERR("Resource '" << resource.first << "' not found.");
			delete source;
			source = 0;
			}
		if (source)
			shps.push_back(source);
		}
	return source;
	}

bool Vga_file::load
	(
	vector<pair<string, int> > sources
	)
	{
	reset();
	int count = sources.size();
	shape_sources.reserve(count);
	files.reserve(count);
	shape_cnts.reserve(count);
	bool is_good = true;
	if (!U7exists(sources[0].first.c_str()))
		is_good = false;
	for (vector<pair<string, int> >::iterator it = sources.begin();
		it != sources.end(); ++it)
		{
		DataSource *source = U7load(*it, files, shape_sources);
		if (source)
			{
			flex = Flex::is_flex(source);
			if (flex)
				{
				source->seek(0x54);	// Get # of shapes.
				int cnt = source->read4();
				num_shapes = num_shapes > cnt ? num_shapes : cnt;
				shape_cnts.push_back(cnt);
				}
			}
		}
	if (!shape_sources.size())
		throw file_open_exception(get_system_path(sources[0].first));
	if (!flex)			// Just one shape, which we preload.
		{
		CERR("Got here: " << shape_sources.size() << ", " << files.size());
		num_shapes = 1;
		shape_cnts.clear();
		shape_cnts.push_back(1);
		shapes = new Shape[1];
		shapes[0].load(shape_sources[shape_sources.size()-1]);
		return true;
		}
					// Set up lists of pointers.
	shapes = new Shape[num_shapes];
	return is_good;
	}

bool Vga_file::is_shape_imported(int shnum)
	{
	std::map<int, imported_map>::iterator it =
			imported_shape_table.find(shnum);
	return it != imported_shape_table.end();
	}

bool Vga_file::get_imported_shape_data(int shnum, imported_map& data)
	{
	std::map<int, imported_map>::iterator it =
			imported_shape_table.find(shnum);
	if (it != imported_shape_table.end())
		{
		data = (*it).second;
		return true;
		}
	return false;
	}

bool Vga_file::import_shapes
	(
	pair<string, int> source,
	vector<pair<int, int> > imports
	)
	{
	DataSource *ds = U7load(source, imported_files, imported_sources);
	if (ds)
		{
		ds->seek(0x54);	// Get # of shapes.
		int cnt = ds->read4();
		imported_cnts.push_back(cnt);
		flex = Flex::is_flex(ds);
		assert(flex);
		imported_shapes.reserve(imported_shapes.size() + imports.size());
		for (vector<pair<int, int> >::iterator it = imports.begin();
			it != imports.end(); ++it)
			{
			imported_map data = {
					(*it).second,	// The real shape
					imported_shapes.size(),	// The index of the data pointer.
					imported_sources.size() - 1};	// The data source index.
			imported_shape_table[(*it).first] = data;
			imported_shapes.push_back(new Shape());
			}
		return true;
		}
	else
		{
		// Set up the import table anyway.
		for (vector<pair<int, int> >::iterator it = imports.begin();
			it != imports.end(); ++it)
			{
			imported_map data = {(*it).second, -1, -1};
			imported_shape_table[(*it).first] = data;
			}
		}
	return false;
	}

void Vga_file::reset()
	{
	if( shapes )
		delete [] shapes;

	for (vector<ifstream *>::iterator it = files.begin();
			it != files.end(); ++it)
		if ((*it)->is_open())
			(*it)->close();
	files.clear();
	shape_sources.clear();
	shape_cnts.clear();

	num_shapes = 0;
	shapes = 0;
	}

void Vga_file::reset_imports()
	{
	for (vector<ifstream *>::iterator it = imported_files.begin();
			it != imported_files.end(); ++it)
		if ((*it)->is_open())
			(*it)->close();
	imported_files.clear();
	imported_sources.clear();
	imported_cnts.clear();
	imported_shapes.clear();
	imported_shape_table.clear();
	}

Vga_file::~Vga_file()
	{
	reset();
	reset_imports();
	}

/*
 *	Make a spot for a new shape, and delete frames in existing shape.
 *
 *	Output:	->shape, or 0 if invalid shapenum.
 */

Shape *Vga_file::new_shape
	(
	int shapenum
	)
	{
	if (shapenum < 0 || shapenum >= c_max_shapes)
		return 0;
	if (shapenum < num_shapes)
		{
		shapes[shapenum].reset();
		shapes[shapenum].num_frames = shapes[shapenum].frames_size = 0;
		}
	else				// Enlarge list.
		{
		if (!flex)
			return 0;	// 1-shape file.
		Shape *newshapes = new Shape[shapenum + 1];
		for (int i = 0; i < num_shapes; i++)
			newshapes[i].take(&shapes[i]);
		delete [] shapes;
		shapes = newshapes;
		num_shapes = shapenum + 1;
		}
	return &shapes[shapenum];
	}

