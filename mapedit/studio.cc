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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <dirent.h>
#include <gtk/gtk.h>
#ifdef XWIN
#include <gdk/gdkx.h>
#endif
#include <glib.h>
#include <unistd.h>
#include <errno.h>

#include <cstdio>			/* These are for sockets. */
#ifdef WIN32
#include <getopt.h>
#include <windows.h>
#include <getopt.h>
#include <ole2.h>
#include "servewin32.h"
#else
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <fcntl.h>

#include "objbrowse.h"
#include "paledit.h"
#include "shapevga.h"
#include "ibuf8.h"
#include "Flex.h"
#include "studio.h"
#include "dirbrowser.h"
#include "utils.h"
#include "u7drag.h"
#include "shapegroup.h"
#include "shapefile.h"
#include "shapedraw.h"
#include "paledit.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

ExultStudio *ExultStudio::self = 0;

enum ExultFileTypes {
	ShapeArchive =1,
	ChunkArchive,
	PaletteFile,
	FlexArchive
};

C_EXPORT void on_filelist_tree_select_row       (GtkCTree        *ctree,
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
		studio->set_browser("Shape Browser", 
					studio->create_browser(text));
		break;
	case ChunkArchive:
		studio->set_browser("Chunk Browser", 
					studio->create_browser(text));
		break;
	case PaletteFile:
		studio->set_browser("Palette Browser", 
					studio->create_palette_browser(text));
		break;
	default:
		break;
	}
}                                     

C_EXPORT void
on_open_static_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->choose_static_path();
}

C_EXPORT void
on_connect_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->connect_to_server();
}

C_EXPORT void
on_save_map_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->write_map();
}

C_EXPORT void
on_read_map_menu_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->read_map();
}

C_EXPORT void
on_save_shape_info1_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->write_shape_info();
}

C_EXPORT void
on_reload_usecode_menu_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->reload_usecode();
}

C_EXPORT void
on_save_groups1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->save_groups();
}

C_EXPORT void
on_play_button_clicked			(GtkToggleButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->set_play(
				gtk_toggle_button_get_active(button));
}

C_EXPORT void
on_tile_grid_button_toggled		(GtkToggleButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->set_tile_grid(
				gtk_toggle_button_get_active(button));
}

C_EXPORT void
on_edit_lift_spin_changed		(GtkSpinButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->set_edit_lift(
				gtk_spin_button_get_value_as_int(button));
}

C_EXPORT void
on_edit_terrain_button_toggled		(GtkToggleButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->set_edit_terrain(
				gtk_toggle_button_get_active(button));
}

void on_choose_directory               (gchar *dir)
{
	ExultStudio::get_instance()->set_static_path(dir);
}

/*
 *	Main window's close button.
 */
C_EXPORT gboolean on_main_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	return FALSE;
	}
C_EXPORT void on_main_window_destroy_event
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	gtk_main_quit();
	}

ExultStudio::ExultStudio(int argc, char **argv): files(0), curfile(0), 
	names(0), glade_path(0),
	vgafile(0), facefile(0), eggwin(0), 
	server_socket(-1), server_input_tag(-1), 
	static_path(0), 
	browser(0), palbuf(0), egg_monster_draw(0), 
	egg_ctx(0),
	waiting_for_server(0), npcwin(0), npc_draw(0), npc_face_draw(0),
	npc_ctx(0), objwin(0), obj_draw(0), shapewin(0), shape_draw(0),
	equipwin(0)
{
	// Initialize the various subsystems
	self = this;
	gtk_init( &argc, &argv );
	gdk_rgb_init();
	glade_init();
					// Get options.
	char *xmldir = 0;		// Default:  Look here for .glade.
	char *gamedir = 0;		// User has to choose 'static'.
	static char *optstring = "x:d:";
	extern int optind, opterr, optopt;
	extern char *optarg;
	opterr = 0;			// Don't let getopt() print errs.
	int optchr;
	while ((optchr = getopt(argc, argv, optstring)) != -1)
		switch (optchr)
			{
		case 'x':		// XML (.glade) directory.
			xmldir = optarg;
			break;
		case 'd':		// Game directory.
			gamedir = optarg;
			break;
			}

	char path[256];			// Set up paths.
	if(xmldir)
		strcpy(path, xmldir);
	else if(U7exists(EXULT_DATADIR"/exult_studio.glade"))
		strcpy(path,EXULT_DATADIR);
	else
		strcpy(path,".");
	strcat(path, "/exult_studio.glade");
	// Load the Glade interface
	app_xml = glade_xml_new(path, NULL);
	app = glade_xml_get_widget( app_xml, "main_window" );
	glade_path = g_strdup(path);

	// More setting up...
					// Connect signals automagically.
	glade_xml_signal_autoconnect(app_xml);
	gtk_widget_show( app );
	if (gamedir)			// Game directory given?
		{
		strcpy(path, gamedir);
		strcat(path, "/static/");// Set up path to static.
		set_static_path(path);
		}
#ifdef WIN32
    OleInitialize(NULL);
#endif
}

