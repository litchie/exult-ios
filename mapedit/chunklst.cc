/**
 **	A GTK widget showing the chunks from 'u7chunks'.
 **
 **	Written: 7/8/01 - JSF
 **/

/*
Copyright (C) 2001 The Exult Team

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
#include "chunklst.h"
#include "vgafile.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"
#include "exult_constants.h"
#include "shapeid.h"

using std::cout;
using std::endl;
using std::strlen;

/*
 *	Blit onto screen.
 */

inline void Chunk_chooser::show
	(
	int x, int y, int w, int h	// Area to blit.
	)
	{
	Shape_draw::show(draw->window, x, y, w, h);
	if (selected >= 0)		// Show selected.
		{
		Rectangle b = info[selected].box;
					// Draw yellow box.
		gdk_draw_rectangle(draw->window, drawgc, FALSE, 
							b.x, b.y, b.w, b.h);
		}
	}

/*
 *	Select an entry.  This should be called after rendering
 *	the chunk.
 */

void Chunk_chooser::select
	(
	int new_sel
	)
	{
	selected = new_sel;
	int chunknum = info[selected].num;
					// Remove prev. selection msg.
//	gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	char buf[150];			// Show new selection.
	g_snprintf(buf, sizeof(buf), "Chunk %d", chunknum);
	gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
	}

/*
 *	Render as many chunks as fit in the chunk chooser window.
 */

void Chunk_chooser::render
	(
	)
	{
	const int border = 2;		// Border at bottom, sides.
					// Look for selected frame.
	int selchunk = -1, new_selected = -1;
	if (selected >= 0)		// Save selection info.
		selchunk = info[selected].num;
					// Remove "selected" message.
	//gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	delete [] info;			// Delete old info. list.
					// Get drawing area dimensions.
	gint winw = draw->allocation.width, winh = draw->allocation.height;
					// Provide more than enough room.
	info = new Chunk_info[256];
	info_cnt = 0;			// Count them.
					// Clear window first.
	iwin->fill8(0);			// ++++Which color?
	int chunknum = chunknum0;
	chunkfile.seekg(chunknum*512);	// Get to first chunk.
					// 16x16 tiles, each 8x8 pixels.
	const int chunkw = 128, chunkh = 128;
	int y = border;
	while (chunknum < num_chunks && y + chunkh + border <= winh)
		{
		int x = border;
		while (chunknum < num_chunks && x + chunkw + border <= winw)
			{
			iwin->set_clip(x, y, chunkw, chunkh);
//			iwin->set_clip(0, 0, winw, winh);
			render_chunk(x, y);
			iwin->clear_clip();
					// Store info. about where drawn.
			info[info_cnt].set(chunknum, x, y, chunkw, chunkh);
			if (chunknum == selchunk)
						// Found the selected chunk.
				new_selected = info_cnt;
			info_cnt++;
			chunknum++;		// Next chunk.
			x += chunkw + border;
			}
		y += chunkh + border;
		}
	if (new_selected == -1)
		unselect(false);
	else
		select(new_selected);
	}

/*
 *	Render one chunk.  Assumes chunkfile is already set at correct spot
 *	to read.
 */

void Chunk_chooser::render_chunk
	(
	int xoff, int yoff		// Where to draw it in iwin.
	)
	{
	unsigned char buf[512];
	chunkfile.read(buf, 512);
	unsigned char *data = &buf[0];
	int y = c_tilesize;
	for (int ty = 0; ty < c_tiles_per_chunk; ty++, y += c_tilesize)
		{
		int x = c_tilesize;
		for (int tx = 0; tx < c_tiles_per_chunk; tx++,
							x += c_tilesize)
			{
			ShapeID id(data);
			Shape_frame *s = ifile->get_shape(id.get_shapenum(),
							id.get_framenum());
			s->paint(iwin, xoff + x - 1, yoff + y -1);
			}
		}
	}
	
/*
 *	Configure the viewing window.
 */

gint Chunk_chooser::configure
	(
	GtkWidget *widget,		// The view window.
	GdkEventConfigure *event,
	gpointer data			// ->Chunk_chooser
	)
	{
	Chunk_chooser *chooser = (Chunk_chooser *) data;
	chooser->Shape_draw::configure(widget);
	chooser->render();
	return (TRUE);
	}


/*
 *	Handle an expose event.
 */

