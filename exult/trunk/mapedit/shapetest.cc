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

#include "../alpha_kludges.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include "shapelst.h"
#include "vgafile.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"


Vga_file *ifile = 0;
char **names = 0;
GtkWidget *topwin = 0;
Shape_chooser *chooser = 0;

/*
 *	Quit.
 */

int Quit
	(
	)
	{
	delete chooser;
	int num_shapes = ifile->get_num_shapes();
	delete ifile;
	for (int i = 0; i < num_shapes; i++)
		delete names[i];
	delete [] names;
	gtk_exit(0);
	return (FALSE);			// Shouldn't get here.
	}

/*
 *	Main program.
 */

int main
	(
	int argc,
	char **argv
	)
	{
					// Open file.
	ifile = new Vga_file("static/shapes.vga");
	if (!ifile->is_good())
		{
		cerr << "Error opening image file 'shapes.vga'.\n";
		return (1);
		}
					// Read in shape names.
	int num_names = ifile->get_num_shapes();
	names = new char *[num_names];
	Flex *items = new Flex("static/text.flx");
	size_t len;
	for (int i = 0; i < num_names; i++)
		names[i] = items->retrieve(i, len);
	delete items;
	gtk_init(&argc, &argv);
	gdk_rgb_init();
					// Create top-level window.
	topwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(topwin), "Shape-Browser Test");
					// Set delete handler.
	gtk_signal_connect(GTK_OBJECT(topwin), "delete_event",
				GTK_SIGNAL_FUNC(Quit), NULL);
					// Set border width of top window.
	gtk_container_border_width(GTK_CONTAINER(topwin), 10);
	/*
 	 *	Create shape chooser.
	 */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(topwin), vbox);
	gtk_widget_show(vbox);
	chooser = new Shape_chooser(ifile, 400, 64);
	chooser->set_shape_names(names);
	gtk_box_pack_start(GTK_BOX(vbox), chooser->get_widget(), TRUE, TRUE, 0);
	gtk_widget_show(topwin);	// Show top window.
	gtk_main();
	return (0);
	}
