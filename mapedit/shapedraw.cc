/**
 **	Shapedraw.cc - Manage a drawing area that shows one or more shapes.
 **
 **	Written: 6/2/2001 - JSF
 **/

/*
Copyright (C) 2001  The Exult Team

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

#include "shapedraw.h"
#include "vgafile.h"
#include "ibuf8.h"

/*
 *	Blit onto screen.
 */

void Shape_draw::show
	(
	GdkDrawable *drawable,
	int x, int y, int w, int h	// Area to blit.
	)
	{
	int stride = iwin->get_line_width();
	gdk_draw_indexed_image(drawable, drawgc, x, y, w, h,
			GDK_RGB_DITHER_NORMAL,
			iwin->get_bits() + y*stride + x, 
			stride, palette);
	}

/*
 *	Draw one shape at a particular place.
 */

void Shape_draw::draw_shape
	(
	Shape_frame *shape,
	int x, int y
	)
	{
	shape->paint(iwin, x + shape->get_xleft(), y + shape->get_yabove());
	}

/*
 *	Draw a shape centered in the drawing area.
 */

void Shape_draw::draw_shape_centered
	(
	int shapenum,
	int framenum
	)
	{
	Shape_frame *shape = ifile->get_shape(shapenum, framenum);
	if (!shape)
		return;
	iwin->fill8(0);			// ++++Which color?
					// Get drawing area dimensions.
	gint winw = draw->allocation.width, winh = draw->allocation.height;
	draw_shape(shape, (winw - shape->get_width())/2,
			  (winh - shape->get_height())/2);
	}

/*
 *	Create.
 */

Shape_draw::Shape_draw
	(
	Vga_file *i,			// Where they're kept.
	unsigned char *palbuf,		// Palette, 3*256 bytes (rgb triples).
	GtkWidget *drw			// Drawing area to use.
	) : ifile(i),
		iwin(0), palette(0), names(0), draw(drw)
	{
	guint32 colors[256];
	for (int i = 0; i < 256; i++)
		colors[i] = (palbuf[3*i]<<16)*4 + (palbuf[3*i+1]<<8)*4 + 
							palbuf[3*i+2]*4;
	palette = gdk_rgb_cmap_new(colors, 256);
	num_shapes = ifile->get_num_shapes();
	}

/*
 *	Delete.
 */

Shape_draw::~Shape_draw
	(
	)
	{
	gdk_rgb_cmap_free(palette);
	delete iwin;
	}
	
/*
 *	Configure the viewing window.
 */

gint Shape_draw::configure
	(
	GtkWidget *widget,		// The view window.
	GdkEventConfigure *event,
	gpointer data			// ->Shape_chooser
	)
	{
	Shape_draw *draw = (Shape_draw *) data;
	if (!draw->iwin)		// First time?
		{
		draw->drawgc = gdk_gc_new(widget->window);
					// Foreground = yellow.
		gdk_rgb_gc_set_foreground(draw->drawgc,
							(255<<16) + (255<<8));
		draw->iwin = new Image_buffer8(
			widget->allocation.width, widget->allocation.height);
		}
	else
		{
		delete draw->iwin;
		draw->iwin = new Image_buffer8(
			widget->allocation.width, widget->allocation.height);
		}
	draw->render();
	return (TRUE);
	}
