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

#ifdef WIN32
#include "Windrag.h"
#endif

#include <gtk/gtk.h>
#ifdef XWIN
#include <gdk/gdkx.h>
#endif
#include <glib.h>
#include "chunklst.h"
#include "vgafile.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"
#include "exult_constants.h"
#include "shapeid.h"
#include "studio.h"
#include "utils.h"

#include <iosfwd>

using std::cout;
using std::endl;
using std::strlen;

const int border = 2;			// Border at bottom, sides of each
					//   chunk.

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
	if (new_sel < 0 || new_sel >= info_cnt)
		return;			// Bad value.
	selected = new_sel;
	enable_controls();
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
	int chunknum = index0;
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
			render_chunk(chunknum, x, y);
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
 *	Read in desired chunk if not already read.
 *
 *	Output:	->chunk, stored in chunklist.
 */

unsigned char *Chunk_chooser::get_chunk
	(
	int chunknum
	)
	{
	unsigned char *data = chunklist[chunknum];
	if (data)
		return data;		// Already have it.
					// Get from server.
	unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	unsigned char *newptr = &buf[0];
	Write2(ptr, chunknum);
	ExultStudio *studio = ExultStudio::get_instance();
	int server_socket = studio->get_server_socket();
	Exult_server::Msg_type id;	// Expect immediate answer.
	int datalen;
	if (!studio->send_to_server(Exult_server::send_terrain, 
							buf, ptr - buf) ||
		!Exult_server::wait_for_response(server_socket, 100) ||
		(datalen = Exult_server::Receive_data(server_socket, 
						id, buf, sizeof(buf))) == -1 ||
		id != Exult_server::send_terrain ||
		Read2(newptr) != chunknum)
		{			// No server?  Get from file.
		data = new unsigned char[512];
		chunklist[chunknum] = data;
		chunkfile.seekg(chunknum*512);
		chunkfile.read(reinterpret_cast<char *>(data), 512);
		if (!chunkfile.good())
			{
			memset(data, 0, 512);
			cout << "Error reading chunk file" << endl;
			}
		}
	else
		set_chunk(buf, datalen);
	return chunklist[chunknum];
	}

/*
 *	Set chunk with data from 'Exult'.
 *
 *	NOTE:  Don't call 'show()' or 'render()' here, since this gets called
 *		from 'render()'.
 */

void Chunk_chooser::set_chunk
	(
	unsigned char *data,		// Message from server.
	int datalen
	)
	{
	int tnum = Read2(data);		// First the terrain #.
	int new_num_chunks = Read2(data);	// Always sends total.
	datalen -= 4;
	if (datalen != 512)
		{
		cout << "Set_chunk:  Wrong data length" << endl;
		return;
		}
	if (tnum < 0 || tnum >= new_num_chunks)
		{
		cout << "Set_chunk:  Bad terrain # (" << tnum <<
						") received" << endl;
		return;
		}
	if (new_num_chunks != num_chunks)
		{			// Update total #.
		num_chunks = new_num_chunks;
		GtkAdjustment *adj = 
			gtk_range_get_adjustment(GTK_RANGE(chunk_scroll));
		adj->upper = num_chunks;
		gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
		}
	unsigned char *chunk = chunklist[tnum];
	if (!chunk)			// Not read yet?
		chunk = chunklist[tnum] = new unsigned char[512];
	memcpy(chunk, data, 512);	// Copy it in.
	}

/*
 *	Render one chunk.
 */

void Chunk_chooser::render_chunk
	(
	int chunknum,			// # to render.
	int xoff, int yoff		// Where to draw it in iwin.
	)
	{
	unsigned char *data = get_chunk(chunknum);
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
			if (s)
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
					// Set new scroll amounts.
	int w = event->width, h = event->height;
	int per_row = (w - border)/(128 + border);
	int num_rows = (h - border)/(128 + border);
	int page_size = per_row*num_rows;
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(
						chooser->chunk_scroll));
	adj->step_increment = per_row;
	adj->page_increment = page_size;
	adj->page_size = page_size;
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
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

#ifdef WIN32

