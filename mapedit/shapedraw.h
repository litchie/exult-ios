/**
 **	Shapedraw.h - Manage a drawing area that shows one or more shapes.
 **
 **	Written: 6/2/2001 - JSF
 **/

#ifndef INCL_SHAPEDRAW
#define INCL_SHAPEDRAW	1

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

#include <gtk/gtk.h>
#include <gdk/gdktypes.h>

class Vga_file;
class Shape_frame;
class Image_buffer8;

typedef void (*Drop_callback)(int filenum,
				int shapenum, int framenum, void *udata);

/*
 *	This class can draw shapes from a .vga file.
 */
class Shape_draw
	{
protected:
	Vga_file *ifile;		// Where the shapes come from.
	char **names;			// Names of shapes (or null).
	GtkWidget *draw;		// GTK draw area to display them in.
	GdkGC *drawgc;			// For drawing in 'draw'.
	Image_buffer8 *iwin;		// What we render into.
	GdkRgbCmap *palette;		// For gdk_draw_indexed_image().
	Drop_callback drop_callback;	// Called when a shape is dropped here.
	void *drop_user_data;
public:
	Shape_draw(Vga_file *i, unsigned char *palbuf, GtkWidget *drw);
	virtual ~Shape_draw();
					// Blit onto screen.
	void show(GdkDrawable *drawable, int x, int y, int w, int h);
	void show(int x, int y, int w, int h)
		{ show(draw->window, x, y, w, h); }
	void show()
		{ show(0, 0, draw->allocation.width, draw->allocation.height);}
	void draw_shape(Shape_frame *shape, int x, int y);
	void draw_shape_centered(int shapenum, int framenum);
	void set_shape_names(char **nms)
		{ names = nms; }
	virtual void render();		// Update what gets shown.
	void set_background_color(guint32 c);
					// Configure when created/resized.
	void configure(GtkWidget *widget);
	void configure()
		{ configure(draw); }
					// Handler for drop.
	static void drag_data_received(GtkWidget *widget, 
		GdkDragContext *context, gint x, gint y, 
		GtkSelectionData *selection_data, guint info, guint time,
		gpointer udata);
	void enable_drop(Drop_callback callback, void *udata);
	};

#endif
