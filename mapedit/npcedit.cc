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
	if (data)
		if (!init_npc_window(data, datalen))
			return;
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
 *	Init. the npc editor with data from Exult.
 *
 *	Output:	0 if error (reported).
 */

int ExultStudio::init_npc_window
	(
	unsigned char *data,
	int datalen
	)
	{
	unsigned long addr;
	int tx, ty, tz;
	int shape, frame;
	std::string name;
	short ident;
	int usecode;
	short properties[12];
	short attack_mode, alignment;
	unsigned long oflags;		// Object flags.
	unsigned long siflags;		// Extra flags for SI.
	unsigned long type_flags;	// Movement flags.
	if (!Npc_actor_in(data, datalen, addr, tx, ty, tz, shape, frame,
		name, ident, usecode, properties, attack_mode, alignment,
			oflags, siflags, type_flags))
		{
		cout << "Error decoding npc" << endl;
		return 0;
		}
					// Store address with window.
	gtk_object_set_user_data(GTK_OBJECT(npcwin), (gpointer) addr);
					// Store name, ident.
	set_entry("npc_name_entry", name.c_str());
	set_entry("npc_ident_entry", ident);
					// Shape/frame.
	set_entry("npc_shape", shape);
	set_entry("npc_frame", frame);

					// Set flag buttons.
	GtkTable *ftable = GTK_TABLE(
			glade_xml_get_widget(app_xml, "npc_flags_table"));
					// Set flag checkboxes.
	for (GList *list = g_list_first(ftable->children); list; 
						list = g_list_next(list))
		{
		GtkTableChild *ent = (GtkTableChild *) list->data;
		GtkCheckButton *cbox = GTK_CHECK_BUTTON(ent->widget);
		assert (cbox != 0);
		const char *name = glade_get_widget_name(GTK_WIDGET(cbox));
					// Names: npc_flag_xx_nn, where
					//   xx = si, of, tf.
		cout << "Flag: " << name << endl;//++++TESTING.
					// ++++Maybe make this a subroutine?
		if (strncmp(name, "npc_flag_", 9) != 0)
			continue;
		long bits = 0;		// Which flag.
		if (strncmp(name + 9, "si", 2) == 0)
			bits = siflags;
		else if (strncmp(name + 9, "of", 2) == 0)
			bits = oflags;
		else if (strncmp(name + 9, "tf", 2) == 0)
			bits = type_flags;
		else
			continue;
		int fnum = atoi(name + 9 + 3);
		if (fnum >= 0 && fnum < 32)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), 
					(bits&(1<<fnum)) != 0);
		}
//++++++++++++
	return 1;
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

/*
 *	A choice was clicked in the 'schedule' dialog.
 */

void ExultStudio::schedule_btn_clicked
	(
	GtkWidget *btn,			// Button on the schedule dialog.
	gpointer data			// Dialog itself.
	)
	{
	static char *sched_names[32] = {
		"Combat", "Horiz. Pace", "Vert. Pace", "Talk", "Dance",
		"Eat", "Farm", "Tend Shop", "Miner", "Hound", "Stand",
		"Loiter", "Wander", "Blacksmith", "Sleep", "Wait", "Sit",
		"Graze", "Bake", "Sew", "Shy", "Lab Work", "Thief", "Waiter",
		"Special", "Kid Games", "Eat at Inn", "Duel", "Preach",
		"Patrol", "Desk Work", "Follow"};
	ExultStudio *studio = ExultStudio::get_instance();
					// Get name assigned in Glade.
	const char *name = glade_get_widget_name(btn);
	const char *numptr = name + 5;	// Past "sched".
	int num = atoi(numptr);
	GtkWidget *schedwin = (GtkWidget *) data;
	GtkLabel *label = (GtkLabel *) gtk_object_get_user_data(
						GTK_OBJECT(schedwin));
					// User data = schedule #.
	gtk_object_set_user_data(GTK_OBJECT(label), (gpointer) num);
	gtk_label_set_text(label, num >= 0 && num < 32
		? sched_names[num] : "-----");
	cout << "Chose schedule " << num << endl;
	gtk_widget_hide(glade_xml_get_widget(studio->get_xml(), 
							"schedule_dialog"));
	}

/*
 *	Set signal handler for each schedule button.
 */

static void Set_sched_btn
	(
	GtkWidget *btn,
	gpointer data
	)
	{
	gtk_signal_connect(GTK_OBJECT(btn), "clicked",
		GTK_SIGNAL_FUNC(ExultStudio::schedule_btn_clicked), data);
	}

/*
 *	Npc window's "set schedule" button.
 */
extern "C" void on_npc_set_sched
	(
	GtkWidget *btn,			// One of the 'set' buttons.
	gpointer user_data
	)
	{
	static int first = 1;		// To initialize signal handlers.
	const char *name = glade_get_widget_name(btn);
	const char *numptr = name + strlen(name) - 1;
	GladeXML *xml = ExultStudio::get_instance()->get_xml();
	char lname[20];			// Set up label name.
	strcpy(lname, "npc_sched");
	strcat(lname, numptr);		// Same number as button.
	GtkLabel *label = GTK_LABEL(glade_xml_get_widget(xml, lname));
	GtkContainer *btns = GTK_CONTAINER(
				glade_xml_get_widget(xml, "sched_btns"));
	GtkWidget *schedwin = glade_xml_get_widget(xml, "schedule_dialog");
	if (!label || !btns || !schedwin)
		return;
	if (first)			// First time?  Set handlers.
		{
		first = 0;
		gtk_container_foreach(btns, Set_sched_btn, schedwin);
		}
					// Store label as dialog's data.
	gtk_object_set_user_data(GTK_OBJECT(schedwin), label);
	gtk_widget_show(schedwin);
	}