static bool win32_button = false;

gint Chunk_chooser::win32_drag_motion
	(
	GtkWidget *widget,		// The view window.
	GdkEventMotion *event,
	gpointer data			// ->Chunk_chooser.
	)
	{
	  if (win32_button)
	  {
		win32_button = false;

		// prepare the dragged data
		windragdata wdata;

		// This call allows us to recycle the data transfer initialization code.
		//  It's clumsy, but far easier to maintain.
		drag_data_get(NULL, NULL, (GtkSelectionData *) &wdata,
		  U7_TARGET_CHUNKID, 0, data);

		POINT pnt;
		GetCursorPos(&pnt);

		LPDROPSOURCE idsrc = (LPDROPSOURCE) new Windropsource(0, 
		  pnt.x, pnt.y);
		LPDATAOBJECT idobj = (LPDATAOBJECT) new Winstudioobj(wdata);
		DWORD dndout;

		HRESULT res = DoDragDrop(idobj, idsrc, DROPEFFECT_COPY, &dndout);
		if (FAILED(res)) {
		  g_warning ("Oops! Something is wrong with OLE2 DnD..");
		}

		delete idsrc;
		idobj->Release();	// Not sure if we really need this. However, it doesn't hurt either.
	  }

	return true;
	};

#endif

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
#ifdef WIN32
// Here, we have to override GTK+'s Drag and Drop, which is non-OLE and
// usually stucks outside the program window. I think it's because
// the dragged shape only receives mouse motion events when the new mouse pointer
// position is *still* inside the shape. So if you move the mouse too fast,
// we are stuck.
			win32_button = true;
#else
			GtkTargetEntry tents[1];
			tents[0].target = U7_TARGET_CHUNKID_NAME;
			tents[0].flags = 0;
			tents[0].info = U7_TARGET_CHUNKID;
			gtk_drag_source_set (chooser->draw, 
				GDK_BUTTON1_MASK, tents, 1,
			   (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE));
#endif

			chooser->selected = i;
			chooser->locate_cx = chooser->locate_cy = -1;
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
#ifdef WIN32
	windragdata *wdata = (windragdata *)seldata;
	wdata->data = buf;
	wdata->id = info;
#else
					// Make us owner of xdndselection.
	gtk_selection_owner_set(widget, gdk_atom_intern("XdndSelection", 0),
								time);
					// Set data.
	gtk_selection_data_set(seldata,
			gdk_atom_intern(U7_TARGET_CHUNKID_NAME, 0),
                                				8, buf, len);
#endif
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
	if (index0 < newindex)	// Going forwards?
		index0 = newindex < num_chunks ? newindex : num_chunks;
	else if (index0 > newindex)	// Backwards?
		index0 = newindex >= 0 ? newindex : 0;
	render();
	show();
	}

/*
 *	Scroll up/down by one row.
 */

void Chunk_chooser::scroll
	(
	bool upwards
	)
	{
	GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(chunk_scroll));
	float delta = adj->step_increment;
	if (upwards)
		delta = -delta;
	adj->value += delta;
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
	scroll((gint) adj->value);
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
 *	Callbacks for controls:
 */
C_EXPORT void
on_loc_chunk_down_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	Chunk_chooser *chooser = (Chunk_chooser *) user_data;
	chooser->locate(false);
}
C_EXPORT void
on_loc_chunk_up_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	Chunk_chooser *chooser = (Chunk_chooser *) user_data;
	chooser->locate(true);
}

C_EXPORT void
on_insert_chunk_new_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	Chunk_chooser *chooser = (Chunk_chooser *) user_data;
	chooser->insert(false);
}
C_EXPORT void
on_insert_chunk_dup_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	Chunk_chooser *chooser = (Chunk_chooser *) user_data;
	chooser->insert(true);
}

C_EXPORT void
on_move_chunk_down_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	Chunk_chooser *chooser = (Chunk_chooser *) user_data;
	chooser->move(false);
}
C_EXPORT void
on_move_chunk_up_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	Chunk_chooser *chooser = (Chunk_chooser *) user_data;
	chooser->move(true);
}
/*
 *	Create box with 'find' and 'edit' controls.
 */

