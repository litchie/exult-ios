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
#include "utils.h"
#include "rect.h"
#include "imagewin.h"
#include "vgafile.h"

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
 */

Shape_frame *Shape_frame::reflect
	(
	)
	{
	int w = get_width(), h = get_height();
	if (w < h)
		w = h;
	else
		h = w;			// Use max. dim.
	Shape_frame *reflected = new Shape_frame();
	reflected->rle = 1;		// Set data.
	reflected->xleft = yabove;
	reflected->yabove = xleft;
	reflected->xright = ybelow;
	reflected->ybelow = xright;
					// Create drawing area.
	Image_buffer8 *ibuf = new Image_buffer8(h, w);
	ibuf->fill8(255);		// Fill with 'transparent' pixel.
					// Figure origin.
	int xoff = reflected->xleft, yoff = reflected->yabove;
	unsigned char *in = data; 	// Point to data, and draw.
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
					// Create an oversized buffer.
	unsigned char *buf = new unsigned char[w*h*2 + 16*h];
	unsigned char *out = buf;
	int newx;			// Gets new x at end of a scan line.
	for (int y = 0; y < h; y++)	// Go through rows.
#if 0	/* ++++Testing. */
		{
		Write2(out, w << 1);
					// Write position.
		Write2(out, 0 - xleft);
		Write2(out, y - yabove);
		memcpy(out, pixels, w);
		pixels += w;
		out += w;
		}
#else
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
				Write2(out, x - xleft);
				Write2(out, y - yabove);
				memcpy(out, pixels, len);
				pixels += len;
				out += len;
				continue;
				}
					// Encoded, so write it with bit0==1.
			Write2(out, ((newx - x)<<1)|1);
					// Write position.
			Write2(out, x - xleft);
			Write2(out, y - yabove);
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
#endif
	Write2(out, 0);			// End with 0 length.
	int datalen = out - buf;	// Create buffer of correct size.
#ifdef DEBUG
	if(datalen > w*h*2 + 16*h)
		cout << "create_rle: datalen: " << datalen << " w: " << w
			<< " h: " << h << endl;
#endif
	delete [] data;			// Delete old data if there.
	data = new unsigned char[datalen];
	memcpy(data, buf, datalen);
	delete [] buf;
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
	long len			// Length of entire frame data.
	)
	{
	shapes.seekg(filepos);		// Get to extents.
	Check_file(shapes);
	xright = Read2(shapes);
	xleft = Read2(shapes);
	yabove = Read2(shapes);
	ybelow = Read2(shapes);
	len -= 8;			// Subtract what we just read.
	data = new unsigned char[len + 2];	// Allocate and read data.
	shapes.read((char*)data, len);
	Check_file(shapes);
	data[len] = 0;			// 0-delimit.
	data[len + 1] = 0;
	rle = 1;
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
	int w = get_width(), h = get_height();
	if (w >= 8 || h >= 8)		// Big enough to check?  Off screen?
		if (!win->is_visible(xoff - xleft, 
						yoff - yabove, w, h))
			return;
	unsigned char *in = data; // Point to data.
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
			win->copy_line8(in, scanlen,
					xoff + scanx, yoff + scany);
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
				win->fill_line8(pix, bcnt,
					xoff + scanx + b, yoff + scany);
				}
			else		// Get that # of bytes.
				{
				win->copy_line8(in, bcnt,
					xoff + scanx + b, yoff + scany);
				in += bcnt;
				}
			b += bcnt;
			}
		}
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
	int w = get_width(), h = get_height();
	if (w >= 8 || h >= 8)		// Big enough to check?  Off screen?
		if (!win->is_visible(xoff - xleft, 
						yoff - yabove, w, h))
			return;
					// First pix. value to transform.
	const int xfstart = 0xff - xfcnt;
	unsigned char *in = data; // Point to data.
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
 *	Create the reflection of a shape.
 */

