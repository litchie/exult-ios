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

#ifndef STUDIO_H

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <vector>
#include "vgafile.h"
#include "servemsg.h"

class Shape_info;
class Shapes_vga_file;
class Equip_row_widgets;
class Shape_file_set;
class Shape_file_info;
class Shape_group_file;
class Shape_draw;
class Object_browser;
class Shape_group;
class Locator;
class Combo_editor;
					// Callback for msgs.
typedef void (*Msg_callback)(Exult_server::Msg_type id, 
			unsigned char *data, int datalen, void *client);

#ifndef WIN32
#define C_EXPORT extern "C"
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "windrag.h"
#define C_EXPORT extern "C" __declspec(dllexport)
#endif

class ExultStudio {
private:
	char			*glade_path;	// Where our .glade file is.
	GtkWidget		*app;
	GladeXML		*app_xml;
	char 			*static_path;
	char			*image_editor;
	char			*default_game;
	guint32			background_color;
	static ExultStudio	*self;
						// Modified one of the .dat's?
	bool			shape_info_modified, shape_names_modified;
	Shape_file_set		*files;		// All the shape files.
	std::vector<GtkWindow*>	group_windows;	// All 'group' windows.
	Shape_file_info		*curfile;	// Current browser file info.
	Shape_file_info		*vgafile;	// Main 'shapes.vga'.
	Shape_file_info		*facefile;	// 'faces.vga'.
	std::vector<char *>	names;
	Object_browser		*browser;
	unsigned char 		*palbuf;	// 3*256 rgb's, each 0-63.
					// Egg editor:
	GtkWidget		*eggwin;// Egg window.
	Shape_draw		*egg_monster_draw;
	int			egg_ctx;
					// Npc editor:
	GtkWidget		*npcwin;
	Shape_draw		*npc_draw, *npc_face_draw;
	int			npc_ctx;
					// Object editor:
	GtkWidget		*objwin;
	Shape_draw		*obj_draw;
					// Shape info. editor:
	GtkWidget		*shapewin;
	Shape_draw		*shape_draw;
	GtkWidget		*equipwin;
					// Map locator:
	Locator			*locwin;
					// Combo editor:
	Combo_editor		*combowin;
	// For Win32 DND
#ifdef WIN32
	HWND			egghwnd;
	Windnd			*eggdnd;
	HWND			npchwnd;
	Windnd			*npcdnd;
	HWND			objhwnd;
	Windnd			*objdnd;
	HWND			shphwnd;
	Windnd			*shpdnd;
#endif
					// Server data.
	int			server_socket;
	gint			server_input_tag;
	Msg_callback		waiting_for_server;
	void			*waiting_client;
public:
	ExultStudio(int argc, char **argv);
	~ExultStudio();
	bool okay_to_close();
	
	static ExultStudio *get_instance()
		{ return self; }
	GladeXML *get_xml() 
		{ return app_xml; }
	int get_server_socket() const
		{ return server_socket; }
	guint32 get_background_color() const
		{ return background_color; }
	char *get_shape_name(int shnum)
		{ return shnum >= 0 && shnum < names.size() ?names[shnum] : 0;}
	const char *get_image_editor()
		{ return image_editor; }
	Shape_file_set *get_files()
		{ return files; }
	Object_browser *get_browser()
		{ return browser; }
	unsigned char *get_palbuf()
		{ return palbuf; }
	Shape_file_info *get_vgafile()	// 'shapes.vga'.
		{ return vgafile; }
	Combo_editor *get_combowin()
		{ return combowin; }
	Shape_group_file *get_cur_groups();
	void set_browser(const char *name, Object_browser *obj);
	bool has_focus();		// Any of our windows has focus?

