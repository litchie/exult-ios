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
#include <windows.h>
#include <ole2.h>
#include "servewin32.h"
#else
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <fcntl.h>
#include <cstdarg>

#include "shapelst.h"
#include "shapevga.h"
#include "Flex.h"
#include "studio.h"
#include "utils.h"
#include "u7drag.h"
#include "shapegroup.h"
#include "shapefile.h"
#include "locator.h"
#include "combo.h"
#include "Configuration.h"
#include "objserial.h"
#include "exceptions.h"
#include "logo.xpm"
#include "fnames.h"
#include "execbox.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ofstream;

ExultStudio *ExultStudio::self = 0;
Configuration *config = 0;
const std::string c_empty_string;	// Used by config. library.

					// Mode menu items:
static char *mode_names[4] = {"move1", "paint1", "paint_with_chunks1", 
							"pick_for_combo1"};

enum ExultFileTypes {
	ShapeArchive =1,
	ChunkArchive,
	PaletteFile,
	FlexArchive,
	ComboArchive
};

typedef struct _FileTreeItem FileTreeItem;
struct _FileTreeItem
{
  const gchar    *label;
  ExultFileTypes datatype;
  FileTreeItem   *children;
};

/* columns */
enum
{
  FOLDER_COLUMN = 0,
  FILE_COLUMN,
  DATA_COLUMN,
  NUM_COLUMNS
};


static void Filelist_selection(GtkTreeView *treeview, GtkTreePath *path)
{
	int type = -1;
	char *text;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, FILE_COLUMN, &text, DATA_COLUMN, 
								&type,-1);
	printf("%s %d\n",text,type);
	
	ExultStudio *studio = ExultStudio::get_instance();
	switch(type) {
	case ShapeArchive:
		studio->set_browser("Shape Browser", 
					studio->create_browser(text));
		break;
	case ComboArchive:
		studio->set_browser("Combo Browser", 
					studio->create_browser(text));
		break;
	case ChunkArchive:
		studio->set_browser("Chunk Browser", 
					studio->create_browser(text));
		break;
	case PaletteFile:
		studio->set_browser("Palette Browser", 
					studio->create_browser(text));
		break;
	default:
		break;
	}
	g_free(text);
}                                     

#if 0
C_EXPORT void on_filelist_tree_select_row(GtkTreeView *treeview, 
		GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	Filelist_selection(treeview, path);
}
#endif

C_EXPORT void on_filelist_tree_cursor_changed(GtkTreeView *treeview)
{
	GtkTreePath *path;
	GtkTreeViewColumn *col;

	gtk_tree_view_get_cursor(treeview, &path, &col);
	Filelist_selection(treeview, path);
}

C_EXPORT void
on_open_game_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->choose_game_path();
}

C_EXPORT void
on_new_game_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->new_game();
}

C_EXPORT void
on_connect_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->connect_to_server();
}

C_EXPORT void
on_save_all1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->save_all();
}

C_EXPORT void
on_new_shapes_file_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->new_shape_file(false);
}

C_EXPORT void
on_new_shape_file_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->new_shape_file(true);
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
on_compile_usecode_menu_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->compile();
}

C_EXPORT void
on_save_groups1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->save_groups();
}

C_EXPORT void
on_save_combos1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->save_combos();
}

C_EXPORT void
on_preferences_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->open_preferences();
}

C_EXPORT void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	unsigned char z = 0;
	ExultStudio::get_instance()->send_to_server(Exult_server::cut,
							&z, 1);
}

C_EXPORT void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	unsigned char o = 1;
	ExultStudio::get_instance()->send_to_server(Exult_server::cut,
							&o, 1);
}

C_EXPORT void
on_paste1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	ExultStudio::get_instance()->send_to_server(Exult_server::paste);
}

C_EXPORT void
on_properties1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	unsigned char o = 0;		// 0=npc/egg properties.
	ExultStudio::get_instance()->send_to_server(
				Exult_server::edit_selected, &o, 1);
}

C_EXPORT void
on_basic_properties1_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	unsigned char o = 1;		// 1=basic object properties.
	ExultStudio::get_instance()->send_to_server(
				Exult_server::edit_selected, &o, 1);
}

C_EXPORT void
on_move1_activate	               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
					// NOTE:  modes are defined in cheat.h.
	if (GTK_CHECK_MENU_ITEM(menuitem)->active)
		ExultStudio::get_instance()->set_edit_mode(0);
}

C_EXPORT void
on_paint1_activate	               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (GTK_CHECK_MENU_ITEM(menuitem)->active)
		ExultStudio::get_instance()->set_edit_mode(1);
}

C_EXPORT void
on_paint_with_chunks1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (GTK_CHECK_MENU_ITEM(menuitem)->active)
		ExultStudio::get_instance()->set_edit_mode(2);
}

C_EXPORT void
on_pick_for_combo1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (GTK_CHECK_MENU_ITEM(menuitem)->active)
		ExultStudio::get_instance()->set_edit_mode(3);
}

C_EXPORT void
on_unused_shapes1_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (EStudio::Prompt(
		"Finding unused shapes may take several minutes\nProceed?",
					"Yes", "No") != 0)
		return;
	ExultStudio::get_instance()->send_to_server(
						Exult_server::unused_shapes);
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
on_hide_lift_spin_changed		(GtkSpinButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->set_hide_lift(
				gtk_spin_button_get_value_as_int(button));
}

C_EXPORT void
on_edit_terrain_button_toggled		(GtkToggleButton *button,
					 gpointer	  user_data)
{
	ExultStudio::get_instance()->set_edit_terrain(
				gtk_toggle_button_get_active(button));
}

void on_choose_directory               (gchar *dir,
					gpointer	user_data)
{
	ExultStudio::get_instance()->set_game_path(dir);
}

/*
 *	Configure main window.
 */
