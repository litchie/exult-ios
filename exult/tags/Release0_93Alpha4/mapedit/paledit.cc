/**
 **	A GTK widget showing a palette's colors.
 **
 **	Written: 12/24/2000 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include "Flex.h"
#include "paledit.h"
#include "u7drag.h"
#include <iostream>

/*
 *	Blit onto screen.
 */

using	std::cout;
using	std::endl;

inline void Palette_edit::show
	(
	int x, int y, int w, int h	// Area to blit.
	)
	{
	int stride = draw->allocation.width;
	gdk_draw_indexed_image(draw->window, drawgc, x, y, w, h,
			GDK_RGB_DITHER_NORMAL,
			image + y*stride + x, 
			stride, palette);
	if (selected >= 0)		// Show selected.
					// Draw yellow box.
		gdk_draw_rectangle(draw->window, drawgc, FALSE, 
			selected_box.x, selected_box.y,
			selected_box.w, selected_box.h);
	}

/*
 *	Select an entry.  This should be called after rendering
 *	the shape.
 */

void Palette_edit::select
	(
	int new_sel
	)
	{
	selected = new_sel;
					// Remove prev. selection msg.
	gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	char buf[150];			// Show new selection.
	g_snprintf(buf, sizeof(buf), "Color %d (0x%02x)", new_sel, new_sel);
	gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
	}

/*
 *	Draw the palette.  This need only be called when it changes.
 */

void Palette_edit::render
	(
	)
	{
	int neww = draw->allocation.width, newh = draw->allocation.height;
					// Changed size?
	if (neww != width || newh != height)
		{
		delete image;
		width = neww;
		height = newh;
		image = new guchar[width*height];
		}
					// Figure cell size.
	int eachw = width/16, eachh = height/16;
					// Figure extra pixels.
	int extraw = width%16, extrah = height%16;
	int color = 0;			// Color index.
	int cury = 0;
	unsigned char *out = image;
	for (int y = 0; y < 16; y++, color += 16)
		{
		int curx = 0;
		int stopy = cury + eachh + (y < extrah);
		for ( ; cury < stopy; cury++)
			for (int x = 0, c = color; x < 16; x++, c++)
				{
				int cntx = eachw + (x < extraw);
				while (cntx--)
					*out++ = c;
				}
		}
	if (selected >= 0)		// Update selected box.
		{
		int selx = selected%16, sely = selected/16;
		selected_box.x = selx*eachw;
		selected_box.y = sely*eachh;
		selected_box.w = eachw;
		selected_box.h = eachh;
		if (selx < extraw)	// Watch for extra pixels.
			{ selected_box.w++; selected_box.x += selx; }
		else
			selected_box.x += extraw;
		if (sely < extrah)
			{ selected_box.h++; selected_box.y += sely; }
		else
			selected_box.y += extrah;
		select(selected);
		}
	}

/*
 *	Color box was closed.
 */

int Palette_edit::color_closed
	(
	GtkWidget *dlg,
	GdkEvent *event,
	gpointer data
	)
	{
	cout << "color_closed" << endl;
	Palette_edit *paled = (Palette_edit *) data;
	paled->colorsel = 0;
	return FALSE;
	}

/*
 *	'Cancel' was hit in the color selector.
 */

void Palette_edit::color_cancel
	(
	GtkWidget *dlg,
	gpointer data
	)
	{
	Palette_edit *paled = (Palette_edit *) data;
	if (paled->colorsel)
		gtk_widget_destroy(GTK_WIDGET(paled->colorsel));
	paled->colorsel = 0;
	}

/*
 *	'Okay' was hit in the color selector.
 */

void Palette_edit::color_okay
	(
	GtkWidget *dlg,
	gpointer data
	)
	{
	Palette_edit *paled = (Palette_edit *) data;
	if (paled->colorsel)
		{
		gdouble rgb[3];
		gtk_color_selection_get_color(
			GTK_COLOR_SELECTION(paled->colorsel->colorsel), rgb);
		unsigned char r = (unsigned char) (rgb[0]*256),
			      g = (unsigned char) (rgb[1]*256),
			      b = (unsigned char) (rgb[2]*256);
		if (paled->selected >= 0)
			paled->palette->colors[paled->selected] = 
							(r<<16) + (g<<8) + b;
		gtk_widget_destroy(GTK_WIDGET(paled->colorsel));
		paled->render();
		paled->show();
		}
	paled->colorsel = 0;
	}

/*
 *	Handle double-click on a color by bringing up a color-selector.
 */

