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

#include <stdio.h>			/* These are for sockets. */
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include "shapelst.h"
#include "paledit.h"
#include "vgafile.h"
#include "ibuf8.h"
#include "Flex.h"
#include "u7drag.h"
#include "studio.h"
#include "dirbrowser.h"
#include "servemsg.h"
#include "objserial.h"
#include "exult_constants.h"

ExultStudio *ExultStudio::self = 0;

enum ExultFileTypes {
	ShapeArchive =1,
	PaletteFile,
	FlexArchive
};

extern "C" void on_filelist_tree_select_row       (GtkCTree        *ctree,
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

extern "C" void
on_open_static_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->choose_static_path();
}

void on_choose_directory               (gchar *dir)
{
	ExultStudio::get_instance()->set_static_path(dir);
}

/*
 *	Main window's close button.
 */
extern "C" gboolean on_main_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	return FALSE;
	}
extern "C" void on_main_window_destroy_event
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	gtk_main_quit();
	}

/*
 *	Open egg window.
 */

extern "C" void on_open_egg_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->open_egg_window();
	}

/*
 *	Egg window's Apply button.
 */
extern "C" void on_egg_apply_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->save_egg_window();
	}

/*
 *	Egg window's Cancel button.
 */
extern "C" void on_egg_cancel_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_egg_window();
	}

/*
 *	Egg window's close button.
 */
extern "C" gboolean on_egg_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_egg_window();
	return TRUE;
	}

ExultStudio::ExultStudio(int argc, char **argv): ifile(0), names(0),
	eggwin(0), server_socket(-1), server_input_tag(-1), 
	static_path(0), browser(0)
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
	strcpy(path, xmldir ? xmldir : ".");
	strcat(path, "/exult_studio.glade");
	// Load the Glade interface
	app_xml = glade_xml_new(path, NULL);
	app = glade_xml_get_widget( app_xml, "main_window" );

	// Connect signals
#if 0
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
#endif
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
}

ExultStudio::~ExultStudio()
{
	delete_shape_browser();
	if (eggwin)
		gtk_widget_destroy(eggwin);
	eggwin = 0;
//Shouldn't be done here	gtk_widget_destroy( app );
	gtk_object_unref( GTK_OBJECT( app_xml ) );
	if (server_input_tag >= 0)
		gdk_input_remove(server_input_tag);
	if (server_socket >= 0)
		close(server_socket);
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
	char *palname = g_strdup_printf("%s%s", static_path, "palettes.flx");
	U7object pal(palname, 0);
	g_free(palname);
	size_t len;
	unsigned char *palbuf;		// this may throw an exception
	palbuf = (unsigned char *) pal.retrieve(len);
	Shape_chooser *chooser = new Shape_chooser(ifile, palbuf, 400, 64);
	delete [] palbuf;
	if(!strcasecmp(fname,"shapes.vga")) {
		// Read in shape names.
		int num_names = ifile->get_num_shapes();
		names = new char *[num_names];
		char *txtname = g_strdup_printf("%s%s", static_path, 
							"text.flx");

		Flex *items = new Flex(txtname);
		g_free(txtname);
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
	int extlen = strlen(ext);
	while(entry=readdir(dir)) {
		char *fname = entry->d_name;
		int flen = strlen(fname);
					// Ignore case of extension.
		if(!strcmp(fname,".")||!strcmp(fname,"..") ||
				strcasecmp(fname + flen - extlen, ext) != 0)
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
	connect_to_server();		// Connect to server with 'gamedat'.
}

/*
 *	Open the egg-editing window.
 */

void ExultStudio::open_egg_window
	(
	unsigned char *data,		// Serialized egg, or null.
	int datalen
	)
	{
	if (!eggwin)			// First time?
		eggwin = glade_xml_get_widget( app_xml, "egg_window" );
	if (data)
		if (!init_egg_window(data, datalen))
			return;
	gtk_widget_show(eggwin);
	}

/*
 *	Close the egg-editing window.
 */

void ExultStudio::close_egg_window
	(
	)
	{
	if (eggwin)
		gtk_widget_hide(eggwin);
	}

/*
 *	Get value of a toggle button (false if not found).
 */

static bool Get_toggle
	(
	GladeXML *app_xml,
	char *name
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	return btn ? gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)) : -1;
	}

/*
 *	Find and set a toggle/checkbox button.
 */

static void Set_toggle
	(
	GladeXML *app_xml,
	char *name,
	bool val
	)
	{
	GtkWidget *btn = glade_xml_get_widget(app_xml, name);
	if (btn)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn), val);
	}

/*
 *	Get value of option-menu button (-1 if unsuccessful).
 */