C_EXPORT gint on_main_window_configure_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventConfigure *event,
	gpointer data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
					// Configure "Hide lift" spin range.
	studio->set_spin("hide_lift_spin", 
				studio->get_spin("hide_lift_spin"), 1, 16);
	return FALSE;
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
	if (!ExultStudio::get_instance()->okay_to_close())
		return TRUE;		// Can't quit.
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
/*
 *	"Exit" in main window.
 */
C_EXPORT void
on_main_window_quit                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (ExultStudio::get_instance()->okay_to_close())
		gtk_main_quit();
}
/*
 *	Main window got focus.
 */
C_EXPORT gboolean on_main_window_focus_in_event
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	Shape_chooser::check_editing_files();
	return TRUE;
	}


/*
 *	Set up everything.
 */

ExultStudio::ExultStudio(int argc, char **argv): files(0), curfile(0), 
	names(0), glade_path(0), shape_info_modified(false),
	shape_names_modified(false),
	vgafile(0), facefile(0), eggwin(0), 
	server_socket(-1), server_input_tag(-1), 
	static_path(0), image_editor(0), default_game(0), background_color(0),
	browser(0), palbuf(0), egg_monster_draw(0), 
	egg_ctx(0),
	waiting_for_server(0), npcwin(0), npc_draw(0), npc_face_draw(0),
	npc_ctx(0), objwin(0), obj_draw(0), shapewin(0), shape_draw(0),
	equipwin(0), locwin(0), combowin(0), compilewin(0), compile_box(0)
{
	// Initialize the various subsystems
	self = this;
	gtk_init( &argc, &argv );
	gdk_rgb_init();
	glade_init();
	config = new Configuration;
	config->read_config_file(USER_CONFIGURATION_FILE);
					// Get options.
	const char *xmldir = 0;		// Default:  Look here for .glade.
	const char *game = 0;		// Game to look up in .exult.cfg.
	static char *optstring = "g:x:d:";
	extern int optind, opterr, optopt;
	extern char *optarg;
	opterr = 0;			// Don't let getopt() print errs.
	int optchr;
	while ((optchr = getopt(argc, argv, optstring)) != -1)
		switch (optchr)
			{
		case 'g':		// Game.  Replaces use of -d, -x.
			game = optarg;
			break;
		case 'x':		// XML (.glade) directory.
			xmldir = optarg;
			break;
			}
	string dirstr, datastr;
	if (!xmldir)
		{
		config->value("config/disk/data_path", datastr, EXULT_DATADIR);
		xmldir = datastr.c_str();
		}
	string defgame;			// Default game name.
	config->value("config/estudio/default_game", defgame, "blackgate");
	default_game = g_strdup(defgame.c_str());
	if (!game)
		game = default_game;
	config->set("config/estudio/default_game", defgame, true);
	char path[256];			// Set up paths.
	if(xmldir)
		strcpy(path, xmldir);
	else if(U7exists(EXULT_DATADIR"/exult_studio.glade"))
		strcpy(path,EXULT_DATADIR);
	else
		strcpy(path,".");
	strcat(path, "/exult_studio.glade");
	// Load the Glade interface
	app_xml = glade_xml_new(path, NULL, NULL);
	app = glade_xml_get_widget( app_xml, "main_window" );
	glade_path = g_strdup(path);

	// More setting up...
					// Connect signals automagically.
	glade_xml_signal_autoconnect(app_xml);
	int w, h;			// Get main window dims.
	config->value("config/estudio/main/width", w, 0);
	config->value("config/estudio/main/height", h, 0);
	if (w > 0 && h > 0)
//++++Used to work	gtk_window_set_default_size(GTK_WINDOW(app), w, h);
		gtk_window_resize(GTK_WINDOW(app), w, h);
	gtk_widget_show( app );
					// Background color for shape browser.
	int bcolor;
	config->value("config/estudio/background_color", bcolor, 0);
	background_color = bcolor;
	if (game)			// Game given?
		{
		string d("config/disk/game/");
		d += game; 
		string gd = d + "/path";
		config->value(gd.c_str(), dirstr, "");
		if (dirstr == "")
			{
			cerr << "Game '" << game << 
				"' path not found in config. file" << endl;
			exit(1);
			}
		string pd = d + "/patch";	// Look up patch dir. too.
		string ptchstr;
		config->value(pd.c_str(), ptchstr, "");
		set_game_path(dirstr.c_str(), ptchstr == "" ? 0 
						: ptchstr.c_str());
		}
	string iedit;			// Get image-editor command.
	config->value("config/estudio/image_editor", iedit, "gimp-remote -n");
	image_editor = g_strdup(iedit.c_str());
	config->set("config/estudio/image_editor", iedit, true);
#ifdef WIN32
    OleInitialize(NULL);
#endif
					// Init. 'Mode' menu, since Glade
					//   doesn't seem to do it right.
	GSList *group = NULL;
	for (int i = 0; i < sizeof(mode_names)/sizeof(mode_names[0]); i++)
		{
		GtkWidget *item = glade_xml_get_widget(app_xml, mode_names[i]);
		gtk_radio_menu_item_set_group(GTK_RADIO_MENU_ITEM(item),
								group);
		group = gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(item));
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(item), i == 0);
		}
	setup_maps_list();		// Init. 'maps' menu.
}

ExultStudio::~ExultStudio()
{
#ifdef WIN32
    OleUninitialize();
#endif
					// Store main window size.
	int w = app->allocation.width, h = app->allocation.height;
					// Finish up external edits.
	Shape_chooser::clear_editing_files();
	config->set("config/estudio/main/width", w, true);
	config->set("config/estudio/main/height", h, true);
	int num_names = names.size();
	for (int i = 0; i < num_names; i++)
		delete names[i];
	names.resize(0);
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
	if (compilewin)
		gtk_widget_destroy(compilewin);
	compilewin = 0;
	delete compile_box;
	compile_box = 0;
	if (locwin)
		delete locwin;
	if (combowin)
		delete combowin;
	locwin = 0;
	g_object_unref( G_OBJECT( app_xml ) );
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
	g_free(image_editor);
	g_free(default_game);
	self = 0;
	delete config;
	config = 0;
}

