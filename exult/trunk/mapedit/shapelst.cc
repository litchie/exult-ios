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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WIN32
#include "Windrag.h"
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#ifdef XWIN
#include <gdk/gdkx.h>
#endif
#include <glib.h>
#include <stdlib.h>
#include "shapelst.h"
#include "shapevga.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"
#include "studio.h"
#include "utils.h"
#include "shapegroup.h"
#include "shapefile.h"
#include "pngio.h"

using std::cout;
using std::endl;
using std::strlen;

/*
 *	Callback for when a shape is dropped on our draw area.
 */

static void Shape_dropped_here
	(
	int file,			// U7_SHAPE_SHAPES.
	int shape,
	int frame,
	void *udata
	)
	{
	((Shape_chooser *) udata)->shape_dropped_here(file, shape, frame);
	}
					// Schedule names.
/*
 *	Blit onto screen.
 */

void Shape_chooser::show
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
 *	Send selected shape/frame to Exult.
 */

void Shape_chooser::tell_server_shape
	(
	)
	{
	int shnum = -1, frnum = 0;
	if (selected >= 0)
		{
		shnum = info[selected].shapenum;
		frnum = info[selected].framenum;
		}
	unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Write2(ptr, shnum);
	Write2(ptr, frnum);
	ExultStudio *studio = ExultStudio::get_instance();
	studio->send_to_server(Exult_server::set_edit_shape, buf, ptr - buf);
	}

/*
 *	Select an entry.  This should be called after rendering
 *	the shape.
 */

void Shape_chooser::select
	(
	int new_sel
	)
	{
	selected = new_sel;
	tell_server_shape();		// Tell Exult.
	int shapenum = info[selected].shapenum;
					// Update spin-button value, range.
	gtk_widget_set_sensitive(fspin, true);
	gtk_adjustment_set_value(frame_adj, info[selected].framenum);
	int nframes = ifile->get_num_frames(shapenum);
	frame_adj->upper = nframes - 1;
	gtk_adjustment_changed(frame_adj);
	gtk_widget_set_sensitive(fspin, true);
					// Remove prev. selection msg.
//	gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	char buf[150];			// Show new selection.
	g_snprintf(buf, sizeof(buf), "Shape %d (%d frames)",
						shapenum, nframes);
	if (names && names[shapenum])
		{
		int len = strlen(buf);
		g_snprintf(buf + len, sizeof(buf) - len, 
						":  '%s'", names[shapenum]);
		}
	gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
	}

const int border = 4;			// Border at bottom, sides.
/*
 *	Render as many shapes as fit in the shape chooser window.
 */

void Shape_chooser::render
	(
	)
	{
	if (frames_mode)
		{
		render_frames();
		return;
		}
					// Look for selected frame.
	int selshape = -1, selframe = -1, new_selected = -1;
	if (selected >= 0)		// Save selection info.
		{
		selshape = info[selected].shapenum;
		selframe = info[selected].framenum;
		}
					// Remove "selected" message.
	//gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	delete [] info;			// Delete old info. list.
					// Get drawing area dimensions.
	gint winw = draw->allocation.width, winh = draw->allocation.height;
					// Provide more than enough room.
	info = new Shape_entry[1024];
					// Clear window first.
	iwin->fill8(0);			// ++++Which color?
	int x = 0;
	info_cnt = 0;			// Count them.
	int curr_y = 0;
	int row_h = 0;
	int row = row0;			// Row #.
	int total_cnt = get_count();
	int index;			// This is shapenum if there's no
					//   filter (group).
	for (int index = index0; index < total_cnt; index++)
		{
		int shapenum = group ? (*group)[index] : index;
		int framenum = shapenum == selshape ? selframe : framenum0;
		Shape_frame *shape = ifile->get_shape(shapenum, framenum);
		if(shape)
			{
			int sh = shape->get_height(),
			    sw = shape->get_width();
			if (sh>row_h)
				row_h = sh;
					// Check if we've exceeded max width
			if (x + sw > winw)
				{	// Next line.
				curr_y += row_h + border;
				row_h = sh;
				x = 0;
				row++;
				if (row == row_indices.size())
					row_indices.push_back(index);
				else if (row < row_indices.size())
					row_indices[row] = index;
				if (curr_y + 36 >= winh)
					break;
				}
			int sy = curr_y+border;	// Get top y-coord.
			shape->paint(iwin, x + shape->get_xleft(),
						sy + shape->get_yabove());
			if (sh > winh)
				{
				sy += sh - winh;
				sh = winh;
				}
					// Store info. about where drawn.
			info[info_cnt].set(shapenum, framenum, x, sy, sw, sh);
			if (shapenum == selshape)
						// Found the selected shape.
				new_selected = info_cnt;
			x += sw + border;
			info_cnt++;
			}
		}
	nrows = row - row0;		// # rows shown.
	nrows += (x > 0);		// Add partial row at end.
	if (new_selected == -1)
		unselect(false);
	else
		select(new_selected);
	adjust_vscrollbar();		// Set new scroll values.
}