void Palette_edit::double_clicked
	(
	)
	{
	cout << "Double-clicked" << endl;
	if (selected < 0 || colorsel)	// Only one at a time.
		return;			// Nothing selected.
	char buf[150];			// Show new selection.
	g_snprintf(buf, sizeof(buf), "Color %d (0x%02x)", selected, selected);
	colorsel = GTK_COLOR_SELECTION_DIALOG(
					gtk_color_selection_dialog_new(buf));
					// Set mouse click handler.
	gtk_signal_connect(GTK_OBJECT(colorsel->ok_button), "clicked",
				GTK_SIGNAL_FUNC(color_okay), this);
	gtk_signal_connect(GTK_OBJECT(colorsel->cancel_button), "clicked",
				GTK_SIGNAL_FUNC(color_cancel), this);
					// Set delete handler.
	gtk_signal_connect(GTK_OBJECT(colorsel), "delete_event",
				GTK_SIGNAL_FUNC(color_closed), this);
					// Get color.
	guint32 c = palette->colors[selected];
	gdouble rgb[3];
	rgb[0] = ((double) ((c>>16)&0xff))/256;
	rgb[1] = ((double) ((c>>8)&0xff))/256;
	rgb[2] = ((double) ((c>>0)&0xff))/256;
	gtk_color_selection_set_color(GTK_COLOR_SELECTION(colorsel->colorsel),
								rgb);
	gtk_widget_show(GTK_WIDGET(colorsel));
	}

/*
 *	Configure the viewing window.
 */

gint Palette_edit::configure
	(
	GtkWidget *widget,		// The view window.
	GdkEventConfigure *event,
	gpointer data			// ->Palette_edit
	)
	{
	Palette_edit *paled = (Palette_edit *) data;
	if (!paled->width)		// First time?
		{
		paled->drawgc = gdk_gc_new(widget->window);
					// Foreground = yellow.
		gdk_rgb_gc_set_foreground(paled->drawgc,
							(255<<16) + (255<<8));
		}
	paled->render();
	return (TRUE);
	}

/*
 *	Handle an expose event.
 */

gint Palette_edit::expose
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Palette_edit.
	)
	{
	Palette_edit *paled = (Palette_edit *) data;
	paled->show(event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Handle a mouse button press event.
 */

gint Palette_edit::mouse_press
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Palette_edit.
	)
	{
	Palette_edit *paled = (Palette_edit *) data;
	if (paled->colorsel)
		return (TRUE);		// Already editing a color.
	int old_selected = paled->selected;
	int width = paled->width, height = paled->height;
	int eventx = (int) event->x, eventy = (int) event->y;
					// Figure cell size.
	int eachw = width/16, eachh = height/16;
					// Figure extra pixels.
	int extraw = width%16, extrah = height%16;
	int extrax = extraw*(eachw + 1);// Total length of extra-sized boxes.
	int extray = extrah*(eachh + 1);
	int selx, sely;			// Gets box indices.
	if (eventx < extrax)
		selx = eventx/(eachw + 1);
	else
		selx = extraw + (eventx - extrax)/eachw;
	if (eventy < extray)
		sely = eventy/(eachh + 1);
	else
		sely = extrah + (eventy - extray)/eachh;
	paled->selected = sely*16 + selx;
	if (paled->selected == old_selected)
		{			// Same square.  Check for dbl-click.
		if (((GdkEvent *) event)->type == GDK_2BUTTON_PRESS)
			paled->double_clicked();
		}
	else
		{
		paled->render();
		paled->show();
		}
#if 0
					// Indicate we can drag.
			GtkTargetEntry tents[1];
			tents[0].target = U7_TARGET_SHAPEID_NAME;
			tents[0].flags = 0;
			tents[0].info = U7_TARGET_SHAPEID;
			gtk_drag_source_set (paled->draw, 
				GDK_BUTTON1_MASK, tents, 1,GDK_ACTION_DEFAULT);
			paled->selected = i;
			paled->render();
			paled->show();
					// Tell client.
			if (paled->sel_changed)
				(*paled->sel_changed)();
			break;
			}
#endif
	return (TRUE);
	}

/*
 *	Someone wants the dragged shape.
 */

void Palette_edit::drag_data_get
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	GtkSelectionData *seldata,	// Fill this in.
	guint info,
	guint time,
	gpointer data			// ->Palette_edit.
	)
	{
	cout << "In DRAG_DATA_GET" << endl;
	Palette_edit *paled = (Palette_edit *) data;
#if 0
	if (paled->selected < 0 || info != U7_TARGET_SHAPEID)
		return;			// Not sure about this.
	guchar buf[30];
	int file = U7_SHAPE_SHAPES;	// +++++For now.
	Shape_info& shinfo = paled->info[paled->selected];
	int len = Store_u7_shapeid(buf, file, shinfo.shapenum, 
							shinfo.framenum);
	cout << "Setting selection data (" << shinfo.shapenum <<
			'/' << shinfo.framenum << ')' << endl;
					// Make us owner of xdndselection.
	gtk_selection_owner_set(widget, gdk_atom_intern("XdndSelection", 0),
								time);
					// Set data.
	gtk_selection_data_set(seldata,
			gdk_atom_intern(U7_TARGET_SHAPEID_NAME, 0),
                                				8, buf, len);
#endif
	}

/*
 *	Another app. has claimed the selection.
 */

gint Palette_edit::selection_clear
	(
	GtkWidget *widget,		// The view window.
	GdkEventSelection *event,
	gpointer data			// ->Palette_edit.
	)
	{
//	Palette_edit *paled = (Palette_edit *) data;
	cout << "SELECTION_CLEAR" << endl;
	return TRUE;
	}