/*
 *	Okay to close game?
 */

bool ExultStudio::okay_to_close
	(
	)
	{
	if (need_to_save())
		{
		int choice = prompt("File(s) modified.  Save?",
						"Yes", "No", "Cancel");
		if (choice == 2)	// Cancel?
			return false;
		if (choice == 0)
			save_all();	// Write out changes.
		}
	return true;
	}

/*
 *	See if given window has focus (experimental).
 */

inline bool Window_has_focus(GtkWindow *win)
	{
	return (win->focus_widget != 0 &&
		GTK_WIDGET_HAS_FOCUS(win->focus_widget));
	}

/*
 *	Do we have focus?  Currently only checks main window and group
 *	windows.
 */

bool ExultStudio::has_focus
	(
	)
	{
	if (Window_has_focus(GTK_WINDOW(app)))
		return true;		// Main window.
					// Group windows:
	vector<GtkWindow*>::const_iterator it;
	for (it = group_windows.begin(); it != group_windows.end(); ++it)
		if (Window_has_focus(*it))
			return true;
	return false;
	}

/*
 *	Set browser area.  NOTE:  obj may be null.
 */
void ExultStudio::set_browser(const char *name, Object_browser *obj)
{
	GtkWidget *browser_frame = glade_xml_get_widget(app_xml, 
							"browser_frame" );
	GtkWidget *browser_box = glade_xml_get_widget(app_xml, "browser_box" );
//+++Now owned by Shape_file_info.	delete browser;
	if (browser)
		gtk_container_remove(GTK_CONTAINER(browser_box),
							browser->get_widget());
	browser = obj;
	
	gtk_frame_set_label( GTK_FRAME( browser_frame ), name );
	gtk_widget_show( browser_box );
	if (browser)
		gtk_box_pack_start(GTK_BOX(browser_box), 
					browser->get_widget(), TRUE, TRUE, 0);
}