/*
 *	Get maximum shape height for all its frames.
 */

static int Get_max_height
	(
	Shape *shape
	)
	{
	int cnt = shape->get_num_frames();
	int maxh = 0;
	for (int i = 0; i < cnt; i++)
		{
		int ht = shape->get_frame(i)->get_height();
		if (ht > maxh)
			maxh = ht;
		}
	return maxh;
	}

/*
 *	Get the x-offset in pixels where a frame will be drawn.
 *
 *	Output:	Offset from left edge of (virtual) drawing area.
 */

static int Get_x_offset
	(
	Shape *shape,
	int framenum
	)
	{
	if (!shape)
		return 0;
	int nframes = shape->get_num_frames();
	if (framenum >= nframes)
		framenum = nframes - 1;
	int xoff = 0;
	for (int i = 0; i < framenum; i++)
		xoff += shape->get_frame(i)->get_width() + border;
	return xoff;
	}

/*
 *	Render one shape per row, showing its frames from left to right.
 */

void Shape_chooser::render_frames
	(
	)
	{
					// Get drawing area dimensions.
	gint winw = draw->allocation.width, winh = draw->allocation.height;
					// Look for selected frame.
	int selshape = -1, selframe = -1, new_selected = -1;
	if (selected >= 0)		// Save selection info.
		{
		selshape = info[selected].shapenum;
		selframe = info[selected].framenum;
		}
					// Remove "selected" message.
	//gtk_statusbar_pop(GTK_STATUSBAR(sbar), sbar_sel);
	delete [] info;			// Delete old info. list.
	iwin->set_clip(0, 0, winw, winh);
					// Provide more than enough room.
	info = new Shape_entry[1024];
					// Clear window first.
	iwin->fill8(0);			// ++++Which color?
	info_cnt = 0;			// Count them.
	int curr_y = 0;
	int row = row0;			// Row #.
	int total_cnt = get_count();
	int index;			// This is shapenum if there's no
					//   filter (group).
	for (int index = index0; index < total_cnt; index++)
		{
		int shapenum = group ? (*group)[index] : index;
					// Get all frames.
		Shape *shape = ifile->extract_shape(shapenum);
		if (!shape)
			continue;
		int nframes = shape->get_num_frames();
		int row_h = Get_max_height(shape);
		int x = -hoffset;
		int sw, sh;
		for (int framenum = 0; framenum < nframes; framenum++,
						x += sw + border)
			{
			if (x >= winw)	// Past right edge?
				break;
			Shape_frame *frame = shape->get_frame(framenum);
			sh = frame->get_height();
			sw = frame->get_width();
			if (x < 0 && x + sw < sw/2)
				continue;// Skip to left of hoffset.
			int sy = curr_y+border;	// Get top y-coord.
			frame->paint(iwin, x + frame->get_xleft(),
						sy + frame->get_yabove());
			if (sh > winh)
				{
				sy += sh - winh;
				sh = winh;
				}
					// Store info. about where drawn.
			info[info_cnt].set(shapenum, framenum, x, sy, sw, sh);
			if (shapenum == selshape && framenum == selframe)
						// Found the selected shape.
				new_selected = info_cnt;
			info_cnt++;
			}
					// Next line.
		curr_y += row_h + border;
		x = 0;
		if (row == row_indices.size())
			row_indices.push_back(index);
		else if (row < row_indices.size())
			row_indices[row] = index;
		row++;
		if (curr_y + 36 >= winh)
			break;
		}
	int nrows = row - row0;		// # rows shown.
	if (new_selected == -1)
		unselect(false);
	else
		select(new_selected);
	iwin->clear_clip();
	adjust_vscrollbar();		// Set new scroll values.
}

/*
 *	Horizontally scroll so that the selected frame is visible (in frames
 *	mode).
 */