ExultStudio::~ExultStudio()
{
#ifdef WIN32
    OleUninitialize();
#endif
	if(names) {
		int num_shapes = vgafile->get_ifile()->get_num_shapes();
		for (int i = 0; i < num_shapes; i++)
			delete names[i];
		delete [] names;
		names = 0;
	}
	g_free(glade_path);
	delete files;
	files = 0;
	delete [] palbuf;
	palbuf = 0;
	if (objwin)
		gtk_widget_destroy(objwin);
	delete obj_draw;
	if (eggwin)
		gtk_widget_destroy(eggwin);
	delete egg_monster_draw;
	eggwin = 0;
	if (npcwin)
		gtk_widget_destroy(npcwin);
	delete npc_draw;
	npcwin = 0;
	if (shapewin)
		gtk_widget_destroy(shapewin);
	delete shape_draw;
	shapewin = 0;
	if (equipwin)
		gtk_widget_destroy(equipwin);
	equipwin = 0;
//Shouldn't be done here	gtk_widget_destroy( app );
	gtk_object_unref( GTK_OBJECT( app_xml ) );
#ifndef WIN32
	if (server_input_tag >= 0)
		gdk_input_remove(server_input_tag);
	if (server_socket >= 0)
		close(server_socket);
#else
	if (server_input_tag >= 0)
		gtk_timeout_remove(server_input_tag);
	if (server_socket >= 0)
		Exult_server::disconnect_from_server();
#endif
	g_free(static_path);
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

Object_browser *ExultStudio::create_browser(const char *fname)
{
	curfile = files->create(fname);

	Object_browser *chooser = curfile->create_browser(vgafile, names,
								palbuf);
	setup_groups(fname);		// Set up 'groups' page.
	return chooser;
}

Object_browser *ExultStudio::create_palette_browser(const char *fname)
{
	char *fullname = g_strdup_printf("%s%s", static_path, fname);
	Palette_edit *paled = new Palette_edit(fullname);
	g_free(fullname);
	return paled;
}

/*
 *	Get current groups.
 */

Shape_group_file *ExultStudio::get_cur_groups
	(
	)
	{
	return curfile ? curfile->get_groups() : 0;
	}

/*
 *	Choose 'static'.
 */
void ExultStudio::choose_static_path()
{
	size_t	bufsize=128;
	char * cwd(new char[bufsize]);
	while(!getcwd(cwd,bufsize))
		{
		if(errno==ERANGE)
			{
			bufsize+=128;
			delete [] cwd;
			cwd=new char[bufsize];
			}
		else
			{
			// Ooops. getcwd() has failed
			delete [] cwd;	// Prevent leakage
			return;
			}
		}
	GtkWidget *dirbrowser = xmms_create_dir_browser("Select static directory",
							cwd, GTK_SELECTION_SINGLE,
							on_choose_directory);
	gtk_signal_connect(GTK_OBJECT(dirbrowser), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &dirbrowser);
        gtk_window_set_transient_for(GTK_WINDOW(dirbrowser), GTK_WINDOW(app));
	gtk_widget_show (dirbrowser);
	delete [] cwd;	// Prevent leakage
}

/*
 *	Add a node to a tree.
 */
static GtkCTreeNode *Create_tree_node
	(
	GtkCTree *ctree,
	char *fname,			// Filename to add.
	GtkCTreeNode *parent,		// Subtree this is to go under.
	GtkCTreeNode *sibling,		// Sibling to go after.
	gpointer data
	)
	{
	char *text[1];
	text[0] = fname;
	GtkCTreeNode *newnd = gtk_ctree_insert_node(ctree, parent, sibling, 
			text, 0,0,0,0,0, TRUE, FALSE );
	gtk_ctree_node_set_row_data(ctree, newnd, data);
	return newnd;
	}

/*
 *	Add a subtree to a clist using files from the patch or static dirs.
 *
 *	Output:	Parent of subtree.
 */
GtkCTreeNode *Create_subtree( GtkCTree *ctree,
			      GtkCTreeNode *previous,
			      const char *name,
			      const char *ext,  // Or whole filename.
			      gpointer data
			    )
{
	struct dirent *entry;
	GtkCTreeNode *parent, *sibling = 0;
	char *text[1];
	text[0] = (char *) name;
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
	int extlen = strlen(ext);
	string spath("<STATIC>/"), ppath("<PATCH>/");
	spath = get_system_path(spath);
	ppath = get_system_path(ppath);
	DIR *dir = opendir(ppath.c_str());// Get names from 'patch' first.
	if (dir) {
		while(entry=readdir(dir)) {
			char *fname = entry->d_name;
			int flen = strlen(fname);
					// Ignore case of extension.
			if(!strcmp(fname,".")||!strcmp(fname,"..") ||
				strcasecmp(fname + flen - extlen, ext) != 0)
				continue;
			sibling = Create_tree_node(ctree, fname, parent,
							sibling, data);
		}
		closedir(dir);
	}
	dir = opendir(spath.c_str());	// Now go through 'static'.
	if (dir) {
		while(entry=readdir(dir)) {
			char *fname = entry->d_name;
			int flen = strlen(fname);
					// Ignore case of extension.
			if(!strcmp(fname,".")||!strcmp(fname,"..") ||
				strcasecmp(fname + flen - extlen, ext) != 0)
				continue;
					// See if also in 'patch'.
			string fullpath(ppath);
			fullpath += "/";
			fullpath += fname;
			if (U7exists(fullpath))
				continue;
			sibling = Create_tree_node(ctree, fname, parent,
							sibling, data);
		}
		closedir(dir);
	}
	return parent;
}

void ExultStudio::set_static_path(const char *path)
{
	if(static_path)
		g_free(static_path);
	static_path = g_strdup(path);	// Set up palette for showing shapes.
	add_system_path("<STATIC>", static_path);
	char *patch_path = g_strdup_printf("%s../%s", static_path, "patch");
	add_system_path("<PATCH>", patch_path);
	g_free(patch_path);
	char *palname = g_strdup_printf("%s%s", static_path, "palettes.flx");
	U7object pal(palname, 0);
	g_free(palname);
	size_t len;
	delete palbuf;			// Delete old.
					// this may throw an exception
	palbuf = (unsigned char *) pal.retrieve(len);
	if(names) {			// Delete old names.
		int num_shapes = vgafile->get_ifile()->get_num_shapes();
		for (int i = 0; i < num_shapes; i++)
			delete names[i];
		delete [] names;
		names = 0;
	}
	delete files;			// Close old shape files.
	files = new Shape_file_set();
	vgafile = files->create("shapes.vga");
	facefile = files->create("faces.vga");
					// Read in shape names.
	int num_names = vgafile->get_ifile()->get_num_shapes();
	names = new char *[num_names];
	char *txtname = g_strdup_printf("%s%s", static_path, "text.flx");
	Flex *items = new Flex(txtname);
	g_free(txtname);
	for (int i = 0; i < num_names; i++)
		names[i] = items->retrieve(i, len);
	delete items;
	GtkWidget *file_list = glade_xml_get_widget( app_xml, "file_list" );
	
	gtk_clist_clear( GTK_CLIST( file_list ) );
	
	gtk_clist_freeze( GTK_CLIST( file_list ) );
	
	GtkCTreeNode *shapefiles = Create_subtree( GTK_CTREE( file_list ),
						   0,
						   "Shape Files",
						   ".vga",
						   (gpointer)ShapeArchive );
	GtkCTreeNode *chunkfiles = Create_subtree( GTK_CTREE( file_list ),
						   0,
						   "Map Files",
						   "u7chunks",
						   (gpointer)ChunkArchive );
	GtkCTreeNode *palettefiles = Create_subtree( GTK_CTREE( file_list ),
						   0,
						   "Palette Files",
						   ".pal",
						   (gpointer)PaletteFile );
					// Always include main palettes file.
	Create_tree_node(GTK_CTREE(file_list), "palettes.flx", palettefiles, 0,
						   (gpointer)PaletteFile );
#if 0	/* Skip this until we can do something with these files. */
	GtkCTreeNode *flexfiles = Create_subtree( GTK_CTREE( file_list ),
						   palettefiles,
						   "FLEX Files",
						   ".flx",
						   (gpointer)FlexArchive );
#endif
	gtk_clist_thaw( GTK_CLIST( file_list ) );
	connect_to_server();		// Connect to server with 'gamedat'.
}

/*
 *	Write out map.
 */

void ExultStudio::write_map
	(
	)
	{
	send_to_server(Exult_server::write_map);
	}

/*
 *	Read in map.
 */

void ExultStudio::read_map
	(
	)
	{
	send_to_server(Exult_server::read_map);
	}

/*
 *	Write out shape info.
 */

void ExultStudio::write_shape_info
	(
	)
	{
	if (vgafile)
		{
		Shapes_vga_file *svga = 
				(Shapes_vga_file *) vgafile->get_ifile();
					// Make sure data's been read in.
		svga->read_info(false, true);//+++++BG?
		svga->write_info(false);//++++BG?
		}
	}

/*
 *	Reload usecode.
 */

void ExultStudio::reload_usecode
	(
	)
	{
	send_to_server(Exult_server::reload_usecode);
	}

/*
 *	Tell Exult to start/stop playing.
 */

void ExultStudio::set_play
	(
	gboolean play			// True to play, false to edit.
	)
	{
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	Write2(ptr, play ? 0 : 1);	// Map_edit = !play.
	send_to_server(Exult_server::map_editing_mode, data, ptr - data);
	}

/*
 *	Tell Exult to show or not show the tile grid.
 */

void ExultStudio::set_tile_grid
	(
	gboolean grid			// True to show.
	)
	{
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	Write2(ptr, grid ? 1 : 0);	// Map_edit = !play.
	send_to_server(Exult_server::tile_grid, data, ptr - data);
	}

/*
 *	Tell Exult to edit at a given lift.
 */

void ExultStudio::set_edit_lift
	(
	int lift
	)
	{
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	Write2(ptr, lift);
	send_to_server(Exult_server::edit_lift, data, ptr - data);
	}

/*
 *	Tell Exult to enter/leave 'terrain-edit' mode.
 */

void ExultStudio::set_edit_terrain
	(
	gboolean terrain		// True/false
	)
	{
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	Write2(ptr, terrain ? 1 : 0);	// NOTE:  Pass -1 to abort.  But I
					//   haven't got an interface yet.
	send_to_server(Exult_server::terrain_editing_mode, data, ptr - data);
	if (browser && !terrain)
		browser->end_terrain_editing();
	}

/*
 *	Get value of a toggle button (false if not found).
 */

bool ExultStudio::get_toggle
	(
	char *name
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	return btn ? gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)) : -1;
	}