	void create_new_game(char *dir);
	void new_game();
	void choose_game_path();
	Object_browser  *create_browser(const char *fname);
	void set_game_path(const char *path);
	void setup_file_list();
	void save_all();		// Write out everything.
	bool need_to_save();		// Anything modified?
	void write_map();
	void read_map();
	void write_shape_info();
	void reload_usecode();
	void set_play(gboolean play);
	void set_tile_grid(gboolean grid);
	void set_edit_lift(int lift);
	void set_hide_lift(int lift);
	void set_edit_terrain(gboolean terrain);
	void set_edit_mode(int md);
	void show_unused_shapes(unsigned char *data, int datalen);
					// Open/create shape files:
	Shape_file_info *open_shape_file(const char *fname);
	void new_shape_file(bool single);
	static void create_shape_file(char *nm, gpointer udata);
					// Groups:
	void setup_groups();
	void setup_group_controls();
	void add_group();
	void del_group();
	void move_group(int from_row, int to_row);
	void open_group_window();
	void close_group_window(GtkWidget *gtkwin);
	void save_groups();
	bool groups_modified();
	void update_group_windows(Shape_group *grp);
					// Objects:
	void open_obj_window(unsigned char *data, int datalen);
	void close_obj_window();
	int init_obj_window(unsigned char *data, int datalen);
	int save_obj_window();
	void show_obj_shape(int x = 0, int y = 0, int w = -1, int h = -1);
	void set_obj_shape(int shape, int frame);
					// Eggs:
	void open_egg_window(unsigned char *data = 0, int datalen = 0);
	void close_egg_window();
	int init_egg_window(unsigned char *data, int datalen);
	int save_egg_window();
	void show_egg_monster(int x = 0, int y = 0, int w = -1, int h = -1);
	void set_egg_monster(int shape, int frame);
					// NPC's:
	void open_npc_window(unsigned char *data = 0, int datalen = 0);
	void close_npc_window();
	int init_npc_window(unsigned char *data, int datalen);
	int save_npc_window();
	void show_npc_shape(int x = 0, int y = 0, int w = -1, int h = -1);
	void set_npc_shape(int shape, int frame);
	void show_npc_face(int x = 0, int y = 0, int w = -1, int h = -1);
	void set_npc_face(int shape, int frame);
	static void schedule_btn_clicked(GtkWidget *btn, gpointer data);
					// Shapes:
	void init_equip_window(int recnum);
	void save_equip_window();
	void open_equip_window(int recnum);
	void close_equip_window();
	void show_equip_shape(Equip_row_widgets *eq,
			int x = 0, int y = 0, int w = -1, int h = -1);
	void init_shape_notebook(Shape_info& info, GtkWidget *book, 
							int shnum, int frnum);
	void save_shape_notebook(Shape_info& info, int shnum, int frnum);
	void open_shape_window(int shnum, int frnum, Vga_file *ifile,
					char *shname, Shape_info *info = 0);
	void save_shape_window();
	void close_shape_window();
	void show_shinfo_shape(int x = 0, int y = 0, int w = -1, int h = -1);
					// Map locator.
	void open_locator_window();
	void open_combo_window();	// Combo-object editor.
	void save_combos();

	void run();
	bool send_to_server(Exult_server::Msg_type id,
				unsigned char *data = 0, int datalen = 0);
	void read_from_server();
	bool connect_to_server();
					// Preferences.
	static void background_color_okay(GtkWidget *dlg, gpointer data);
	void open_preferences();
	void save_preferences();
					// GTK/Glade utils:
	bool get_toggle(char *name);
	void set_toggle(char *name, bool val);
	void set_bit_toggles(char **names, int num, unsigned char bits);
	unsigned char get_bit_toggles(char **names, int num);
	int get_optmenu(char *name);
	void set_optmenu(char *name, int val);
	int get_spin(char *name);
	void set_spin(char *name, int val, bool sensitive = true);
	void set_spin(char *name, int val, int low, int high);
	int get_num_entry(char *name);
	char *get_text_entry(char *name);
	void set_entry(char *name, int val, bool hex = false,
						bool sensitive = true);
	void set_entry(char *name, const char *val, bool sensitive = true);
	void set_statusbar(char *name, int context, char *msg);
	void set_button(char *name, const char *text);
	void set_visible(char *name, bool vis);
	void set_sensitive(char *name, bool vis);
	int prompt(const char *msg, const char *choice0, 
			const char *choice1 = 0,const char *choice2 = 0);
};

					// Utilities:
namespace EStudio {
int Prompt(const char *msg, const char *choice0, 
			const char *choice1 = 0,const char *choice2 = 0);
void Alert(const char *msg, ...);
GtkWidget *Add_menu_item(GtkWidget *menu, const char *label = 0,
			GtkSignalFunc func = 0, gpointer func_data = 0);
GtkWidget *Create_arrow_button(GtkArrowType dir, GtkSignalFunc clicked,
							gpointer func_data);
}

#endif
