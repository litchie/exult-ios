/**
 **	A GTK widget showing a list of shapes from an image file.
 **
 **	Written: 7/25/99 - JSF
 **/

/*
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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include "shapelst.h"
#include "vgafile.h"
#include "ibuf8.h"

/*
 *	Blit onto screen.
 */

inline void Shape_chooser::show
	(
	int x, int y, int w, int h	// Area to blit.
	)
	{
	gdk_draw_indexed_image(draw->window, drawgc, x, y, w, h,
			GDK_RGB_DITHER_NORMAL,
			iwin->get_bits(), iwin->get_line_width(), palette);
	}

/*
 *	Select an entry (and show it).  This should be called after rendering
 *	the shape.
 */

void Shape_chooser::select
	(
	int new_sel
	)
	{
	selected = new_sel;
	int shapenum = info[selected].shapenum;
//	char *sname = ifile->get_shape_name(shapenum);
					// Remove prev. selection msg.
	gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	char buf[150];			// Show new selection.
	g_snprintf(buf, sizeof(buf), "Selected shape %d, frame %d",
			shapenum, info[selected].framenum);
	gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
	if (!iwin)
		return;
	Rectangle b = info[selected].box;
#if 0	/* ++++++++*/
					// Draw yellow box.
					// FOR NOW, black:
	gdk_draw_rectangle(iwin, draw->style->black_gc, FALSE,
						b.x, b.y, b.w, b.h);
#endif
	}

/*
 *	Render as many shapes as fit in the shape chooser window.
 */

void Shape_chooser::render
	(
	)
	{
					// Look for selected frame.
	int selshape = -1, selframe = -1;
	int prev_selected = selected;
	if (selected >= 0)
		{
		selshape = info[selected].shapenum;
		selframe = info[selected].framenum;
		selected = -1;		// Got to find it again.
		}
					// Remove "selected" message.
	gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	delete [] info;			// Delete old info. list.
	int shapenum = shapenum0, framenum = framenum0;
					// Get drawing area dimensions.
	gint winw = draw->allocation.width, winh = draw->allocation.height;
					// Provide more than enough room.
	info = new Shape_info[2*winw/8];
					// Clear window first.
	iwin->fill8(0);			// ++++Which color?
	int x = 0;
					// Get first shape.
	Shape_frame *shape = ifile->get_shape(shapenum, framenum);
	int sw;
	info_cnt = 0;			// Count them.
	while (shape && x + (sw = shape->get_width()) <= winw)
		{
					// Get height.
		int sh = shape->get_height();
		shape->paint(iwin, x + shape->get_xleft(),
					winh - sh + shape->get_yabove());
		if (sh > winh)
			sh = winh;
					// Store info. about where drawn.
		info[info_cnt].set(shapenum, framenum, x, winh - sh, sw, sh);
		if (shapenum == selshape && framenum == selframe)
					// Found the selected shape.
			select(info_cnt);
					// Get next.
//		shape = ifile->get_next_frame(shapenum, framenum);
		shapenum++;		// ++++For now, just next shape.
		framenum = 0;
		shape = shapenum >= num_shapes ? 0 
				: ifile->get_shape(shapenum, framenum);
		x += sw;
		info_cnt++;
		}
	if (selected != prev_selected && sel_changed)
		(*sel_changed)();	// Tell client if sel. removed.
	}

/*
 *	Configure the viewing window.
 */

gint Shape_chooser::configure
	(
	GtkWidget *widget,		// The view window.
	GdkEventConfigure *event,
	gpointer data			// ->Shape_chooser
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
	if (!chooser->iwin)		// First time?
		{
		chooser->drawgc = gdk_gc_new(widget->window);
		chooser->iwin = new Image_buffer8(
			widget->allocation.width, widget->allocation.height);
		}
	else
		{
		delete chooser->iwin;
		chooser->iwin = new Image_buffer8(
			widget->allocation.width, widget->allocation.height);
		}
	chooser->render();
	return (TRUE);
	}

/*
 *	Handle an expose event.
 */