/*
 *	Find and set a toggle/checkbox button.
 */

void ExultStudio::set_toggle
	(
	char *name,
	bool val
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	if (btn)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn), val);
	}

/*
 *	Get an 8-bit set of flags from a group of toggles.
 */

unsigned char ExultStudio::get_bit_toggles
	(
	char **names,			// Names for bit 0, 1, 2,...
	int num				// # of names/bits.
	)
	{
	unsigned char bits = 0;
	for (int i = 0; i < num; i++)
		bits |= (get_toggle(names[i]) ? 1 : 0) << i;
	return bits;
	}

/*
 *	Set a group of toggles based on a sequential (0, 1, 2...) set of bit
 *	flags.
 */

void ExultStudio::set_bit_toggles
	(
	char **names,			// Names for bit 0, 1, 2,...
	int num,			// # of names/bits.
	unsigned char bits
	)
	{
	for (int i = 0; i < num; i++)
		set_toggle(names[i], (bits&(1<<i)) != 0);
	}

/*
 *	Get value of option-menu button (-1 if unsuccessful).
 */

int ExultStudio::get_optmenu
	(
	char *name
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	if (!btn)
		return -1;
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(btn));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	return g_list_index(GTK_MENU_SHELL(menu)->children, active);
	}

/*
 *	Find and set an option-menu button.
 */

