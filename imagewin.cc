/**
 **	Imagewin.cc - A window to blit images into.
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

#include "imagewin.h"
#include <string.h>
#include <iostream.h>

/*
 *	Create buffer.
 */

Image_buffer_base::Image_buffer_base
	(
	unsigned int w,			// Desired width, height.
	unsigned int h,
	int dpth			// Depth (bits/pixel).
	) : width(w), height(h), depth(dpth),
	    clipx(0), clipy(0),
	    clipw(w), cliph(h), bits(0), line_width(w)
	{
	switch (depth)			// What depth?
		{
	case 8: 
		pixel_size = 1; break;
	case 15:
	case 16:
		pixel_size = 2; break;
	case 32:
		pixel_size = 4; break;
		}
	}


/*
 *	Copy an area of the image within itself.
 */

void Image_buffer8::copy
	(
	int srcx, int srcy,		// Where to start.
	int srcw, int srch,		// Dimensions to copy.
	int destx, int desty		// Where to copy to.
	)
	{
	int ynext, yfrom, yto;		// Figure y stuff.
	if (srcy >= desty)		// Moving up?
		{
		ynext = line_width;
		yfrom = srcy;
		yto = desty;
		}
	else				// Moving down.
		{
		ynext = -line_width;
		yfrom = srcy + srch - 1;
		yto = desty + srch - 1;
		}
	char *to = bits + yto*line_width + destx;
	char *from = bits + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		memmove((char *) to, (char *) from, srcw);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Fill with a given 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix
	)
	{
	char *pixels = bits;
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with an 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer8::copy8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer8::copy_transparent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = chr;
			}
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Create a default palette for 8-bit graphics on a 16-bit display.
 */

void Image_buffer16::create_default_palette
	(
	)
	{
	delete palette;			// Delete old.
	palette = new unsigned short[256];
	for (int i = 0; i < 256; i++)
		palette[i] = rgb(4*((i >> 5) & 0x7), 
			4*((i >> 2) & 0x7), 9*(i & 0x3));
	}

/*
 *	Fill with a given 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix
	)
	{
	unsigned short *pixels = get_pixels();
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with a 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *pixels = get_pixels() + desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy16
	(
	unsigned short *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned short *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy an area of the image within itself.
 */

void Image_buffer16::copy
	(
	int srcx, int srcy,		// Where to start.
	int srcw, int srch,		// Dimensions to copy.
	int destx, int desty		// Where to copy to.
	)
	{
	int ynext, yfrom, yto;		// Figure y stuff.
	if (srcy >= desty)		// Moving up?
		{
		ynext = line_width;
		yfrom = srcy;
		yto = desty;
		}
	else				// Moving down.
		{
		ynext = -line_width;
		yfrom = srcy + srch - 1;
		yto = desty + srch - 1;
		}
	unsigned short *to = get_pixels() + yto*line_width + destx;
	unsigned short *from = get_pixels() + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		memmove((char *) to, (char *) from, srcw * 2);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Convert rgb value.
 */

inline unsigned char Get_color16
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned int c = (((unsigned int) val)*brightness*32)/
							(100*(maxval + 1));
	return (c < 32 ? c : 31);
	}

/*
 *	Set palette.
 */

void Image_buffer16::set_palette
	(
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
					// Get the colors.
	for (int i = 0; i < 3*256; i += 3)
		{
		unsigned char r = Get_color16(rgbs[i], maxval, brightness);
		unsigned char g = Get_color16(rgbs[i + 1], maxval, brightness);
		unsigned char b = Get_color16(rgbs[i + 2], maxval, brightness);
		set_palette_color(i/3, r, g, b);
		}
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = palette[*from++];
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer16::copy_transparent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = palette[chr];
			}
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Create top-level image buffer.
 */

Image_buffer::Image_buffer
	(
	unsigned int w,			// Desired width, height.
	unsigned int h,
	int dpth			// Color depth (bits/pixel).
	)
	{
	switch (dpth)			// What depth?
		{
	case 8: 
		ibuf = new Image_buffer8(w, h);
		break;
	case 15:
	case 16:
		ibuf = new Image_buffer16(w, h, dpth);
		break;
	case 32:
	default:
		ibuf = 0;		// Later.
		}
	}