Object_browser *ExultStudio::create_browser(const char *fname)
{
	curfile = open_shape_file(fname);
	if (!curfile)
		return 0;
	Object_browser *chooser = curfile->get_browser(vgafile, palbuf);
	setup_groups();			// Set up 'groups' page.
	return chooser;
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
 *	Test for a directory 'slash' or colon.
 */

inline bool Is_dir_marker
	(
	char c
	)
	{
	return (c == '/' || c == '\\' || c == ':');
	}

/*
 *	New game directory was chosen.
 */

void on_choose_new_game_dir
	(
	const char *dir,
	gpointer udata			// ->studio.
	)
	{
	((ExultStudio *) udata)->create_new_game(dir);
	}
void ExultStudio::create_new_game
	(
	const char *dir			// Directory for new game.
	)
	{
					// Take basename as game name.
	const char *eptr = dir + strlen(dir) - 1;
	while (eptr >= dir && Is_dir_marker(*eptr))
		eptr--;
	eptr++;
	const char *ptr = eptr - 1;
	for ( ; ptr >= dir; ptr--)
		if (Is_dir_marker(*ptr))
			{
			ptr++;
			break;
			}
	if (ptr < dir || eptr - ptr <= 0)
		{
		EStudio::Alert("Can't find base game name in '%s'", dir);
		return;
		}
	string gamestr(ptr, eptr - ptr);
	string dirstr(dir);
	string static_path = dirstr + "/static";
	if (U7exists(static_path))
		{
		string msg("Directory '");
		msg += static_path;
		msg += "' already exists.\n";
		msg += "Files within may be overwritten.\n";
		msg += "Proceed?";
		if (prompt(msg.c_str(), "Yes", "No") != 0)
			return;
		}
	U7mkdir(dir, 0755);		// Create "game", "game/static",
					//   "game/patch".
	U7mkdir(static_path.c_str(), 0755);
	string patch_path = dirstr + "/patch";
	U7mkdir(patch_path.c_str(), 0755);
					// Set .exult.cfg.
	string d("config/disk/game/");
	string gameconfig = d + gamestr;
	d = gameconfig + "/path";
	config->set(d.c_str(), dirstr, false);
	d = gameconfig + "/patch";
	config->set(d.c_str(), patch_path, false);
	d = gameconfig + "/editing";	// We are editing.
	config->set(d.c_str(), "yes", true);
	string esdir;			// Get dir. for new files.
	config->value("config/disk/data_path", esdir, EXULT_DATADIR);
	esdir += "/estudio/new";
	DIR *dirrd = opendir(esdir.c_str());
	if (!dirrd)
		EStudio::Alert("'%s' for initial data files not found",
								esdir.c_str());
	else
		{
		struct dirent *entry;
		while(entry=readdir(dirrd)) 
			{
			char *fname = entry->d_name;
			int flen = strlen(fname);
					// Ignore case of extension.
			if(!strcmp(fname, ".") || !strcmp(fname,".."))
				continue;
			string src = esdir + '/' + fname;
			string dest = static_path + '/' + fname;
			try {
				U7copy(src.c_str(), dest.c_str());
			} catch (exult_exception& e) {
				EStudio::Alert(e.what());
			}
			}
		closedir(dirrd);
		}
	set_game_path(dir);		// Open as current game.
	write_shape_info(true);		// Create initial .dat files.
	}

/*
 *	Prompt for a 'new game' directory.
 */

void ExultStudio::new_game()
{
	if (!okay_to_close())		// Okay to close prev. game?
		return;
	GtkFileSelection *fsel = Create_file_selection(
			"Choose new game directory",
			on_choose_new_game_dir, this);
	gtk_widget_show(GTK_WIDGET(fsel));
}

/*
 *	Choose game directory.
 */
void ExultStudio::choose_game_path()
{
	if (!okay_to_close())		// Okay to close prev. game?
		return;
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
	GtkFileSelection *fsel = Create_file_selection(
					"Select game directory",
			(File_sel_okay_fun) on_choose_directory, 0L);
	gtk_file_selection_set_filename(fsel, cwd);
	gtk_widget_show(GTK_WIDGET(fsel));
	delete [] cwd;	// Prevent leakage
}



/*
 *	Note:	Patchpath may be NULL,in which case gamepath/patch is used.
 */
void ExultStudio::set_game_path(const char *gamepath, const char *patchpath)
{
					// Finish up external edits.
	Shape_chooser::clear_editing_files();
					// Set top-level path.
	add_system_path("<GAME>", gamepath);
	if (static_path)
		g_free(static_path);
					// Set up path to static.
	static_path = g_strdup_printf("%s/static", gamepath);
	add_system_path("<STATIC>", static_path);
	char *patch_path = patchpath ? g_strdup(patchpath) :
					g_strdup_printf("%s/patch", gamepath);
	add_system_path("<PATCH>", patch_path);
	if (!U7exists(patch_path))	// Create patch if not there.
		U7mkdir(patch_path, 0755);
	g_free(patch_path);
					// Clear file cache!
	U7FileManager::get_ptr()->reset();
	delete palbuf;			// Delete old.
	string palname("<PATCH>/");	// 1st look in patch for palettes.
	palname += "palettes.flx";
	size_t len;
	if (!U7exists(palname.c_str()))
		(palname = "<STATIC>/") += "palettes.flx";
	if (!U7exists(palname.c_str()))
		{			// No palette file, so create fake.
		palbuf = new unsigned char[3*256];	// How about all white?
		memset(palbuf, (63<<16)|(63<<8)|63, 3*256);
		}
	else
		{
		U7object pal(palname.c_str(), 0);
					// this may throw an exception
		palbuf = (unsigned char *) pal.retrieve(len);
		}
					// Set background color.
	palbuf[3*255] = (background_color>>18)&0x3f;
	palbuf[3*255 + 1] = (background_color>>10)&0x3f;
	palbuf[3*255 + 2] = (background_color>>2)&0x3f;
					// Delete old names.
	int num_names = names.size();
	for (int i = 0; i < num_names; i++)
		delete names[i];
	names.resize(0);
	delete files;			// Close old shape files.
	browser = 0;			// This was owned by 'files'.
	files = new Shape_file_set();
	vgafile = open_shape_file("shapes.vga");
	facefile = open_shape_file("faces.vga");
					// Read in shape names.
	string txtname("<PATCH>/text.flx");
	if (!U7exists(txtname.c_str()))
		txtname = "<STATIC>/text.flx";
	if (U7exists(txtname.c_str()))
		{
		Flex *items = new Flex(txtname.c_str());
		int num_names = items->number_of_objects();
		names.resize(num_names);
		int i;
		for (i = 0; i < num_names; i++)
			names[i] = items->retrieve(i, len);
		delete items;
		}
	setup_file_list();		// Set up file-list window.
	set_browser("", 0);		// No browser.
	connect_to_server();		// Connect to server with 'gamedat'.
}

void add_to_tree(GtkTreeStore *model, const char *folderName, const char *files, ExultFileTypes file_type)
{
	struct dirent *entry;
	GtkTreeIter iter;
	
	// First, we add the folder
	gtk_tree_store_append(model, &iter, NULL);
	gtk_tree_store_set(model, &iter,
		FOLDER_COLUMN, folderName,
		FILE_COLUMN, NULL,
		DATA_COLUMN, -1,
		-1);
	
	// Scan the files which are separated by commas
	GtkTreeIter child_iter;
	char *startpos = (char *)files;
	int adding_children = 1;
	do {
		char *pattern;
		char *commapos = strstr(startpos, ",");
		if(commapos==0) {
			pattern = strdup(startpos);
			adding_children = 0;
		} else {
			pattern = g_strndup(startpos, commapos-startpos);
			startpos = commapos+1;
		}
		
		string spath("<STATIC>"), ppath("<PATCH>");
	        spath = get_system_path(spath);
        	ppath = get_system_path(ppath);
        	char *ext = strstr(pattern,"*");
		if(!ext)
			ext = pattern;
		else
			ext++;
		DIR *dir = opendir(ppath.c_str());// Get names from 'patch' first.
		if (dir) {
			while(entry=readdir(dir)) {
				char *fname = entry->d_name;
                        	int flen = strlen(fname);
                                        // Ignore case of extension.
                        	if(!strcmp(fname,".")||!strcmp(fname,"..") ||strcasecmp(fname + flen - strlen(ext), ext) != 0)
                                	continue;
				gtk_tree_store_append (model, &child_iter, &iter);
				gtk_tree_store_set (model, &child_iter,
			      		FOLDER_COLUMN, NULL,
			      		FILE_COLUMN, fname,
					DATA_COLUMN, file_type,
			      		-1);
                        	
                	}
                	closedir(dir);
        	}
		dir = opendir(spath.c_str());   // Now go through 'static'.
        	if (dir) {
                	while(entry=readdir(dir)) {
                        	char *fname = entry->d_name;
                        	int flen = strlen(fname);
                                        // Ignore case of extension.
                        	if(!strcmp(fname,".")||!strcmp(fname,"..")||strcasecmp(fname + flen - strlen(ext), ext) != 0)
                                	continue;
                                        // See if also in 'patch'.
                        	string fullpath(ppath);
                        	fullpath += "/";
                        	fullpath += fname;
                        	if (U7exists(fullpath))
                                	continue;
				gtk_tree_store_append (model, &child_iter, &iter);
				gtk_tree_store_set (model, &child_iter,
			      		FOLDER_COLUMN, NULL,
			      		FILE_COLUMN, fname,
					DATA_COLUMN, file_type,
			      		-1);
                	}
                	closedir(dir);
        	}

		free(pattern);
	} while (adding_children);
}

/*
 *	Set up the file list to the left of the main window.
 */

void ExultStudio::setup_file_list() {
	GtkWidget *file_list = glade_xml_get_widget( app_xml, "file_list" );
	
  	/* create tree store */
	GtkTreeModel *oldmod = gtk_tree_view_get_model(
						GTK_TREE_VIEW(file_list));
  	GtkTreeStore *model;
	if (oldmod)
		model = GTK_TREE_STORE(oldmod);
	else				// Create the first time.
		{
		model = gtk_tree_store_new(
			NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
		gtk_tree_view_set_model(GTK_TREE_VIEW(file_list), 
							GTK_TREE_MODEL(model));
		g_object_unref(model);
		gint col_offset;
  		GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
 	 	GtkTreeViewColumn *column;
  	
		/* column for folder names */
		g_object_set (renderer, "xalign", 0.0, NULL);
		col_offset = gtk_tree_view_insert_column_with_attributes(
					GTK_TREE_VIEW(file_list),
					-1, "Folders",
					renderer, "text",
					FOLDER_COLUMN, NULL);
		column = gtk_tree_view_get_column(GTK_TREE_VIEW(file_list), 
							col_offset - 1);
		gtk_tree_view_column_set_clickable(
					GTK_TREE_VIEW_COLUMN(column), TRUE);
		/* column for file names */
		col_offset = gtk_tree_view_insert_column_with_attributes(
					GTK_TREE_VIEW(file_list),
					-1, "Files",
					renderer, "text",
					FILE_COLUMN, NULL);
		column = gtk_tree_view_get_column(GTK_TREE_VIEW(file_list), 
							col_offset - 1);
		gtk_tree_view_column_set_clickable(
					GTK_TREE_VIEW_COLUMN(column), TRUE);
		}
	gtk_tree_store_clear(model);
	add_to_tree(model, "Shape Files", "*.vga,*.shp,combos.flx", ShapeArchive);
	add_to_tree(model, "Map Files", "u7chunks", ChunkArchive);
	add_to_tree(model, "Palette Files", "*.pal,palettes.flx", PaletteFile);
	
#if 0	/* Skip this until we can do something with these files. */
	add_to_tree(model, "FLEX Files", "*.flx", FlexArchive);
#endif
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(file_list), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(file_list)), GTK_SELECTION_SINGLE);
}