void ExultStudio::set_optmenu
	(
	char *name,
	int val
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	if (btn)
		gtk_option_menu_set_history(GTK_OPTION_MENU(btn), val);
	}

/*
 *	Get value of spin button (-1 if unsuccessful).
 */

int ExultStudio::get_spin
	(
	char *name
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	return btn ? gtk_spin_button_get_value_as_int(
						GTK_SPIN_BUTTON(btn)) : -1;
	}

/*
 *	Find and set a spin button.
 */

void ExultStudio::set_spin
	(
	char *name,
	int val,
	bool sensitive
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	if (btn)
		{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(btn), val);
		gtk_widget_set_sensitive(btn, sensitive);
		}
	}

/*
 *	Find and set a spin button, along with its range.
 */

void ExultStudio::set_spin
	(
	char *name,
	int val,
	int low, int high		// Range.
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	if (btn)
		{
		gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(btn),
			GTK_ADJUSTMENT(
			gtk_adjustment_new (0, low, high, 1, 10, 10)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(btn), val);
		}
	}

/*
 *	Get number from a text field.
 *
 *	Output:	Number, or -1 if not found.
 */

int ExultStudio::get_num_entry
	(
	char *name
	)
	{
	GtkWidget *field = glade_xml_get_widget(app_xml, name);
	if (!field)
		return -1;
	char *txt = gtk_entry_get_text(GTK_ENTRY(field));
	if (!txt)
		return -1;
	while (*txt == ' ')
		txt++;			// Skip space.
	if (txt[0] == '0' && txt[1] == 'x')
		return (int) strtoul(txt + 2, 0, 16);	// Hex.
	else
		return atoi(txt);
	}

