/**
 **	A GTK widget showing the list of NPC's.
 **
 **	Written: 7/6/2005 - JSF
 **/

/*
Copyright (C) 2005  The Exult Team

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
#include <windows.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#ifdef XWIN
#include <gdk/gdkx.h>
#endif
#include <glib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "npclst.h"
#include "shapevga.h"
#include "ibuf8.h"
#include "u7drag.h"
#include "studio.h"
#include "utils.h"
#include "shapegroup.h"
#include "shapefile.h"
#include "pngio.h"
#include "fontgen.h"

using std::cout;
using std::cerr;
using std::endl;
using std::strlen;
using std::string;
using std::vector;
using EStudio::Prompt;
using EStudio::Alert;
using EStudio::Add_menu_item;

/*
 *	Blit onto screen.
 */

void Npc_chooser::show
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
			b.x, b.y - voffset, b.w, b.h);
		}
	}

/*
 *	Select an entry.  This should be called after rendering
 *	the shape.
 */

void Npc_chooser::select
	(
	int new_sel
	)
	{
	selected = new_sel;
	update_statusbar();
	}

const int border = 4;			// Border at bottom, sides.
/*
 *	Render as many shapes as fit in the shape chooser window.
 */

void Npc_chooser::render
	(
	)
	{
	vector<Estudio_npc>& npcs = get_npcs();
					// Get drawing area dimensions.
	gint winw = draw->allocation.width, winh = draw->allocation.height;
					// Clear window first.
	iwin->fill8(255);		// Set to background_color.
	int curr_y = -row0_voffset;
	int total_cnt = get_count();
	int index;			// This is shapenum if there's no
					//   filter (group).
	for (int rownum = row0; curr_y  < winh && rownum < rows.size(); 
								++rownum)
		{
		Npc_row& row = rows[rownum];
		int cols = get_num_cols(rownum);
		assert(cols >= 0);
		for (int index = row.index0; cols; --cols, ++index)
			{
			int npcnum = info[index].npcnum;
			int shapenum = npcs[npcnum].shapenum;
			Shape_frame *shape = ifile->get_shape(shapenum, 0);
			if(shape)
				{
				int sx = info[index].box.x;
				int sy = info[index].box.y - voffset;
				shape->paint(iwin, sx + shape->get_xleft(),
						sy + shape->get_yabove());
				if (npcs[npcnum].unused)
					shape->paint_rle_outline(iwin,
						sx + shape->get_xleft(),
						sy + shape->get_yabove(), red);
				last_npc = npcnum;
				}
			}
		curr_y += rows[rownum].height;
		}
	}

/*
 *	Find where everything goes.
 */

void Npc_chooser::setup_info
	(
	bool savepos			// Try to keep current position.
	)
	{
	int oldind = rows[row0].index0;
	info.resize(0);
	rows.resize(0);
	row0 = row0_voffset = 0;
	last_npc = 0;
	/* +++++NOTE:  index0 is always 0 for the NPC browse.  It should
		probably be removed from the base Obj_browser class	*/
	index0 = 0;
	voffset = 0;
	total_height = 0;
	setup_shapes_info();
	setup_vscrollbar();
	if (savepos)
		goto_index(oldind);
	}

/*
 *	Setup info.
 */

void Npc_chooser::setup_shapes_info
	(
	)
	{
	vector<Estudio_npc>& npcs = get_npcs();
	if (!npcs.size())		// No NPC's?  Try to get them.
		((Npcs_file_info *) file_info)->setup();
					// Get drawing area dimensions.
	gint winw = draw->allocation.width;
	int x = 0;
	int curr_y = 0;
	int row_h = 0;
	int total_cnt = get_count(), num_shapes = ifile->get_num_shapes();
	int index;			// This is shapenum if there's no
					//   filter (group).
	rows.resize(1);			// Start 1st row.
	rows[0].index0 = 0;
	rows[0].y = 0;
	for (int index = 0; index < total_cnt; index++)
		{
		int npcnum = group ? (*group)[index] : index;
		if (npcnum >= 356 && npcnum <= 359)
			continue;
		int shapenum = npcs[npcnum].shapenum;
		if (shapenum < 0 || shapenum >= num_shapes)
			continue;
		Shape_frame *shape = ifile->get_shape(shapenum, 0);
		if (!shape)
			continue;
		int sh = shape->get_height(),
		    sw = shape->get_width();
				// Check if we've exceeded max width
		if (x + sw > winw && x)		// But don't leave row empty.
			{	// Next line.
			rows.back().height = row_h + border;
			curr_y += row_h + border;
			row_h = 0;
			x = 0;
			rows.push_back(Npc_row());
			rows.back().index0 = info.size();
			rows.back().y = curr_y;
			}
		if (sh>row_h)
			row_h = sh;
		int sy = curr_y+border;	// Get top y-coord.
				// Store info. about where drawn.
		info.push_back(Npc_entry());
		info.back().set(npcnum, x, sy, sw, sh);
		x += sw + border;
		}
	rows.back().height = row_h + border;
	total_height = curr_y + rows.back().height + border;
	}

