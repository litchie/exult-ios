/**
 **	Maps.cc - Multimap handling.
 **
 **	Written: February 2, 2004 - JSF
 **/

/*
Copyright (C) 2001-2004 The Exult Team

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

#include "studio.h"
#include "servemsg.h"
#include "exult_constants.h"
#include "utils.h"

using	std::cout;
using	std::endl;

/*
 *	Find highest map #.
 */

static int Find_highest_map
	(
	)
	{
	int n = 0, next;

	while ((next = Find_next_map(n + 1, 10)) != -1)
		n = next;
	return n;
	}

/*
 *	Jump to desired map.
 */

C_EXPORT void on_main_map_activate
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	// ExultStudio::get_instance()->goto_map(0);
	}
static void on_map_activate
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	// ExultStudio::get_instance()->goto_map((int) udata);
	}

/*
 *	Open new-map dialog.
 */

C_EXPORT void on_newmap_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->new_map_dialog();
	}
void ExultStudio::new_map_dialog
	(
	)
	{
	GtkWidget *win = glade_xml_get_widget(app_xml, "newmap_dialog");
	gtk_window_set_modal(GTK_WINDOW(win), true);
	int highest = Find_highest_map();
	set_spin("newmap_num", highest + 1, 1, 100);
	set_entry("newmap_name", "", false);	// LATER.
	set_spin("newmap_copy_num", 0, 0, highest);
	set_toggle("newmap_copy_flats", false);
	set_toggle("newmap_copy_fixed", false);
	set_toggle("newmap_copy_ireg", false);
	gtk_widget_show(win);
	}


