/*
Copyright (C) 2000-2001 The Exult Team

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
#include "paledit.h"
#include "vgafile.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"
#include "studio.h"

Vga_file *ifile = 0;
char **names = 0;
Shape_chooser *chooser = 0;
Palette_edit *paled = 0;

ExultStudio::ExultStudio(int argc, char **argv)
{
	gtk_init( &argc, &argv );
	gdk_rgb_init();
	glade_init();
	app_xml = glade_xml_new( "./exult_studio.glade", "main_window" );
	app = glade_xml_get_widget( app_xml, "main_window" );
	// glade_xml_signal_autoconnect( app_xml );
	gtk_widget_show( app );
	create_shape_browser();
	create_palette_browser();
}

ExultStudio::~ExultStudio()
{
	gtk_widget_destroy( app );
	gtk_object_unref( GTK_OBJECT( app_xml ) );
	delete chooser;
	delete paled;
	int num_shapes = ifile->get_num_shapes();
	delete ifile;
	for (int i = 0; i < num_shapes; i++)
		delete names[i];
	delete [] names;
}

void ExultStudio::create_shape_browser()
{
	ifile = new Vga_file("static/shapes.vga");
	if (!ifile->is_good()) {
		cerr << "Error opening image file 'shapes.vga'.\n";
		abort();
	}
	
	// Read in shape names.
	int num_names = ifile->get_num_shapes();
	names = new char *[num_names];
	Flex *items = new Flex("static/text.flx");
	size_t len;
	for (int i = 0; i < num_names; i++)
		names[i] = items->retrieve(i, len);
	delete items;
	
	
	GtkWidget *shape_browser_box = glade_xml_get_widget( app_xml, "shape_browser_box" );
	gtk_widget_show( shape_browser_box );
	chooser = new Shape_chooser(ifile, names, 400, 64);
	gtk_box_pack_start(GTK_BOX(shape_browser_box), chooser->get_widget(), TRUE, TRUE, 0);
}

void ExultStudio::create_palette_browser()
{
	U7object pal("static/palettes.flx", 0);
	size_t len;
	unsigned char *buf;		// this may throw an exception
	buf = (unsigned char *) pal.retrieve(len);
	guint32 colors[256];
	for (int i = 0; i < 256; i++)
		colors[i] = (buf[3*i]<<16)*4 + (buf[3*i+1]<<8)*4 + 
							buf[3*i+2]*4;
	GtkWidget *palette_browser_box = glade_xml_get_widget( app_xml, "palette_browser_box" );
	gtk_widget_show( palette_browser_box );
	paled = new Palette_edit(colors, palette_browser_box, 128, 128);	
}
	
void ExultStudio::run()
{
	gtk_main();
}