/*
 *	Scroll so a desired index is in view.
 */

void Npc_chooser::goto_index
	(
	int index			// Desired index in 'info'.
	)
	{
	if (index < 0 || index >= info.size())
		return;			// Illegal index or empty chooser.
	Npc_entry& inf = info[index];	// Already in view?
	int midx = inf.box.x + inf.box.w/2;
	int midy = inf.box.y + inf.box.h/2;
	Rectangle winrect(0, voffset, config_width, config_height);
	if (winrect.has_point(midx, midy))
		return;
	int start = 0, count = rows.size();
	while (count > 1)		// Binary search.
		{
		int mid = start + count/2;
		if (index < rows[mid].index0)
			count = mid - start;
		else
			{
			count = (start + count) - mid;
			start = mid;
			}
		}
	if (start < rows.size())
		{
					// Get to right spot again!
		GtkAdjustment *adj = gtk_range_get_adjustment(
						GTK_RANGE(vscroll));
		gtk_adjustment_set_value(adj, rows[start].y);
		}
	}

/*
 *	Find index for a given NPC #.
 */

int Npc_chooser::find_npc
	(
	int npcnum
	)
	{
	if (group)			// They're not ordered.
		{
		int cnt = info.size();
		for (int i = 0; i < cnt; ++i)
			if (info[i].npcnum == npcnum)
				return i;
		return -1;
		}
	int start = 0, count = info.size();
	while (count > 1)		// Binary search.
		{
		int mid = start + count/2;
		if (npcnum < info[mid].npcnum)
			count = mid - start;
		else
			{
			count = (start + count) - mid;
			start = mid;
			}
		}
	if (start < info.size())
		return start;
	else
		return -1;
	}

/*
 *	Configure the viewing window.
 */

static gint Configure_chooser
	(
	GtkWidget *widget,		// The drawing area.
	GdkEventConfigure *event,
	gpointer data			// ->Npc_chooser
	)
	{
	Npc_chooser *chooser = (Npc_chooser *) data;
	return chooser->configure(event);
	}
gint Npc_chooser::configure
	(
	GdkEventConfigure *event
	)
	{
	Shape_draw::configure();
					// Did the size change?
	if (event->width != config_width || event->height != config_height)
		{
		config_width = event->width;
		config_height = event->height;
		setup_info(true);
		render();
		update_statusbar();
		}
	else
		render();		// Same size?  Just render it.
	if (group)			// Filtering?
		enable_drop();		// Can drop NPCs here.
	return (TRUE);
	}

/*
 *	Handle an expose event.
 */