void Shape_chooser::scroll_to_frame
	(
	)
	{
	if (selected >= 0)		// Save selection info.
		{
		int selshape = info[selected].shapenum;
		int selframe = info[selected].framenum;
		Shape *shape = ifile->extract_shape(selshape);
		int xoff = Get_x_offset(shape, selframe);
		if (xoff < hoffset)	// Left of visual area?
			hoffset = xoff > border ? xoff - border : 0;
		else
			{
			gint winw = draw->allocation.width;
			int sw = shape->get_frame(selframe)->get_width();
			if (xoff + sw + border - hoffset > winw)
				hoffset = xoff + sw + border - winw;
			}
		GtkAdjustment *adj = gtk_range_get_adjustment(
						GTK_RANGE(shape_hscroll));
		gtk_adjustment_set_value(adj, hoffset);
		}
	}

/*
 *	Find start of next row.
 *
 *	Output:	Index of start of next row, or -1 if given is last.
 */

int Shape_chooser::next_row
	(
	int start			// Index of row to start at.
	)
	{
	int total_cnt = get_count();
	if (frames_mode)		// Easy if 1 shape/row.
		return start < total_cnt - 1 ? (start + 1) : -1;
	int selshape = -1, selframe = -1;
	if (selected >= 0)		// Save selection info.
		{
		selshape = info[selected].shapenum;
		selframe = info[selected].framenum;
		}
	gint winw = draw->allocation.width;
	int index = start;
	int x = 0;
	while (index < total_cnt)
		{
		int shapenum = group ? (*group)[index] : index;
		int framenum = shapenum == selshape ? selframe : framenum0;
		Shape_frame *shape = ifile->get_shape(shapenum, framenum);
		if(shape)
			{
			int sw = shape->get_width();
			if (x + sw > winw)
				break;	// Done.
			x += sw + border;
			}
		index++;
		}
	if (index == start)		// Always advance at least 1.
		index++;
	if (index == total_cnt)
		return -1;		// Past end.
	return index;
	}	

#if 0	/* ++++++I think not needed */
/*
 *	Find start of prev. row.
 *
 *	Output:	Index of start of prev row, or -1 if given is first.
 */

int Shape_chooser::prev_row
	(
	int start			// Index of row to start at.
	)
	{
	if (start == 0)
		return -1;		// Easy.
	int selshape = -1, selframe = -1;
	if (selected >= 0)		// Save selection info.
		{
		selshape = info[selected].shapenum;
		selframe = info[selected].framenum;
		}
	int x = draw->allocation.width;
	int index = start;
	while (index > 0)
		{
					// Look at preceding shape.
		int shapenum = group ? (*group)[index - 1] : index;
		int framenum = shapenum == selshape ? selframe : framenum0;
		Shape_frame *shape = ifile->get_shape(shapenum, framenum);
		if(shape)
			{
			int sw = shape->get_width();
			if (x - sw < 0)
				break;	// Done.
			x -= sw + border;
			}
		index--;
		}
	if (index == start)		// Always advance at least 1.
		index--;
	return index;
	}	
#endif

/*
 *	Scroll so a desired index is in view.
 */

