/**
 **	Imagebuf.h - A buffer for blitting.
 **
 **	Written: 8/13/98 - JSF
 **/

/*
Copyright (C) 1998 Jeffrey S. Freedman

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/

#ifndef INCL_IMAGEBUF
#define INCL_IMAGEBUF	1


					// Table for translating palette vals.:
typedef unsigned char *Xform_palette;	// Should be 256-bytes.

/*
 *	Here's a generic off-screen buffer.  It's up to the derived classes
 *	to set the data.
 */
class Image_buffer
	{
protected:
	unsigned int width, height;	// Dimensions (in pixels).
	int depth;			// # bits/pixel.
	int pixel_size;			// # bytes/pixel.
	unsigned char *bits;		// Allocated image buffer.
	unsigned int line_width;	// # words/scan-line.
private:
	int clipx, clipy, clipw, cliph; // Clip rectangle.
					// Clip.  Rets. 0 if nothing to draw.
	int clip_internal(int& srcx, int& srcw, int& destx, 
							int clips, int clipl)
		{
		if (destx < clips)
			{
			if ((srcw += (destx - clips)) <= 0)
				return (0);
			srcx -= (destx - clips);
			destx = clips;
			}
		if (destx + srcw > (clips + clipl))
			if ((srcw = ((clips + clipl) - destx)) <= 0)
				return (0);
		return (1);
		}
protected:
	int clip_x(int& srcx, int& srcw, int& destx, int desty)
		{ return desty < clipy || desty >= clipy + cliph ? 0
			: clip_internal(srcx, srcw, destx, clipx, clipw); }
	int clip(int& srcx, int& srcy, int& srcw, int& srch, 
						int& destx, int& desty)
		{
					// Start with x-dim.
		return (clip_internal(srcx, srcw, destx, clipx, clipw) &&
		        clip_internal(srcy, srch, desty, clipy, cliph));
		}
	Image_buffer(unsigned int w, unsigned int h, int dpth);
public:
	friend class Image_buffer8;
	friend class Image_buffer16;
	virtual ~Image_buffer()
		{
		delete [] bits;		// In case Image_window didn't.
		}
	friend class Image_window;
	unsigned char *get_bits()	// Get ->data.
		{ return bits; }
	unsigned int get_width()
		{ return width; }
	unsigned int get_height()
		{ return height; }
	unsigned int get_line_width()
		{ return line_width; }
	void clear_clip()		// Reset clip to whole window.
		{ clipx = clipy = 0; clipw = width; cliph = height; }
					// Set clip.
	void set_clip(int x, int y, int w, int h)
		{
		clipx = x;
		clipy = y;
		clipw = w;
		cliph = h;
		}
					// Is rect. visible within clip?
	int is_visible(int x, int y, int w, int h)
		{ return (!(x >= clipx + clipw || y >= clipy + cliph ||
			x + w <= clipx || y + h <= clipy)); }
	/*
	 *	16-bit color methods.  Default is to ignore them.
	 */
					// Fill with given pixel.
	virtual void fill16(unsigned short pix)
		{  }
					// Fill rect. wth pixel.
	virtual void fill16(unsigned short pix, int srcw, int srch,
						int destx, int desty)
		{  }
					// Copy rectangle into here.
	virtual void copy16(unsigned short *src_pixels,
				int srcw, int srch, int destx, int desty)
		{  }
					// Copy rect. with transp. color.
	virtual void copy_transparent16(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{  }
	/*
	 *	8-bit color methods:
	 */
					// Fill with given (8-bit) value.
	virtual void fill8(unsigned char val) = 0;
					// Fill rect. with pixel.
	virtual void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty) = 0;
					// Fill line with pixel.
	virtual void fill_line8(unsigned char val, int srcw,
						int destx, int desty) = 0;
					// Copy rectangle into here.
	virtual void copy8(unsigned char *src_pixels,
				int srcw, int srch, int destx, int desty) = 0;
					// Copy line to here.
	virtual void copy_line8(unsigned char *src_pixels, int srcw,
						int destx, int desty) = 0;
					// Copy with translucency table.
	virtual void copy_line_translucent8(
		unsigned char *src_pixels, int srcw,
		int destx, int desty, int first_translucent,
		int last_translucent, Xform_palette *xforms) = 0;
					// Apply translucency to a line.
	virtual void fill_line_translucent8(unsigned char val,
		int srcw, int destx, int desty, Xform_palette xform) = 0;
					// Apply translucency to a rectangle
	virtual void fill_translucent8(unsigned char val, int srcw, int srch, 
				int destx, int desty, Xform_palette xform) = 0;
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty) = 0;
	/*
	 *	Depth-independent methods:
	 */
	virtual Image_buffer *create_another(int w, int h) = 0;
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
						int destx, int desty) = 0;
					// Get rect. into another buf.
	virtual void get(Image_buffer *dest, int srcx, int srcy) = 0;
					// Put rect. back.
	virtual void put(Image_buffer *src, int destx, int desty) = 0;
	
	virtual void fill_static(int black, int gray, int white) = 0;

	};

#endif
