/**
 **	A GTK widget showing the chunks from 'u7chunks'.
 **
 **	Written: 7/8/01 - JSF
 **/

#ifndef INCL_CHUNKLST
#define INCL_CHUNKLST	1

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

#include <vector>
#include "objbrowse.h"
#include "rect.h"
#include "shapedraw.h"

class Image_buffer8;
class Shape_group;
#include <iosfwd>

/*
 *	Store information about an individual chunk shown in the list.
 */
class Chunk_info
	{
	friend class Chunk_chooser;
	int num;
	Rectangle box;			// Box where drawn.
	Chunk_info() {  }
	void set(int n, int rx, int ry, int rw, int rh)
		{
		num = n;
		box = Rectangle(rx, ry, rw, rh);
		}
	};

/*
 *	This class manages the list of chunks.
 */
class Chunk_chooser: public Object_browser, public Shape_draw
	{
	std::istream& chunkfile;	// Where chunks are read from (each is
					//   256 shape ID's = 512 bytes).
	GtkWidget *sbar;		// Status bar.
	guint sbar_sel;			// Status bar context for selection.
	GtkWidget *chunk_scroll;	// Vertical scrollbar.
	int num_chunks;			// Total # of chunks.
					// List of chunks we've read in.
	std::vector<unsigned char *> chunklist;
	int index0;			// Index (chunk) # of leftmost in
					//   displayed list.
	Chunk_info *info;		// An entry for each chunk drawn.
	int info_cnt;			// # entries in info.
	int selected;			// Index of user-selected entry.
	int locate_cx, locate_cy;	// Last chunk found by 'locate'.
					// Various controls.
	GtkWidget *loc_chunk_down, *loc_chunk_up, *insert_chunk_dup,
		  *move_chunk_down, *move_chunk_up;
	void (*sel_changed)();		// Called when selection changes.
					// Blit onto screen.
	void show(int x, int y, int w, int h);
	void show()
		{ show(0, 0, draw->allocation.width, draw->allocation.height);}
	void select(int new_sel);	// Show new selection.
	virtual void render();		// Draw list.
	unsigned char *get_chunk(int chunknum);
	void set_chunk(unsigned char *data, int datalen);
	void render_chunk(int chunknum, int xoff, int yoff);
	void scroll(int newindex);	// Scroll.
	void scroll(bool upwards);
	GtkWidget *create_controls();
	void enable_controls();		// Enable/disable controls after sel.
					//   has changed.
public:
	Chunk_chooser(Vga_file *i, std::istream& cfile, unsigned char *palbuf, 
					int w, int h, Shape_group *g = 0);
	virtual ~Chunk_chooser();
	virtual bool server_response(int id, unsigned char *data, int datalen);
	virtual void end_terrain_editing();
					// Turn off selection.
	void unselect(bool need_render = true);
	int is_selected()		// Is a chunk selected?
		{ return selected >= 0; }
	void set_selected_callback(void (*fun)())
		{ sel_changed = fun; }
	int get_selected()		// Get selected chunk, or return -1.
		{ return selected >= 0 ? info[selected].num : -1; }
					// Configure when created/resized.
	static gint configure(GtkWidget *widget, GdkEventConfigure *event,
							gpointer data);
					// Blit to screen.
	static gint expose(GtkWidget *widget, GdkEventExpose *event,
							gpointer data);
					// Handle mouse press.
	static gint mouse_press(GtkWidget *widget, GdkEventButton *event,
							gpointer data);
					// Give dragged chunk.
	static void drag_data_get(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *data, guint info, guint time, gpointer data);
					// Someone else selected.
	static gint selection_clear(GtkWidget *widget,
				GdkEventSelection *event, gpointer data);
	static gint drag_begin(GtkWidget *widget, GdkDragContext *context,
							gpointer data);
					// Handle scrollbar.
	static void scrolled(GtkAdjustment *adj, gpointer data);
	void locate(bool upwards);	// Locate terrain on game map.
	void locate_response(unsigned char *data, int datalen);
	void insert(bool dup);		// Insert new chunk.
	void insert_response(unsigned char *data, int datalen);
	void move(bool upwards);	// Move current selected chunk.
	void swap_response(unsigned char *data, int datalen);
#ifdef WIN32
	static gint win32_drag_motion(GtkWidget *widget, GdkEventMotion *event,
		gpointer data);
#endif
	};

#endif