gint Npc_chooser::expose
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Npc_chooser.
	)
	{
	Npc_chooser *chooser = (Npc_chooser *) data;
	chooser->show(event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Handle a mouse drag event.
 */

#ifdef WIN32

static bool win32_button = false;

gint Npc_chooser::win32_drag_motion
	(
	GtkWidget *widget,		// The view window.
	GdkEventMotion *event,
	gpointer data			// ->Npc_chooser.
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
		  U7_TARGET_NPCID, 0, data);

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
#else
gint Npc_chooser::drag_motion
	(
	GtkWidget *widget,		// The view window.
	GdkEventMotion *event,
	gpointer data			// ->Npc_chooser.
	)
	{
	Npc_chooser *chooser = (Npc_chooser *) data;
	if (!chooser->dragging && chooser->selected >= 0)
		chooser->start_drag(U7_TARGET_NPCID_NAME, 
			U7_TARGET_NPCID, (GdkEvent *) event);
	return true;
	}
#endif

/*
 *	Handle a mouse button-press event.
 */
gint Npc_chooser::mouse_press
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event
	)
	{
	gtk_widget_grab_focus(widget);

	if (event->button == 4) {
        	if (row0 > 0)
            		scroll_row_vertical(row0-1);
        	return(TRUE);
    	} else if (event->button == 5) {
        	scroll_row_vertical(row0+1);
        	return(TRUE);
	}
	int old_selected = selected, new_selected = -1;
	int i;				// Search through entries.
	int infosz = info.size();
	int absx = (int) event->x, absy = (int) event->y + voffset;
	for (i = rows[row0].index0; i < infosz; i++)
		{
		if (info[i].box.has_point(absx, absy))
			{		// Found the box?
					// Indicate we can drag.
#ifdef WIN32
// Here, we have to override GTK+'s Drag and Drop, which is non-OLE and
// usually stucks outside the program window. I think it's because
// the dragged shape only receives mouse motion events when the new mouse pointer
// position is *still* inside the shape. So if you move the mouse too fast,
// we are stuck.
			win32_button = true;
#endif
			new_selected = i;
			break;
			}
		else if (info[i].box.y - voffset >= config_height)
			break;		// Past bottom of screen.
		}
	if (new_selected >= 0)
		{
		select(new_selected);
		render();
		show();
		if (sel_changed)	// Tell client.
			(*sel_changed)();
		}
	if (new_selected < 0 && event->button == 1)
		unselect(true);		// No selection.
	else if (selected == old_selected && old_selected >= 0)
		{			// Same square.  Check for dbl-click.
		if (((GdkEvent *) event)->type == GDK_2BUTTON_PRESS)
			edit_npc();
		}
	if (event->button == 3)
		gtk_menu_popup(GTK_MENU(create_popup()), 
				0, 0, 0, 0, event->button, event->time);
	return (TRUE);
	}

/*
 *	Handle mouse button press/release events.
 */
static gint Mouse_press
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Npc_chooser.
	)
	{
	Npc_chooser *chooser = (Npc_chooser *) data;
	return chooser->mouse_press(widget, event);
	}
static gint Mouse_release
	(
	GtkWidget *widget,		// The view window.
	GdkEventButton *event,
	gpointer data			// ->Npc_chooser.
	)
	{
	Npc_chooser *chooser = (Npc_chooser *) data;
	chooser->mouse_up();
	return true;
	}

/*
 *	Keystroke in draw-area.
 */
C_EXPORT gboolean
on_npc_draw_key_press			(GtkEntry	*entry,
					 GdkEventKey	*event,
					 gpointer	 user_data)
{
	Npc_chooser *chooser = (Npc_chooser *) user_data;
#if 0
	switch (event->keyval)
		{
	case GDK_Delete:
		chooser->del_frame();
		return TRUE;
	case GDK_Insert:
		chooser->new_frame();
		return TRUE;
		}
#endif
	return FALSE;			// Let parent handle it.
}

/*
 *	Bring up the NPC editor.
 */

void Npc_chooser::edit_npc
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	int npcnum = info[selected].npcnum;
	Estudio_npc& npc = get_npcs()[npcnum];
	unsigned char buf[Exult_server::maxlength], *ptr;
	ptr = &buf[0];
	Write2(ptr, npcnum);
	if (!studio->send_to_server(Exult_server::edit_npc, buf, ptr - buf))
		cerr << "Error sending data to server." << endl;
	const gchar* const * locales = g_get_language_names();
	if (!locales)
		{
		cerr << "No locales!" << endl;
		return;
		}
	while (*locales)
		{
		cerr << "\"" << *locales << "\"" << endl;
		locales++;
		}
	}

/*
 *	Update NPC information.
 */

void Npc_chooser::update_npc
	(
	int num
	)
	{
	((Npcs_file_info *) file_info)->read_npc(num);
	update_statusbar();
	}

/*
 *	Someone wants the dragged shape.
 */