void Shape_chooser::goto_index
	(
	int index			// Desired index.
	)
	{
	int total = get_count();	// Total #entries.
	assert (index >= 0 && index < total);
	if (index < index0)		// Above current view?
		{
		do
			index0 = row_indices[--row0];
		while (index < index0);
		}
	else if (index >= index0 + info_cnt)
		{			// Below current view.
		do
			{
			if (row0 < row_indices.size() - 1)
				index0 = row_indices[++row0];
			else
				{
				int i = next_row(index0);
				if (i < 0)	// Past end.  Shouldn't happen.
					break;
				row_indices.push_back(i);
				row0++;
				index0 = i;
				}
			}
		while (index0 < index);
		if (index != index0)
			row0--;		// We passed it.
		index0 = row_indices[row0];
		info_cnt = 0;
		}
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
	chooser->Shape_draw::configure(widget);
	chooser->row_indices.resize(1);	// Start over with row info.
	chooser->row0 = 0;
	chooser->info_cnt = 0;
	int i0 = chooser->index0;	// Get back to where we were.
	chooser->index0 = 0;
	chooser->goto_index(i0);
	chooser->index0 = chooser->row_indices[chooser->row0];
	chooser->adjust_hscrollbar(-1);
	chooser->render();		// This also adjusts scrollbar.
					// Set handler for shape dropped here,
					//   BUT not more than once.
	if (chooser->drop_callback != Shape_dropped_here)
		chooser->enable_drop(Shape_dropped_here, chooser);
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

#ifdef WIN32

static bool win32_button = false;

gint Shape_chooser::win32_drag_motion
	(
	GtkWidget *widget,		// The view window.
	GdkEventMotion *event,
	gpointer data			// ->Shape_chooser.
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
		  U7_TARGET_SHAPEID, 0, data);

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

gint Shape_chooser::mouse_press
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
	int old_selected = chooser->selected;
					// Search through entries.
	for (int i = 0; i < chooser->info_cnt; i++)
		if (chooser->info[i].box.has_point(
					(int) event->x, (int) event->y))
			{		// Found the box?
//			if (i == old_selected)
//				return TRUE;
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
			tents[0].target = U7_TARGET_SHAPEID_NAME;
			tents[0].flags = 0;
			tents[0].info = U7_TARGET_SHAPEID;
			gtk_drag_source_set (chooser->draw, 
				GDK_BUTTON1_MASK, tents, 1,
			   (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE));
#endif

			chooser->selected = i;
			chooser->render();
			chooser->show();
					// Tell client.
			if (chooser->sel_changed)
				(*chooser->sel_changed)();
			break;
			}
	if (chooser->selected == old_selected && old_selected >= 0)
		{			// Same square.  Check for dbl-click.
		if (((GdkEvent *) event)->type == GDK_2BUTTON_PRESS)
			chooser->edit_shape_info();
		}
	if (event->button == 3 && chooser->selected >= 0)
		gtk_menu_popup(GTK_MENU(chooser->create_popup()), 
				0, 0, 0, 0, event->button, event->time);
	return (TRUE);
	}

/*
 *	Bring up the shape-info editor for the selected shape.
 */

void Shape_chooser::edit_shape_info
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	int shnum = info[selected].shapenum,
	    frnum = info[selected].framenum;
	Shape_info *info = 0;
	if (shapes_file)
		{			// Read info. the first time.
		shapes_file->read_info(false, true);//+++++BG?
		info = &shapes_file->get_info(shnum);
		}
	studio->open_shape_window(shnum, frnum, ifile,
					names ? names[shnum] : 0, info);
	}

/*
 *	Bring up the user's image-editor for the selected shape.
 */

void Shape_chooser::edit_shape
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	int shnum = info[selected].shapenum,
	    frnum = info[selected].framenum;
	string filestr("<GAME>");	// Set up filename.
	filestr += "/itmp";		// "Image tmp" directory.
	U7mkdir(filestr.c_str(), 0755);	// Create if not already there.
					// Create name from file,shape,frame.
	char *ext = g_strdup_printf("/%s.s%d_f%d.png",
				file_info->get_basename(), shnum, frnum);
	filestr += ext;
	g_free(ext);
					// Lookup <GAME>.
	filestr = get_system_path(filestr);
	cout << "Writing image '" << filestr.c_str() << "'" << endl;
	Shape_frame *frame = ifile->get_shape(shnum, frnum);
	int w = frame->get_width(), h = frame->get_height();
	Image_buffer8 img(w, h);	// Render into a buffer.
	const unsigned char transp = 255;
	img.fill8(transp);		// Fill with transparent pixel.
	frame->paint(&img, frame->get_xleft(), frame->get_yabove());
	int xoff = 0, yoff = 0;
	if (frame->is_rle())
		{
		xoff = -frame->get_xright();
		yoff = -frame->get_ybelow();
		}
	unsigned char pal[3*256];	// Set up palette.
	for (int i = 0; i < 256; i++)
		{
		pal[3*i] = (palette->colors[i]>>16)&0xff;
		pal[3*i + 1] = (palette->colors[i]>>8)&0xff;
		pal[3*i + 2] = palette->colors[i]&0xff;
		}
	const char *fname = filestr.c_str();
					// Write out to the .png.
	if (!Export_png8(fname, transp, w, h, w, xoff, yoff, img.get_bits(),
					&pal[0], 256))
		{
		string msg("Error creating file:  ");
		msg += fname;
		studio->prompt(msg.c_str(), "Continue");
		return;
		}
	string cmd(studio->get_image_editor());
	cmd += ' ';
	cmd += fname;
	cmd += '&';			// Background.  WIN32?????+++++++
	system(cmd.c_str());
	//+++++++FINISH  Got to store this info somewhere and monitor file.
	}

