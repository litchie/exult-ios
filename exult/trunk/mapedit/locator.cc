/**
 **	Locator.cc - Locate game positions.
 **
 **	Written: March 2, 2002 - JSF
 **/

/*
Copyright (C) 2001-2002 The Exult Team

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

#include "studio.h"
#include "servemsg.h"
#include "exult_constants.h"
#include "locator.h"
#include "utils.h"

using	std::cout;
using	std::endl;

/*
 *	Open locator window.
 */

C_EXPORT void on_locator1_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->open_locator_window();
	}
void ExultStudio::open_locator_window
	(
	)
	{
	if (!locwin)			// First time?
		{
		locwin = new Locator();
		}
	locwin->show(true);
	}

/*
 *	Locator window's close button.
 */
C_EXPORT void on_loc_close_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	Locator *loc = (Locator *) gtk_object_get_user_data(
			GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(btn))));
	loc->show(false);
	}

/*
 *	Locator window's X button.
 */
C_EXPORT gboolean on_loc_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	Locator *loc = (Locator *) 
				gtk_object_get_user_data(GTK_OBJECT(widget));
	loc->show(false);
	return TRUE;
	}

/*
 *	Draw area created, or size changed.
 */
C_EXPORT gint on_loc_draw_configure_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventConfigure *event,
	gpointer data			// ->Shape_chooser
	)
	{
	Locator *loc = (Locator *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(widget))));
	loc->configure(widget);
	return TRUE;
	}
C_EXPORT gint on_loc_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	Locator *loc = (Locator *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(widget))));
	loc->render(&event->area);
	return TRUE;
	}

/*
 *	Create locator window.
 */

Locator::Locator
	(
	) : drawgc(0), tx(0), ty(0), txs(40), tys(25), scale(1),
	    dragging(false)
	{
	GladeXML *app_xml = ExultStudio::get_instance()->get_xml();
	win = glade_xml_get_widget(app_xml, "loc_window");
	gtk_object_set_user_data(GTK_OBJECT(win), this);
	draw = glade_xml_get_widget(app_xml, "loc_draw");
					// Set up scales.
	GtkWidget *scale = glade_xml_get_widget(app_xml, "loc_hscale");
	hadj = gtk_range_get_adjustment(GTK_RANGE(scale));
	scale = glade_xml_get_widget(app_xml, "loc_vscale");
	vadj = gtk_range_get_adjustment(GTK_RANGE(scale));
	hadj->upper = vadj->upper = c_num_chunks;
	hadj->page_increment = vadj->page_increment = c_chunks_per_schunk;
	hadj->page_size = vadj->page_size = 2;
	gtk_signal_emit_by_name(GTK_OBJECT(hadj), "changed");
	gtk_signal_emit_by_name(GTK_OBJECT(vadj), "changed");
					// Set scrollbar handlers.
	gtk_signal_connect(GTK_OBJECT(hadj), "value_changed",
					GTK_SIGNAL_FUNC(hscrolled), this);
	gtk_signal_connect(GTK_OBJECT(vadj), "value_changed",
					GTK_SIGNAL_FUNC(vscrolled), this);
	}

/*
 *	Delete.
 */

Locator::~Locator
	(
	)
	{
	gtk_widget_destroy(win);
	}

/*
 *	Show/hide.
 */

void Locator::show
	(
	bool tf
	)
	{
	if (tf)
		gtk_widget_show(win);
	else
		gtk_widget_hide(win);
	}

/*
 *	Configure the draw window.
 */

void Locator::configure
	(
	GtkWidget *widget		// The draw window.
	)
	{
	if (!widget->window)
		return;			// Not ready yet.
	if (!drawgc)			// First time?
		{
		drawgc = gdk_gc_new(widget->window);
		}
	}

/*
 *	Display.
 */

void Locator::render
	(
	GdkRectangle *area		// 0 for whole draw area.
	)
	{
	GdkRectangle all;
	if (!area)
		{
		all.x = all.y = 0;
		all.width = draw->allocation.width;
		all.height = draw->allocation.height;
		area = &all;
		}
	gdk_gc_set_clip_rectangle(drawgc, area);
					// Background is dark blue.
	gdk_rgb_gc_set_foreground(drawgc, 64);
	gdk_draw_rectangle(draw->window, drawgc, TRUE, area->x, area->y,
					area->width, area->height);
					// Figure where to draw box.
	int draww = draw->allocation.width,
	    drawh = draw->allocation.height;
	int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
	int x = (cx*draww)/c_num_chunks,
	    y = (cy*drawh)/c_num_chunks;
	int w = (txs*draww)/c_num_tiles,
	    h = (tys*drawh)/c_num_tiles;
	if (w == 0)
		w = 1;
	if (h == 0)
		h = 1;
					// Draw location in yellow.
	gdk_rgb_gc_set_foreground(drawgc, (255<<16) + (255<<8));
	gdk_draw_rectangle(draw->window, drawgc, FALSE, x, y, w, h);
	viewbox.x = x; viewbox.y = y;	// Save location.
	viewbox.width = w; viewbox.height = h;
					// Put a light-red box around it.
	gdk_rgb_gc_set_foreground(drawgc, (255<<16) + (128<<8) + 128);
	gdk_draw_rectangle(draw->window, drawgc, FALSE, 
					x - 3, y - 3, w + 6, h + 6);
	}

