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
#include <dirent.h>
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

ExultStudio *ExultStudio::self = 0;

void on_filelist_select_row(GtkCList        *clist,
				   gint             row,
				   gint             column,
				   GdkEvent        *event,
				   gpointer         user_data)
{
	char *text;
	gtk_clist_get_text( clist, row, 0, &text );
	ExultStudio::get_instance()->create_shape_browser(text);
}

void
on_open_static_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->choose_static_dir();
}

ExultStudio::ExultStudio(int argc, char **argv)
{
	// Initialize the various subsystems
	self = this;
	gtk_init( &argc, &argv );
	gdk_rgb_init();
	glade_init();
	
	// Load the Glade interface
	app_xml = glade_xml_new( "./exult_studio.glade", "main_window" );
	app = glade_xml_get_widget( app_xml, "main_window" );
	
	// Connect signals
	GtkWidget *temp;
	temp = glade_xml_get_widget( app_xml, "exit" );
	gtk_signal_connect(GTK_OBJECT(temp), "activate",
				GTK_SIGNAL_FUNC(gtk_main_quit), 0);
	temp = glade_xml_get_widget( app_xml, "file_list" );
	gtk_signal_connect(GTK_OBJECT(temp), "select_row",
				GTK_SIGNAL_FUNC(on_filelist_select_row), this);
	temp = glade_xml_get_widget( app_xml, "open_static" );
	gtk_signal_connect(GTK_OBJECT(temp), "activate",
				GTK_SIGNAL_FUNC(on_open_static_activate), this);
	
	// More setting up...
	gtk_widget_show( app );
	chooser = 0;
	static_path = "./static/";
	scan_static_path();
}

ExultStudio::~ExultStudio()
{
	gtk_widget_destroy( app );
	gtk_object_unref( GTK_OBJECT( app_xml ) );
	delete_shape_browser();
	self = 0;
}

void ExultStudio::set_browser_frame_name(const char *name)
{
	GtkWidget *browser_frame = glade_xml_get_widget( app_xml, "browser_frame" );
	gtk_frame_set_label( GTK_FRAME( browser_frame ), name );
}

void ExultStudio::create_shape_browser(const char *fname)
{
	delete_shape_browser();
	char *fullname = g_strdup_printf("%s%s", static_path, fname);
	
	ifile = new Vga_file(fullname);
	g_free(fullname);
	if (!ifile->is_good()) {
		cerr << "Error opening image file '" << fname << "'.\n";
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
	
	
	GtkWidget *browser_box = glade_xml_get_widget( app_xml, "browser_box" );
	gtk_widget_show( browser_box );
	chooser = new Shape_chooser(ifile, 400, 64);
	chooser->set_shape_names(names);
	gtk_box_pack_start(GTK_BOX(browser_box), chooser->get_widget(), TRUE, TRUE, 0);
	set_browser_frame_name("Shape Browser");
}

void ExultStudio::delete_shape_browser()
{
	if(chooser) {
		delete chooser;
		chooser = 0;
		int num_shapes = ifile->get_num_shapes();
		delete ifile;
		ifile = 0;
		for (int i = 0; i < num_shapes; i++)
		delete names[i];
		delete [] names;
		names = 0;
	}
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

void ExultStudio::choose_static_dir()
{

}

void ExultStudio::scan_static_path()
{
	GtkWidget *filelist = glade_xml_get_widget( app_xml, "file_list" );
	struct dirent *entry;
	DIR *dir = opendir(static_path);
	if(!dir)
		return;
	gtk_clist_freeze( GTK_CLIST( filelist ) );
	gtk_clist_clear( GTK_CLIST( filelist ) );
	while(entry=readdir(dir)) {
		char *name = entry->d_name;
		if(!strcmp(name,".")||!strcmp(name,"..")||!strstr(name,".vga"))
			continue;
		
		char *text[2];
		text[0] = name;
		text[1] = "N/A";
		gtk_clist_append( GTK_CLIST( filelist ), text );
	}
	gtk_clist_thaw( GTK_CLIST( filelist ) );
	closedir(dir);
}
	
void ExultStudio::run()
{
	gtk_main();
}