/*
 *	Get text from a text field.
 *
 *	Output:	->text, or null if not found.
 */

char *ExultStudio::get_text_entry
	(
	char *name
	)
	{
	GtkWidget *field = glade_xml_get_widget(app_xml, name);
	if (!field)
		return 0;
	return gtk_entry_get_text(GTK_ENTRY(field));
	}

/*
 *	Find and set a text field to a number.
 */

void ExultStudio::set_entry
	(
	char *name,
	int val,
	bool hex,
	bool sensitive
	)
	{
	GtkWidget *field = glade_xml_get_widget(app_xml, name);
	if (field)
		{
		char *txt = hex ? g_strdup_printf("0x%x", val)
				: g_strdup_printf("%d", val);
		gtk_entry_set_text(GTK_ENTRY(field), txt);
		g_free(txt);
		gtk_widget_set_sensitive(field, sensitive);
		}
	}

/*
 *	Set text field.
 */

void ExultStudio::set_entry
	(
	char *name,
	const char *val,
	bool sensitive
	)
	{
	GtkWidget *field = glade_xml_get_widget(app_xml, name);
	if (field)
		{
		gtk_entry_set_text(GTK_ENTRY(field), val);
		gtk_widget_set_sensitive(field, sensitive);
		}
	}

/*
 *	Set statusbar.
 */

void ExultStudio::set_statusbar
	(
	char *name,
	int context,
	char *msg
	)
	{
	GtkWidget *sbar = glade_xml_get_widget(app_xml, name);
	if (sbar)
		gtk_statusbar_push(GTK_STATUSBAR(sbar), context, msg);
	}

/*
 *	Set a button's text.
 */

void ExultStudio::set_button
	(
	char *name,
	const char *text
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	GtkLabel *label = GTK_LABEL(GTK_BIN(btn)->child);
	gtk_label_set_text(label, text);
	}

