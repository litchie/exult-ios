/**
 **	Locator.cc - Locate game positions.
 **
 **	Written: March 2, 2002 - JSF
 **/

/*
Copyright (C) 2001-2002 The Exult Team

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

using	std::cout;
using	std::endl;

/*
 *	Open locator window.
 */

C_EXPORT void on_locator1_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->open_locator_window();
	}
void ExultStudio::open_locator_window
	(
	)
	{
	if (!locwin)			// First time?
		{
		locwin = glade_xml_get_widget( app_xml, "loc_window" );
		}
	gtk_widget_show(locwin);
	}

/*
 *	Locator window's close button.
 */
C_EXPORT void on_loc_close_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	gtk_widget_hide(gtk_widget_get_toplevel(GTK_WIDGET(btn)));
	}

/*
 *	Locator window's X button.
 */
C_EXPORT gboolean on_loc_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	gtk_widget_hide(widget);
	return TRUE;
	}

