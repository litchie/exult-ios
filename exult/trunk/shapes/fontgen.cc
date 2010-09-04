/**
 **	Create RLE font shapes from a given font.
 **
 **	Written: 4/8/2002 - JSF
 **/

/*
Copyright (C) 2002  The Exult Team

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

#include <cstdio>
#include <string.h>
#include "vgafile.h"

/*
 *	Generate a shadow around a character.
 */

static void Gen_shadow
	(
	unsigned char *pixels,
	int w, int h,			// Dimensions.
	unsigned char fg,		// Foreground color index.
	unsigned char shadow		// Shadow color index
	)
	{
	int r, c;

	for (r = 0; r < h; r++)
		for (c = 0; c < w; c++)
			{
			if (pixels[r*w+c] != fg)
				continue;
			int rr, cc;	// Fill surrounding pixels;
			for (rr = r - 1; rr <= r + 1; rr++)
				{
				if (rr < 0 || rr >= h)
					continue;
				for (cc = c - 1; cc <= c + 1; cc++)
					if (cc >= 0 && cc < w &&
							pixels[rr*w+cc] != fg)
						pixels[rr*w+cc] = shadow;
				}
			}
	}

#define USE_WIN32_FONTGEN

#if defined(WIN32) && defined(USE_WIN32_FONTGEN)

#undef NOGDI
#ifdef _WIN32_WINNT 
#undef _WIN32_WINNT 
#endif
#define _WIN32_WINNT 0x0500

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 
#include <windows.h>

static bool Gen_font_shape_win32
(
 HDC dc,
 HFONT font,
 Shape *shape,			// Shape to set frames.
 int nframes,			// # frames to generate, starting at 0.
 int pixels_ht,			// Desired height in pixels.
 unsigned char fg,		// Foreground color index.
 unsigned char bg,		// Background color index.
 int shadow			// Shadow color, or -1
 )
{
	MAT2 matrix;
	memset(&matrix,0,sizeof(matrix));
	matrix.eM11.value = 1;
	matrix.eM22.value = 1;
	//matrix.eM22.fract = 0x10000 * 10 / 12;

	shape->resize(nframes);		// Make it big enough.
	for (wchar_t chr = 0; chr < nframes; chr++)
	{			// Get each glyph.
		GLYPHMETRICS metrics;

		int buffsize = GetGlyphOutline( dc, chr, GGO_BITMAP, &metrics, 0, 0, &matrix);
		int offset = 0;		// Starting row, col.

		if (buffsize == GDI_ERROR || buffsize == 0)
		{
			SIZE size;
			GetTextExtentPoint32W(dc, &chr, 1, &size);


			if (size.cy == 0 || size.cx == 0)  {
				size.cx = size.cy = 1;
			}
			else if (shadow != -1) { // Make room for shadow.
			
				size.cx += 2;
				size.cy += 2;
				offset = 1;
			}
			uint8 *pixels = new uint8[size.cy * size.cx];
			memset(pixels,bg,size.cy * size.cx);

			// Not sure about dims here+++++
			Shape_frame *frame = new Shape_frame(pixels,
				size.cx, size.cy, offset, offset, true);
			delete [] pixels;
			shape->set_frame(frame, chr);
		}
		else
		{
			uint32 * buffer = new uint32[buffsize];
			if (GetGlyphOutline( dc, chr, GGO_BITMAP, &metrics, buffsize, buffer, &matrix) == GDI_ERROR)
			{
				delete [] buffer;
				Shape_frame *frame = new Shape_frame(&bg, 1, 1, 0, 0, true);
				shape->set_frame(frame, chr);
				continue;
			}
			int sw = metrics.gmBlackBoxX, sh = metrics.gmBlackBoxY;	// Shape width/height.

			if (sw < metrics.gmCellIncX) sw = metrics.gmCellIncX;
			if (sh < metrics.gmCellIncY) sh = metrics.gmCellIncY;

			if (shadow != -1)	// Make room for shadow.
			{
				sw += 2;
				sh += 2;
				offset = 1;
			}

			// Allocate our buffer.
			int cnt = sw*sh;	// Total #pixels.
			unsigned char *pixels = new unsigned char[cnt];
			memset(pixels, bg, cnt);// Fill with background.

			unsigned char *dest = pixels + offset*sw + offset;
			const uint8 *src = (uint8 *)buffer;
			for (int row = 0; row < metrics.gmBlackBoxY; row++)
			{
				for (int b = 0; b < metrics.gmBlackBoxX; b++)
					if (src[b/8]&(0x80>>(b%8)))
						dest[b] = fg;
				dest += sw;	// Advance to next row.
				src += (metrics.gmBlackBoxX+31)/32*4;
			}
			delete [] buffer;

			if (shadow >= 0)
				Gen_shadow(pixels, sw, sh, fg, static_cast<unsigned char>(shadow));
			// Not sure about dims here+++++
			Shape_frame *frame = new Shape_frame(pixels, sw, sh, offset-metrics.gmptGlyphOrigin.x, offset+metrics.gmptGlyphOrigin.y, true);
			delete [] pixels;
			shape->set_frame(frame, chr);
		}
	}
	return true;
}

