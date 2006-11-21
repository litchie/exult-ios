/**
 **	Ucbrowse.cc - Browse usecode functions.
 **
 **	Written: Nov. 19, 2006 - JSF
 **/

/*
Copyright (C) 2001-2006 The Exult Team

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
#include "ucbrowse.h"
#include "ucsymtbl.h"
#include "utils.h"

/*
 *	Open browser window.
 */

void ExultStudio::open_usecode_browser_window
	(
	)
	{
	if (!ucbrowsewin)			// First time?
		{
		ucbrowsewin = new Usecode_browser();
		}
	ucbrowsewin->show(true);
	}

/*
 *	Usecode_browser window's cancel button.
 */
C_EXPORT void on_usecodes_cancel_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) gtk_object_get_user_data(
			GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(btn))));
	ucb->cancel();
	}

/*
 *	Usecode_browser window's X button.
 */
C_EXPORT gboolean on_usecodes_dialog_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) 
				gtk_object_get_user_data(GTK_OBJECT(widget));
	
	ucb->cancel();
	return TRUE;
	}

/*
 *	Create usecode browser window.
 */

Usecode_browser::Usecode_browser
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GladeXML *app_xml = studio->get_xml();
	win = glade_xml_get_widget(app_xml, "usecodes_dialog");
	gtk_object_set_user_data(GTK_OBJECT(win), this);
	string ucname = get_system_path("<PATCH>/usecode");
	if (!U7exists(ucname.c_str()))
		{
		ucname = get_system_path("<STATIC>/usecode");
		if (!U7exists(ucname.c_str()))
			ucname = "";
		}
	studio->set_entry("usecodes_file", ucname.c_str());
	}

/*
 *	Delete.
 */

Usecode_browser::~Usecode_browser
	(
	)
	{
	gtk_widget_destroy(win);
	}

/*
 *	Show/hide.
 */

void Usecode_browser::show
	(
	bool tf
	)
	{
	if (tf)
		gtk_widget_show(win);
	else
		gtk_widget_hide(win);
	}

