/**
 **	Import/export .PNG files.
 **
 **	Written: 6/9/99 - JSF
 **/

/*
Copyright (C) 2002  The Exult Team
Copyright (C) 1999  Jeffrey S. Freedman

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

#ifdef HAVE_PNG_H

#include <png.h>
#include <setjmp.h>


/*
 *	Read in an 8-bit .png file.  Each pixel returned is one byte,
 *	an offset into the palette (also returned).
 *
 *	Output:	0 if failed.
 */

int Import_png8
	(
	char *pngname,
	int& width, int& height,	// Image dimensions returned.
	int& rowbytes,			// # bytes/row returned.  (Should be
					//   width.)
	int& xoff, int& yoff,		// (X,Y) offsets from top-left of
					//   image returned.  (-1, -1) if not
					//   specified in file.
	unsigned char *& pixels,	// ->(allocated) pixels returned.
	unsigned char *& palette,	// ->(allocated) palette returned,
					//   each entry 3 bytes (RGB).
	int& pal_size			// # entries in palette returned.
	)
	{
	pixels = 0;			// In case we fail.
					// Open file.
	FILE *fp = fopen(pngname, "rb");
	if (!fp)
		return (0);
	unsigned char sigbuf[4];		// Make sure it's a .png.
	if (fread(sigbuf, 1, sizeof(sigbuf), fp) != sizeof(sigbuf) ||
	    png_sig_cmp(sigbuf, 0, sizeof(sigbuf)))
		{
		fclose(fp);
		return (0);
		}
					// Initialize.
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
						0, 0, 0);
	if (!png)
		{
		fclose(fp);
		return (0);
		}
					// Allocate info. structure.
	png_infop info = png_create_info_struct(png);
	if (setjmp(png->jmpbuf))	// Handle errors.
		{
		png_destroy_read_struct(&png, &info, 0);
		fclose(fp);
		return (0);
		}
	png_init_io(png, fp);		// Init. for reading.
					// Indicate we already read something.
	png_set_sig_bytes(png, sizeof(sigbuf));
	png_read_info(png, info);	// Read in image info.
	unsigned long w, h;
	int depth, color, interlace;
	png_get_IHDR(png, info, &w, &h, &depth, &color,
		&interlace, 0, 0);
	width = (int) w;
	height = (int) h;
	if (color != PNG_COLOR_TYPE_PALETTE)
		{
		png_destroy_read_struct(&png, &info, 0);
		fclose(fp);
		return (0);
		}
	if (depth < 8)
		png_set_packing(png);
	png_colorp pngpal;		// Get palette.
	if (png_get_PLTE(png, info, &pngpal, &pal_size) != 0)
		palette = new unsigned char[3*pal_size];
	else				// No palette??
		{
		pal_size = 0;
		palette = 0;
		}
	png_uint_32 pngxoff, pngyoff;	// Get offsets.
	int utype;
	if (png_get_oFFs(png, info, (png_int_32 *)(&pngxoff), (png_int_32 *)(&pngyoff), &utype) &&
	    utype == PNG_OFFSET_PIXEL)
		{
		xoff = pngxoff;
		yoff = pngyoff;
		}
	else
		xoff = yoff = -1;
	for (int i = 0; i < pal_size; i++)
		{
		palette[3*i] = pngpal[i].red;
		palette[3*i + 1] = pngpal[i].green;
		palette[3*i + 2] = pngpal[i].blue;
		}
					// Get updated info.
	png_read_update_info(png, info);
					// Allocate pixel buffer.
	rowbytes = png_get_rowbytes(png, info);
	png_bytep image = new png_byte[height*rowbytes];
	pixels = image;			// Return ->.
	png_bytep rowptr;		// Read in rows.
	int r;
	for (r = 0, rowptr = image; r < height; r++, rowptr += rowbytes)
		png_read_rows(png, &rowptr, 0, 1);
	png_read_end(png, info);	// Get the rest.
					// Clean up.
	png_destroy_read_struct(&png, &info, 0);
	fclose(fp);
	return (1);
	}


/*
 *	Write out an 8-bit .png file.  
 *
 *	Output:	0 if failed.
 */