GtkWidget *Chunk_chooser::create_controls
	(
	)
	{
					// Create main box.
	GtkWidget *topframe = gtk_frame_new (NULL);
	gtk_widget_show(topframe);
	GtkWidget *hbox0 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox0);
	gtk_container_add (GTK_CONTAINER (topframe), hbox0);
	/*
	 *	The 'Locate' controls.
	 */
	GtkWidget *frame = gtk_frame_new ("Locate");
	gtk_widget_show(frame);
	gtk_box_pack_start (GTK_BOX (hbox0), frame, FALSE, FALSE, 2);
	GtkWidget *hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox);
	gtk_container_add (GTK_CONTAINER (frame), hbuttonbox);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), 
							GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox), 0);

	loc_chunk_down = gtk_button_new_with_label ("Down");
	gtk_widget_show (loc_chunk_down);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), loc_chunk_down);
	GTK_WIDGET_SET_FLAGS (loc_chunk_down, GTK_CAN_DEFAULT);

	loc_chunk_up = gtk_button_new_with_label ("Up");
	gtk_widget_show (loc_chunk_up);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), loc_chunk_up);
	GTK_WIDGET_SET_FLAGS (loc_chunk_up, GTK_CAN_DEFAULT);
	/*
	 *	The 'Insert' controls.
	 */
	frame = gtk_frame_new ("Insert");
	gtk_widget_show(frame);
	gtk_box_pack_start (GTK_BOX (hbox0), frame, FALSE, FALSE, 2);
	hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox);
	gtk_container_add (GTK_CONTAINER (frame), hbuttonbox);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), 
							GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox), 0);

	GtkWidget *insert_chunk_new = gtk_button_new_with_label ("New");
	gtk_widget_show (insert_chunk_new);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), insert_chunk_new);
	GTK_WIDGET_SET_FLAGS (insert_chunk_new, GTK_CAN_DEFAULT);

	insert_chunk_dup = gtk_button_new_with_label ("Dup");
	gtk_widget_show (insert_chunk_dup);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), insert_chunk_dup);
	GTK_WIDGET_SET_FLAGS (insert_chunk_dup, GTK_CAN_DEFAULT);
	/*
	 *	The 'Move' controls.
	 */
	frame = gtk_frame_new ("Move");
	gtk_widget_show(frame);
	gtk_box_pack_start (GTK_BOX (hbox0), frame, FALSE, FALSE, 2);
	hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox);
	gtk_container_add (GTK_CONTAINER (frame), hbuttonbox);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox), 
							GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox), 0);

	move_chunk_down = gtk_button_new_with_label ("Down");
	gtk_widget_show (move_chunk_down);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), move_chunk_down);
	GTK_WIDGET_SET_FLAGS (move_chunk_down, GTK_CAN_DEFAULT);

	move_chunk_up = gtk_button_new_with_label ("Up");
	gtk_widget_show (move_chunk_up);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), move_chunk_up);
	GTK_WIDGET_SET_FLAGS (move_chunk_up, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT (loc_chunk_down), "clicked",
                      GTK_SIGNAL_FUNC (on_loc_chunk_down_clicked),
                      this);
	gtk_signal_connect (GTK_OBJECT (loc_chunk_up), "clicked",
                      GTK_SIGNAL_FUNC (on_loc_chunk_up_clicked),
                      this);
	gtk_signal_connect (GTK_OBJECT (insert_chunk_new), "clicked",
			GTK_SIGNAL_FUNC (on_insert_chunk_new_clicked),
			this);
	gtk_signal_connect (GTK_OBJECT (insert_chunk_dup), "clicked",
			GTK_SIGNAL_FUNC (on_insert_chunk_dup_clicked),
			this);
	gtk_signal_connect (GTK_OBJECT (move_chunk_down), "clicked",
			GTK_SIGNAL_FUNC (on_move_chunk_down_clicked),
			this);
	gtk_signal_connect (GTK_OBJECT (move_chunk_up), "clicked",
			GTK_SIGNAL_FUNC (on_move_chunk_up_clicked),
			this);
	return topframe;
	}