void Npc_chooser::drag_data_get
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	GtkSelectionData *seldata,	// Fill this in.
	guint info,
	guint time,
	gpointer data			// ->Npc_chooser.
	)
	{
	cout << "In DRAG_DATA_GET" << endl;
	Npc_chooser *chooser = (Npc_chooser *) data;
	if (chooser->selected < 0 || info != U7_TARGET_NPCID)
		return;			// Not sure about this.
	guchar buf[30];
	int npcnum = chooser->info[chooser->selected].npcnum;
	int len = Store_u7_npcid(buf, npcnum);
	cout << "Setting selection data (" << npcnum << ')' << endl;
#ifdef WIN32
	windragdata *wdata = (windragdata *)seldata;
	wdata->assign(info, len, buf);
#else
					// Make us owner of xdndselection.
	gtk_selection_owner_set(widget, gdk_atom_intern("XdndSelection", 0),
								time);
					// Set data.
	gtk_selection_data_set(seldata,
			gdk_atom_intern(U7_TARGET_NPCID_NAME, 0),
                                				8, buf, len);
#endif
	}

/*
 *	Another app. has claimed the selection.
 */

gint Npc_chooser::selection_clear
	(
	GtkWidget *widget,		// The view window.
	GdkEventSelection *event,
	gpointer data			// ->Npc_chooser.
	)
	{
//	Npc_chooser *chooser = (Npc_chooser *) data;
	cout << "SELECTION_CLEAR" << endl;
	return TRUE;
	}

/*
 *	Beginning of a drag.
 */

gint Npc_chooser::drag_begin
	(
	GtkWidget *widget,		// The view window.
	GdkDragContext *context,
	gpointer data			// ->Npc_chooser.
	)
	{
	cout << "In DRAG_BEGIN" << endl;
	Npc_chooser *chooser = (Npc_chooser *) data;
	if (chooser->selected < 0)
		return FALSE;		// ++++Display a halt bitmap.
					// Get ->npc.
	int npcnum = chooser->info[chooser->selected].npcnum;
	Estudio_npc& npc = chooser->get_npcs()[npcnum];
	Shape_frame *shape = chooser->ifile->get_shape(npc.shapenum, 0);
	if (!shape)
		return FALSE;
	chooser->set_drag_icon(context, shape);	// Set icon for dragging.
	return TRUE;
	}

/*
 *	Chunk was dropped here.
 */

void Npc_chooser::drag_data_received
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
	Npc_chooser *chooser = (Npc_chooser *) udata;
	cout << "Npc drag_data_received" << endl;
	if (seldata->type == gdk_atom_intern(U7_TARGET_NPCID_NAME, 0) &&
	    seldata->format == 8 && seldata->length > 0)
		{
		int npcnum;
		Get_u7_npcid(seldata->data, npcnum);
		chooser->group->add(npcnum);
		chooser->setup_info(true);
		chooser->render();
		}
	}

/*
 *	Set to accept drops from drag-n-drop of a chunk.
 */

void Npc_chooser::enable_drop
	(
	)
	{
	if (drop_enabled)		// More than once causes warning.
		return;
	drop_enabled = true;
	gtk_widget_realize(draw);//???????
#ifndef WIN32
	GtkTargetEntry tents[1];
	tents[0].target = const_cast<char *>(U7_TARGET_NPCID_NAME);
	tents[0].flags = 0;
	tents[0].info = U7_TARGET_NPCID;
	gtk_drag_dest_set(draw, GTK_DEST_DEFAULT_ALL, tents, 1,
			(GdkDragAction) (GDK_ACTION_COPY | GDK_ACTION_MOVE));

	gtk_signal_connect(GTK_OBJECT(draw), "drag_data_received",
				GTK_SIGNAL_FUNC(drag_data_received), this);
#endif
	}

/*
 *	Scroll to a new shape/frame.
 */

void Npc_chooser::scroll_row_vertical
	(
	int newrow			// Abs. index of row to show.
	)
	{
	if (newrow >= rows.size())
		return;
	row0 = newrow;
	row0_voffset = 0;
	render();
	show();
	}

/*
 *	Scroll to new pixel offset.
 */

