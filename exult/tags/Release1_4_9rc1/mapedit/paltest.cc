/**
 **	A GTK widget showing a palette's colors.
 **
 **	Written: 12/24/2000 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

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
#ifdef XWIN
#include <gdk/gdkx.h>
#endif
#include <glib.h>
#include "Flex.h"
#include "paledit.h"
#include "u7drag.h"


GtkWidget *topwin = 0;
Palette_edit *paled = 0;

/*
 *	Quit.
 */

int Quit
	(
	)
	{
	delete paled;
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
	gtk_init(&argc, &argv);
	gdk_rgb_init();
					// Create top-level window.
	topwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(topwin), "Palette Editor Test");
					// Set delete handler.
	gtk_signal_connect(GTK_OBJECT(topwin), "delete_event",
				GTK_SIGNAL_FUNC(Quit), NULL);
					// Set border width of top window.
	gtk_container_border_width(GTK_CONTAINER(topwin), 10);
	/*
 	 *	Create palette editor.
	 */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(topwin), vbox);
	gtk_widget_show(vbox);
	U7object pal("static/palettes.flx", 0);
	size_t len;
	unsigned char *buf;		// this may throw an exception
	buf = (unsigned char *) pal.retrieve(len);
	guint32 colors[256];
	for (int i = 0; i < 256; i++)
		colors[i] = (buf[3*i]<<16)*4 + (buf[3*i+1]<<8)*4 + 
							buf[3*i+2]*4;
	paled = new Palette_edit(colors, 128, 128);
	gtk_box_pack_start(GTK_BOX(vbox), paled->get_widget(), TRUE, TRUE, 0);
	gtk_widget_show(topwin);	// Show top window.
	gtk_main();
	return (0);
	}