/*
 *	Save all the files we've changed:
 *		Object browser.
 *		Group files.
 *		Shape files.
 *		Shape info (tfa.dat, weapons.dat, etc.)
 *		Game map.
 */

void ExultStudio::save_all
	(
	)
	{
	save_groups();			// Group files.
	if (files)
		{			// Get back any changed images.
		Shape_chooser::check_editing_files();
		files->flush();		// Write out the .vga files.
		}
	write_shape_info();
	write_map();
	}

/*
 *	Any files need saving?
 */

bool ExultStudio::need_to_save
	(
	)
	{
	if (groups_modified())
		return true;
	if (files && files->is_modified())
		return true;
	if (shape_info_modified || shape_names_modified)
		return true;
					// Ask Exult about the map.
	if (Send_data(server_socket, Exult_server::info) != -1)
		{			// Should get immediate answer.
		unsigned char data[Exult_server::maxlength];
		Exult_server::Msg_type id;
		Exult_server::wait_for_response(server_socket, 100);
		int len = Exult_server::Receive_data(server_socket, 
						id, data, sizeof(data));
		unsigned char *ptr = &data[0];
		int vers, edlift, hdlift, edmode;
		bool editing, grid, mod;
		if (id == Exult_server::info &&
		    Game_info_in(data, len, vers, edlift, hdlift, 
						editing, grid, mod, edmode) &&
		    mod == true)
			return true;
		}
	return false;
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
 *	Write out shape info. and text.
 */

void ExultStudio::write_shape_info
	(
	bool force			// If set, always write.
	)
	{
	if ((force || shape_info_modified) && vgafile)
		{
		Shapes_vga_file *svga = 
				(Shapes_vga_file *) vgafile->get_ifile();
					// Make sure data's been read in.
		svga->read_info(false, true);//+++++BG?
		svga->write_info(false);//++++BG?
		}
	shape_info_modified = false;
	if (force || shape_names_modified)
		{
		shape_names_modified = false;
		int cnt = names.size();
		std::ofstream out;
		U7open(out, "<PATCH>/text.flx");
		Flex_writer writer(out, "Text created by ExultStudio", cnt);
		for (int i = 0; i < cnt; i++)
			{
			char *str = names[i];
			if (str)
				out << str;
			out.put((char) 0);	// 0-delimit.
			writer.mark_section_done();
			}
		if (!writer.close())
			EStudio::Alert("Error writing 'text.flx'");
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
 *	Tell Exult to hide objects at or above a given lift.
 */

void ExultStudio::set_hide_lift
	(
	int lift
	)
	{
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	Write2(ptr, lift);
	send_to_server(Exult_server::hide_lift, data, ptr - data);
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
	if (!terrain)
		{			// Turning it off.
		if (browser)
			browser->end_terrain_editing();
					// FOR NOW, skip_lift is reset.
		set_spin("hide_lift_spin", 16, true);
		}
	else				// Disable "Hide lift".
		set_sensitive("hide_lift_spin", false);
					// Set edit-mode to paint.
	GtkWidget *mitem = glade_xml_get_widget(app_xml, 
						terrain ? "paint1" : "move1");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitem), TRUE);
	}

void ExultStudio::set_edit_mode
	(
	int md				// 0-2 (drag, paint, pick.
	)
	{
	unsigned char data[Exult_server::maxlength];
	unsigned char *ptr = &data[0];
	Write2(ptr, md);
					//   haven't got an interface yet.
	send_to_server(Exult_server::set_edit_mode, data, ptr - data);
	}

/*
 *	Insert 0-delimited text into an edit box.
 */

static void Insert_text
	(
	GtkEditable *ed,
	const char *text,
	int& pos			// Where to insert.  Updated.
	)
	{
	gtk_editable_insert_text(ed, text, strlen(text), &pos);
	}

/*
 *	Show unused shape list received from exult.
 */

void ExultStudio::show_unused_shapes
	(
	unsigned char *data,		// Bits set for unused shapes.
	int datalen			// #bytes.
	)
	{
	int nshapes = datalen*8;
	GtkTextView *text = GTK_TEXT_VIEW(glade_xml_get_widget(app_xml, "msg_text"));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(text);
	gtk_text_buffer_set_text(buffer, "", 0);	// Clear out old text
	set_visible("msg_win", TRUE);	// Show message window.
	int pos = 0;
	GtkEditable *ed = GTK_EDITABLE(text);
	Insert_text(ed,
		"The following shapes were not found.\n", pos);
	Insert_text(ed,
		"Note that some may be created by usecode (script)\n\n", pos);
	for (int i = 0x96; i < nshapes; i++)	// Ignore flats (<0x96).
		if (!(data[i/8]&(1<<(i%8))))
			{
			const char *nm = get_shape_name(i);
			char *msg = g_strdup_printf("  Shape %4d:    %s\n",
							i, nm ? nm : "");
			Insert_text(ed, msg, pos);
			g_free(msg);
			}
	// FIXME: gtk_text_set_point(text, 0);	// Scroll back to top.
	}


/*
 *	Open a shape (or chunks) file in 'patch' or 'static' directory.
 *
 *	Output:	->file info (may already have existed), or 0 if error.
 */

Shape_file_info *ExultStudio::open_shape_file
	(
	const char *basename		// Base name of file to open.
	)
	{
	Shape_file_info *info = files->create(basename);
	if (!info)
		EStudio::Alert("'%s' not found in static or patch directories",
							basename);
	return info;
	}

/*
 *	Prompt for a new shape file name.
 */

void ExultStudio::new_shape_file
	(
	bool single			// Not a FLEX file.
	)
	{
	GtkFileSelection *fsel = Create_file_selection(
		single ? "Write new .shp file" : "Write new .vga file",
			(File_sel_okay_fun) create_shape_file,
							(gpointer) single);
//	This doesn't work very well in GTK 1.2.  Try again later.
//	gtk_file_selection_complete(fsel, single ? "*.shp" : "*.vga");
	if (is_system_path_defined("<PATCH>"))
		{			// Default to 'patch'.
		string patch = get_system_path("<PATCH>/");
		gtk_file_selection_set_filename(fsel, patch.c_str());
		}
	gtk_widget_show(GTK_WIDGET(fsel));
	}

/*
 *	Create a new shape/shapes file.
 */

void ExultStudio::create_shape_file
	(
	char *pathname,			// Full path.
	gpointer udata			// 1 if NOT a FLEX file.
	)
	{
	bool oneshape = (bool) udata;
	Shape *shape = 0;
	if (oneshape)			// Single-shape?
		{			// Create one here.
		const int w = 8, h = 8;
		unsigned char pixels[w*h];	// Create an 8x8 shape.
		memset(&pixels[0], 1, w*h);	// Just use color #1.
		shape = new Shape(new Shape_frame(&pixels[0],
				w, h, w - 1, h - 1, true));
		}
	try {				// Write file.
		if (oneshape)
			Image_file_info::write_file(pathname, &shape, 1, true);
		else
			Image_file_info::write_file(pathname, 0, 0, false);
	}
	catch (const exult_exception& e) {
		EStudio::Alert(e.what());
	}
	ExultStudio *studio = ExultStudio::get_instance();
	studio->setup_file_list();	// Rescan list of shape files.
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
	const gchar *txt = gtk_entry_get_text(GTK_ENTRY(field));
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

const gchar *ExultStudio::get_text_entry
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
	static GdkPixmap *logo_pixmap = NULL;
	static GdkBitmap *logo_mask = NULL;
	if (!logo_pixmap)		// First time?
		{
		logo_pixmap = gdk_pixmap_create_from_xpm_d(app->window, 
				&logo_mask, NULL, logo_xpm);
		GtkWidget *pix = gtk_pixmap_new(logo_pixmap, logo_mask);
		gtk_widget_show(pix);
		GtkWidget *hbox = glade_xml_get_widget(app_xml, 
							"prompt3_hbox");
		gtk_box_pack_start(GTK_BOX(hbox), pix, FALSE, FALSE, 12);
					// Make logo show to left.
		gtk_box_reorder_child(GTK_BOX(hbox), pix, 0);
		}
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
	assert(prompt_choice >= 0 && prompt_choice <= 2);
	return prompt_choice;
	}

namespace EStudio {

/*
 *	Same as ExultStudio::prompt, but as a routine.
 */
int Prompt
	(
	const char *msg,		// Question to ask.
	const char *choice0,		// 1st choice.
	const char *choice1,		// 2nd choice, or NULL.
	const char *choice2		// 3rd choice, or NULL.
	)
	{
	return ExultStudio::get_instance()->prompt(msg, choice0, choice1,
							choice2);
	}

/*
 *	Just print a message (usually an error).
 */
void Alert
	(
	const char *msg,		// May be in printf format.
	...
	)
	{
	std::va_list args;
	va_start(args, msg);
	char *fullmsg = g_strdup_vprintf(msg, args);
	Prompt(fullmsg, "Okay");
	g_free(fullmsg);
	}

/*
 *	Add a menu item to a menu.
 *
 *	Output:	Menu item.
 */

GtkWidget *Add_menu_item
	(
	GtkWidget *menu,		// Menu to add to.
	const char *label,		// What to put.  NULL for separator.
	GtkSignalFunc func,		// Handle menu choice.
	gpointer func_data		// Data passed to func().
	)
	{
	GtkWidget *mitem = label ? gtk_menu_item_new_with_label(label) :
				gtk_menu_item_new();
	gtk_widget_show(mitem);
	gtk_menu_append(GTK_MENU(menu), mitem);
	if (!label)			// Want separator?
		gtk_widget_set_sensitive(mitem, FALSE);
	if (func)			// Function?
		gtk_signal_connect(GTK_OBJECT(mitem), "activate",
				GTK_SIGNAL_FUNC(func), func_data);
	return mitem;
	}

/*
 *	Create an arrow button.
 */

GtkWidget *Create_arrow_button
	(
	GtkArrowType dir,		// Direction.
	GtkSignalFunc clicked,		// Call this when clicked.
	gpointer func_data		// Passed to 'clicked'.
	)
	{
	GtkWidget *btn = gtk_button_new();
	gtk_widget_show(btn);
	GTK_WIDGET_SET_FLAGS(btn, GTK_CAN_DEFAULT);
	GtkWidget *arrow = gtk_arrow_new(dir, GTK_SHADOW_OUT);
	gtk_widget_show(arrow);
	gtk_container_add(GTK_CONTAINER(btn), arrow);
	gtk_signal_connect(GTK_OBJECT(btn), "clicked", clicked, func_data);
	return btn;
	}

} // namespace EStudio

/*
 *	'Preferences' window events.
 */

C_EXPORT void
on_prefs_cancel_clicked			(GtkButton *button,
					 gpointer   user_data)
{
	gtk_widget_hide(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}
C_EXPORT void
on_prefs_apply_clicked			(GtkButton *button,
					 gpointer   user_data)
{
	ExultStudio::get_instance()->save_preferences();
}
C_EXPORT void
on_prefs_okay_clicked			(GtkButton *button,
					 gpointer   user_data)
{
	ExultStudio::get_instance()->save_preferences();
	gtk_widget_hide(gtk_widget_get_toplevel(GTK_WIDGET(button)));
}

/*
 *	'Okay' was hit in the color selector.
 */

void ExultStudio::background_color_okay
	(
	GtkWidget *dlg,
	gpointer data
	)
	{
	GtkColorSelectionDialog *colorsel = GTK_COLOR_SELECTION_DIALOG(dlg);
	gdouble rgb[3];
	gtk_color_selection_get_color(
			GTK_COLOR_SELECTION(colorsel->colorsel), rgb);
	unsigned char r = (unsigned char) (rgb[0]*256),
		      g = (unsigned char) (rgb[1]*256),
		      b = (unsigned char) (rgb[2]*256);
	ExultStudio *studio = ExultStudio::get_instance();
	studio->background_color = (r<<16) + (g<<8) + b;
					// Show new color.
	GtkWidget *backgrnd = glade_xml_get_widget(studio->app_xml, 
							"prefs_background");
	gtk_object_set_user_data(GTK_OBJECT(backgrnd), 
					(gpointer) studio->background_color);
	GdkRectangle area = {0, 0, backgrnd->allocation.width, 
					backgrnd->allocation.height};
	gtk_widget_draw(backgrnd, &area);
	gtk_widget_destroy(dlg);
	}

C_EXPORT void
on_prefs_background_choose_clicked	(GtkButton *button,
					 gpointer   user_data)
{
	GtkColorSelectionDialog *colorsel = GTK_COLOR_SELECTION_DIALOG(
		gtk_color_selection_dialog_new("Background color"));
	gtk_window_set_modal(GTK_WINDOW(colorsel), true);
					// Set mouse click handler.
	gtk_signal_connect_object(GTK_OBJECT(colorsel->ok_button), "clicked",
		GTK_SIGNAL_FUNC(ExultStudio::background_color_okay), 
						GTK_OBJECT(colorsel));
	gtk_signal_connect_object(GTK_OBJECT(colorsel->cancel_button), 
		"clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
						GTK_OBJECT(colorsel));
					// Set delete handler.
	gtk_signal_connect(GTK_OBJECT(colorsel), "delete_event",
				GTK_SIGNAL_FUNC(gtk_false), 0L);
					// Get color.
	guint32 c = ExultStudio::get_instance()->get_background_color();
	gdouble rgb[3];
	rgb[0] = ((double) ((c>>16)&0xff))/256;
	rgb[1] = ((double) ((c>>8)&0xff))/256;
	rgb[2] = ((double) ((c>>0)&0xff))/256;
	gtk_color_selection_set_color(GTK_COLOR_SELECTION(colorsel->colorsel),
								rgb);
	gtk_widget_show(GTK_WIDGET(colorsel));
}
					// Background color area exposed.
C_EXPORT gboolean on_prefs_background_expose_event
	(
	GtkWidget *widget,		// The draw area.
	GdkEventExpose *event,
	gpointer data
	)
	{
	guint32 color = (guint32) gtk_object_get_user_data(GTK_OBJECT(widget));
	GdkGC *gc = (GdkGC *) 
			gtk_object_get_data(GTK_OBJECT(widget), "color_gc");
	if (!gc)
		{
		gc = gdk_gc_new(widget->window);
		gtk_object_set_data(GTK_OBJECT(widget), "color_gc", gc);
		}
	gdk_rgb_gc_set_foreground(gc, color);
	gdk_draw_rectangle(widget->window, gc, TRUE, event->area.x, 
			event->area.y, event->area.width, event->area.height);
	return (TRUE);
	}

					// X at top of window.
C_EXPORT gboolean on_prefs_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	gtk_widget_hide(widget);
	return TRUE;
	}

/*
 *	Open preferences window.
 */

void ExultStudio::open_preferences
	(
	)
	{
	set_entry("prefs_image_editor", image_editor ? image_editor : "");
	set_entry("prefs_default_game", default_game ? default_game : "");
	GtkWidget *backgrnd = glade_xml_get_widget(app_xml, 
							"prefs_background");
	gtk_object_set_user_data(GTK_OBJECT(backgrnd), 
						(gpointer) background_color);
	GtkWidget *win = glade_xml_get_widget(app_xml, "prefs_window");
	gtk_widget_show(win);
	}

/*
 *	Save preferences.
 */

void ExultStudio::save_preferences
	(
	)
	{
	const char *text = get_text_entry("prefs_image_editor");
	g_free(image_editor);
	image_editor = g_strdup(text);
	config->set("config/estudio/image_editor", image_editor, true);
	text = get_text_entry("prefs_default_game");
	g_free(default_game);
	default_game = g_strdup(text);
	config->set("config/estudio/default_game", default_game, true);
	GtkWidget *backgrnd = glade_xml_get_widget(app_xml, 
							"prefs_background");
	background_color = (guint32) gtk_object_get_user_data(
						GTK_OBJECT(backgrnd));
	config->set("config/estudio/background_color", background_color, true);
					// Set background color.
	palbuf[3*255] = (background_color>>18)&0x3f;
	palbuf[3*255 + 1] = (background_color>>10)&0x3f;
	palbuf[3*255 + 2] = (background_color>>2)&0x3f;
	if (browser)			// Repaint browser.
		browser->set_background_color(background_color);
	}


/*
 *	Main routine.
 */
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
	case Exult_server::delete_terrain:
	case Exult_server::locate_shape:
	case Exult_server::game_pos:
		if (waiting_for_server)	// Send msg. to callback.
			{
			waiting_for_server(id, data, datalen, waiting_client);
			waiting_for_server = 0;
			waiting_client = 0;
			}
		else if (browser)
			browser->server_response((int) id, data, datalen);
		break;
	case Exult_server::info:
		info_received(data, datalen);
		break;
	case Exult_server::view_pos:
		if (locwin)
			locwin->view_changed(data, datalen);
		break;
	case Exult_server::combo_pick:
		open_combo_window();	// Open if necessary.
		if (combowin)
			combowin->add(data, datalen);
		break;
	case Exult_server::unused_shapes:
		show_unused_shapes(data, datalen);
		break;
	case Exult_server::select_status:
		set_edit_menu(data[0] != 0, data[1] != 0);
		break;
	case Exult_server::usecode_debugging:
		std::cerr << "Warning: got a usecode debugging message! (ignored)"
				  << std::endl;
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

	char *home = getenv("HOME");
	addr.sun_path[0] = 0;
	if (home)			// Use $HOME/.exult/exultserver
		{			//   if possible.
		strcpy(addr.sun_path, home);
		strcat(addr.sun_path, "/.exult/exultserver");
		if (!U7exists(addr.sun_path))
			addr.sun_path[0] = 0;
		}
	if (!addr.sun_path[0])		// Default to game/gamedat.
		{
	        strcpy(addr.sun_path, static_path);
        	char *pstatic = strrchr(addr.sun_path, '/');
	        if (pstatic && !pstatic[1])     // End of path?
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
		}
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
	server_input_tag = gdk_input_add(server_socket,
			GDK_INPUT_READ, Read_from_server, this);
#else
		// Close existing socket.
	if (server_socket != -1) Exult_server::disconnect_from_server();
	if (server_input_tag != -1) gtk_timeout_remove(server_input_tag);
	server_socket = server_input_tag = -1;

	if (Exult_server::try_connect_to_server(static_path) > 0)
		server_input_tag = gtk_timeout_add(50, Read_from_server, this);
	else
		return false;
#endif
	cout << "Connected to server" << endl;
	send_to_server(Exult_server::info);	// Request version, etc.
	set_edit_menu(false, false);	// For now, init. edit menu.
	return true;
	}
