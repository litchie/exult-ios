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
 *	Draw shape in NPC shape area.
 */
extern "C" gboolean on_npc_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	ExultStudio::get_instance()->show_npc_shape(
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Npc shape # lost focus, so update shape displayed.
 */
extern "C" gboolean on_npc_shape_focus_out_event
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->show_npc_shape();
	return TRUE;
	}

/*
 *	Callback for when a shape is dropped on the NPC draw area.
 */

static void Npc_shape_dropped
	(
	int file,			// U7_SHAPE_SHAPES.
	int shape,
	int frame,
	void *udata
	)
	{
	if (file == U7_SHAPE_SHAPES && shape >= 0 && shape < 1024)
		((ExultStudio *) udata)->set_npc_shape(shape, frame);
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
			npc_draw = new Shape_draw(vgafile, palbuf,
			    glade_xml_get_widget(app_xml, "npc_draw"));
			npc_draw->enable_drop(Npc_shape_dropped, this);
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

/*
 *	Paint the shape in the NPC draw area.
 */

void ExultStudio::show_npc_shape
	(
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!npc_draw)
		return;
	npc_draw->configure();
					// Yes, this is kind of redundant...
	int shnum = get_num_entry("npc_shape");
	int frnum = get_num_entry("npc_frame");
	if (!shnum)			// Don't draw shape 0.
		shnum = -1;
	npc_draw->draw_shape_centered(shnum, frnum);
	if (w != -1)
		npc_draw->show(x, y, w, h);
	else
		npc_draw->show();
	}

/*
 *	Set NPC shape.
 */

void ExultStudio::set_npc_shape
	(
	int shape,
	int frame
	)
	{
	set_entry("npc_shape", shape);
	set_entry("npc_frame", frame);
	show_npc_shape();
	}
