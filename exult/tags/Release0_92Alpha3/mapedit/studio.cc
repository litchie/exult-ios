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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <dirent.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <unistd.h>
#include "shapelst.h"
#include "paledit.h"
#include "vgafile.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"
#include "studio.h"
#include "dirbrowser.h"

ExultStudio *ExultStudio::self = 0;

enum ExultFileTypes {
	ShapeArchive =1,
	PaletteFile,
	FlexArchive
};

void on_filelist_tree_select_row       (GtkCTree        *ctree,
                                        GtkCTreeNode    *node,
                                        gint             column,
                                        gpointer         user_data)
{
	int type = (int)gtk_ctree_node_get_row_data( ctree, node );
	gboolean leaf;
	char *text;
	gtk_ctree_get_node_info( ctree, node, &text, 0, 0, 0, 0, 0, &leaf, 0);
	ExultStudio *studio = ExultStudio::get_instance();
	switch(type) {
	case ShapeArchive:
		studio->set_browser("Shape Browser", studio->create_shape_browser(text));
		break;
	case PaletteFile:
		studio->set_browser("Palette Browser", studio->create_palette_browser(text));
		break;
	default:
		break;
	}
}                                     

void
on_open_static_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->choose_static_path();
}

void on_choose_directory               (gchar *dir)
{
	ExultStudio::get_instance()->set_static_path(dir);
}

ExultStudio::ExultStudio(int argc, char **argv): ifile(0), names(0)
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
	gtk_signal_connect(GTK_OBJECT(temp), "tree_select_row",
				GTK_SIGNAL_FUNC(on_filelist_tree_select_row), this);
	temp = glade_xml_get_widget( app_xml, "open_static" );
	gtk_signal_connect(GTK_OBJECT(temp), "activate",
				GTK_SIGNAL_FUNC(on_open_static_activate), this);
	
	// More setting up...
	gtk_widget_show( app );
	browser = 0;
	static_path = 0;
}

ExultStudio::~ExultStudio()
{
	delete_shape_browser();
	gtk_widget_destroy( app );
	gtk_object_unref( GTK_OBJECT( app_xml ) );
	self = 0;
}

void ExultStudio::set_browser(const char *name, Object_browser *obj)
{
	if(browser)
		delete browser;
	browser = obj;
	
	GtkWidget *browser_frame = glade_xml_get_widget( app_xml, "browser_frame" );
	gtk_frame_set_label( GTK_FRAME( browser_frame ), name );
	
	GtkWidget *browser_box = glade_xml_get_widget( app_xml, "browser_box" );
	gtk_widget_show( browser_box );
	gtk_box_pack_start(GTK_BOX(browser_box), browser->get_widget(), TRUE, TRUE, 0);
}

Object_browser *ExultStudio::create_shape_browser(const char *fname)
{
	delete_shape_browser();
	char *fullname = g_strdup_printf("%s%s", static_path, fname);
	
	ifile = new Vga_file(fullname);
	g_free(fullname);
	if (!ifile->is_good()) {
		cerr << "Error opening image file '" << fname << "'.\n";
		abort();
	}
	
	Shape_chooser *chooser = new Shape_chooser(ifile, 400, 64);
	if(!strcasecmp(fname,"shapes.vga")) {
		// Read in shape names.
		int num_names = ifile->get_num_shapes();
		names = new char *[num_names];
		Flex *items = new Flex("static/text.flx");
		size_t len;
		for (int i = 0; i < num_names; i++)
			names[i] = items->retrieve(i, len);
		delete items;
		chooser->set_shape_names(names);
	}	
		
	return chooser;
}

void ExultStudio::delete_shape_browser()
{
	if(ifile) {
		int num_shapes = ifile->get_num_shapes();
		delete ifile;
		ifile = 0;
		if(names) {
			for (int i = 0; i < num_shapes; i++)
				delete names[i];
			delete [] names;
			names = 0;
		}
	}
}

Object_browser *ExultStudio::create_palette_browser(const char *fname)
{
	char *fullname = g_strdup_printf("%s%s", static_path, fname);
	U7object pal(fullname, 0);
	g_free(fullname);
	size_t len;
	unsigned char *buf;		// this may throw an exception
	buf = (unsigned char *) pal.retrieve(len);
	guint32 colors[256];
	for (int i = 0; i < 256; i++)
		colors[i] = (buf[3*i]<<16)*4 + (buf[3*i+1]<<8)*4 + 
							buf[3*i+2]*4;
	Palette_edit *paled = new Palette_edit(colors, 128, 128);
	return paled;
}

void ExultStudio::choose_static_path()
{
	char cwd[1024];
	getcwd(cwd, 1024);
	GtkWidget *dirbrowser = xmms_create_dir_browser("Select static directory",
							cwd, GTK_SELECTION_SINGLE,
							on_choose_directory);
	gtk_signal_connect(GTK_OBJECT(dirbrowser), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &dirbrowser);
        gtk_window_set_transient_for(GTK_WINDOW(dirbrowser), GTK_WINDOW(app));
	gtk_widget_show (dirbrowser);
}

GtkCTreeNode *create_subtree( GtkCTree *ctree,
			      GtkCTreeNode *previous,
			      const char *name,
			      const char *path,
			      const char *ext,
			      gpointer data
			    )
{
	struct dirent *entry;
	DIR *dir = opendir(path);
	if(!dir)
		return 0;
	GtkCTreeNode *parent, *sibling;
	char *text[1];
	text[0] = (char *)name;
	parent = gtk_ctree_insert_node( ctree,
					0,
					previous,
					text,
					0,
					0,0,0,0,
					FALSE,
					TRUE );
	gtk_ctree_node_set_selectable( ctree,
				       parent, 
				       FALSE );
	sibling = 0;	
	while(entry=readdir(dir)) {
		char *fname = entry->d_name;
		if(!strcmp(fname,".")||!strcmp(fname,"..")||!strstr(fname,ext))
			continue;
		text[0] = fname;
		sibling = gtk_ctree_insert_node( ctree,
						 parent,
						 sibling,
						 text,
						 0,
						 0,0,0,0,
						 TRUE,
						 FALSE );
		gtk_ctree_node_set_row_data( ctree,
					     sibling,
					     data );
		
	}
	closedir(dir);			
	return parent;
}

void ExultStudio::set_static_path(const char *path)
{
	if(static_path)
		g_free(static_path);
	static_path = g_strdup(path);	
	GtkWidget *file_list = glade_xml_get_widget( app_xml, "file_list" );
	
	gtk_clist_clear( GTK_CLIST( file_list ) );
	
	gtk_clist_freeze( GTK_CLIST( file_list ) );
	
	GtkCTreeNode *shapefiles = create_subtree( GTK_CTREE( file_list ),
						   0,
						   "Shape Files",
						   static_path,
						   ".vga",
						   (gpointer)ShapeArchive );
	GtkCTreeNode *palettefiles = create_subtree( GTK_CTREE( file_list ),
						   shapefiles,
						   "Palette Files",
						   static_path,
						   ".pal",
						   (gpointer)PaletteFile );
	GtkCTreeNode *flexfiles = create_subtree( GTK_CTREE( file_list ),
						   palettefiles,
						   "FLEX Files",
						   static_path,
						   ".flx",
						   (gpointer)FlexArchive );
	gtk_clist_thaw( GTK_CLIST( file_list ) );
}
	
void ExultStudio::run()
{
	gtk_main();
}