/*
 *	Beginning of a drag.
 */

gint Palette_edit::drag_begin
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	gpointer data			// ->Palette_edit.
	)
	{
	cout << "In DRAG_BEGIN" << endl;
	Palette_edit *paled = (Palette_edit *) data;
#if 0
	if (paled->selected < 0)
		return FALSE;		// ++++Display a halt bitmap.
					// Get ->shape.
	Shape_info& shinfo = paled->info[paled->selected];
	Shape_frame *shape = paled->ifile->get_shape(shinfo.shapenum, 
							shinfo.framenum);
	if (!shape)
		return FALSE;
	int w = shape->get_width(), h = shape->get_height(),
		xright = shape->get_xright(), ybelow = shape->get_ybelow();
	Image_buffer8 tbuf(w, h);	// Create buffer to render to.
	tbuf.fill8(0xff);		// Fill with 'transparent' pixel.
	unsigned char *tbits = tbuf.get_bits();
	shape->paint(&tbuf, w - 1 - xright, h - 1 - ybelow);
					// Put shape on a pixmap.
	GdkPixmap *pixmap = gdk_pixmap_new(widget->window, w, h, -1);
	gdk_draw_indexed_image(pixmap, paled->drawgc, 0, 0, w, h,
			GDK_RGB_DITHER_NORMAL, tbits,
			tbuf.get_line_width(), paled->palette);
	int mask_stride = (w + 7)/8;	// Round up to nearest byte.
	char *mdata = new char[mask_stride*h];
	for (int y = 0; y < h; y++)	// Do each row.
					// Do each byte.
		for (int b = 0; b < mask_stride; b++)
			{
			char bits = 0;
			unsigned char *vals = tbits + y*w + b*8;
			for (int i = 0; i < 8; i++)
				if (vals[i] != 0xff)
					bits |= (1<<i);
			mdata[y*mask_stride + b] = bits;
			}
	GdkBitmap *mask = gdk_bitmap_create_from_data(widget->window,
							mdata, w, h);
	delete mdata;
					// This will be the shape dragged.
	gtk_drag_set_icon_pixmap(context,
			gdk_window_get_colormap(widget->window), pixmap, mask,
					w - 2 - xright, h - 2 - ybelow);
	gdk_pixmap_unref(pixmap);
	gdk_bitmap_unref(mask);
#endif
	return TRUE;
	}

/*
 *	Create the list.
 */

Palette_edit::Palette_edit
	(
	guint32 *colors,		// 256-entry RGB palette.
	int w, int h			// Dimensions.
	) : image(0), width(0), height(0),
		palette(0), colorsel(0),
		selected(-1)
	{
	palette = gdk_rgb_cmap_new(colors, 256);
					// Put things in a vert. box.
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	set_widget(vbox);
					// A frame looks nice.
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	draw = gtk_drawing_area_new();	// Create drawing area window.
	gtk_drawing_area_size(GTK_DRAWING_AREA(draw), w, h);
					// Indicate the events we want.
	gtk_widget_set_events(draw, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK
		| GDK_POINTER_MOTION_HINT_MASK |
		GDK_BUTTON1_MOTION_MASK);
					// Set "configure" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "configure_event",
				GTK_SIGNAL_FUNC(configure), this);
					// Set "expose" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "expose_event",
				GTK_SIGNAL_FUNC(expose), this);
					// Set mouse click handler.
	gtk_signal_connect(GTK_OBJECT(draw), "button_press_event",
				GTK_SIGNAL_FUNC(mouse_press), this);
					// Mouse motion.
	gtk_signal_connect(GTK_OBJECT(draw), "drag_begin",
				GTK_SIGNAL_FUNC(drag_begin), this);
	gtk_signal_connect (GTK_OBJECT(draw), "drag_data_get",
				GTK_SIGNAL_FUNC(drag_data_get), this);
	gtk_signal_connect (GTK_OBJECT(draw), "selection_clear_event",
				GTK_SIGNAL_FUNC(selection_clear), this);
	gtk_container_add (GTK_CONTAINER (frame), draw);
	gtk_drawing_area_size(GTK_DRAWING_AREA(draw), w, h);
	gtk_widget_show(draw);
					// At bottom, a status bar.
	sbar = gtk_statusbar_new();
	sbar_sel = gtk_statusbar_get_context_id(GTK_STATUSBAR(sbar),
							"selection");
					// At the bottom, status bar & frame:
	gtk_box_pack_start(GTK_BOX(vbox), sbar, FALSE, FALSE, 0);
	gtk_widget_show(sbar);
	}

/*
 *	Delete.
 */

Palette_edit::~Palette_edit
	(
	)
	{
	gdk_rgb_cmap_free(palette);
	gtk_widget_destroy(get_widget());
	delete image;
	}

/*
 *	Unselect.
 */

void Palette_edit::unselect
	(
	bool need_render			// 1 to render and show.
	)
	{
	if (selected >= 0)
		{
		selected = -1;
		gtk_drag_source_unset(draw);
		if (need_render)
			{
			render();
			show();
			}
		}
	}