void Npc_chooser::scroll_vertical
	(
	int newoffset
	)
	{
	int delta = newoffset - voffset;
	while (delta > 0 && row0 < rows.size() - 1)
		{			// Going down.
		int rowh = rows[row0].height - row0_voffset;
		if (delta < rowh)
			{		// Part of current row.
			voffset += delta;
			row0_voffset += delta;
			delta = 0;
			}
		else
			{		// Go down to next row.
			voffset += rowh;
			delta -= rowh;
			++row0;
			row0_voffset = 0;
			}
		}
	while (delta < 0)
		{
		if (-delta <= row0_voffset)
			{
			voffset += delta;
			row0_voffset += delta;
			delta = 0;
			}
		else if (row0_voffset)
			{
			voffset -= row0_voffset;
			delta += row0_voffset;
			row0_voffset = 0;
			}
		else
			{
			if (row0 <= 0)
				break;
			--row0;
			row0_voffset = 0;
			voffset -= rows[row0].height;
			delta += rows[row0].height;
			if (delta > 0)
				{
				row0_voffset = delta;
				voffset += delta;
				delta = 0;
				}
			}
		}
	render();
	show();
	update_statusbar();
	}		

/*
 *	Adjust vertical scroll amounts after laying out shapes.
 */

void Npc_chooser::setup_vscrollbar
	(
	)
	{	
	GtkAdjustment *adj = gtk_range_get_adjustment(
						GTK_RANGE(vscroll));
	adj->value = 0;
	adj->lower = 0;
	adj->upper = total_height;
	adj->step_increment = 16;	// +++++FOR NOW.
	adj->page_increment = config_height;
	adj->page_size = config_height;
	gtk_signal_emit_by_name(GTK_OBJECT(adj), "changed");
	}

/*
 *	Handle a scrollbar event.
 */

void Npc_chooser::vscrolled		// For vertical scrollbar.
	(
	GtkAdjustment *adj,		// The adjustment.
	gpointer data			// ->Npc_chooser.
	)
	{
	Npc_chooser *chooser = (Npc_chooser *) data;
cout << "Scrolled to " << adj->value << '\n';
	gint newindex = (gint) adj->value;
	chooser->scroll_vertical(newindex);
	}

/*
 *	Get # shapes we can display.
 */

int Npc_chooser::get_count
	(
	)
	{
	return group ? group->size() : get_npcs().size();
	}

/*
 *	Get NPC list.
 */

vector<Estudio_npc>& Npc_chooser::get_npcs
	(
	)
	{
	return ((Npcs_file_info *) file_info)->get_npcs();
	}

/*
 *	Search for an entry.
 */

void Npc_chooser::search
	(
	const char *srch,		// What to search for.
	int dir				// 1 or -1.
	)
	{
	int total = get_count();
	if (!total)
		return;			// Empty.
	vector<Estudio_npc>& npcs = get_npcs();
	ExultStudio *studio = ExultStudio::get_instance();
					// Start with selection, or top.
	int start = selected >= 0 ? selected : rows[row0].index0;
	int i;
	start += dir;
	int stop = dir == -1 ? -1 : (int) info.size();
	for (i = start; i != stop; i += dir)
		{
		int npcnum = info[i].npcnum;
		const char *nm = npcnum < npcs.size() ? 
					npcs[npcnum].name.c_str() : 0;
		if (nm && search_name(nm, srch))
			break;		// Found it.
		}
	if (i == stop)
		return;			// Not found.
	goto_index(i);
	select(i);
	show();
	}

/*
 *	Locate NPC on game map.
 */

void Npc_chooser::locate
	(
	bool upwards
	)
	{
	if (selected < 0)
		return;			// Shouldn't happen.
	int npcnum = info[selected].npcnum;
	if (get_npcs()[npcnum].unused)
		{
		EStudio::Alert("Npc %d is unused.", npcnum);
		return;
		}
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	Write2(ptr, npcnum);
	ExultStudio *studio = ExultStudio::get_instance();
	studio->send_to_server(
			Exult_server::locate_npc, data, ptr - data);
	}

/*
 *	Handle popup menu items.
 */

void on_npc_popup_edit_activate
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	((Npc_chooser *) udata)->edit_npc();
	}

/*
 *	Set up popup menu for shape browser.
 */

GtkWidget *Npc_chooser::create_popup
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	// Create popup with groups, but not files.
	Object_browser::create_popup(false);
	if (selected >= 0)		// Add editing choices.
		{
		Add_menu_item(popup, "Edit...",
				GTK_SIGNAL_FUNC(on_npc_popup_edit_activate),
								 this);
		}
	return popup;
	}

/*
 *	Create the list.
 */

