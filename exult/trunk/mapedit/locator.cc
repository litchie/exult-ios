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
	) : drawgc(0), tx(0), ty(0), txs(40), tys(25), scale(1)
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
	hadj->page_increment = vadj->page_increment = 
		hadj->page_size = vadj->page_size = c_chunks_per_schunk;
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
					// Put a red box around it.
	gdk_rgb_gc_set_foreground(drawgc, (255<<16) + (64<<8) + 64);
	gdk_draw_rectangle(draw->window, drawgc, FALSE, 
					x - 2, y - 2, w + 4, h + 4);
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
//	render(&area);
					// Update scrolls.
	gtk_adjustment_set_value(hadj, tx/c_tiles_per_chunk);
	gtk_adjustment_set_value(vadj, ty/c_tiles_per_chunk);
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
	loc->tx = ((gint) adj->value)*c_tiles_per_chunk;
	loc->render();	//++++Maybe wrong.  Should send msg. to Exult.
	}
void Locator::hscrolled			// For horizontal scrollbar.
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Locator.
	)
	{
	Locator *loc = (Locator *) data;
	loc->ty = ((gint) adj->value)*c_tiles_per_chunk;
	loc->render();	//++++Maybe wrong.  Should send msg. to Exult.
	}