/*
 *	Message was received from Exult that the view has changed.
 */

void Locator::view_changed
	(
	unsigned char *data,		// Tx, Ty, xtiles, ytiles, scale.
	int datalen
	)
	{
	if (datalen < 4*4)
		return;			// Bad length.
	tx = Read4(data);
	ty = Read4(data);
	txs = Read4(data);
	tys = Read4(data);
					// ++++Scale?  Later.
					// Do things by chunk.
	int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
	tx = cx*c_tiles_per_chunk;
	ty = cy*c_tiles_per_chunk;
					// Update scrolls.
	gtk_adjustment_set_value(hadj, cx);
	gtk_adjustment_set_value(vadj, cy);
	}

/*
 *	Handle a scrollbar event.
 */

void Locator::vscrolled			// For vertical scrollbar.
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Shape_chooser.
	)
	{
	Locator *loc = (Locator *) data;
	int oldty = loc->ty;
	loc->ty = ((gint) adj->value)*c_tiles_per_chunk;
	cout << "Vscrolled:  New ty is " << loc->ty << endl;
	loc->render();
	if (loc->ty != oldty)		// (Already equal if this event came
					//   from Exult msg.).
		loc->send_location();
	}
void Locator::hscrolled			// For horizontal scrollbar.
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Locator.
	)
	{
	Locator *loc = (Locator *) data;
	int oldtx = loc->tx;
	loc->tx = ((gint) adj->value)*c_tiles_per_chunk;
	cout << "Hscrolled:  New tx is " << loc->tx << endl;
	loc->render();
	if (loc->tx != oldtx)		// (Already equal if this event came
					//   from Exult msg.).
		loc->send_location();
	}

/*
 *	Send location to Exult.
 */

void Locator::send_location
	(
	)
	{
	unsigned char data[50];
	unsigned char *ptr = &data[0];
	Write4(ptr, tx);
	Write4(ptr, ty);
	Write4(ptr, txs);
	Write4(ptr, tys);
	Write4(ptr, -1);		// Don't change.
	ExultStudio::get_instance()->send_to_server(Exult_server::view_pos,
					&data[0], ptr - data);
	}

/*
 *	Go to a mouse location in the draw area.
 */

void Locator::goto_mouse
	(
	int mx, int my			// Pixel coords. in draw area.
	)
	{
	int newtx = (mx*c_num_tiles)/draw->allocation.width;
	int newty = (my*c_num_tiles)/draw->allocation.height;
	int cx = newtx/c_tiles_per_chunk, cy = newty/c_tiles_per_chunk;
	if (cx > c_num_chunks - 2)
		cx = c_num_chunks - 2;
	if (cy > c_num_chunks - 2)
		cy = c_num_chunks - 2;
					// Update scrolls.  This will result in
					//   tx, ty being updated and Exult
					//   getting the message.
	gtk_adjustment_set_value(hadj, cx);
	gtk_adjustment_set_value(vadj, cy);
	}

/*
 *	Handle a mouse-press event.
 */

gint Locator::mouse_press
	(
	GdkEventButton *event
	)
	{
	dragging = false;
	if (event->button != 1)
		return FALSE;		// Handling left-click.
					// Get mouse position, draw dims.
	int mx = (int) event->x, my = (int) event->y;
	int draww = draw->allocation.width,
	    drawh = draw->allocation.height;
					// Double-click?
	if (((GdkEvent *) event)->type == GDK_2BUTTON_PRESS)
		{
		goto_mouse(mx, my);
		return TRUE;
		}
					// On (or close to) view box?
	if (mx < viewbox.x - 3 || my < viewbox.y - 3 ||
	    mx > viewbox.x + viewbox.width + 6 ||
	    my > viewbox.y + viewbox.height + 6)
		return FALSE;
	dragging = true;
	drag_relx = mx - viewbox.x;	// Save rel. pos.
	drag_rely = my - viewbox.y;
	return (TRUE);
	}

/*
 *	Handle a mouse-motion event.
 */

gint Locator::mouse_motion
	(
	GdkEventMotion *event
	)
	{
	if (!dragging || !(event->state & GDK_BUTTON1_MASK))
		return FALSE;		// Not dragging with left button.
	int mx = (int) event->x, my = (int) event->y;
	cout << "Locator dragging: (" << mx << ',' << my << ')' << endl;
	goto_mouse(mx + drag_relx, my + drag_rely);
	return TRUE;
	}