/*
 *	Someone wants the dragged shape.
 */

void Shape_chooser::drag_data_get
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	GtkSelectionData *seldata,	// Fill this in.
	guint info,
	guint time,
	gpointer data			// ->Shape_chooser.
	)
	{
	cout << "In DRAG_DATA_GET" << endl;
	Shape_chooser *chooser = (Shape_chooser *) data;
	if (chooser->selected < 0 || info != U7_TARGET_SHAPEID)
		return;			// Not sure about this.
	guchar buf[30];
	int file = chooser->ifile->get_u7drag_type();
	if (file == U7_SHAPE_UNK)
		U7_SHAPE_SHAPES;	// Just assume it's shapes.vga.
	Shape_entry& shinfo = chooser->info[chooser->selected];
	int len = Store_u7_shapeid(buf, file, shinfo.shapenum, 
							shinfo.framenum);
	cout << "Setting selection data (" << shinfo.shapenum <<
			'/' << shinfo.framenum << ')' << endl;
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
			gdk_atom_intern(U7_TARGET_SHAPEID_NAME, 0),
                                				8, buf, len);
#endif
	}

/*
 *	Another app. has claimed the selection.
 */

gint Shape_chooser::selection_clear
	(
	GtkWidget *widget,		// The view window.
	GdkEventSelection *event,
	gpointer data			// ->Shape_chooser.
	)
	{
//	Shape_chooser *chooser = (Shape_chooser *) data;
	cout << "SELECTION_CLEAR" << endl;
	return TRUE;
	}

/*
 *	Beginning of a drag.
 */

gint Shape_chooser::drag_begin
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	gpointer data			// ->Shape_chooser.
	)
	{
	cout << "In DRAG_BEGIN" << endl;
	Shape_chooser *chooser = (Shape_chooser *) data;
	if (chooser->selected < 0)
		return FALSE;		// ++++Display a halt bitmap.
					// Get ->shape.
	Shape_entry& shinfo = chooser->info[chooser->selected];
	Shape_frame *shape = chooser->ifile->get_shape(shinfo.shapenum, 
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
					// This will be the shape dragged.
	gtk_drag_set_icon_pixmap(context,
			gdk_window_get_colormap(widget->window), pixmap, mask,
					w - 2 - xright, h - 2 - ybelow);
	gdk_pixmap_unref(pixmap);
	gdk_bitmap_unref(mask);
	return TRUE;
	}

/*
 *	Scroll to a new shape/frame.
 */

void Shape_chooser::vscroll
	(
	int newindex			// Abs. index of row to show.
	)
	{
					// Already know where this is?
	if (newindex < row_indices.size())
		{
		index0 = row_indices[newindex];
		row0 = newindex;
		}
	else
		{			// Start with last known row.
		row0 = row_indices.size() - 1;
		int index = row_indices[row0];
		while (row0 < newindex && (index = next_row(index)) >= 0)
			{
			index0 = index;
			row0++;
			row_indices.push_back(index);
			}
		}
	int total = get_count();
	render();
	show();
	}

/*
 *	Adjust vertical scroll amounts.
 */

void Shape_chooser::adjust_vscrollbar
	(
	)
	{	
	GtkAdjustment *adj = gtk_range_get_adjustment(
						GTK_RANGE(shape_vscroll));
	int known_rows = row_indices.size() - 1;
	float num_per_row = known_rows > 0 ? 
		((float) row_indices[known_rows])/known_rows : 1;
					// This may change for the group.
	adj->upper = 1 + get_count()/num_per_row;
	adj->step_increment = 1;
	adj->page_increment = nrows;
	adj->page_size = nrows;
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
	}

/*
 *	Adjust horizontal scroll amounts.
 */

void Shape_chooser::adjust_hscrollbar
	(
	int newmax			// New max., or -1 to leave alone.
	)
	{	
	GtkAdjustment *adj = gtk_range_get_adjustment(
						GTK_RANGE(shape_hscroll));
	if (newmax > 0)
		adj->upper = newmax;
	adj->page_increment = draw->allocation.width;
	adj->page_size = draw->allocation.width;
	if (adj->page_size > adj->upper)
		adj->upper = adj->page_size;
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
	}

/*
 *	Handle a scrollbar event.
 */

void Shape_chooser::vscrolled		// For vertical scrollbar.
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Shape_chooser.
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
cout << "Scrolled to " << adj->value << '\n';
	gint newindex = (gint) adj->value;
	chooser->vscroll(newindex);
	}