Shape_frame *Shape::reflect
	(
	ifstream& shapes,		// "Shapes.vga" file to read.
	int shapenum,			// Shape #.
	int framenum			// Frame # without the 'reflect' bit.
	)
	{
					// Get normal frame.
	Shape_frame *normal = get(shapes, shapenum, framenum);
	if (!normal)
		return (0);
					// Reflect it.
	Shape_frame *reflected = normal->reflect();
	if (!reflected)
		return (0);
	framenum |= 32;			// Put back 'reflect' flag.
	if (framenum >= num_frames - 1)	// Expand list of necessary.
		{
		Shape_frame **newframes = new Shape_frame *[framenum + 1];
		int i;
		for (i = 0; i < num_frames; i++)
			newframes[i] = frames[i];
		num_frames = framenum + 1;
		for ( ; i < num_frames; i++)
			newframes[i] = 0;
		delete [] frames;
		frames = newframes;
		}
	frames[framenum] = reflected;	// Store new frame.
	return reflected;
	}

/*
 *	Read in a frame, or convert an existing one if reflection is
 *	desired.
 *
 *	Output:	->frame, or 0 if failed.
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
#if 0
	if (shapenum == 465)
		cout << "Shape 465 (iolo)" << endl;	//++++++
#endif
					// Read it in and get frame count.
	int nframes = frame->read(shapes, shapeoff, shapelen, framenum);
	if (!num_frames)		// 1st time?
		num_frames = nframes;
	if (framenum >= nframes &&	// Compare against #frames in file.
	    (framenum&32))		// Reflection desired?
		{
		delete frame;
		return (reflect(shapes, shapenum, framenum&0x1f));
		}
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

Shape::~Shape()
	{
	for(int i = 0; i < num_frames; i++)
		delete frames[i];
	delete [] frames;
	}

/*
 *	Read in all shapes from a single-shape file.
 */

Shape_file::Shape_file
	(
	const char *nm			// Path to file.
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
	const char *nm			// Path to file.
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

Vga_file::~Vga_file()
	{
	delete [] shapes;
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
		tfa.read((char*)&info[i].tfa[0], 3);
		info[i].set_tfa_data();
		}
	ifstream ready;
	if (!U7open(ready, READY))
		return (0);
	int cnt = Read1(ready);		// Get # entries.
	for (int i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(ready);
		unsigned char type = Read1(ready);
		info[shapenum].ready_type = type;
		ready.seekg(6, ios::cur);// Skip 9 bytes.
		}
	ready.close();
#if 1	/* ++++Uncomment when tested. */
	ifstream armor;
	if (!U7open(armor, ARMOR))
		return (0);
	cnt = Read1(armor);
	for (int i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(armor);
		unsigned char points = Read1(armor);
		info[shapenum].armor = points;
		armor.seekg(7, ios::cur);// Skip 7 bytes.
		}
	armor.close();
	ifstream weapon;
	if (!U7open(weapon, WEAPONS))
		return (0);
	cnt = Read1(weapon);
	for (int i = 0; i < cnt; i++)
		{
		unsigned short shapenum = Read2(weapon);
		short points = Read2(weapon);
		unsigned short ammoshape = Read2(weapon);
		if (ammoshape == shapenum)
			ammoshape = 0;
		else if (ammoshape == points)
			points = 0;
		weapon.seekg(15, ios::cur);	// Skip unknown.
		info[shapenum].weapon = new Weapon_info(-points, ammoshape);
		}
	weapon.close();	
#endif
	// Load data about drawing the weapon in an actor's hand
	ifstream wihh;
	unsigned short offsets[1024];
	if (!U7open(wihh, WIHH))
		return (0);
	for (int i = 0; i < 1024; i++)
		offsets[i] = Read2(wihh);
	for (int i = 0; i < 1024; i++)
		// A zero offset means there is no record
		if(offsets[i] == 0)
			info[i].weapon_offsets = 0;
		else
			{
			wihh.seekg(offsets[i]);
			// There are two bytes per frame: 64 total
			info[i].weapon_offsets = new unsigned char[64];
			for(int j = 0; j < 32; j++)
				{
				unsigned char x = Read1(wihh);
				unsigned char y = Read1(wihh);
				// Set x/y to 255 if weapon is not to be drawn
				// In the file x/y are either 64 or 255:
				// I am assuming that they mean the same
				if(x > 63 || y > 63)
					x = y = 255;
				info[i].weapon_offsets[j * 2] = x;
				info[i].weapon_offsets[j * 2 + 1] = y;
				}
			}
	return (1);
	}

Shape_info::~Shape_info()
	{
	delete weapon;
	if(weapon_offsets)
		delete [] weapon_offsets;
	}

Shapes_vga_file::~Shapes_vga_file()
	{
	}