/*
 *	Show/hide a widget.
 */

void ExultStudio::set_visible
	(
	char *name,
	bool vis
	)
	{
	GtkWidget *widg = glade_xml_get_widget(app_xml, name);
	if (widg)
		{
		if (vis)
			gtk_widget_show(widg);
		else
			gtk_widget_hide(widg);
		}
	}

/*
 *	Enable/disable a widget.
 */

void ExultStudio::set_sensitive
	(
	char *name,
	bool tf
	)
	{
	GtkWidget *widg = glade_xml_get_widget(app_xml, name);
	if (widg)
		gtk_widget_set_sensitive(widg, tf);
	}

/*
 *	Handle 'prompt' dialogs:
 */
static int prompt_choice = 0;		// Gets prompt choice.

C_EXPORT void
on_prompt3_yes_clicked			(GtkToggleButton *button,
					 gpointer	  user_data)
{
	prompt_choice = 0;
}
C_EXPORT void
on_prompt3_no_clicked			(GtkToggleButton *button,
					 gpointer	  user_data)
{
	prompt_choice = 1;
}
C_EXPORT void
on_prompt3_cancel_clicked		(GtkToggleButton *button,
					 gpointer	  user_data)
{
	prompt_choice = 2;
}


/*
 *	Prompt for one of two/three answers.
 *
 *	Output:	0 for 1st choice, 1 for 2nd, 2 for 3rd.
 */

int ExultStudio::prompt
	(
	const char *msg,		// Question to ask.
	const char *choice0,		// 1st choice.
	const char *choice1,		// 2nd choice, or NULL.
	const char *choice2		// 3rd choice, or NULL.
	)
	{
	GtkWidget *dlg = glade_xml_get_widget(app_xml, "prompt3_dialog");
	gtk_label_set_text(
		GTK_LABEL(glade_xml_get_widget(app_xml, "prompt3_label")),
								msg);
	set_button("prompt3_yes", choice0);
	if (choice1)
		{
		set_button("prompt3_no", choice1);
		set_visible("prompt3_no", true);
		}
	else
		set_visible("prompt3_no", false);
	if (choice2)			// 3rd choice?  Show btn if so.
		{
		set_button("prompt3_cancel", choice2);
		set_visible("prompt3_cancel", true);
		}
	else
		set_visible("prompt3_cancel", false);
	prompt_choice = -1;
	gtk_window_set_modal(GTK_WINDOW(dlg), true);
	gtk_widget_show(dlg);		// Should be modal.
	while (prompt_choice == -1)	// Spin.
		gtk_main_iteration();	// (Blocks).
	gtk_widget_hide(dlg);
	assert(prompt_choice == 0 || prompt_choice == 1);
	return prompt_choice;
	}

void ExultStudio::run()
{
	gtk_main();
}

/*
 *	This is called every few seconds to try to reconnect to Exult.
 */

static gint Reconnect
	(
	gpointer data			// ->ExultStudio.
	)
	{
	ExultStudio *studio = (ExultStudio *) data;
	if (studio->connect_to_server())
		return 0;		// Cancel timer.  We succeeded.
	else
		return 1;
	}

/*
 *	Send message to server.
 *
 *	Output:	false if error sending (reported).
 */

bool ExultStudio::send_to_server
	(
	Exult_server::Msg_type id,
	unsigned char *data, 
	int datalen
	)
	{
	if (Send_data(server_socket, id, data, datalen) == -1)
		{
		cerr << "Error sending to server" << endl;
		return false;
		}
	return true;
	}

/*
 *	Input from server is available.
 */

#ifndef WIN32
static void Read_from_server
	(
	gpointer data,			// ->ExultStudio.
	gint socket,
	GdkInputCondition condition
	)
	{
	ExultStudio *studio = (ExultStudio *) data;
	studio->read_from_server();
	}
#else
static gint Read_from_server
	(
	gpointer data			// ->ExultStudio.
	)
	{
	ExultStudio *studio = (ExultStudio *) data;
	studio->read_from_server();
	return TRUE;
	}
#endif

gint Do_Drop_Callback(gpointer data);

