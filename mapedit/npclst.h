/**
 **	A GTK widget showing the list of NPC's.
 **
 **	Written: 7/6/2005 - JSF
 **/

#ifndef INCL_NPCLST
#define INCL_NPCLST	1

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

#include "objbrowse.h"
#include "shapedraw.h"
#include "rect.h"
#include <vector>
#include <ctime>
#include <string>

class Vga_file;
class Image_buffer8;
class Shapes_vga_file;
class Editing_file;
class Estudio_npc;

/*
 *	Store information about an NPC shown in the list.
 */
class Npc_entry
	{
	friend class Npc_chooser;
	short npcnum;
	Rectangle box;			// Box where drawn.
public:
	Npc_entry() {  }
	void set(int num, int rx, int ry, int rw, int rh)
		{
		npcnum = num;
		box = Rectangle(rx, ry, rw, rh);
		}
	};

/*
 *	One row.
 */
class Npc_row
	{
	friend class Npc_chooser;
	short height;			// In pixels.
	long y;				// Absolute y-coord. in pixels.
	int index0;			// Index of 1st Npc_entry in row.
public:
	Npc_row() : height(0)
		{  }
	};

/*
 *	This class manages a list of NPC's.
 */
class Npc_chooser: public Object_browser, public Shape_draw
	{
	GtkWidget *sbar;		// Status bar.
	guint sbar_sel;			// Status bar context for selection.
	std::vector<Npc_entry> info;	// Pos. of each shape/frame.
	std::vector<Npc_row> rows;
	int row0;			// Row # at top of window.
	int row0_voffset;		// Vert. pos. (in pixels) of top row.
	long total_height;		// In pixels, for all rows.
	int last_npc;			// Last shape visible in window.
	int voffset;			// Vertical offset in pixels.
	int status_id;			// Statusbar msg. ID.
	int red;			// Index of color red in palbuf.
	void (*sel_changed)();		// Called when selection changes.
					// Blit onto screen.
	virtual void show(int x, int y, int w, int h);
	virtual void show()
		{ Npc_chooser::show(0, 0, 
			draw->allocation.width, draw->allocation.height);}
	void select(int new_sel);	// Show new selection.
	virtual void render();		// Draw list.
	virtual void set_background_color(guint32 c)
		{ Shape_draw::set_background_color(c); }
	virtual void setup_info(bool savepos = true);
	void setup_shapes_info();
	int find_npc(int npcnum);	// Find index for given NPC.
	void goto_index(int index);	// Get desired index in view.
	virtual int get_selected_id()
		{ return selected; }
	void scroll_row_vertical(int newrow);
	void scroll_vertical(int newindex);	// Scroll.
	void setup_vscrollbar();	// Set new scroll amounts.
	virtual GtkWidget *create_popup();	// Popup menu.
public:
	Npc_chooser(Vga_file *i, unsigned char *palbuf, int w, int h,
				Shape_group *g = 0, Shape_file_info *fi = 0);
	virtual ~Npc_chooser();
	int get_count();		// Get # shapes we can display.
	std::vector<Estudio_npc>& get_npcs();
	virtual void search(const char *srch, int dir);
	virtual void locate(bool upwards);	// Locate NPC on game map.
					// Turn off selection.
	void unselect(bool need_render = true);
	void update_statusbar();
	int is_selected()		// Is a shape selected?
		{ return selected >= 0; }
	void set_selected_callback(void (*fun)())
		{ sel_changed = fun; }
	int get_num_cols(int rownum)
		{ 
		return  ((rownum < rows.size() - 1) ? rows[rownum + 1].index0
				: info.size()) - rows[rownum].index0;
		}
					// Configure when created/resized.
	gint configure(GdkEventConfigure *event);
					// Blit to screen.
	static gint expose(GtkWidget *widget, GdkEventExpose *event,
							gpointer data);
					// Handle mouse press.
	gint mouse_press(GtkWidget *widget, GdkEventButton *event);
#if 0	/* Maybe allow dragging into game?? */
					// Give dragged shape.
	static void drag_data_get(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *data, guint info, guint time, gpointer data);
#endif
					// Someone else selected.
	static gint selection_clear(GtkWidget *widget,
				GdkEventSelection *event, gpointer data);
#if 0
	static gint drag_begin(GtkWidget *widget, GdkDragContext *context,
							gpointer data);
#endif
					// Handle scrollbar.
	static void vscrolled(GtkAdjustment *adj, gpointer data);
#if 0
#ifdef WIN32
	static gint win32_drag_motion(GtkWidget *widget, GdkEventMotion *event,
		gpointer data);
#else
	static gint drag_motion(GtkWidget *widget, GdkEventMotion *event,
		gpointer data);
#endif
#endif
	};

#endif
