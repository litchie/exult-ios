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

#include "rect.h"

class Vga_file;
class Image_buffer8;

/*
 *	Store information about an individual shape shown in the list.
 */
class Shape_info
	{
	friend class Shape_chooser;
	int shapenum, framenum;		// The given shape/frame.
	Rectangle box;			// Box where drawn.
	Shape_info() {  }
	void set(int shnum, int frnum, int rx, int ry, int rw, int rh)
		{
		shapenum = shnum; framenum = frnum;
		box = Rectangle(rx, ry, rw, rh);
		}
	};

/*
 *	This class manages a list of shapes from an image file.
 */
class Shape_chooser
	{
	Vga_file *ifile;		// Where the shapes come from.
	int num_shapes;			// Total # shapes in ifile.
	char **names;			// Names of shapes (or null).
	GtkWidget *draw;		// GTK draw area to display them in.
	GdkGC *drawgc;			// For drawing in 'draw'.
	Image_buffer8 *iwin;		// What we render into.
	GdkRgbCmap *palette;		// For gdk_draw_indexed_image().
	GtkWidget *sbar;		// Status bar.
	guint sbar_sel;			// Status bar context for selection.
	int shapenum0, framenum0;	// Shape, frame # of leftmost in
					//   displayed list.
	Shape_info *info;		// An entry for each shape drawn.
	int info_cnt;			// # entries in info.
	int selected;			// Index of user-selected entry.
	void (*sel_changed)();		// Called when selection changes.
					// Blit onto screen.
	void show(int x, int y, int w, int h);
	void show()
		{ show(0, 0, draw->allocation.width, draw->allocation.height);}
	void select(int new_sel);	// Show new selection.
	void render();			// Draw list.
	void scroll(int newindex);	// Scroll.
public:
	Shape_chooser(Vga_file *i, char **nms, GtkWidget *box, int w, int h);
	~Shape_chooser();
	void unselect();		// Turn off selection.
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
					// Configure when created/resized.
	static gint configure(GtkWidget *widget, GdkEventConfigure *event,
							gpointer data);
					// Blit to screen.
	static gint expose(GtkWidget *widget, GdkEventExpose *event,
							gpointer data);
					// Handle mouse press.
	static gint mouse_press(GtkWidget *widget, GdkEventButton *event,
							gpointer data);
					// Handle scrollbar.
	static void scrolled(GtkAdjustment *adj, gpointer data);
	};

#endif
