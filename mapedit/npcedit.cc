/**
 **	Npcedit.cc - Npc-editing methods.
 **
 **	Written: 6/8/01 - JSF
 **/

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

#include "studio.h"
#include "u7drag.h"
#include "servemsg.h"
#include "objserial.h"
#include "exult_constants.h"

/*
 *	Open npc window.
 */

extern "C" void on_open_npc_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->open_npc_window();
	}

/*
 *	Npc window's Apply button.
 */
extern "C" void on_npc_apply_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
//++++++++	ExultStudio::get_instance()->save_npc_window();
	}

/*
 *	Npc window's Cancel button.
 */
extern "C" void on_npc_cancel_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_npc_window();
	}

/*
 *	Npc window's close button.
 */
extern "C" gboolean on_npc_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_npc_window();
	return TRUE;
	}

/*
 *	Open the npc-editing window.
 */

void ExultStudio::open_npc_window
	(
	unsigned char *data,		// Serialized npc, or null.
	int datalen
	)
	{
	if (!npcwin)			// First time?
		{
		npcwin = glade_xml_get_widget( app_xml, "npc_window" );
		if (vgafile && palbuf)
			{
//			npc_monster_draw = new Shape_draw(vgafile, palbuf,
//			    glade_xml_get_widget(app_xml, "npc_monster_draw"));
//			npc_monster_draw->enable_drop(Npc_monster_dropped,
//								this);
			}
		npc_ctx = gtk_statusbar_get_context_id(
			GTK_STATUSBAR(glade_xml_get_widget(
				app_xml, "npc_status")), "Npc Editor");
		}
					// Init. npc address to null.
	gtk_object_set_user_data(GTK_OBJECT(npcwin), 0);
					// Make 'apply' sensitive.
	gtk_widget_set_sensitive(glade_xml_get_widget(app_xml, 
						"npc_apply_btn"), true);
//	if (data)
//		if (!init_npc_window(data, datalen))
//			return;
	gtk_widget_show(npcwin);
	}

/*
 *	Close the npc-editing window.
 */

void ExultStudio::close_npc_window
	(
	)
	{
	if (npcwin)
		gtk_widget_hide(npcwin);
	}