Npc_chooser::Npc_chooser
	(
	Vga_file *i,			// Where they're kept.
	unsigned char *palbuf,		// Palette, 3*256 bytes (rgb triples).
	int w, int h,			// Dimensions.
	Shape_group *g,
	Shape_file_info *fi
	) : Object_browser(g, fi),
		Shape_draw(i, palbuf, gtk_drawing_area_new()),
		info(0), row0(0), rows(0),
		row0_voffset(0), total_height(0), status_id(-1),
		sel_changed(0), voffset(0), drop_enabled(false)
	{
	rows.reserve(40);
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
		| GDK_BUTTON_RELEASE_MASK
		| GDK_POINTER_MOTION_HINT_MASK |
		GDK_BUTTON1_MOTION_MASK | GDK_KEY_PRESS_MASK);
					// Set "configure" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "configure_event",
				GTK_SIGNAL_FUNC(Configure_chooser), this);
					// Set "expose" handler.
	gtk_signal_connect(GTK_OBJECT(draw), "expose_event",
				GTK_SIGNAL_FUNC(expose), this);
					// Keystroke.
	gtk_signal_connect(GTK_OBJECT(draw), "key-press-event",
		      GTK_SIGNAL_FUNC (on_npc_draw_key_press),
		      this);
	GTK_WIDGET_SET_FLAGS(draw, GTK_CAN_FOCUS);
					// Set mouse click handler.
	gtk_signal_connect(GTK_OBJECT(draw), "button_press_event",
				GTK_SIGNAL_FUNC(Mouse_press), this);
	gtk_signal_connect(GTK_OBJECT(draw), "button_release_event",
				GTK_SIGNAL_FUNC(Mouse_release), this);
					// Mouse motion.
	gtk_signal_connect(GTK_OBJECT(draw), "drag_begin",
				GTK_SIGNAL_FUNC(drag_begin), this);
#ifdef WIN32
// required to override GTK+ Drag and Drop
	gtk_signal_connect(GTK_OBJECT(draw), "motion_notify_event",
				GTK_SIGNAL_FUNC(win32_drag_motion), this);
#else
	gtk_signal_connect(GTK_OBJECT(draw), "motion_notify_event",
				GTK_SIGNAL_FUNC(drag_motion), this);
#endif
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
	vscroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(shape_adj));
	gtk_box_pack_start(GTK_BOX(hbox), vscroll, FALSE, TRUE, 0);
					// Set scrollbar handler.
	gtk_signal_connect(GTK_OBJECT(shape_adj), "value_changed",
					GTK_SIGNAL_FUNC(vscrolled), this);
	gtk_widget_show(vscroll);
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
					// Add search controls to bottom.
	gtk_box_pack_start(GTK_BOX(vbox), 
		create_controls(find_controls | locate_controls),
						FALSE, FALSE, 0);
	red = ExultStudio::get_instance()->find_palette_color(63, 5, 5);
	}

/*
 *	Delete.
 */

Npc_chooser::~Npc_chooser
	(
	)
	{
	gtk_widget_destroy(get_widget());
	}
	
/*
 *	Unselect.
 */

void Npc_chooser::unselect
	(
	bool need_render			// 1 to render and show.
	)
	{
	if (selected >= 0)
		{
		selected = -1;
		if (need_render)
			{
			render();
			show();
			}
		if (sel_changed)	// Tell client.
			(*sel_changed)();
		}
	update_statusbar();
	}

/*
 *	Show selection or range in window.
 */

void Npc_chooser::update_statusbar
	(
	)
	{
	char buf[150];
	if (status_id >= 0)		// Remove prev. selection msg.
		gtk_statusbar_remove(GTK_STATUSBAR(sbar), sbar_sel, status_id);
	if (selected >= 0)
		{
		int npcnum = info[selected].npcnum;
		Estudio_npc& npc = get_npcs()[npcnum];
		g_snprintf(buf, sizeof(buf), "Npc %d:  '%s'%s",
					npcnum, npc.name.c_str(),
				npc.unused ? " (unused)" : "");
		status_id = gtk_statusbar_push(GTK_STATUSBAR(sbar), 
							sbar_sel, buf);
		}
	else if (!info.empty() && !group)
		{
		g_snprintf(buf, sizeof(buf), "NPCs %d to %d",
			info[rows[row0].index0].npcnum, last_npc);
		status_id = gtk_statusbar_push(GTK_STATUSBAR(sbar), 
								sbar_sel, buf);
		}
	else
		status_id = -1;
	}


