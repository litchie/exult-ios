/**
 **	Imagewin.h - A window to blit images into.
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

#ifndef INCL_IMAGEWIN
#define INCL_IMAGEWIN	1

#if !(defined(XWIN) || defined(DOS))

#define __WIN32
#include <windows.h>
#include <windowsx.h>

#endif


class Font_face;

/*
 *	Here's a generic off-screen buffer.  It's up to the derived classes
 *	to set the data.
 */
class Image_buffer_base
	{
protected:
	unsigned int width, height;	// Dimensions (in pixels).
	int depth;			// # bits/pixel.
	int pixel_size;			// # bytes/pixel.
	char *bits;			// Allocated image buffer.
	unsigned int line_width;	// # words/scan-line.

	int clipx, clipy, clipw, cliph; // Clip rectangle.
					// Clip.  Rets. 0 if nothing to draw.
	int clip(int& srcx, int& srcy, int& srcw, int& srch, 
							int& destx, int& desty)
		{
		if (destx < clipx)
			{
			if ((srcw += (destx - clipx)) <= 0)
				return (0);
			srcx -= (destx - clipx);
			destx = clipx;
			}
		if (desty < clipy)
			{
			if ((srch += (desty - clipy)) <= 0)
				return (0);
			srcy -= (desty - clipy);
			desty = clipy;
			}
		if (destx + srcw > (clipx + clipw))
			if ((srcw = ((clipx + clipw) - destx)) <= 0)
				return (0);
		if (desty + srch > (clipy + cliph))
			if ((srch = ((clipy + cliph) - desty)) <= 0)
				return (0);
		return (1);
		}
	Image_buffer_base(unsigned int w, unsigned int h, int dpth);
public:
	~Image_buffer_base()
		{
		delete bits;		// In case Image_window didn't.
		}
	friend class Image_window;
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
	virtual void copy16(unsigned short *src_pixels, int srcw, int srch,
						int destx, int desty)
		{  }
					// Copy rect. with transp. color.
	virtual void copy_transparent16(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{  }
	/*
	 *	8-bit color methods:
	 */
					// Set palette.
	virtual void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100) = 0;
					// Fill with given (8-bit) value.
	virtual void fill8(unsigned char val) = 0;
					// Fill rect. wth pixel.
	virtual void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty) = 0;
					// Copy rectangle into here.
	virtual void copy8(unsigned char *src_pixels, int srcw, int srch,
						int destx, int desty) = 0;
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty) = 0;
	/*
	 *	Depth-independent methods:
	 */
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
						int destx, int desty) = 0;
					// Open a (truetype) font file.
	static Font_face *open_font(char *fname, int points);
	static void close_font(Font_face *font);// Close font.
					// Draw text in given rectangle.
	void draw_text_box(Font_face *font, char *text,
						int x, int y, int w, int h);
					// Draw text at given point.
	int draw_text(Font_face *font, char *text, int x, int y);
	int draw_text(Font_face *font, char *text, int textlen, int x, int y);
					// Draw a char. at given point.
	int draw_char(Font_face *font, short chr, int x, int y);
					// Get width of text.
	static int get_text_width(Font_face *font, char *text);
	static int get_text_width(Font_face *font, char *text, int textlen);
					// Get height.
	static int get_text_height(Font_face *font);
	};

/*
 *	An 8-bit image buffer:
 */
class Image_buffer8 : public Image_buffer_base
	{
public:
	Image_buffer8(unsigned int w, unsigned int h)
		: Image_buffer_base(w, h, 8)
		{  }
	/*
	 *	Depth-independent methods:
	 */
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
							int destx, int desty);
	/*
	 *	8-bit color methods:
	 */
					// Set palette.
	virtual void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100)
		{  }
					// Fill with given (8-bit) value.
	virtual void fill8(unsigned char val);
					// Fill rect. wth pixel.
	virtual void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty);
					// Copy rectangle into here.
	virtual void copy8(unsigned char *src_pixels, int srcw, int srch,
						int destx, int desty);
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty);
	};

