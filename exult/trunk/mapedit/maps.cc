/**
 ** Maps.cc - Multimap handling.
 **
 ** Written: February 2, 2004 - JSF
 **/

/*
Copyright (C) 2001-2013 The Exult Team

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

#include <gtk/gtkradiomenuitem.h>
#include "studio.h"
#include "servemsg.h"
#include "exult_constants.h"
#include "utils.h"
#include "fnames.h"

using   std::cout;
using   std::endl;
using EStudio::Add_menu_item;
using EStudio::Alert;

/*
 *  Find highest map #.
 */

static int Find_highest_map(
) {
	int n = 0, next;

	while ((next = Find_next_map(n + 1, 10)) != -1)
		n = next;
	return n;
}

/*
 *  Jump to desired map.
 */

static void on_map_activate(
    GtkMenuItem *item,
    gpointer udata
) {
	unsigned char data[50];
	unsigned char *ptr = &data[0];
	Write2(ptr, (uintptr) udata);
	ExultStudio::get_instance()->send_to_server(Exult_server::goto_map,
	        &data[0], ptr - data);
}
C_EXPORT void on_main_map_activate(
    GtkMenuItem *item,
    gpointer udata
) {
	on_map_activate(item, (gpointer) 0);
}

/*
 *  Open new-map dialog.
 */

C_EXPORT void on_newmap_activate(
    GtkMenuItem     *menuitem,
    gpointer         user_data
) {
	ExultStudio *studio = ExultStudio::get_instance();
	studio->new_map_dialog();
}
void ExultStudio::new_map_dialog(
) {
	GtkWidget *win = glade_xml_get_widget(app_xml, "newmap_dialog");
	gtk_window_set_modal(GTK_WINDOW(win), true);
	int highest = Find_highest_map();
	set_spin("newmap_num", highest + 1, 1, 100);
	set_entry("newmap_name", "", false);    // LATER.
	set_spin("newmap_copy_num", 0, 0, highest);
	set_toggle("newmap_copy_flats", false);
	set_toggle("newmap_copy_fixed", false);
	set_toggle("newmap_copy_ireg", false);
	gtk_widget_show(win);
}

/*
 *  Copy from patch, else from static directory to new map in patch.
 */

static bool Copy_static_file(
    const char *sname,          // Name in static, from fnames.h.
    const char *pname,          // Name in patch.
    int frommap,            // # of map to copy from.
    int tomap           // # of map to copy to.
) {
	char srcname[128], destname[128];
	Get_mapped_name(pname, tomap, destname);
	Get_mapped_name(pname, frommap, srcname);   // Patch?
	if (U7exists(srcname) && EStudio::Copy_file(srcname, destname))
		return true;
	Get_mapped_name(sname, frommap, srcname);   // Static next.
	if (U7exists(srcname))
		return EStudio::Copy_file(srcname, destname);
	return true;
}

/*
 *  Create new map.
 */
C_EXPORT void on_newmap_ok_clicked(
    GtkMenuItem     *menuitem,
    gpointer         user_data
) {
	char fname[128], sname[128];
	ExultStudio *studio = ExultStudio::get_instance();
	GtkWidget *win = glade_xml_get_widget(studio->get_xml(),
	                                      "newmap_dialog");
	int num = studio->get_spin("newmap_num");

	if (U7exists(Get_mapped_name("<PATCH>/", num, fname)) ||
	        U7exists(Get_mapped_name("<STATIC>/", num, sname))) {
		EStudio::Alert("Map %02x already exists", num);
		return;         // Leave dialog open.
	}
	U7mkdir(fname, 0755);       // Create map directory in 'patch'.
	int frommap = studio->get_spin("newmap_copy_num");
	if (studio->get_toggle("newmap_copy_flats"))
		Copy_static_file(U7MAP, PATCH_U7MAP, frommap, num);
	if (studio->get_toggle("newmap_copy_fixed")) {
		for (int schunk = 0; schunk < 12 * 12; schunk++) {
			char pname[128], sname[128];
			sprintf(pname, "%s%02x", PATCH_U7IFIX, schunk);
			sprintf(sname, "%s%02x", U7IFIX, schunk);
			if (!Copy_static_file(sname, pname, frommap, num))
				break;
		}
	}
	U7mkdir(Get_mapped_name(GAMEDAT, num, fname), 0755);
	if (studio->get_toggle("newmap_copy_ireg")) {
		char tname[128];
		for (int schunk = 0; schunk < 12 * 12; schunk++) {
			Get_mapped_name(U7IREG, frommap, fname);
			sprintf(fname + strlen(fname), "%02x", schunk);
			Get_mapped_name(U7IREG, num, tname);
			sprintf(tname + strlen(tname), "%02x", schunk);
			if (U7exists(fname))
				if (!EStudio::Copy_file(fname, tname))
					break;
		}
	}
	studio->setup_maps_list();
	gtk_widget_hide(win);
}

/*
 *  Set up the list of maps in the "maps" menu.
 */

void ExultStudio::setup_maps_list(
) {
	GtkWidget *maps = gtk_menu_item_get_submenu(
	                      GTK_MENU_ITEM(glade_xml_get_widget(app_xml, "map1")));
	GList *items = gtk_container_get_children(GTK_CONTAINER(maps));
	GList *each = g_list_last(items);
	GSList *group = NULL;
	int curmap = 0;

	while (each) {
		GtkMenuItem *item = GTK_MENU_ITEM(each->data);
		GtkWidget *label = gtk_bin_get_child(GTK_BIN(item));
		const char *text = gtk_label_get_label(GTK_LABEL(label));
		if (strcmp(text, "Main") == 0) {
			group = gtk_radio_menu_item_get_group(
			            GTK_RADIO_MENU_ITEM(item));
			gtk_object_set_user_data(GTK_OBJECT(item),
			                         (gpointer) 0);
			if (curmap == 0)
				gtk_check_menu_item_set_active(
				    GTK_CHECK_MENU_ITEM(item), TRUE);
			break;
		}
		if (gtk_check_menu_item_get_active(
		            GTK_CHECK_MENU_ITEM(item)))
			curmap = (sintptr) gtk_object_get_user_data(
			             GTK_OBJECT(item));
		GList *prev = g_list_previous(each);
		gtk_container_remove(GTK_CONTAINER(maps), GTK_WIDGET(item));
		each = prev;
	}
	int num = 0;
	while ((num = Find_next_map(num + 1, 10)) != -1) {
		char name[40];
		sprintf(name, "Map #%02x", num);
		GtkWidget *item =
		    Add_menu_item(maps, name, GTK_SIGNAL_FUNC(on_map_activate),
		                  (gpointer) num, group);
		gtk_object_set_user_data(GTK_OBJECT(item), (gpointer) num);
		if (curmap == num)
			gtk_check_menu_item_set_active(
			    GTK_CHECK_MENU_ITEM(item), TRUE);
		group = gtk_radio_menu_item_get_group(
		            GTK_RADIO_MENU_ITEM(item));
	}
}