#ifdef XWIN
#include <X11/Xutil.h>
Display *Image_window::display = 0;	// The X-windows display.

/*
 *	Create window.
 */

Image_window::Image_window
	(
	unsigned int w,			// Desired width, height.
	unsigned int h
	) : Image_buffer(w, h, DisplayPlanes(display, DefaultScreen(display))),
	    image(0), colormap(0)
	{
	int screen_num = DefaultScreen(display);
					// Get color display type.
	visual = DefaultVisual(display, screen_num);
					// Shared memory available?
	int shm_major,shm_minor, shm_pixmaps, shm_completion;
	shm = XShmQueryVersion (display, &shm_major, &shm_minor, &shm_pixmaps);
	XSetWindowAttributes atts;	// Set up attributes.
	atts.border_pixel = BlackPixel(display, screen_num);
	atts.background_pixel = WhitePixel(display, screen_num);
	win = XCreateWindow(display, 
		RootWindow(display, screen_num), 0, 0, ibuf->width, 
								ibuf->height,
		4,			// Border width.
		DisplayPlanes(display, screen_num),
		CopyFromParent, CopyFromParent, CWBackPixel | CWBorderPixel,
		&atts);
					// Get graphics context.
	XGCValues gcvalues;
	gc = XCreateGC(display, win, 0L, &gcvalues);
					// Set foreground color for drawing.
	XSetForeground(display, gc, BlackPixel(display, screen_num));
	}

/*
 *	Destroy window.
 */

Image_window::~Image_window
	(
	)
	{
	XFreeGC(display, gc);
	free_image();
	}

/*
 *	Free the image.
 */

void Image_window::free_image
	(
	)
	{
	if (!image)
		return;
	if (shm)			// MIT shared mem?
		{			// Detach memory.
		XShmDetach(display, &shm_info);
					// Mark for deletion.
		shmctl(shm_info.shmid, IPC_RMID, 0);
		shmdt(shm_info.shmaddr);
		}
	else
		delete ibuf->bits;
	ibuf->bits = image->data = 0;
	XDestroyImage(image);
	image = 0;
	}

/*
 *	Window was resized.
 */

void Image_window::resized
	(
	unsigned int neww, 
	unsigned int newh
	)
	{
	if (image)
		{
		if (neww == ibuf->width && newh == ibuf->height)
			return;		// Nothing changed.
		free_image();		// Delete old image.
		}
	ibuf->width = neww;
	ibuf->height = newh;
	if (shm)
		{
		image = XShmCreateImage(display, 0, 
			ibuf->depth, ZPixmap, 0,
			&shm_info, ibuf->width, ibuf->height);
					// Get shared memory.
		shm_info.shmid = shmget(IPC_PRIVATE,
				image->bytes_per_line * image->height,
				IPC_CREAT|0777);
		if (shm_info.shmid < 0 ||
		    (shm_info.shmaddr = (char *) shmat(shm_info.shmid, 0, 0))
							== ((char *) -1))
			shm = 0;
		else
			{
			ibuf->bits = image->data = shm_info.shmaddr;
			shm_info.readOnly = False;
			XShmAttach(display, &shm_info);
			cout << "Using MIT shared memory ext.\n";
			}
		}
	if (!shm)
		{			// Create new image.
		int line_size = ibuf->width * ibuf->pixel_size;
		if (line_size & 3)	// Round up to nearest 32-bits.
			line_size += (4 - (line_size & 3));
		ibuf->bits = new char[line_size * ibuf->height];
		image = XCreateImage(display, 
			visual,
			ibuf->depth,
			ZPixmap, 0, ibuf->bits, ibuf->width, ibuf->height,
			32, line_size);
		}
					// Update line size in words.
	ibuf->line_width = image->bytes_per_line/ibuf->pixel_size;
	cout << "Bytes_per_line = " << image->bytes_per_line <<
			" Width = " << ibuf->width << '\n';
	}

/*
 *	Repaint window.
 */

void Image_window::show
	(
	)
	{
	if (!ready())
		return;
	if (shm)
		XShmPutImage(display, win, gc, image, 0, 0, 0, 0,
				ibuf->width, ibuf->height, True);
	else
		XPutImage(display, win, gc, image, 0, 0, 0, 0, 
					ibuf->width, ibuf->height);
	}