gint Chunk_chooser::expose
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Chunk_chooser.
	)
	{
	Chunk_chooser *chooser = (Chunk_chooser *) data;
	chooser->show(event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Handle a mouse button press event.
 */

gint Chunk_chooser::mouse_press
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Chunk_chooser.
	)
	{
	Chunk_chooser *chooser = (Chunk_chooser *) data;
	int old_selected = chooser->selected;
					// Search through entries.
	for (int i = 0; i < chooser->info_cnt; i++)
		if (chooser->info[i].box.has_point(
					(int) event->x, (int) event->y))
			{		// Found the box?
			if (i == old_selected)
				return TRUE;
					// Indicate we can dra.
			GtkTargetEntry tents[1];
			tents[0].target = U7_TARGET_CHUNKID_NAME;
			tents[0].flags = 0;
			tents[0].info = U7_TARGET_CHUNKID;
			gtk_drag_source_set (chooser->draw, 
				GDK_BUTTON1_MASK, tents, 1,
			   (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE));
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
 *	Someone wants the dragged chunk.
 */

void Chunk_chooser::drag_data_get
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	GtkSelectionData *seldata,	// Fill this in.
	guint info,
	guint time,
	gpointer data			// ->Chunk_chooser.
	)
	{
	cout << "In DRAG_DATA_GET" << endl;
	Chunk_chooser *chooser = (Chunk_chooser *) data;
	if (chooser->selected < 0 || info != U7_TARGET_CHUNKID)
		return;			// Not sure about this.
	guchar buf[30];
	Chunk_info& shinfo = chooser->info[chooser->selected];
	int len = Store_u7_chunkid(buf, shinfo.num);
	cout << "Setting selection data (" << shinfo.num << ')' << endl;
					// Make us owner of xdndselection.
	gtk_selection_owner_set(widget, gdk_atom_intern("XdndSelection", 0),
								time);
					// Set data.
	gtk_selection_data_set(seldata,
			gdk_atom_intern(U7_TARGET_CHUNKID_NAME, 0),
                                				8, buf, len);
	}

/*
 *	Another app. has claimed the selection.
 */

gint Chunk_chooser::selection_clear
	(
	GtkWidget *widget,		// The view window.
	GdkEventSelection *event,
	gpointer data			// ->Chunk_chooser.
	)
	{
//	Chunk_chooser *chooser = (Chunk_chooser *) data;
	cout << "SELECTION_CLEAR" << endl;
	return TRUE;
	}

/*
 *	Beginning of a drag.
 */

gint Chunk_chooser::drag_begin
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	gpointer data			// ->Chunk_chooser.
	)
	{
	cout << "In DRAG_BEGIN" << endl;
	Chunk_chooser *chooser = (Chunk_chooser *) data;
	if (chooser->selected < 0)
		return FALSE;		// ++++Display a halt bitmap.
#if 0
					// Get ->chunk.
	Chunk_info& shinfo = chooser->info[chooser->selected];
	Chunk_frame *chunk = chooser->ifile->get_chunk(shinfo.chunknum, 
							shinfo.framenum);
	if (!chunk)
		return FALSE;
	int w = chunk->get_width(), h = chunk->get_height(),
		xright = chunk->get_xright(), ybelow = chunk->get_ybelow();
	Image_buffer8 tbuf(w, h);	// Create buffer to render to.
	tbuf.fill8(0xff);		// Fill with 'transparent' pixel.
	unsigned char *tbits = tbuf.get_bits();
	chunk->paint(&tbuf, w - 1 - xright, h - 1 - ybelow);
					// Put chunk on a pixmap.
	GdkPixmap *pixmap = gdk_pixmap_new(widget->window, w, h, -1);
	gdk_draw_indexed_image(pixmap, chooser->drawgc, 0, 0, w, h,
			GDK_RGB_DITHER_NORMAL, tbits,
			tbuf.get_line_width(), chooser->palette);
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
					// This will be the chunk dragged.
	gtk_drag_set_icon_pixmap(context,
			gdk_window_get_colormap(widget->window), pixmap, mask,
					w - 2 - xright, h - 2 - ybelow);
	gdk_pixmap_unref(pixmap);
	gdk_bitmap_unref(mask);
#endif
	return TRUE;
	}

/*
 *	Scroll to a new chunk/frame.
 */

void Chunk_chooser::scroll
	(
	int newindex			// Abs. index of leftmost to show.
	)
	{
	if (chunknum0 < newindex)	// Going forwards?
		chunknum0 = newindex < num_chunks ? newindex : num_chunks;
	else if (chunknum0 > newindex)	// Backwards?
		chunknum0 = newindex >= 0 ? newindex : 0;
	render();
	show();
	}

/*
 *	Handle a scrollbar event.
 */

void Chunk_chooser::scrolled
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Chunk_chooser.
	)
	{
	Chunk_chooser *chooser = (Chunk_chooser *) data;
cout << "Scrolled to " << adj->value << '\n';
	gint newindex = (gint) adj->value;
	chooser->scroll(newindex);
	}

