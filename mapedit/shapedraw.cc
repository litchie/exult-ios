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
#include "u7drag.h"

using std::cout;
using std::endl;

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
	int shapenum,			// -1 to not draw shape.
	int framenum
	)
	{
	iwin->fill8(255);		// Background (transparent) color.
	if (shapenum < 0 || shapenum >= ifile->get_num_shapes())
		return;
	Shape_frame *shape = ifile->get_shape(shapenum, framenum);
	if (!shape)
		return;
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
	    iwin(0), palette(0), draw(drw), drawgc(0),
	    drop_callback(0), drop_user_data(0)
	{
	guint32 colors[256];
	for (int i = 0; i < 256; i++)
		colors[i] = (palbuf[3*i]<<16)*4 + (palbuf[3*i+1]<<8)*4 + 
							palbuf[3*i+2]*4;
	palette = gdk_rgb_cmap_new(colors, 256);
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
 *	Default render.
 */

void Shape_draw::render
	(
	)
	{
	}

/*
 *	Set background color and repaint.
 */

void Shape_draw::set_background_color
	(
	guint32 c
	)
	{
	palette->colors[255] = c;
	render();
	show();
	}

/*
 *	Configure the viewing window.
 */

void Shape_draw::configure
	(
	GtkWidget *widget		// The view window.
	)
	{
	if (!widget->window)
		return;			// Not ready yet.
	if (!iwin)			// First time?
		{
		drawgc = gdk_gc_new(widget->window);
					// Foreground = yellow.
		gdk_rgb_gc_set_foreground(drawgc, (255<<16) + (255<<8));
		iwin = new Image_buffer8(
			widget->allocation.width, widget->allocation.height);
		}
	else if (iwin->get_width() != widget->allocation.width ||
		 iwin->get_height() != widget->allocation.height)
		{
		delete iwin;
		iwin = new Image_buffer8(
			widget->allocation.width, widget->allocation.height);
		}
	}

/*
 *	Shape was dropped.
 */

void Shape_draw::drag_data_received
	(
	GtkWidget *widget,
	GdkDragContext *context,
	gint x,
	gint y,
	GtkSelectionData *seldata,
	guint info,
	guint time,
	gpointer udata			// Should point to Shape_draw.
	)
	{
	Shape_draw *draw = (Shape_draw *) udata;
	cout << "drag_data_received" << endl;
	if (draw->drop_callback &&
	    seldata->type == gdk_atom_intern(U7_TARGET_SHAPEID_NAME, 0) &&
	    seldata->format == 8 && seldata->length > 0)
		{
		int file, shape, frame;
		Get_u7_shapeid(seldata->data, file, shape, frame);
		(*draw->drop_callback)(file, shape, frame,
							draw->drop_user_data);
		}
	}

/*
 *	Set to accept drops from drag-n-drop of a shape.
 */

void Shape_draw::enable_drop
	(
	Drop_callback callback,		// Call this when shape dropped.
	void *udata			// Passed to callback.
	)
	{
	gtk_widget_realize(draw);//???????
#ifndef WIN32
	drop_callback = callback;
	drop_user_data = udata;
	GtkTargetEntry tents[1];
	tents[0].target = U7_TARGET_SHAPEID_NAME;
	tents[0].flags = 0;
	tents[0].info = U7_TARGET_SHAPEID;
	gtk_drag_dest_set(draw, GTK_DEST_DEFAULT_ALL, tents, 1,
			(GdkDragAction) (GDK_ACTION_COPY | GDK_ACTION_MOVE));

	gtk_signal_connect(GTK_OBJECT(draw), "drag_data_received",
				GTK_SIGNAL_FUNC(drag_data_received), this);
#endif
	}
