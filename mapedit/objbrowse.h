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

#ifndef OBJBROWSE_H
#define OBJBROWSE_H

#include <gtk/gtk.h>

//#include <glade/glade.h>

class Shape_group;
class Shape_file_info;

class Object_browser {
private:
	GtkWidget *widget;
protected:
	int selected;			// Index of user-selected entry.
	Shape_group *group;		// Non-null to use filter.
	GtkWidget *popup;		// Popup menu in draw area.
	Shape_file_info *file_info;	// Our creator (or null).
	void set_widget(GtkWidget *w);
public:
	Object_browser(Shape_group *grp = 0, Shape_file_info *fi = 0);
	virtual ~Object_browser();
	
	GtkWidget *get_widget();
	Shape_group *get_group()
		{ return group; }
	int get_selected()		// Return index of selected item.
		{ return selected; }	// (-1 if none.)
	virtual void render() = 0;
					// Blit onto screen.
	virtual void show(int x, int y, int w, int h) = 0;
	virtual void show() = 0;
	virtual int get_selected_id()
		{ return -1; }
	virtual bool server_response(int id, unsigned char *data, int datalen);
	virtual void end_terrain_editing();
					// About to close this browser.
	virtual bool closing(bool can_cancel = false);
					// Menu items:
	static void on_shapes_popup_add2group_activate(
					GtkMenuItem *item, gpointer udata);
					// Add 'Add to group...' submenu.
	void add_group_submenu(GtkWidget *popup);
};

#endif