static int Get_optmenu
	(
	GladeXML *app_xml,
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

static void Set_optmenu
	(
	GladeXML *app_xml,
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

static int Get_spin
	(
	GladeXML *app_xml,
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

static void Set_spin
	(
	GladeXML *app_xml,
	char *name,
	int val,
	bool sensitive = true
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
 *	Get number from a text field.
 *
 *	Output:	Number, or -1 if not found.
 */

static int Get_num_entry
	(
	GladeXML *app_xml,
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
		return strtol(txt + 2, 0, 16);	// Hex.
	else
		return atoi(txt);
	}

/*
 *	Find and set a text field to a number.
 */

static void Set_entry
	(
	GladeXML *app_xml,
	char *name,
	int val,
	bool hex = false,
	bool sensitive = true
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
 *	Init. the egg editor with data from Exult.
 *
 *	Output:	0 if error (reported).
 */

int ExultStudio::init_egg_window
	(
	unsigned char *data,
	int datalen
	)
	{
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame;
	int type;
	int criteria;
	int probability;
	int distance;
	bool nocturnal, once, hatched, auto_reset;
	int data1, data2;
	if (!Egg_object_in(data, datalen, addr, tx, ty, tz, shape, frame,
		type, criteria, probability, distance, 
		nocturnal, once, hatched, auto_reset,
		data1, data2))
		{
		cout << "Error decoding egg" << endl;
		return 0;
		}
					// Store address with window.
	gtk_object_set_user_data(GTK_OBJECT(eggwin), (gpointer) addr);
	GtkWidget *notebook = glade_xml_get_widget(app_xml, "notebook1");
	if (notebook)			// 1st is monster (1).
		gtk_notebook_set_page(GTK_NOTEBOOK(notebook), type - 1);
	Set_spin(app_xml, "probability", probability);
	Set_spin(app_xml, "distance", distance);
	Set_optmenu(app_xml, "criteria", criteria);
	Set_toggle(app_xml, "nocturnal", nocturnal);
	Set_toggle(app_xml, "once", once);
	Set_toggle(app_xml, "hatched", hatched);
	Set_toggle(app_xml, "autoreset", auto_reset);
	switch (type)			// Set notebook page.
		{
	case 1:				// Monster:
		Set_entry(app_xml, "monst_shape", data2&1023);
		Set_entry(app_xml, "monst_frame", data2>>10);
		Set_optmenu(app_xml, "monst_schedule", data1>>8);
		Set_optmenu(app_xml, "monst_align", data1&3);
		Set_spin(app_xml, "monst_count", (data1&0xff)>>2);
		break;
	case 2:				// Jukebox:
		Set_spin(app_xml, "juke_song", data1&0xff);
		Set_toggle(app_xml, "juke_cont", (data1>>8)&0x01);
		break;
	case 3:				// Sound effect:
		break;			// +++++++Later!
	case 4:				// Voice:
		Set_spin(app_xml, "speech_number", data1&0xff);
		break;
	case 5:				// Usecode:
		Set_entry(app_xml, "usecode_number", data2, true);
		Set_spin(app_xml, "usecode_quality", data1&0xff);
		break;
	case 6:				// Missile:
		Set_entry(app_xml, "missile_shape", data1); 
		Set_optmenu(app_xml, "missile_dir", data2&0xff);
		Set_spin(app_xml, "missile_delay", data2>>8);
		break;
	case 7:				// Teleport:
		{
		int qual = data1&0xff;
		if (qual == 255)
			{
			Set_toggle(app_xml, "teleport_coord", true);
			int schunk = data1 >> 8;
			Set_entry(app_xml, "teleport_x",
				(schunk%12)*c_tiles_per_schunk +(data2&0xff),
								true);
			Set_entry(app_xml, "teleport_y",
				(schunk/12)*c_tiles_per_schunk +(data2>>8),
								true);
			Set_spin(app_xml, "teleport_eggnum", 0, false);
			}
		else			// Egg #.
			{
			Set_toggle(app_xml, "teleport_coord", false);
			Set_entry(app_xml, "teleport_x", 0, false, false);
			Set_entry(app_xml, "teleport_y", 0, false, false);
			Set_spin(app_xml, "teleport_eggnum", qual);
			}
		break;
		}
	case 8:				// Weather:
		Set_optmenu(app_xml, "weather_type", data1&0xff); 
		Set_spin(app_xml, "weather_length", data1>>8);
		break;
	case 9:				// Path:
		Set_spin(app_xml, "pathegg_num", data1&0xff);
		break;
	case 10:			// Button:
		Set_spin(app_xml, "btnegg_distance", data1&0xff);
		break;
	default:
		break;
		}
	return 1;
	}

/*
 *	Send updated egg info. back to Exult.
 *
 *	Output:	0 if error (reported).
 */

int ExultStudio::save_egg_window
	(
	)
	{
	cout << "In save_egg_window()" << endl;
	unsigned char data[Exult_server::maxlength];
	unsigned long addr = (unsigned long) gtk_object_get_user_data(
							GTK_OBJECT(eggwin));
	int tx = -1, ty = -1, tz = -1;	// +++++For now.
	int shape = -1, frame = -1;	// For now.
	int type = -1;
	GtkWidget *notebook = glade_xml_get_widget(app_xml, "notebook1");
	if (notebook)			// 1st is monster (1).
		type = 1 + gtk_notebook_get_current_page(
						GTK_NOTEBOOK(notebook));
	else
		{
		cout << "Can't find notebook widget" << endl;
		return 0;
		}
	int criteria = Get_optmenu(app_xml, "criteria");
	int probability = Get_spin(app_xml, "probability");
	int distance = Get_spin(app_xml, "distance");
	bool nocturnal = Get_toggle(app_xml, "nocturnal"),
		once = Get_toggle(app_xml, "once"),
		hatched = Get_toggle(app_xml, "hatched"), 
		auto_reset = Get_toggle(app_xml, "autoreset");
	int data1 = -1, data2 = -1;
	switch (type)			// Set notebook page.
		{
	case 1:				// Monster:
		data2 = (Get_num_entry(app_xml, "monst_shape")&1023) +
			(Get_num_entry(app_xml, "monst_frame")<<10);
		data1 = (Get_optmenu(app_xml, "monst_schedule")<<8) +
			(Get_optmenu(app_xml, "monst_align")&3) +
			(Get_spin(app_xml, "monst_count")<<2);
		break;
	case 2:				// Jukebox:
		data1 = (Get_spin(app_xml, "juke_song")&0xff) +
			(Get_toggle(app_xml, "juke_cont")<<8);
		break;
	case 3:				// Sound effect:
		break;			// +++++++Later!
	case 4:				// Voice:
		data1 = Get_spin(app_xml, "speech_number")&0xff;
		break;
	case 5:				// Usecode:
		data2 = Get_num_entry(app_xml, "usecode_number");
		data1 = Get_spin(app_xml, "usecode_quality")&0xff;
		break;
	case 6:				// Missile:
		data1 = Get_num_entry(app_xml, "missile_shape");
		data2 = (Get_optmenu(app_xml, "missile_dir")&0xff) +
			(Get_spin(app_xml, "missile_delay")<<8);
		break;
	case 7:				// Teleport:
		if (Get_toggle(app_xml, "teleport_coord"))
			{		// Abs. coords.
			int tx = Get_num_entry(app_xml, "teleport_x"),
			    ty = Get_num_entry(app_xml, "teleport_y");
			data2 = (tx&0xff) + ((ty&0xff)<<8);
			int sx = tx/c_tiles_per_schunk,
			    sy = ty/c_tiles_per_schunk;
			data1 = 255 + (sy*12 + sx);
			}
		else			// Egg #.
			data1 = Get_spin(app_xml, "teleport_eggnum")&0xff;
		break;
	case 8:				// Weather:
		data1 = (Get_optmenu(app_xml, "weather_type")&0xff) +
			(Get_spin(app_xml, "weather_length")<<8);
		break;
	case 9:				// Path:
		data1 = Get_spin(app_xml, "pathegg_num")&0xff;
		break;
	case 10:			// Button:
		data1 = Get_spin(app_xml, "btnegg_distance")&0xff;
		break;
	default:
		cout << "Unknown egg type" << endl;
		return 0;
		}
	if (!Egg_object_out(server_socket, addr, tx, ty, tz,
			shape, frame, type, criteria, probability, distance,
			nocturnal, once, hatched, auto_reset, data1, data2))
		{
		cout << "Error sending egg data to server" <<endl;
		return 0;
		}
	cout << "Sent egg data to server" << endl;
	close_egg_window();
	return 1;
	}

void ExultStudio::run()
{
	gtk_main();
}

/*
 *	Input from server is available.
 */

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

void ExultStudio::read_from_server
	(
	)
	{
	unsigned char data[Exult_server::maxlength];
	Exult_server::Msg_type id;
	int datalen = Exult_server::Receive_data(server_socket, id, data,
							sizeof(data));
	if (!datalen)
		{
		cout << "Error reading from server" << endl;
		if (server_socket == -1)// Socket closed?
			{
			gdk_input_remove(server_input_tag);
			server_input_tag = -1;
			}
		return;
		}
	cout << "Read " << datalen << " bytes from server" << endl;
	cout << "ID = " << (int) id << endl;
	if (id == Exult_server::egg)
		open_egg_window(data, datalen);
	}

/*
 *	Try to connect to the Exult game.
 */
void ExultStudio::connect_to_server
	(
	)
	{
	if (!static_path)
		return;			// No place to go.
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
		return;
		}
	strcpy(pstatic + 1, "gamedat/exultserver");
	server_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (server_socket < 0)
		{
		perror("Failed to open map-editor socket");
		return;
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
		}
	else
		{
		server_input_tag = gdk_input_add(server_socket,
			GDK_INPUT_READ, Read_from_server, this);
		cout << "Connected to server" << endl;
		}
	}