int CALLBACK EnumFontFamProc(
  ENUMLOGFONT *lpelf,    // logical-font data
  NEWTEXTMETRIC *lpntm,  // physical-font data
  DWORD FontType,        // type of font
  LPARAM lParam          // application-defined data
)
{
	//MessageBox(NULL,(const char*)lpelf->elfFullName,"lpelf->elfFullName",MB_OK);
	//MessageBox(NULL,(const char*)lpelf->elfStyle,"lpelf->elfStyle",MB_OK);

	if (!lParam) return 0;
	if (!_strcmpi((const char*)lParam,(const char*)lpelf->elfFullName))
		return 0;
	if (!_strcmpi((const char*)lParam,(const char*)lpelf->elfStyle))
		return 0;
	return 1;
}

static bool Gen_font_shape_win32
(
 Shape *shape,			// Shape to set frames.
 const char *famname,		
 const char *stylename,
 int nframes,			// # frames to generate, starting at 0.
 int pixels_ht,			// Desired height in pixels.
 unsigned char fg,		// Foreground color index.
 unsigned char bg,		// Background color index.
 int shadow			// Shadow color, or -1
 )
{
	HDC dc = CreateCompatibleDC(NULL);

	HFONT font = 0;

	LOGFONT logfont;

	logfont.lfHeight = pixels_ht; 
	logfont.lfWidth = 0; 
	logfont.lfEscapement = 0; 
	logfont.lfOrientation = 0; 
	logfont.lfWeight = FW_DONTCARE; 
	logfont.lfItalic = FALSE; 
	logfont.lfUnderline = FALSE; 
	logfont.lfStrikeOut = FALSE; 
	logfont.lfCharSet = ANSI_CHARSET; 
	logfont.lfOutPrecision = OUT_RASTER_PRECIS; 
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS; 
	logfont.lfQuality = DEFAULT_QUALITY; 
	logfont.lfPitchAndFamily = DEFAULT_PITCH; 
  
	if (font == 0 && stylename) {
		snprintf(logfont.lfFaceName,LF_FACESIZE-1,"%s %s", famname, stylename);
		logfont.lfFaceName[LF_FACESIZE-1] = 0;

		if (!EnumFontFamilies(dc,logfont.lfFaceName,(FONTENUMPROC)&EnumFontFamProc,0))
			font = CreateFontIndirect(&logfont);
	}
	if (font == 0 && stylename) {
		snprintf(logfont.lfFaceName,LF_FACESIZE-1,"%s%s", famname, stylename);
		logfont.lfFaceName[LF_FACESIZE-1] = 0;
		if (!EnumFontFamilies(dc,logfont.lfFaceName,(FONTENUMPROC)&EnumFontFamProc,0))
			font = CreateFontIndirect(&logfont);
	}
	if (font  == 0) {
		strncpy(logfont.lfFaceName,famname,LF_FACESIZE-1);
		logfont.lfFaceName[LF_FACESIZE-1] = 0;
		if (!EnumFontFamilies(dc,logfont.lfFaceName,(FONTENUMPROC)&EnumFontFamProc,(LPARAM)stylename))
			font = CreateFontIndirect(&logfont);
	}

	if (font == 0) {
		DeleteDC(dc);
		return false;
	}

	HBITMAP bmp = CreateCompatibleBitmap(dc, 256, 256);

	SelectObject(dc, bmp);
	SelectObject(dc, font);

	bool ret = Gen_font_shape_win32(dc,font,shape,nframes,pixels_ht,fg,bg,shadow);
	DeleteObject(bmp);
	DeleteObject(font);
	DeleteDC(dc);
	return ret;
}

#endif
#if defined(HAVE_FREETYPE2) && !defined(__zaurus__)

#include <ft2build.h>
#include FT_FREETYPE_H