void Shape_chooser::hscrolled		// For horizontal scrollbar.
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Shape_chooser.
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
	chooser->hoffset = (gint) adj->value;
	chooser->render_frames();
	chooser->show();
	}

/*
 *	Handle a change to the 'frame' spin button.
 */

void Shape_chooser::frame_changed
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Shape_chooser.
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
cout << "Frame changed to " << adj->value << '\n';
	gint newframe = (gint) adj->value;
	if (chooser->selected >= 0)
		{
		Shape_entry& shinfo = chooser->info[chooser->selected];
		int nframes = chooser->ifile->get_num_frames(shinfo.shapenum);
		if (newframe >= nframes)	// Just checking
			return;
		shinfo.framenum = newframe;
		if (chooser->frames_mode)	// Get sel. frame in view.
			chooser->scroll_to_frame();
		chooser->render();
		chooser->show();
		}
	}

/*
 *	'All frames' toggled.
 */

void Shape_chooser::all_frames_toggled
	(
	GtkToggleButton *btn,
        gpointer data
	)
	{
	Shape_chooser *chooser = (Shape_chooser *) data;
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	chooser->frames_mode = on;
	if (on)				// Frame => show horiz. scrollbar.
		gtk_widget_show(chooser->shape_hscroll);
	else
		gtk_widget_hide(chooser->shape_hscroll);
	chooser->render();
	chooser->show();
	}

/*
 *	Handle popup menu items.
 */

void Shape_chooser::on_shapes_popup_info_activate
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	((Shape_chooser *) udata)->edit_shape_info();
	}

void Shape_chooser::on_shapes_popup_edit_activate
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	((Shape_chooser *) udata)->edit_shape();
	}

/*
 *	Handle a shape dropped on our draw area.
 */

void Shape_chooser::shape_dropped_here
	(
	int file,			// U7_SHAPE_SHAPES.
	int shape,
	int frame
	)
	{
					// Got to be from same file type.
	if (ifile->get_u7drag_type() == file && group != 0)
		{			// Add to group.
		group->add(shape);
					// Update all windows for this group.
		ExultStudio::get_instance()->update_group_windows(group);
		}
	}

/*
 *	Get # shapes we can display.
 */

int Shape_chooser::get_count
	(
	)
	{
	return group ? group->size() : num_shapes;
	}

/*
 *	Search for an entry.
 */

void Shape_chooser::search
	(
	char *srch,			// What to search for.
	int dir				// 1 or -1.
	)
	{
	if (!names)
		return;			// In future, maybe find shape #?
	int total = get_count();
	if (!total)
		return;			// Empty.
					// Start with selection, or top.
	int start = index0 + (selected >= 0 ? selected : 0) + dir;
//	int start = info[selected >= 0 ? selected : 0].shapenum + dir;
	int stop = dir == -1 ? -1 : total;
	int i;
	for (i = start; i != stop; i += dir)
		{
		int shnum = group ? (*group)[i] : i;
		if (strstr(names[shnum], srch))
			break;		// Found it.
		}
	if (i == stop)
		return;			// Not found.
	goto_index(i);
	GtkAdjustment *adj = gtk_range_get_adjustment(
						GTK_RANGE(shape_vscroll));
	if (row0 >= adj->value)		// Beyond apparent end?
		adjust_vscrollbar();	// Needs updating.
	gtk_adjustment_set_value(adj, row0);
	int newsel = i - row_indices[row0];
	if (newsel >= 0 && newsel < info_cnt)
		select(newsel);
	show();
	}

/*
 *	Callbacks for 'search' buttons:
 */
C_EXPORT void
on_find_shape_down_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	Shape_chooser *chooser = (Shape_chooser *) user_data;
	chooser->search(gtk_entry_get_text(
			GTK_ENTRY(chooser->get_find_text())), 1);
}
C_EXPORT void
on_find_shape_up_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	Shape_chooser *chooser = (Shape_chooser *) user_data;
	chooser->search(gtk_entry_get_text(
			GTK_ENTRY(chooser->get_find_text())), -1);
}
C_EXPORT gboolean
on_find_shape_key			(GtkEntry	*entry,
					 GdkEventKey	*event,
					 gpointer	 user_data)
{
	if (event->keyval == GDK_Return)
		{
		Shape_chooser *chooser = (Shape_chooser *) user_data;
		chooser->search(gtk_entry_get_text(
			GTK_ENTRY(chooser->get_find_text())), 1);
		return TRUE;
		}
	return FALSE;			// Let parent handle it.
}