/*
 *	A 16-bit image buffer:
 */
class Image_buffer16 : public Image_buffer_base
	{
	unsigned short *palette;	// Palette for 8-bit graphics.
	void create_default_palette();
	unsigned short *get_pixels()	// Cast bits.
		{ return (unsigned short *) bits; }
public:
	Image_buffer16(unsigned int w, unsigned int h, int dpth)
		: Image_buffer_base(w, h, dpth), palette(0)
		{
		create_default_palette();
		}
	~Image_buffer16()
		{
		delete palette;
		}
					// Create pixel from r,g,b values.
	unsigned short rgb(unsigned short r, unsigned short g,
							unsigned short b)
		{ return ((r&0x1f) << 11) + ((g&0x1f) << 6) + (b&0x1f); }
					// Set a palette color.
	void set_palette_color(unsigned char num,
		unsigned short r, unsigned short g, unsigned short b)
		{
		palette[num] = rgb(r, g, b);
		}
	/*
	 *	16-bit color methods.
	 */
					// Fill with given pixel.
	virtual void fill16(unsigned short pix);
					// Fill rect. wth pixel.
	virtual void fill16(unsigned short pix, int srcw, int srch,
						int destx, int desty);
					// Copy rectangle into here.
	virtual void copy16(unsigned short *src_pixels, int srcw, int srch,
						int destx, int desty);
	/*
	 *	Depth-independent methods:
	 */
					// Copy within itself.
	virtual void copy(int srcx, int srcy, int srcw, int srch, 
							int destx, int desty);
	/*
	 *	8-bit color methods:
	 */
					// Set palette.
	virtual void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100);
					// Fill with given (8-bit) value.
	virtual void fill8(unsigned char val)
		{ Image_buffer16::fill16(palette[val]); }
					// Fill rect. wth pixel.
	virtual void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty)
		{ Image_buffer16::fill16(
				palette[val], srcw, srch, destx, desty); }
					// Copy rectangle into here.
	virtual void copy8(unsigned char *src_pixels, int srcw, int srch,
						int destx, int desty);
					// Copy rect. with transp. color.
	virtual void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty);
	};

/*
 *	Here's the top-level class to use for image buffers.  Image_window
 *	should be derived from it.
 */
