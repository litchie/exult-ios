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

class ExultStudio {
private:
	GtkWidget		*app;
	GladeXML		*app_xml;
	char 			*static_path;
	static ExultStudio	*self;
	Vga_file		*ifile;
	char			**names;
	Object_browser		*browser;
	GtkWidget		*eggwin;// Egg window.
	int			server_socket;
	gint			server_input_tag;
public:
	ExultStudio(int argc, char **argv);
	~ExultStudio();
	
	static ExultStudio *get_instance()
		{ return self; }

	void set_browser(const char *name, Object_browser *obj);

	void choose_static_path();
	Object_browser  *create_shape_browser(const char *fname);
	void delete_shape_browser();
	Object_browser  *create_palette_browser(const char *fname);
	void set_static_path(const char *path);
	void open_egg_window();
	void close_egg_window();
	void run();
	void read_from_server();
	void connect_to_server();
};

#endif