/*
 *	Set up popup menu for shape browser.
 */

GtkWidget *Shape_chooser::create_popup
	(
	)
	{
	if (popup)			// Clean out old.
		gtk_widget_destroy(popup);
	popup = gtk_menu_new();		// Create popup menu.
	GtkWidget *mitem = gtk_menu_item_new_with_label("Info...");
	gtk_widget_show(mitem);
	gtk_menu_append(GTK_MENU(popup), mitem);
	gtk_signal_connect (GTK_OBJECT (mitem), "activate",
		GTK_SIGNAL_FUNC(on_shapes_popup_info_activate), this);
	add_group_submenu(popup);
	if (selected >= 0 &&		// Add editing choices.
	    ExultStudio::get_instance()->get_image_editor())
		{
		mitem = gtk_menu_item_new_with_label("Edit...");
		gtk_widget_show(mitem);
		gtk_menu_append(GTK_MENU(popup), mitem);
		gtk_signal_connect(GTK_OBJECT(mitem), "activate",
			GTK_SIGNAL_FUNC(on_shapes_popup_edit_activate), this);
		}
	return popup;
	}

/*
 *	Create box with 'find' and 'history' controls.
 */

GtkWidget *Shape_chooser::create_search_controls
	(
	)
	{
	GtkWidget *frame = gtk_frame_new (NULL);
	gtk_widget_show(frame);

	GtkWidget *hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (frame), hbox1);

	GtkWidget *hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox2, FALSE, FALSE, 0);

	GtkWidget *label1 = gtk_label_new ("Find:");
	gtk_widget_show (label1);
	gtk_box_pack_start (GTK_BOX (hbox2), label1, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label1), 4, 0);

	find_text = gtk_entry_new ();
	gtk_widget_show (find_text);
	gtk_box_pack_start (GTK_BOX (hbox2), find_text, FALSE, FALSE, 0);
	gtk_widget_set_usize (find_text, 110, -2);

	GtkWidget *hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (hbox2), hbuttonbox1, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), 
							GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox1), 0);
//	gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX(hbuttonbox1), 0, -1);

	GtkWidget *find_shape_down = gtk_button_new_with_label ("Down");
	gtk_widget_show (find_shape_down);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), find_shape_down);
	GTK_WIDGET_SET_FLAGS (find_shape_down, GTK_CAN_DEFAULT);

	GtkWidget *find_shape_up = gtk_button_new_with_label ("Up");
	gtk_widget_show (find_shape_up);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), find_shape_up);
	GTK_WIDGET_SET_FLAGS (find_shape_up, GTK_CAN_DEFAULT);
#if 0
	GtkWidget *hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (hbox1), hbox3, FALSE, FALSE, 0);

	GtkWidget *label2 = gtk_label_new ("History:");
	gtk_widget_show (label2);
	gtk_box_pack_start (GTK_BOX (hbox3), label2, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label2), 4, 0);

	GtkWidget *combo1 = gtk_combo_new ();
	gtk_widget_show (combo1);
	gtk_box_pack_start (GTK_BOX (hbox3), combo1, TRUE, TRUE, 0);

	GtkWidget *history_combo = GTK_COMBO (combo1)->entry;
	gtk_widget_show (history_combo);
	gtk_widget_set_usize (history_combo, 120, -2);
#endif

	gtk_signal_connect (GTK_OBJECT (find_shape_down), "clicked",
                      GTK_SIGNAL_FUNC (on_find_shape_down_clicked),
                      this);
	gtk_signal_connect (GTK_OBJECT (find_shape_up), "clicked",
                      GTK_SIGNAL_FUNC (on_find_shape_up_clicked),
                      this);
	gtk_signal_connect (GTK_OBJECT (find_text), "key-press-event",
		      GTK_SIGNAL_FUNC (on_find_shape_key),
		      this);
	return frame;
	}

/*
 *	Create the list.
 */