/*
 *	Convert rgb value.
 */

inline unsigned short Get_color8
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned long c = (((unsigned long) val)*brightness*65535L)/
							(100*(maxval + 1));
	return (c <= 65535L ? (unsigned short) c : 65535);
	}

/*
 *	Set palette.
 */

void Image_window::set_palette
	(
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
	if (ibuf->depth != 8)		// Need to handle in software?
		{
		Image_buffer::set_palette(rgbs, maxval, brightness);
		return;
		}
	XColor colors[256];
					// Get the colors.
	for (int i = 0; i < 256; i++)
		{
		colors[i].pixel = i;
		colors[i].flags = DoRed | DoGreen | DoBlue;
		colors[i].red = Get_color8(rgbs[3*i], maxval, brightness);
		colors[i].green = Get_color8(rgbs[3*i + 1], maxval,
							brightness);
		colors[i].blue = Get_color8(rgbs[3*i + 2], maxval,
							brightness);
		}
	if (!colormap)			// First time?
		{
		colormap = XCreateColormap(display, win, visual, AllocAll);
		XSetWindowColormap(display, win, colormap);
		}
	XStoreColors(display, colormap, colors, 256);
	}
#endif	/* XWIN */

#ifdef DOS

/**
 **	Note:  Much of this code was adapted from the EZVGA library.
 **/

__dpmi_regs Image_window::regs;		// Registers for DPMI functions.
unsigned long Image_window::win_count = 0;

/*
 *	Clear all the registers.
 */

void Image_window::clear_registers
	(
	)
	{
	memset((char *) &regs, 0, sizeof(regs));
	}

/*
 *	Create window.	The first one is always 320x200 for the whole screen.
 */

Image_window::Image_window
	(
	unsigned int w,			// Desired width, height.
	unsigned int h
	) : Image_buffer(win_count ? w : 320, win_count ? h : 200, 8)
	{
	win = win_count++;		// Assign unique ID.
	ibuf->bits = new char[w*h];	// Initialize buffer.
	}

/*
 *	Convert rgb value.
 */

inline unsigned short Get_color8
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned long c = (((unsigned long) val)*brightness)/
							(100);
	return (c <= 63 ? c : 63);
	}

/*
 *	Set palette.
 */

void Image_window::set_palette
	(
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
					// Get the colors.
	for (int i = 0; i < 256; i++)
		{
		outportb(0x03c8, i);
		outportb(0x03c9, Get_color8(rgbs[(i << 1) + i],
						maxval, brightness));
		outportb(0x03c9, Get_color8(rgbs[(i << 1) + i + 1],
						maxval, brightness));
		outportb(0x03c9, Get_color8(rgbs[(i << 1) + i + 2],
						maxval, brightness));
		}
	}

/*
 *	Set up the display.
 */

void Image_window::set_display
	(
	)
	{
	clear_registers();
	regs.x.ax = 0x13;
	__dpmi_int(0x10, &regs);
	}

/*
 *	Restore display.
 */

void Image_window::restore_display
	(
	)
	{
	clear_registers();
	regs.x.ax = 0x03;
	__dpmi_int(0x10, &regs);
	}

/*
 *	Initialize the mouse.
 *
 *	Output: 0 if no mouse.
 */

int Image_window::init_mouse
	(
	)
	{
	clear_registers();
	regs.x.ax = 0;
	__dpmi_int(0x33, &regs);
	return (regs.x.ax == 0xffff);
	}


#endif	/* DOS	*/

#ifdef __WIN32

HWND InitInstance(int, int); //in exult.cpp