/*
 *	'Info' message received.
 */

void ExultStudio::info_received
	(
	unsigned char *data,		// Message from Exult.
	int len
	)
	{
	int vers, edlift, hdlift, edmode;
	bool editing, grid, mod;
	Game_info_in(data, len, vers, edlift, hdlift, 
					editing, grid, mod, edmode);
	if (vers != Exult_server::version)
		{			// Wrong version of Exult.
		EStudio::Alert("Expected ExultServer version %d, but got %d",
				Exult_server::version, vers);
#ifndef WIN32
		close(server_socket);
		gdk_input_remove(server_input_tag);
#else
		Exult_server::disconnect_from_server();
		gtk_timeout_remove(server_input_tag);
#endif
		server_socket = server_input_tag = -1;
		return;
		}
					// Set controls to what Exult thinks.
	set_spin("edit_lift_spin", edlift);
	set_spin("hide_lift_spin", hdlift);
	set_toggle("play_button", !editing);
	set_toggle("tile_grid_button", grid);
	if (edmode >= 0 && edmode < sizeof(mode_names)/sizeof(mode_names[0]))
		{
		GtkWidget *mitem = glade_xml_get_widget(app_xml, 
							mode_names[edmode]);

		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitem),
									TRUE);
		}
	}

/*
 *	Set edit menu entries depending on whether there's a selection or
 *	clipboard available.
 */

void ExultStudio::set_edit_menu
	(
	bool sel,			// Selection available.
	bool clip			// Clipboard isn't empty.
	)
	{
	set_sensitive("cut1", sel);
	set_sensitive("copy1", sel);
	set_sensitive("paste1", clip);
	set_sensitive("properties1", sel);
	set_sensitive("basic_properties1", sel);
	}