Shape_chooser::Shape_chooser
	(
	Vga_file *i,			// Where they're kept.
	unsigned char *palbuf,		// Palette, 3*256 bytes (rgb triples).
	int w, int h,			// Dimensions.
	Shape_group *g,
	Shape_file_info *fi
	) : Object_browser(g, fi),
		Shape_draw(i, palbuf, gtk_drawing_area_new()), find_text(0),
		shapes_file(0), index0(0), framenum0(0),
		info(0), info_cnt(0), row0(0), nrows(0),
		sel_changed(0), frames_mode(false), hoffset(0)
	{
	row_indices.reserve(40);
	row_indices.push_back(0);	// First row is 0.
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
					// Want vert. scrollbar for the shapes.
	GtkObject *shape_adj = gtk_adjustment_new(0, 0, 
				get_count()/4, 1, 1, 1);
	shape_vscroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(shape_adj));
					// Update window when it stops.
	gtk_range_set_update_policy(GTK_RANGE(shape_vscroll),
					GTK_UPDATE_DELAYED);
	gtk_box_pack_start(GTK_BOX(hbox), shape_vscroll, FALSE, TRUE, 0);
					// Set scrollbar handler.
	gtk_signal_connect(GTK_OBJECT(shape_adj), "value_changed",
					GTK_SIGNAL_FUNC(vscrolled), this);
	gtk_widget_show(shape_vscroll);
					// Horizontal scrollbar.
	shape_adj = gtk_adjustment_new(0, 0, 640, 8, 16, 16);
	shape_hscroll = gtk_hscrollbar_new(GTK_ADJUSTMENT(shape_adj));
					// Update window when it stops.
	gtk_range_set_update_policy(GTK_RANGE(shape_hscroll),
					GTK_UPDATE_DELAYED);
	gtk_box_pack_start(GTK_BOX(vbox), shape_hscroll, FALSE, TRUE, 0);
					// Set scrollbar handler.
	gtk_signal_connect(GTK_OBJECT(shape_adj), "value_changed",
					GTK_SIGNAL_FUNC(hscrolled), this);
//++++	gtk_widget_hide(shape_hscroll);	// Only shown in 'frames' mode.
					// At the bottom, status bar & frame:
	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
	gtk_widget_show(hbox1);
					// At left, a status bar.
	sbar = gtk_statusbar_new();
	sbar_sel = gtk_statusbar_get_context_id(GTK_STATUSBAR(sbar),
							"selection");
	gtk_box_pack_start(GTK_BOX(hbox1), sbar, TRUE, TRUE, 0);
	gtk_widget_show(sbar);
	GtkWidget *label = gtk_label_new("Frame:");
	gtk_box_pack_start(GTK_BOX(hbox1), label, FALSE, FALSE, 4);
	gtk_widget_show(label);
					// A spin button for frame#.
	frame_adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 
				16, 1, 
				4, 1.0));
	fspin = gtk_spin_button_new(GTK_ADJUSTMENT(frame_adj), 
									1, 0);
	gtk_signal_connect(GTK_OBJECT(frame_adj), "value_changed",
					GTK_SIGNAL_FUNC(frame_changed), this);
	gtk_box_pack_start(GTK_BOX(hbox1), fspin, FALSE, FALSE, 0);
	gtk_widget_show(fspin);
					// A toggle for 'All Frames'.
	GtkWidget *allframes = gtk_toggle_button_new_with_label("Frames");
	gtk_box_pack_start(GTK_BOX(hbox1), allframes, FALSE, FALSE, 4);
	gtk_widget_show(allframes);
	gtk_signal_connect(GTK_OBJECT(allframes), "toggled",
				GTK_SIGNAL_FUNC(all_frames_toggled), this);
					// Add search controls to bottom.
	gtk_box_pack_start(GTK_BOX(vbox), create_search_controls(),
						FALSE, FALSE, 0);
	}

/*
 *	Delete.
 */

Shape_chooser::~Shape_chooser
	(
	)
	{
	gtk_widget_destroy(get_widget());
	delete [] info;
	}
	
/*
 *	Unselect.
 */

void Shape_chooser::unselect
	(
	bool need_render			// 1 to render and show.
	)
	{
	if (selected >= 0)
		{
		selected = -1;
					// Update spin button for frame #.
		gtk_adjustment_set_value(frame_adj, 0);
		gtk_widget_set_sensitive(fspin, false);
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
		g_snprintf(buf, sizeof(buf), "Shapes %d to %d",
			info[0].shapenum, info[info_cnt - 1].shapenum);
		gtk_statusbar_push(GTK_STATUSBAR(sbar), sbar_sel, buf);
		}
	}


