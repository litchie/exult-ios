/**
 **	A GTK widget showing a list of shapes from an image file.
 **
 **	Written: 7/25/99 - JSF
 **/

#ifndef INCL_SHAPELST
#define INCL_SHAPELST	1

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

#include "objbrowse.h"
#include "shapedraw.h"
#include "rect.h"
#include <vector>

class Vga_file;
class Image_buffer8;
class Shapes_vga_file;

/*
 *	Store information about an individual shape shown in the list.
 */
class Shape_entry
	{
	friend class Shape_chooser;
	int shapenum, framenum;		// The given shape/frame.
	Rectangle box;			// Box where drawn.
	Shape_entry() {  }
	void set(int shnum, int frnum, int rx, int ry, int rw, int rh)
		{
		shapenum = shnum; framenum = frnum;
		box = Rectangle(rx, ry, rw, rh);
		}
	};

/*
 *	This class manages a list of shapes from an image file.
 */
class Shape_chooser: public Object_browser, public Shape_draw
	{
	Shapes_vga_file *shapes_file;	// Non-null if 'shapes.vga'.
	GtkWidget *sbar;		// Status bar.
	guint sbar_sel;			// Status bar context for selection.
	GtkWidget *fspin;		// Spin button for frame #.
	GtkWidget *shape_vscroll;	// Vertical scrollbar.
	GtkWidget *shape_hscroll;	// Horizontal scrollbar.
	GtkWidget *find_text;		// For searching.
	GtkAdjustment *frame_adj;	// Adjustment for frame spin btn.
	int index0;			// Index of top-leftmost in
					//   displayed list.
	int framenum0;			// Default frame # to display.
	Shape_entry *info;		// An entry for each shape drawn.
	int info_cnt;			// # entries in info.
	std::vector<short> row_indices;	// Index at start of each row.
	int row0;			// Row # at top of window.
	int nrows;			// Last #rows rendered.
	bool frames_mode;		// Show all frames horizontally.
	int hoffset;			// Horizontal offset in pixels (when in
					//   frames_mode).
	void (*sel_changed)();		// Called when selection changes.
					// Blit onto screen.
	virtual void show(int x, int y, int w, int h);
	virtual void show()
		{ Shape_chooser::show(0, 0, 
			draw->allocation.width, draw->allocation.height);}
	void tell_server_shape();	// Tell Exult what shape is selected.
	void select(int new_sel);	// Show new selection.
	virtual void render();		// Draw list.
	void render_frames();		// Show all frames.
	void scroll_to_frame();		// Scroll so sel. frame is visible.
	int next_row(int start);	// Down/up 1 row.
	void goto_index(int index);	// Get desired index in view.
	virtual int get_selected_id()
		{ return selected < 0 ? -1 : info[selected].shapenum; }
	void vscroll(int newindex);	// Scroll.
	void adjust_vscrollbar();	// Set new scroll amounts.
	void adjust_hscrollbar(int newmax);
	GtkWidget *create_search_controls();
public:
	Shape_chooser(Vga_file *i, unsigned char *palbuf, int w, int h,
					Shape_group *g = 0);
	virtual ~Shape_chooser();
	void set_shapes_file(Shapes_vga_file *sh)
		{ shapes_file = sh; }	
	void set_framenum0(int f)
		{ framenum0 = f; }
	void shape_dropped_here(int file, int shapenum, int framenum);
	int get_count();		// Get # shapes we can display.
	void search(char *srch, int dir);
					// Turn off selection.
	void unselect(bool need_render = true);
	int is_selected()		// Is a shape selected?
		{ return selected >= 0; }
	void set_selected_callback(void (*fun)())
		{ sel_changed = fun; }
					// Get selected shape, or return 0.
	int get_selected(int& shapenum, int& framenum)
		{
		if (selected == -1)
			return (0);
		shapenum = info[selected].shapenum;
		framenum = info[selected].framenum;
		return (1);
		}
	GtkWidget *get_find_text()	// Get 'find' text widget.
		{ return find_text; }
					// Configure when created/resized.
	static gint configure(GtkWidget *widget, GdkEventConfigure *event,
							gpointer data);
					// Blit to screen.
	static gint expose(GtkWidget *widget, GdkEventExpose *event,
							gpointer data);
					// Handle mouse press.
	static gint mouse_press(GtkWidget *widget, GdkEventButton *event,
							gpointer data);
	void edit_shape();		// Edit selected shape.
					// Give dragged shape.
	static void drag_data_get(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *data, guint info, guint time, gpointer data);
					// Someone else selected.
	static gint selection_clear(GtkWidget *widget,
				GdkEventSelection *event, gpointer data);
	static gint drag_begin(GtkWidget *widget, GdkDragContext *context,
							gpointer data);
					// Handle scrollbar.
	static void vscrolled(GtkAdjustment *adj, gpointer data);
	static void hscrolled(GtkAdjustment *adj, gpointer data);
					// Handle spin-button for frames.
	static void frame_changed(GtkAdjustment *adj, gpointer data);
	static void all_frames_toggled(GtkToggleButton *btn,
						        gpointer user_data);
#ifdef WIN32
	static gint win32_drag_motion(GtkWidget *widget, GdkEventMotion *event,
		gpointer data);
#endif
					// Menu items:
	static void on_shapes_popup_info_activate(
					GtkMenuItem *item, gpointer udata);
	};

#endif