class Image_buffer
	{
protected:
	Image_buffer_base *ibuf;	// Where the data is actually stored.
public:
	Image_buffer(unsigned int w, unsigned int h, int dpth);
	~Image_buffer()
		{ delete ibuf; }
	/*
	 *	16-bit color methods.
	 */
					// Fill with given pixel.
	void fill16(unsigned short pix)
		{ ibuf->fill16(pix); }
					// Fill rect. wth pixel.
	void fill16(unsigned short pix, int srcw, int srch,
						int destx, int desty)
		{ ibuf->fill16(pix, srcw, srch, destx, desty); }
					// Copy rectangle into here.
	void copy16(unsigned short *src_pixels, int srcw, int srch,
						int destx, int desty)
		{ ibuf->copy16(src_pixels, srcw, srch, destx, desty); }
					// Copy rect. with transp. color.
	void copy_transparent16(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{ ibuf->copy_transparent16(src_pixels, srcw, srch,
							destx, desty); }
	/*
	 *	8-bit color methods:
	 */
					// Set palette.
	void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100)
		{ ibuf->set_palette(rgbs, maxval, brightness); }
					// Fill with given (8-bit) value.
	void fill8(unsigned char val)
		{ ibuf->fill8(val); }
					// Fill rect. wth pixel.
	void fill8(unsigned char val, int srcw, int srch,
						int destx, int desty)
		{ ibuf->fill8(val, srcw, srch, destx, desty); }
					// Copy rectangle into here.
	void copy8(unsigned char *src_pixels, int srcw, int srch,
						int destx, int desty)
		{ ibuf->copy8(src_pixels, srcw, srch, destx, desty); }
					// Copy rect. with transp. color.
	void copy_transparent8(unsigned char *src_pixels, int srcw,
					int srch, int destx, int desty)
		{ ibuf->copy_transparent8(src_pixels, srcw, srch,
							destx, desty); }
	/*
	 *	Depth-independent methods:
	 */
	void clear_clip()		// Reset clip to whole window.
		{ ibuf->clear_clip(); }
					// Set clip.
	void set_clip(int x, int y, int w, int h)
		{ ibuf->set_clip(x, y, w, h); }
					// Copy within itself.
	void copy(int srcx, int srcy, int srcw, int srch, 
						int destx, int desty)
		{ ibuf->copy(srcx, srcy, srcw, srch, destx, desty); }
					// Open a (truetype) font file.
	static Font_face *open_font(char *fname, int points)
		{ return Image_buffer_base::open_font(fname, points); }
					// Close font.
	static void close_font(Font_face *font)
		{ Image_buffer_base::close_font(font); }
					// Draw text in given rectangle.
	void draw_text_box(Font_face *font, char *text,
						int x, int y, int w, int h)
		{ ibuf->draw_text_box(font, text, x, y, w, h); }
					// Draw text at given point.
	int draw_text(Font_face *font, char *text, int x, int y)
		{ return ibuf->draw_text(font, text, x, y); }
	int draw_text(Font_face *font, char *text, int textlen, int x, int y)
		{ return ibuf->draw_text(font, text, textlen, x, y); }
					// Draw a char. at given point.
	int draw_char(Font_face *font, short chr, int x, int y)
		{ return ibuf->draw_char(font, chr, x, y); }
					// Get width of text.
	static int get_text_width(Font_face *font, char *text)
		{ return Image_buffer_base::get_text_width(font, text); }
	static int get_text_width(Font_face *font, char *text, int textlen)
		{ return Image_buffer_base::get_text_width(
							font, text, textlen); }
					// Get height.
	static int get_text_height(Font_face *font)
		{ return Image_buffer_base::get_text_height(font); }
	};

#ifdef XWIN
#include <X11/Xlib.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

/*
 *	Here's a window for showing and manipulating images in X.
 */
class Image_window : public Image_buffer
	{
	int shm;			// 1 if using MITSHM.
	XShmSegmentInfo shm_info;	// Shared memory info.
	static Display *display;	// X-windows display.
	Window win;			// The window handle.
	GC gc;				// Graphics context for drawing.
	Visual *visual;			// Color characteristics.
	Colormap colormap;		// The palette for 8-bit displays.
	/*
	 *	Image info.
	 */
	XImage *image;			// Image containing 'bits'.
	void free_image();		// Free image.
public:
					// You MUST call this first.
	static void set_display(Display *dpy)
		{ display = dpy; }
	static Display *get_display()
		{ return display; }
	Image_window(unsigned int w, unsigned int h);
	~Image_window();
	Window get_win()		// Get handle.
		{ return win; }
	GC get_gc()
		{ return gc; }
	unsigned int get_width()	// Get dims.
		{ return ibuf->width; }
	unsigned int get_height()
		{ return ibuf->height; }
	int ready()			// Ready to draw?
		{ return (ibuf->bits != 0); }
					// Set title.
	void set_title(char *title)
		{
		XStoreName(display, win, title);
		}
					// Indicate events we want.
	void select_input(unsigned int mask)
		{
		XSelectInput(display, win, mask);
		}
	void map()			// Display it.
		{
		XMapWindow(display, win);
		}
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int newh);
	void show();			// Repaint entire window.
					// Set palette.
	void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100);
	};
#endif	/* XWIN */

#ifdef DOS

#include <dpmi.h>
#include <pc.h>
#include <string.h>

typedef long Window;			// For Window ID's.