/*
 *	Enable/disable controls after selection changed.
 */

void Chunk_chooser::enable_controls
	(
	)
	{
	if (selected == -1)		// No selection.
		{
		gtk_widget_set_sensitive(loc_chunk_down, false);
		gtk_widget_set_sensitive(loc_chunk_up, false);
		gtk_widget_set_sensitive(insert_chunk_dup, false);
		gtk_widget_set_sensitive(move_chunk_down, false);
		gtk_widget_set_sensitive(move_chunk_up, false);
		return;
		}
	gtk_widget_set_sensitive(loc_chunk_down, true);
	gtk_widget_set_sensitive(loc_chunk_up, true);
	gtk_widget_set_sensitive(insert_chunk_dup, true);
	gtk_widget_set_sensitive(move_chunk_down, 
					info[selected].num < num_chunks - 1);
	gtk_widget_set_sensitive(move_chunk_up, info[selected].num > 0);
	}

/*
 *	Create the list.
 */

Chunk_chooser::Chunk_chooser
	(
	Vga_file *i,			// Where they're kept.
	std::istream& cfile,		// Chunks file (512bytes/entry).
	unsigned char *palbuf,		// Palette, 3*256 bytes (rgb triples).
	int w, int h,			// Dimensions.
	Shape_group *g			// Filter, or null.
	) : Object_browser(g), Shape_draw(i, palbuf, gtk_drawing_area_new()),
		chunkfile(cfile), index0(0),
		info(0), info_cnt(0), selected(-1), sel_changed(0),
		locate_cx(-1), locate_cy(-1)
	{
	chunkfile.seekg(0, std::ios::end);	// Figure total #chunks.
	num_chunks = chunkfile.tellg()/(c_tiles_per_chunk*c_tiles_per_chunk*2);
	chunklist.resize(num_chunks);	// Init. list of ->'s to chunks.
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
#ifdef WIN32
// required to override GTK+ Drag and Drop
	gtk_signal_connect(GTK_OBJECT(draw), "motion_notify_event",
				GTK_SIGNAL_FUNC(win32_drag_motion), this);
#endif
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
	chunk_scroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(chunk_adj));
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
					// Add search/edit controls to bottom.
	gtk_box_pack_start(GTK_BOX(vbox), create_controls(), FALSE, FALSE, 0);
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
	int i;
	for (i = 0; i < num_chunks; i++)// Delete all the chunks.
		delete chunklist[i];
	}

/*
 *	Handle response from server.
 *
 *	Output:	true if handled here.
 */

bool Chunk_chooser::server_response
	(
	int id,
	unsigned char *data,
	int datalen
	)
	{
	switch ((Exult_server::Msg_type) id)
		{
	case Exult_server::locate_terrain:
		locate_response(data, datalen);
		return true;
	case Exult_server::insert_terrain:
		insert_response(data, datalen);
		return true;
	case Exult_server::swap_terrain:
		swap_response(data, datalen);
		return true;
	case Exult_server::send_terrain:
		set_chunk(data, datalen);
		render();
		show();
		return true;
	default:
		return false;
		}
	}

/*
 *	Done with terrain editing.
 */

void Chunk_chooser::end_terrain_editing
	(
	)
	{
					// Clear out cache of chunks.
	for (int i = 0; i < num_chunks; i++)
		{
		delete chunklist[i];
		chunklist[i] = 0;
		}
	render();
	show();
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
		locate_cx = locate_cy = -1;
		gtk_drag_source_unset(draw);
		if (need_render)
			{
			render();
			show();
			}
		if (sel_changed)	// Tell client.
			(*sel_changed)();
		}
	enable_controls();		// Enable/disable controls.
	char buf[150];			// Show new selection.
	if (info_cnt > 0)
		{
//		gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
		g_snprintf(buf, sizeof(buf), "Chunks %d to %d",
			info[0].num, info[info_cnt - 1].num);
		gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
		}
	}

/*
 *	Locate terrain on game map.
 */