int Export_png8
	(
	char *pngname,
	int width, int height,		// Image dimensions.
	int rowbytes,			// # bytes/row.  (Should be
					//   width.)
	int xoff, int yoff,		// (X,Y) offsets from top-left of
					//   image.
	unsigned char *pixels,		// ->pixels to write.
	unsigned char *palette,		// ->palette,
					//   each entry 3 bytes (RGB).
	int pal_size			// # entries in palette.
	)
	{
					// Open file.
	FILE *fp = fopen(pngname, "wb");
	if (!fp)
		return (0);
					// Initialize.
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
						0, 0, 0);
	if (!png)
		{
		fclose(fp);
		return (0);
		}
					// Allocate info. structure.
	png_infop info = png_create_info_struct(png);
	if (setjmp(png->jmpbuf))	// Handle errors.
		{
		png_destroy_write_struct(&png, &info);
		fclose(fp);
		return (0);
		}
	png_init_io(png, fp);		// Init. for reading.
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
						PNG_FILTER_TYPE_DEFAULT);
	png_color pngpal[256];		// Set palette.
	for (int i = 0; i < pal_size; i++)
		{
		pngpal[i].red = palette[3*i];
		pngpal[i].green = palette[3*i + 1];
		pngpal[i].blue = palette[3*i + 2];
		}
	png_set_PLTE(png, info, &pngpal[0], pal_size);
	png_set_oFFs(png, info, xoff, yoff, PNG_OFFSET_PIXEL);
					// Write out info.
	png_write_info(png, info);
	png_bytep rowptr;		// Write out rows.
	int r;
	for (r = 0, rowptr = pixels; r < height; r++, rowptr += rowbytes)
		png_write_row(png, rowptr);
	png_write_end(png, 0);		// Done.
					// Clean up.
	png_destroy_write_struct(&png, &info);
	fclose(fp);
	return (1);
	}

#if 0	/* ++++++ Returns 32-bit data. */
/*
 *	Read in a .png file.  Each pixel returned is 4 bytes: RGBA,
 *	where A is the alpha channel (0 = transparent, 255 = opaque).
 *
 *	Output:	0 if failed.
 */

int Image_file::import_png
	(
	char *pngname,
	int& width, int& height,	// Image dimensions returned.
	int& rowbytes,			// # bytes/row returned.  (Should be
					//   4*width.)
	unsigned char *& pixels		// ->(allocated) pixels returned.
	)
	{
	pixels = 0;			// In case we fail.
					// Open file.
	FILE *fp = fopen(pngname, "rb");
	if (!fp)
		return (0);
	unsigned char sigbuf[4];		// Make sure it's a .png.
	if (fread(sigbuf, 1, sizeof(sigbuf), fp) != sizeof(sigbuf) ||
	    png_sig_cmp(sigbuf, 0, sizeof(sigbuf)))
		{
		fclose(fp);
		return (0);
		}
					// Initialize.
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
						0, 0, 0);
	if (!png)
		{
		fclose(fp);
		return (0);
		}
					// Allocate info. structure.
	png_infop info = png_create_info_struct(png);
	if (setjmp(png->jmpbuf))	// Handle errors.
		{
		png_destroy_read_struct(&png, &info, 0);
		fclose(fp);
		return (0);
		}
	png_init_io(png, fp);		// Init. for reading.
					// Indicate we already read something.
	png_set_sig_bytes(png, sizeof(sigbuf));
	png_read_info(png, info);	// Read in image info.
	unsigned long w, h;
	int depth, color, interlace;
	png_get_IHDR(png, info, &w, &h, &depth, &color,
		&interlace, 0, 0);
	width = (int) w;
	height = (int) h;
	png_set_strip_16(png);		// Want 8 bits/color.
	if (color == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png);	// Expand if paletted.
	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_expand(png);	// Want an alpha byte.
	else if (depth == 8 && color == PNG_COLOR_TYPE_RGB)
		png_set_filler(png, 0xff, PNG_FILLER_AFTER);
					// Get updated info.
	png_read_update_info(png, info);
					// Allocate pixel buffer.
	rowbytes = png_get_rowbytes(png, info);
	png_bytep image = new png_byte[height*rowbytes];
	pixels = image;			// Return ->.
	png_bytep rowptr;		// Read in rows.
	int r;
	for (r = 0, rowptr = image; r < height; r++, rowptr += rowbytes)
		png_read_rows(png, &rowptr, 0, 1);
	png_read_end(png, info);	// Get the rest.
					// Clean up.
	png_destroy_read_struct(&png, &info, 0);
	fclose(fp);
	return (1);
	}
#endif

#endif	/* HAVE_PNG_H */