//constructor
Image_window::Image_window (unsigned int w, unsigned int h):
  Image_buffer(w, h, 8) {

  img_width = w;
  img_height = h;

  win = InitInstance(2*w, 2*h); //create and show window

  HWND ActiveWindow = win;

  HDC ScrDC = GetDC ( ActiveWindow );
  BMInfo = new MyBITMAPINFO;
  PalInfo = new MyLOGPALETTE;

  HPALETTE PalHan;

  //set bitmap parameters
  BMInfo->bmiHeader.biSize = sizeof ( BITMAPINFOHEADER );
  BMInfo->bmiHeader.biWidth = img_width;
  BMInfo->bmiHeader.biHeight = -img_height;  //top-down
  BMInfo->bmiHeader.biPlanes = 1;
  BMInfo->bmiHeader.biBitCount = 8;
  BMInfo->bmiHeader.biCompression = BI_RGB;
  BMInfo->bmiHeader.biSizeImage = NULL;
  BMInfo->bmiHeader.biXPelsPerMeter = NULL;
  BMInfo->bmiHeader.biYPelsPerMeter = NULL;
  BMInfo->bmiHeader.biClrUsed = 256;
  BMInfo->bmiHeader.biClrImportant = 256;

  //load palette (all black for now)
	for (int i = 0; i < 256; i ++) {
		unsigned char r = 0;//Get_color32(rgbs[3*i], maxval, brightness);
		unsigned char g = 0;//Get_color32(rgbs[3*i + 1], maxval, brightness);
		unsigned char b = 0;//Get_color32(rgbs[3*i + 2], maxval, brightness);
    PalInfo->palPalEntry[i].peRed = r;
    PalInfo->palPalEntry[i].peGreen = g;
    PalInfo->palPalEntry[i].peBlue = b;
    PalInfo->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
    BMInfo->bmiColors[i].rgbBlue = b;
    BMInfo->bmiColors[i].rgbGreen = g;
    BMInfo->bmiColors[i].rgbRed = r;
	}

  //realize palette
  PalHan = CreatePalette ( (tagLOGPALETTE*) PalInfo );
  SelectPalette ( ScrDC, PalHan, FALSE );
  RealizePalette ( ScrDC );

  //create Device Independent Bitmap
  BMHan =
    CreateDIBSection ( ScrDC, (BITMAPINFO*) BMInfo, DIB_RGB_COLORS,
      (void**) &(ibuf->bits), NULL, NULL);

  ReleaseDC ( ActiveWindow, ScrDC );


}

Image_window::~Image_window () {
//  delete[] (ibuf->bits);

  DeleteObject(BMHan); //free bitmap
  BMHan = 0;

//  delete ibuf;  //???

  delete BMInfo;
  delete PalInfo;

}

//show buffer on screen
void Image_window::show () {
  if (!ready())
    return;

  int cwidth, cheight;


  HWND ActiveWindow = win;
  HDC ScrDC;

  RECT ClientRect;
  GetClientRect(ActiveWindow, &ClientRect);
  cwidth = ClientRect.right;
  cheight = ClientRect.bottom;

  if ((ScrDC = ScreenDC) == NULL)
    ScrDC = GetDC ( ActiveWindow );

  HDC Context = CreateCompatibleDC ( 0 );
  HBITMAP DefaultBitmap = SelectBitmap ( Context, BMHan );

  //TO DO: get dimensions

  StretchBlt ( ScrDC , 0, 0, cwidth /*ClientWidth*/, cheight /*ClientHeight*/,
      Context, 0, 0, img_width, img_height, SRCCOPY); //blt to screen

  SelectBitmap ( Context, DefaultBitmap );
  DeleteDC ( Context );

  if (ScreenDC == NULL)
    ReleaseDC ( ActiveWindow, ScrDC );
}

inline unsigned char Get_color32
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned int c = (((unsigned int) val)*brightness*256)/
							(100*(maxval + 1));
	return (c < 256 ? c : 255);
	}

void Image_window::set_palette(unsigned char* rgbs, int maxval, int brightness) {
  Image_buffer::set_palette(rgbs, maxval, brightness);

  HDC Context = CreateCompatibleDC ( 0 );
  HBITMAP DefaultBitmap = SelectBitmap ( Context, BMHan );

  RGBQUAD Pal[256];
  //load palette
	for (int i = 0; i < 256; i ++) {
		unsigned char r = Get_color32(rgbs[3*i], maxval, brightness);
		unsigned char g = Get_color32(rgbs[3*i + 1], maxval, brightness);
		unsigned char b = Get_color32(rgbs[3*i + 2], maxval, brightness);
    Pal[i].rgbBlue = b;
    Pal[i].rgbGreen = g;
    Pal[i].rgbRed = r;
	}

  SetDIBColorTable(Context, 0, 256, Pal);

  SelectBitmap ( Context, DefaultBitmap );
  DeleteDC ( Context );

}

#endif