void Chunk_chooser::locate
	(
	bool upwards
	)
	{
	if (selected < 0)
		return;			// Shouldn't happen.
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	int tnum = info[selected].num;	// Terrain #.
	Write2(ptr, tnum);
	Write2(ptr, locate_cx);		// Current chunk, or -1.
	Write2(ptr, locate_cy);
	*ptr++ = upwards ? 1 : 0;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->send_to_server(
			Exult_server::locate_terrain, data, ptr - data);
	}

/*
 *	Response from server to a 'locate'.
 */

void Chunk_chooser::locate_response
	(
	unsigned char *data,
	int datalen
	)
	{
	unsigned char *ptr = data;
	int tnum = Read2(ptr);
	if (selected < 0 || tnum != info[selected].num)
		return;			// Not the current selection.
	short cx = (short) Read2(ptr);	// Get chunk found.
	short cy = (short) Read2(ptr);
	ptr++;				// Skip upwards flag.
	if (!*ptr)
		cout << "Terrain not found." << endl;
	else
		{
		locate_cx = cx;		// Save new chunk.
		locate_cy = cy;
		}
	}

/*
 *	Insert a new chunk terrain into the list.
 */

void Chunk_chooser::insert
	(
	bool dup
	)
	{
	if (dup && selected < 0)
		return;			// Shouldn't happen.
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	int tnum = selected >= 0 ? info[selected].num : -1;
	Write2(ptr, tnum);
	*ptr++ = dup ? 1 : 0;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->send_to_server(
			Exult_server::insert_terrain, data, ptr - data);
	}

/*
 *	Response from server to an 'insert'.
 */

void Chunk_chooser::insert_response
	(
	unsigned char *data,
	int datalen
	)
	{
	unsigned char *ptr = data;
	int tnum = (short) Read2(ptr);
	bool dup = *ptr++ ? true : false;
	bool okay = *ptr ? true : false;
	if (!*ptr)
		cout << "Terrain insert failed." << endl;
	else
		{			// Insert in our list.
		unsigned char *data = new unsigned char[512];
		if (dup && tnum >= 0 && tnum < num_chunks && chunklist[tnum])
			memcpy(data, chunklist[tnum], 512);
		else
			memset(data, 0, 512);
		if (tnum >= 0 && tnum < num_chunks - 1)
			chunklist.insert(chunklist.begin() + tnum + 1, data);
		else			// If -1, append to end.
			chunklist.push_back(data);
		num_chunks++;
		render();
		show();
		}
	}

/*
 *	Move currently-selected chunk up or down.
 */

void Chunk_chooser::move
	(
	bool upwards
	)
	{
	if (selected < 0)
		return;			// Shouldn't happen.
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	int tnum = info[selected].num;
	if ((tnum == 0 && upwards) || (tnum == num_chunks - 1 && !upwards))
		return;
	if (upwards)			// Going to swap tnum & tnum+1.
		tnum--;
	Write2(ptr, tnum);
	ExultStudio *studio = ExultStudio::get_instance();
	studio->send_to_server(
			Exult_server::swap_terrain, data, ptr - data);
	}

/*
 *	Response from server to a 'swap'.
 */

void Chunk_chooser::swap_response
	(
	unsigned char *data,
	int datalen
	)
	{
	unsigned char *ptr = data;
	int tnum = (short) Read2(ptr);
	bool okay = *ptr ? true : false;
	if (!*ptr)
		cout << "Terrain insert failed." << endl;
	else if (tnum >= 0 && tnum < num_chunks - 1)
		{
		unsigned char *tmp = get_chunk(tnum);
		chunklist[tnum] = get_chunk(tnum + 1);
		chunklist[tnum + 1] = tmp;
		if (selected >= 0)	// Update selected.
			{
			if (info[selected].num == tnum)
				{	// Moving downwards.
				if (selected >= info_cnt - 1)
					scroll(false);
				select(selected + 1);
				}
			else if (info[selected].num == tnum + 1)
				{
				if (selected <= 0)
					scroll(true);
				select(selected - 1);
				}
			}
		render();
		show();
		}
	}