/*
 *	Create the list.
 */

Chunk_chooser::Chunk_chooser
	(
	Vga_file *i,			// Where they're kept.
	istream& cfile,			// Chunks file (512bytes/entry).
	unsigned char *palbuf,		// Palette, 3*256 bytes (rgb triples).
	int w, int h			// Dimensions.
	) : Shape_draw(i, palbuf, gtk_drawing_area_new()),
		chunkfile(cfile), chunknum0(0),
		info(0), info_cnt(0), selected(-1), sel_changed(0)
	{
	chunkfile.seekg(0, ios::end);	// Figure total #chunks.
	num_chunks = chunkfile.tellg()/(c_tiles_per_chunk*2);
	guint32 colors[256];
	for (int i = 0; i < 256; i++)
		colors[i] = (palbuf[3*i]<<16)*4 + (palbuf[3*i+1]<<8)*4 + 
							palbuf[3*i+2]*4;
	palette = gdk_rgb_cmap_new(colors, 256);
					// Put things in a vert. box.
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	set_widget(vbox); // This is our "widget"
	gtk_widget_show(vbox);
	
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	
					// A frame looks nice.
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
					// NOTE:  draw is in Shape_draw.
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
//	gtk_signal_connect(GTK_OBJECT(draw), "motion_notify_event",
//				GTK_SIGNAL_FUNC(Mouse_drag_motion), this);
	gtk_signal_connect (GTK_OBJECT(draw), "drag_data_get",
				GTK_SIGNAL_FUNC(drag_data_get), this);
	gtk_signal_connect (GTK_OBJECT(draw), "selection_clear_event",
				GTK_SIGNAL_FUNC(selection_clear), this);
	gtk_container_add (GTK_CONTAINER (frame), draw);
	gtk_drawing_area_size(GTK_DRAWING_AREA(draw), w, h);
	gtk_widget_show(draw);
					// Want a scrollbar for the chunks.
	GtkObject *chunk_adj = gtk_adjustment_new(0, 0, 
				num_chunks, 1, 
				4, 1.0);
	GtkWidget *chunk_scroll = gtk_vscrollbar_new(
					GTK_ADJUSTMENT(chunk_adj));
					// Update window when it stops.
	gtk_range_set_update_policy(GTK_RANGE(chunk_scroll),
					GTK_UPDATE_DELAYED);
	gtk_box_pack_start(GTK_BOX(hbox), chunk_scroll, FALSE, TRUE, 0);
					// Set scrollbar handler.
	gtk_signal_connect(GTK_OBJECT(chunk_adj), "value_changed",
					GTK_SIGNAL_FUNC(scrolled), this);
	gtk_widget_show(chunk_scroll);
					// At the bottom, status bar:
	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
	gtk_widget_show(hbox1);
					// At left, a status bar.
	sbar = gtk_statusbar_new();
	sbar_sel = gtk_statusbar_get_context_id(GTK_STATUSBAR(sbar),
							"selection");
	gtk_box_pack_start(GTK_BOX(hbox1), sbar, TRUE, TRUE, 0);
	gtk_widget_show(sbar);
	}

/*
 *	Delete.
 */

Chunk_chooser::~Chunk_chooser
	(
	)
	{
	gtk_widget_destroy(get_widget());
	delete [] info;
	}
	
/*
 *	Unselect.
 */

void Chunk_chooser::unselect
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
		if (sel_changed)	// Tell client.
			(*sel_changed)();
		}
	char buf[150];			// Show new selection.
	if (info_cnt > 0)
		{
//		gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
		g_snprintf(buf, sizeof(buf), "Chunks %d to %d",
			info[0].num, info[info_cnt - 1].num);
		gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
		}
	}