void ExultStudio::read_from_server
	(
	)
	{
#ifdef WIN32
	// Nothing
	int len = Exult_server::peek_pipe();

	//Do_Drop_Callback(&len);

	if (len == -1)  {
		cout << "Disconnected from server" << endl;
		gtk_timeout_remove(server_input_tag);
		Exult_server::disconnect_from_server();
		server_input_tag = -1;
		server_socket = -1;
				// Try again every 4 secs.
		gtk_timeout_add(4000, Reconnect, this);

		return;
	}
	if (len < 1) return;
#endif
	unsigned char data[Exult_server::maxlength];
	Exult_server::Msg_type id;
	int datalen = Exult_server::Receive_data(server_socket, id, data,
							sizeof(data));
	if (datalen < 0)
		{
		cout << "Error reading from server" << endl;
		if (server_socket == -1)// Socket closed?
			{
#ifndef WIN32
			gdk_input_remove(server_input_tag);
#else
			gtk_timeout_remove(server_input_tag);
			Exult_server::disconnect_from_server();
#endif
			server_input_tag = -1;
					// Try again every 4 secs.
			gtk_timeout_add(4000, Reconnect, this);
			}
		return;
		}
	cout << "Read " << datalen << " bytes from server" << endl;
	cout << "ID = " << (int) id << endl;
	switch (id)
		{
	case Exult_server::obj:
		open_obj_window(data, datalen);
		break;
	case Exult_server::egg:
		open_egg_window(data, datalen);
		break;
	case Exult_server::npc:
		open_npc_window(data, datalen);
		break;
	case Exult_server::user_responded:
	case Exult_server::cancel:
	case Exult_server::locate_terrain:
	case Exult_server::swap_terrain:
	case Exult_server::insert_terrain:
		if (waiting_for_server)	// Send msg. to callback.
			{
			waiting_for_server(id, data, datalen, waiting_client);
			waiting_for_server = 0;
			waiting_client = 0;
			}
		else if (browser)
			browser->server_response((int) id, data, datalen);
		break;
		}
	}

/*
 *	Try to connect to the Exult game.
 *
 *	Output:	True if succeeded.
 */
bool ExultStudio::connect_to_server
	(
	)
	{
	if (!static_path)
		return false;		// No place to go.
#ifndef WIN32
	if (server_socket >= 0)		// Close existing socket.
		{
		close(server_socket);
		gdk_input_remove(server_input_tag);
		}
	server_socket = server_input_tag = -1;
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
					// Set up path to gamedat.
	strcpy(addr.sun_path, static_path);
	char *pstatic = strrchr(addr.sun_path, '/');
	if (pstatic && !pstatic[1])	// End of path?
		{
		pstatic[0] = 0;
		pstatic = strrchr(addr.sun_path, '/');
		}
	if (!pstatic)
		{
		cout << "Can't find gamedat for socket" << endl;
		return false;
		}
	strcpy(pstatic + 1, "gamedat/exultserver");
	server_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (server_socket < 0)
		{
		perror("Failed to open map-editor socket");
		return false;
		}
					// Set to be non-blocking.
//	fcntl(server_socket, F_SETFL, 
//				fcntl(server_socket, F_GETFL) | O_NONBLOCK);
	cout << "Trying to connect to server at '" << addr.sun_path << "'"
							<< endl;
	if (connect(server_socket, (struct sockaddr *) &addr, 
		      sizeof(addr.sun_family) + strlen(addr.sun_path)) == -1)
		{
		perror("Socket connect");
		close(server_socket);
		server_socket = -1;
		return false;
		}
	else
		{
		server_input_tag = gdk_input_add(server_socket,
			GDK_INPUT_READ, Read_from_server, this);
		cout << "Connected to server" << endl;
		return true;
		}
#else
		// Close existing socket.
	if (server_socket != -1) Exult_server::disconnect_from_server();
	if (server_input_tag != -1) gtk_timeout_remove(server_input_tag);
	server_socket = server_input_tag = -1;

	if (Exult_server::try_connect_to_server(static_path) > 0) {
		server_input_tag = gtk_timeout_add(50, Read_from_server, this);
		cout << "Connected to server" << endl;
		return true;
	}
#endif
	return false;
	}