gint Shape_chooser::expose
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
	chooser->show(event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Handle a mouse button press event.
 */

gint Shape_chooser::mouse_press
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
					// Search through entries.
	for (int i = 0; i < chooser->info_cnt; i++)
		if (chooser->info[i].box.has_point(
					(int) event->x, (int) event->y))
			{		// Found the box?
			chooser->selected = i;
			chooser->render();
			chooser->show();
					// Tell client.
			if (chooser->sel_changed)
				(*chooser->sel_changed)();
			break;
			}
	return (TRUE);
	}

/*
 *	Scroll to a new shape/frame.
 */

void Shape_chooser::scroll
	(
	int newindex			// Abs. index of leftmost to show.
	)
	{
	if (shapenum0 < newindex)	// Going forwards?
		shapenum0 = newindex < num_shapes ? newindex : num_shapes;
	else if (shapenum0 > newindex)	// Backwards?
		shapenum0 = newindex >= 0 ? newindex : 0;
	render();
	show();
	}

/*
 *	Handle a scrollbar event.
 */

void Shape_chooser::scrolled
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Shape_chooser.
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
cout << "Scrolled to " << adj->value << '\n';
	gint newindex = (gint) adj->value;
	chooser->scroll(newindex);
	}

/*
 *	Create the list.
 */

Shape_chooser::Shape_chooser
	(
	Vga_file *i,			// Where they're kept.
	GtkWidget *box,			// Where to put this.
	int w, int h			// Dimensions.
	) : ifile(i), iwin(0), shapenum0(0), framenum0(0),
		info(0), info_cnt(0), selected(-1), sel_changed(0)
	{
					// Put things in a vert. box.
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), vbox, TRUE, TRUE, 0);
	gtk_widget_show(vbox);
	num_shapes = ifile->get_num_shapes();
					// A frame looks nice.
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	draw = gtk_drawing_area_new();	// Create drawing area window.
					// Indicate the events we want.
	gtk_widget_set_events(draw, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);
					// Set "configure" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "configure_event",
				GTK_SIGNAL_FUNC(configure), this);
					// Set "expose" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "expose_event",
				GTK_SIGNAL_FUNC(expose), this);
					// Set mouse click handler.
	gtk_signal_connect(GTK_OBJECT(draw), "button_press_event",
				GTK_SIGNAL_FUNC(mouse_press), this);
	gtk_container_add (GTK_CONTAINER (frame), draw);
	gtk_drawing_area_size(GTK_DRAWING_AREA(draw), w, h);
	gtk_widget_show(draw);
					// Want a scrollbar for the shapes.
	GtkObject *shape_adj = gtk_adjustment_new(0, 0, 
				num_shapes, 1, 
				1 + num_shapes/10, 1.0);
	GtkWidget *shape_scroll = gtk_hscrollbar_new(
					GTK_ADJUSTMENT(shape_adj));
					// Update window when it stops.
	gtk_range_set_update_policy(GTK_RANGE(shape_scroll),
					GTK_UPDATE_DELAYED);
	gtk_box_pack_start(GTK_BOX(vbox), shape_scroll, FALSE, TRUE, 0);
					// Set scrollbar handler.
	gtk_signal_connect(GTK_OBJECT(shape_adj), "value_changed",
					GTK_SIGNAL_FUNC(scrolled), this);
	gtk_widget_show(shape_scroll);
					// Finally, a status bar.
	sbar = gtk_statusbar_new();
	sbar_sel = gtk_statusbar_get_context_id(GTK_STATUSBAR(sbar),
							"selection");
	gtk_box_pack_start(GTK_BOX(vbox), sbar, FALSE, FALSE, 0);
	gtk_widget_show(sbar);
	}

/*
 *	Delete.
 */

Shape_chooser::~Shape_chooser
	(
	)
	{
	delete [] info;
	delete iwin;
	}

/*
 *	Unselect.
 */

void Shape_chooser::unselect
	(
	)
	{
	if (selected >= 0)
		{
		selected = -1;
		render();
		show();
		if (sel_changed)	// Tell client.
			(*sel_changed)();
		}
	}