/*
 *	Fill a shape with each frame containing the glyph for its ASCII
 *	code.  The shape has 128 frames.
 *
 *	Output:	True if successful, false if error.
 */

bool Gen_font_shape
	(
	Shape *shape,			// Shape to set frames.
	const char *fontfile,		// Filename of font.
	int nframes,			// # frames to generate, starting at 0.
	int pixels_ht,			// Desired height in pixels.
	unsigned char fg,		// Foreground color index.
	unsigned char bg,		// Background color index.
	int shadow			// Shadow color, or -1
	)
	{
	FT_Library library;		// Initialize.
	int error = FT_Init_FreeType(&library);
	if (error)
		return false;
	FT_Face face;			// Gets the font.
	error = FT_New_Face(library, fontfile, 0, &face);
	if (error)
	{
		FT_Done_FreeType(library);

		// Try to get windows to load it for us
#if defined(WIN32) && defined(USE_WIN32_FONTGEN)
		return Gen_font_shape_win32(shape,fontfile,0,nframes,pixels_ht,fg,bg,shadow);
#else
		return false;
#endif
	}

#if defined(WIN32) && defined(USE_WIN32_FONTGEN)
	static HANDLE (WINAPI* AddFontResourceExA)(LPCSTR,DWORD,PVOID);
	if (AddFontResourceExA == 0) 
	{
		AddFontResourceExA = (HANDLE (WINAPI*)(LPCSTR,DWORD,PVOID)) 
			GetProcAddress(LoadLibrary("GDI32"),"AddFontResourceExA");

	}
	if (AddFontResourceExA && AddFontResourceExA(fontfile,FR_PRIVATE,0) != 0)
	{
		//if (face->family_name) MessageBox(NULL,face->family_name,"face->family_name",MB_OK);
		//if (face->style_name) MessageBox(NULL,face->style_name,"face->style_name",MB_OK);
		if (Gen_font_shape_win32(shape,face->family_name,face->style_name,nframes,pixels_ht,fg,bg,shadow))
		{
			FT_Done_FreeType(library);
			return true;
		}
	}
#endif

	error = FT_Set_Pixel_Sizes(face, 0, pixels_ht);
					// Glyphs are rendered here:
	FT_GlyphSlot glyph = face->glyph;
	if (error)
		return false;
	shape->resize(nframes);		// Make it big enough.
	for (int chr = 0; chr < nframes; chr++)
		{			// Get each glyph.
		error = FT_Load_Char(face, chr, 
				FT_LOAD_RENDER|FT_LOAD_MONOCHROME);
		if (error)
			{
			//+++++Do we need to store an empty frame?
			Shape_frame *frame = new Shape_frame(&bg, 1, 1, 0, 0, true);
			shape->set_frame(frame, chr);

			continue;
			}
		int w = glyph->bitmap.width, h = glyph->bitmap.rows;
		int sw = w, sh = h;	// Shape width/height.
		int offset = 0;		// Starting row, col.
		if (!sw)		// 0 width (like for a space)?
			sw = static_cast<int>(glyph->metrics.horiAdvance)/64;	// Guessin...
		if (!sh)
			sh = static_cast<int>(glyph->metrics.vertAdvance)/64;
		if (shadow != -1)	// Make room for shadow.
			{
			sw += 2;
			sh += 2;
			offset = 1;
			}
					// Allocate our buffer.
		int cnt = sw*sh;	// Total #pixels.
		unsigned char *pixels = new unsigned char[cnt];
		memset(pixels, bg, cnt);// Fill with background.
					// I believe this is 1 bit/pixel:
		unsigned char *src = glyph->bitmap.buffer;
		unsigned char *dest = pixels + offset*sw;
		for (int row = 0; row < h; row++)
			{
			for (int b = 0; b < w; b++)
				if (src[b/8]&(0x80>>(b%8)))
					dest[offset + b] = fg;
			dest += sw;	// Advance to next row.
			src += glyph->bitmap.pitch;
			}
		if (shadow >= 0)
			Gen_shadow(pixels, sw, sh, fg, static_cast<unsigned char>(shadow));
					// Not sure about dims here+++++
		Shape_frame *frame = new Shape_frame(pixels,
			sw, sh, glyph->bitmap_left + offset, 
				glyph->bitmap_top + offset, true);
		delete [] pixels;
		shape->set_frame(frame, chr);
		}
	FT_Done_FreeType(library);
	return true;
	}

#endif	/* HAVE_FREETYPE2 */

