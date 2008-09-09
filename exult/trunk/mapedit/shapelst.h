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
#include <ctime>

class Vga_file;
class Image_buffer8;
class Shapes_vga_file;
class Editing_file;

/*
 *	Store information about an individual shape shown in the list.
 */
class Shape_entry
	{
	friend class Shape_chooser;
	short shapenum, framenum;	// The given shape/frame.
	Rectangle box;			// Box where drawn.
public:
	Shape_entry() {  }
	void set(int shnum, int frnum, int rx, int ry, int rw, int rh)
		{
		shapenum = (short) shnum; framenum = (short) frnum;
		box = Rectangle(rx, ry, rw, rh);
		}
	};

/*
 *	One row.
 */
class Shape_row
	{
	friend class Shape_chooser;
	short height;			// In pixels.
	long y;				// Absolute y-coord. in pixels.
	int index0;			// Index of 1st Shape_entry in row.
public:
	Shape_row() : height(0)
		{  }
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
	GtkAdjustment *frame_adj;	// Adjustment for frame spin btn.
	int framenum0;			// Default frame # to display.
	std::vector<Shape_entry> info;	// Pos. of each shape/frame.
	std::vector<Shape_row> rows;
	int row0;			// Row # at top of window.
	int row0_voffset;		// Vert. pos. (in pixels) of top row.
	long total_height;		// In pixels, for all rows.
	int last_shape;			// Last shape visible in window.
	bool frames_mode;		// Show all frames horizontally.
	int hoffset;			// Horizontal offset in pixels (when in
					//   frames_mode).
	int voffset;			// Vertical offset in pixels.
	int status_id;			// Statusbar msg. ID.
	void (*sel_changed)();		// Called when selection changes.
					// List of files being edited by an
					//   external program (Gimp, etc.)
	static std::vector<Editing_file*> editing_files;
	static int check_editing_timer;	// For monitoring files being edited.
					// Blit onto screen.
	virtual void show(int x, int y, int w, int h);
	virtual void show()
		{ Shape_chooser::show(0, 0, 
			draw->allocation.width, draw->allocation.height);}
	void tell_server_shape();	// Tell Exult what shape is selected.
	void select(int new_sel);	// Show new selection.
	virtual void render();		// Draw list.
	virtual void set_background_color(guint32 c)
		{ Shape_draw::set_background_color(c); }
	virtual void setup_info(bool savepos = true);
	void setup_shapes_info();
	void setup_frames_info();
	void scroll_to_frame();		// Scroll so sel. frame is visible.
	int find_shape(int shnum);	// Find index for given shape.
	void goto_index(int index);	// Get desired index in view.
	virtual int get_selected_id()
		{ return selected < 0 ? -1 : info[selected].shapenum; }
	void scroll_row_vertical(int newrow);
	void scroll_vertical(int newindex);	// Scroll.
	void setup_vscrollbar();	// Set new scroll amounts.
	void setup_hscrollbar(int newmax);
	virtual GtkWidget *create_popup();	// Popup menu.
public:
	Shape_chooser(Vga_file *i, unsigned char *palbuf, int w, int h,
				Shape_group *g = 0, Shape_file_info *fi = 0);
	virtual ~Shape_chooser();
	void set_shapes_file(Shapes_vga_file *sh)
		{ shapes_file = sh; }	
	void set_framenum0(int f)
		{ framenum0 = f; }
	void shape_dropped_here(int file, int shapenum, int framenum);
	int get_count();		// Get # shapes we can display.
	virtual void search(const char *srch, int dir);
	virtual void locate(bool upwards);	// Locate shape on game map.
					// Turn off selection.
	void unselect(bool need_render = true);
	void update_statusbar();
	int is_selected()		// Is a shape selected?
		{ return selected >= 0; }
	void set_selected_callback(void (*fun)())
		{ sel_changed = fun; }
#if 0	/* ++++Unused */
					// Get selected shape, or return 0.
	int get_selected(int& shapenum, int& framenum)
		{
		if (selected == -1)
			return (0);
		shapenum = info[selected].shapenum;
		framenum = info[selected].framenum;
		return (1);
		}
#endif
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
					// Export current frame as a PNG.
	time_t export_png(const char *fname);
					// Export given image as a PNG.
	time_t export_png(const char *fname, Image_buffer8& img,
							int xoff, int yoff);
					// Export frames tiled.
	time_t export_tiled_png(const char *fname, int tiles, bool bycols);
	void edit_shape_info();		// Edit selected shape's info.
					// Edit selected shape-frame.
	void edit_shape(int tiles = 0, bool bycols = false);
					// Deal with list of files being edited
					//   by an external prog. (Gimp).
	static gint check_editing_files_cb(gpointer data);
	static gint check_editing_files();
	static void read_back_edited(Editing_file *ed);
	static void clear_editing_files();
					// Import/export from file selector.
	static void export_frame(char *fname, gpointer user_data);
	static void import_frame(char *fname, gpointer user_data);
	static void export_all_frames(char *fname, gpointer user_data);
	static void export_shape(char *fname, gpointer user_data);
	void export_all_pngs(char *fname, int shnum);
	static void import_all_frames(char *fname, gpointer user_data);
	static void import_shape(char *fname, gpointer user_data);
	void import_all_pngs(char *fname, int shnum);
	void new_frame();		// Add/del.
	void from_font_toggled(bool on);
	void new_shape();
	void create_new_shape();
	void del_frame();
					// Give dragged shape.
	static void drag_data_get(GtkWidget *widget, GdkDragContext *context,
		GtkSelectionData *selection_data, guint info, guint time, gpointer data);
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
#else
	static gint drag_motion(GtkWidget *widget, GdkEventMotion *event,
		gpointer data);
#endif
					// Menu items:
	static void on_shapes_popup_info_activate(
					GtkMenuItem *item, gpointer udata);
	static void on_shapes_popup_edit_activate(
					GtkMenuItem *item, gpointer udata);
	static void on_shapes_popup_edtiles_activate(
					GtkMenuItem *item, gpointer udata);
	};

#endif