/*
 *	Here's a window for showing and manipulating images in DOS.
 */
class Image_window : public Image_buffer
	{
	static __dpmi_regs regs;	// Registers for DPMI functions.
	static unsigned long win_count; // For assigning window ID's.
	Window win;			// The window handle.
	static void clear_registers();	// Clear regs.
public:
	Image_window(unsigned int w, unsigned int h);
	~Image_window()
		{  }
	Window get_win()		// Get handle.
		{ return win; }
	unsigned int get_width()	// Get dims.
		{ return ibuf->width; }
	unsigned int get_height()
		{ return ibuf->height; }
	int ready()			// Ready to draw?
		{ return 1; }
	void set_title(char *title)	// Set title (ignored).
		{  }
	void map()			// Always mapped in DOS.
		{  }
					// For now, ignore resizes.
	void resized(unsigned int neww, unsigned int newh)
		{  }
	void show();			// Repaint entire window.
					// Set palette.
	void set_palette(unsigned char *rgbs, int maxval, 
						int brightness = 100);
	/*
	 *	Dos-specific methods:
	 */
	static void set_display();	// You MUST call this first.
	static void restore_display();	// You MUST call this at end.
	static int init_mouse();	// Init. mouse.	 Returns 0 if error.
	static int show_mouse(int on)	// Turn on/off mouse cursor.
		{
		clear_registers();
		regs.x.ax = on ? 1 : 2;;
		__dpmi_int(0x33, &regs);
		}
					// Get mouse position.
	static void get_mouse(int& x, int& y)
		{
		clear_registers();
		regs.x.ax = 3;
		__dpmi_int(0x33, &regs);
		x = regs.x.cx >> 1;
		y = regs.x.dx;
		}
					// Get mouse buttons (1, 2, or 3).
	static int get_mouse_buttons()
		{
		clear_registers();
		regs.x.ax = 3;
		__dpmi_int(0x33, &regs);
		if (regs.x.bx >= 1 && regs.x.bx <= 3)
			return (regs.x.bx);
		else
			return (0);	// No button down.
		}
					// Set mouse position.
	static void set_mouse(int x, int y)
		{
		clear_registers();
		regs.x.ax = 4;
		regs.x.cx = x << 1;
		regs.x.dx = y;
		__dpmi_int(0x33, &regs);
		}
	};

//	Blit to screen.
inline void Image_window::show()
	{
	show_mouse(0);		// Turn off mouse before blitting.
	dosmemput(ibuf->bits, 64000, 0xa0000);
	show_mouse(1);
	}


#endif	/* DOS	*/


#ifdef __WIN32

typedef struct {
      BITMAPINFOHEADER bmiHeader;
      RGBQUAD bmiColors[256];
} MyBITMAPINFO;

typedef struct {
      WORD palVersion;
      WORD palNumEntries;
      PALETTEENTRY palPalEntry[256];
} MyLOGPALETTE;


typedef HWND Window;
typedef long Display;

class Image_window : public Image_buffer {
  private:
    MyBITMAPINFO *BMInfo;
    MyLOGPALETTE *PalInfo;

  public:
    int img_width, img_height;
    HBITMAP BMHan;
    HDC ScreenDC;

	  Window win;			// The window handle.
	Image_window(unsigned int w, unsigned int h);
	~Image_window();
	Window get_win()		// Get handle.
		{ return win; }
	unsigned int get_width()	// Get dims.
		{ return ibuf->width; }
	unsigned int get_height()
		{ return ibuf->height; }
	int ready()			// Ready to draw?
		{ return (ibuf->bits != 0); }
					// Set title.
	void set_title(char *title) {
	SetWindowText(win, title);
		}
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int newh) {};
	void show();			// Repaint entire window.
					// Set palette.
	void set_palette(unsigned char *rgbs, int maxval,
						int brightness = 100);
  void map() { }
};

#endif //__WIN32

#endif	/* INCL_IMAGEWIN	*/
