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
#include "shapelst.h"
#include "paledit.h"
#include "vgafile.h"
#include "servemsg.h"

class Shape_info;
class Shapes_vga_file;

					// Callback for msgs.
typedef void (*Msg_callback)(Exult_server::Msg_type id, 
			unsigned char *data, int datalen, void *client);

class ExultStudio {
private:
	GtkWidget		*app;
	GladeXML		*app_xml;
	char 			*static_path;
	static ExultStudio	*self;
	Vga_file		*ifile;
	Vga_file		*vgafile;	// Main 'shapes.vga'.
	Vga_file		*facefile;	// 'faces.vga'.
	std::ifstream		*chunkfile;	// 'u7chunks'.
	char			**names;
	Object_browser		*browser;
	unsigned char 		*palbuf;
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
					// Server data.
	int			server_socket;
	gint			server_input_tag;
	Msg_callback		waiting_for_server;
	void			*waiting_client;
public:
	ExultStudio(int argc, char **argv);
	~ExultStudio();
	
	static ExultStudio *get_instance()
		{ return self; }
	GladeXML *get_xml() 
		{ return app_xml; }
	int get_server_socket() const
		{ return server_socket; }

	void set_browser(const char *name, Object_browser *obj);

	void choose_static_path();
	Object_browser  *create_shape_browser(const char *fname);
	void delete_shape_browser();
	Object_browser  *create_chunk_browser(const char *fname);
	void delete_chunk_browser();
	Object_browser  *create_palette_browser(const char *fname);
	void set_static_path(const char *path);
	void write_map();
	void read_map();
	void reload_usecode();
	void set_play(gboolean play);
	void set_tile_grid(gboolean grid);
	void set_edit_lift(int lift);
	void set_edit_terrain(gboolean terrain);
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
	void init_shape_notebook(Shape_info& info, GtkWidget *book, 
							int shnum, int frnum);
	void save_shape_notebook(Shape_info& info, int shnum, int frnum);
	void open_shape_window(int shnum, int frnum, Vga_file *ifile,
					char *shname, Shape_info *info = 0);
	void close_shape_window();
	void show_shinfo_shape(int x = 0, int y = 0, int w = -1, int h = -1);

	void run();
	bool send_to_server(Exult_server::Msg_type id,
				unsigned char *data = 0, int datalen = 0);
	void read_from_server();
	bool connect_to_server();
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
	void set_visible(char *name, bool vis);
};

#endif
