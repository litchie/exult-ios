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
#include "utils.h"
#include "shapefile.h"
#include "shapedraw.h"

#ifdef WIN32
#include "windrag.h"
#endif

using	std::cout;
using	std::endl;
/*
 *	Open npc window.
 */

C_EXPORT void on_open_npc_activate
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
C_EXPORT void on_npc_apply_btn_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->save_npc_window();
	}

/*
 *	Npc window's Cancel button.
 */
C_EXPORT void on_npc_cancel_btn_clicked
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
C_EXPORT gboolean on_npc_window_delete_event
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
C_EXPORT gboolean on_npc_draw_expose_event
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
C_EXPORT gboolean on_npc_shape_focus_out_event
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->show_npc_shape();
	return FALSE;
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

					// Schedule names.
/*
 *	Draw face.
 */
C_EXPORT gboolean on_npc_face_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	ExultStudio::get_instance()->show_npc_face(
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Callback for when a shape is dropped on the NPC face area.
 */

static void Npc_face_dropped
	(
	int file,			// U7_SHAPE_FACES
	int shape,
	int frame,
	void *udata
	)
	{
	if (file == U7_SHAPE_FACES && shape >= 0 && shape < 1024)
		((ExultStudio *) udata)->set_npc_face(shape, frame);
	}

					// Schedule names.
static char *sched_names[32] = {
		"Combat", "Horiz. Pace", "Vert. Pace", "Talk", "Dance",
		"Eat", "Farm", "Tend Shop", "Miner", "Hound", "Stand",
		"Loiter", "Wander", "Blacksmith", "Sleep", "Wait", "Sit",
		"Graze", "Bake", "Sew", "Shy", "Lab Work", "Thief", "Waiter",
		"Special", "Kid Games", "Eat at Inn", "Duel", "Preach",
		"Patrol", "Desk Work", "Follow"};

/*
 *	Set a line in the schedule page.
 */

static void Set_schedule_line
	(
	GladeXML *app_xml,
	int time,			// 0-7.
	int type,			// Activity (0-31, or -1 for none).
	int tx, int ty, int tz = 0	// Location.
	)
	{
	char *lname = g_strdup_printf("npc_sched%d", time);
	GtkLabel *label = GTK_LABEL(glade_xml_get_widget(app_xml, lname));
	g_free(lname);
					// User data = schedule #.
	gtk_object_set_user_data(GTK_OBJECT(label), (gpointer) type);
	gtk_label_set_text(label, 
			type >= 0 && type < 32 ? sched_names[type] : "-----");
					// Set location.
	char *locname = g_strdup_printf("sched_loc%d", time);
	GtkBox *box = GTK_BOX(glade_xml_get_widget(app_xml, locname));
	g_free(locname);
	GList *list = g_list_first(box->children);
	GtkWidget *spin = ((GtkBoxChild *) list->data)->widget;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), tx);
	list = g_list_next(list);
	spin = ((GtkBoxChild *) list->data)->widget;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), ty);
	list = g_list_next(list);
	spin = ((GtkBoxChild *) list->data)->widget;
					// Current engine assumes tz==0.
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 0);
	gtk_widget_set_sensitive(spin, FALSE);
	}

/*
 *	Get info. from line in the schedule page.
 *
 *	Output:	False if schedule isn't set.
 */

static bool Get_schedule_line
	(
	GladeXML *app_xml,
	int time,			// 0-7.
	Serial_schedule& sched		// Filled in if 'true' returned.
	)
	{
	char *lname = g_strdup_printf("npc_sched%d", time);
	GtkLabel *label = GTK_LABEL(glade_xml_get_widget(app_xml, lname));
	g_free(lname);
					// User data = schedule #.
	sched.type = (int) gtk_object_get_user_data(GTK_OBJECT(label));
	if (sched.type < 0 || sched.type > 31)
		return false;
					// Get location.
	char *locname = g_strdup_printf("sched_loc%d", time);
	GtkBox *box = GTK_BOX(glade_xml_get_widget(app_xml, locname));
	g_free(locname);
	GList *list = g_list_first(box->children);
	GtkWidget *spin = ((GtkBoxChild *) list->data)->widget;
	sched.tx = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
	list = g_list_next(list);
	spin = ((GtkBoxChild *) list->data)->widget;
	sched.ty = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
//	list = g_list_next(list);
//	spin = ((GtkBoxChild *) list->data)->widget;
//	sched.tz = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
	sched.time = time;
	return true;
	}

/*
 *	Open the npc-editing window.
 */

#ifdef WIN32

static void Drop_dragged_shape(int shape, int frame, int x, int y, void *data)
{
	cout << "Dropped a shape: " << shape << "," << frame << " " << data << endl;

	Npc_shape_dropped(U7_SHAPE_SHAPES, shape, frame, data);
}

static void Drop_dragged_face(int shape, int frame, int x, int y, void *data)
{
	cout << "Dropped a face: " << shape << "," << frame << " " << data << endl;

	Npc_face_dropped(U7_SHAPE_FACES, shape, frame, data);
}

#endif

void ExultStudio::open_npc_window
	(
	unsigned char *data,		// Serialized npc, or null.
	int datalen
	)
	{
	bool first_time = false;
	if (!npcwin)			// First time?
		{
		first_time = true;
		npcwin = glade_xml_get_widget( app_xml, "npc_window" );

		if (vgafile && palbuf)
			{
			npc_draw = new Shape_draw(vgafile->get_ifile(), palbuf,
			    glade_xml_get_widget(app_xml, "npc_draw"));
			npc_draw->enable_drop(Npc_shape_dropped, this);
			}
		if (facefile && palbuf)
			{
			npc_face_draw = new Shape_draw(facefile->get_ifile(),
								palbuf,
			    glade_xml_get_widget(app_xml, "npc_face_draw"));
			npc_face_draw->enable_drop(Npc_face_dropped, this);
			}
		npc_ctx = gtk_statusbar_get_context_id(
			GTK_STATUSBAR(glade_xml_get_widget(
				app_xml, "npc_status")), "Npc Editor");
		for (int i = 0; i < 24/3; i++)	// Init. schedules' user_data.
			Set_schedule_line(app_xml, i, -1, 0, 0, 0);

		}
					// Init. npc address to null.
	gtk_object_set_user_data(GTK_OBJECT(npcwin), 0);
					// Make 'apply' sensitive.
	gtk_widget_set_sensitive(glade_xml_get_widget(app_xml, 
						"npc_apply_btn"), true);
	if (data)
		{
		if (!init_npc_window(data, datalen))
			return;
		}
	else				// Got to get what new NPC # will be.
		{
		int npc_num = -1;
		if (Send_data(server_socket, Exult_server::npc_info) != -1)
			{		// Should get immediate answer.
			unsigned char data[Exult_server::maxlength];
			Exult_server::Msg_type id;
			Exult_server::wait_for_response(server_socket, 100);
			int len = Exult_server::Receive_data(server_socket, 
						id, data, sizeof(data));
			unsigned char *ptr = &data[0];
			int npcs = Read2(ptr);
			int first_unused = Read2(ptr);
			npc_num = first_unused;
			//++++++Get data if existing unused??
			set_entry("npc_num_entry", npc_num, true, false);
					// Usually, usecode = 0x400 + num.
			set_entry("npc_usecode_entry", 0x400 + npc_num, true);
					// Usually, face = npc_num.
			set_npc_face(npc_num, 0);
			}
		}
	gtk_widget_show(npcwin);
#ifdef WIN32
	if (first_time || !npcdnd)
		Windnd::CreateStudioDropDest(npcdnd, npchwnd, Drop_dragged_shape, NULL, Drop_dragged_face, (void*) this);

#endif
	}

/*
 *	Close the npc-editing window.
 */

void ExultStudio::close_npc_window
	(
	)
	{
	if (npcwin) {
		gtk_widget_hide(npcwin);
#ifdef WIN32
		Windnd::DestroyStudioDropDest(npcdnd, npchwnd);
#endif
	}
	}

/*
 *	Get one of the NPC 'property' spin buttons from the table in the NPC
 *	dialog box.
 *
 *	Output:	true if successful, with spin, pnum returned.
 */

static bool Get_prop_spin
	(
	GList *list,			// Entry in table of properties.
	GtkSpinButton *& spin,		// Spin button returned.
	int& pnum			// Property number (0-11) returned.
	)
	{
	GtkTableChild *ent = (GtkTableChild *) list->data;
	GtkBin *frame = GTK_BIN(ent->widget);
	spin = GTK_SPIN_BUTTON(frame->child);
	assert (spin != 0);
	const char *name = glade_get_widget_name(GTK_WIDGET(spin));
					// Names: npc_prop_nn.
	if (strncmp(name, "npc_prop_", 9) != 0)
		return false;
	pnum = atoi(name + 9);
	return (pnum >= 0 && pnum < 12);
	}

/*
 *	Get one of the NPC flag checkbox's from the table in the NPC
 *	dialog box.
 *
 *	Output:	true if successful, with cbox, fnum returned.
 */

static bool Get_flag_cbox
	(
	GList *list,			// Entry in table of flags.
	unsigned long *oflags,		// Object flags.
	unsigned long *siflags,		// Serpent Isle flags.
	unsigned long *type_flags,	// Type (movement) flags.
	GtkCheckButton *& cbox,		// Checkbox returned.
	unsigned long *& bits,		// ->one of 3 flags above.
	int& fnum			// Flag # (0-31) returned.
	)
	{
	GtkTableChild *ent = (GtkTableChild *) list->data;
	cbox = GTK_CHECK_BUTTON(ent->widget);
	assert (cbox != 0);
	const char *name = glade_get_widget_name(GTK_WIDGET(cbox));
					// Names: npc_flag_xx_nn, where
					//   xx = si, of, tf.
	if (strncmp(name, "npc_flag_", 9) != 0)
		return false;
					// Which flag.
	if (strncmp(name + 9, "si", 2) == 0)
		bits = siflags;
	else if (strncmp(name + 9, "of", 2) == 0)
		bits = oflags;
	else if (strncmp(name + 9, "tf", 2) == 0)
		bits = type_flags;
	else
		return false;
	fnum = atoi(name + 9 + 3);
	return (fnum >= 0 && fnum < 32);
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
	int shape, frame, face;
	std::string name;
	short npc_num, ident;
	int usecode;
	int properties[12];
	short attack_mode, alignment;
	unsigned long oflags;		// Object flags.
	unsigned long siflags;		// Extra flags for SI.
	unsigned long type_flags;	// Movement flags.
	short num_schedules;
	Serial_schedule schedules[8];
	if (!Npc_actor_in(data, datalen, addr, tx, ty, tz, shape, frame, face,
		name, npc_num, ident, usecode, properties, 
			attack_mode, alignment,
			oflags, siflags, type_flags, num_schedules, schedules))
		{
		cout << "Error decoding npc" << endl;
		return 0;
		}
					// Store address with window.
	gtk_object_set_user_data(GTK_OBJECT(npcwin), (gpointer) addr);
					// Store name, ident, num.
	set_entry("npc_name_entry", name.c_str());
					// (Not allowed to change npc#.).
	set_entry("npc_num_entry", npc_num, true, false);
	set_entry("npc_ident_entry", ident);
					// Shape/frame.
	set_entry("npc_shape", shape);
	set_entry("npc_frame", frame);
	set_npc_face(face, 0);
					// Usecode #.
	set_entry("npc_usecode_entry", usecode, true);
					// Combat:
	set_optmenu("npc_attack_mode", attack_mode);
	set_optmenu("npc_alignment", alignment);
					// Set flag buttons.
	GtkTable *ftable = GTK_TABLE(
			glade_xml_get_widget(app_xml, "npc_flags_table"));
					// Set flag checkboxes.
	for (GList *list = g_list_first(ftable->children); list; 
						list = g_list_next(list))
		{
		GtkCheckButton *cbox;
		unsigned long *bits;
		int fnum;
		if (Get_flag_cbox(list, &oflags, &siflags, &type_flags, cbox,
						bits, fnum))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbox), 
					(*bits&(1<<fnum)) != 0);
		}
					// Set properties.
	GtkTable *ptable = GTK_TABLE(
			glade_xml_get_widget(app_xml, "npc_props_table"));
	for (GList *list = g_list_first(ptable->children); list; 
						list = g_list_next(list))
		{
		GtkSpinButton *spin;
		int pnum;
		if (Get_prop_spin(list, spin, pnum))
			gtk_spin_button_set_value(spin, properties[pnum]);
		}
					// Set schedules.
	for (int i = 0; i < 24/3; i++)	// First init. to empty.
		Set_schedule_line(app_xml, i, -1, 0, 0, 0);
	for (int i = 0; i < num_schedules; i++)
		{
		Serial_schedule& sched = schedules[i];
		int time = sched.time;	// 0-7.
		if (time < 0 || time > 7)
			continue;	// Invalid.
		Set_schedule_line(app_xml,time,sched.type, sched.tx, sched.ty);
		}
	return 1;
	}

/*
 *	Callback for when user clicked where NPC should be inserted.
 */

static void Npc_response
	(
	Exult_server::Msg_type id,
	unsigned char *data,
	int datalen,
	void * /* client */
	)
	{
	if (id == Exult_server::user_responded)
		ExultStudio::get_instance()->close_npc_window();
	//+++++cancel??
	}

/*
 *	Send updated NPC info. back to Exult.
 *
 *	Output:	0 if error (reported).
 */

int ExultStudio::save_npc_window
	(
	)
	{
	cout << "In save_npc_window()" << endl;
	unsigned char data[Exult_server::maxlength];
					// Get npc (null if creating new).
	unsigned long addr = (unsigned long) gtk_object_get_user_data(
							GTK_OBJECT(npcwin));
	int tx = -1, ty = -1, tz = -1;	// +++++For now.
	std::string name(get_text_entry("npc_name_entry"));
	short npc_num = get_num_entry("npc_num_entry");
	short ident = get_num_entry("npc_ident_entry");
	int shape = get_num_entry("npc_shape");
	int frame = get_num_entry("npc_frame");
	GtkWidget *fw = glade_xml_get_widget(app_xml, "npc_face_frame");
	int face = (int) gtk_object_get_user_data(GTK_OBJECT(fw));
	int usecode = get_num_entry("npc_usecode_entry");
	short attack_mode = get_optmenu("npc_attack_mode");
	short alignment = get_optmenu("npc_alignment");

	unsigned long oflags = 0;	// Object flags.
	unsigned long siflags = 0;	// Extra flags for SI.
	unsigned long type_flags = 0;	// Movement flags.
					// Set flag buttons.
	GtkTable *ftable = GTK_TABLE(
			glade_xml_get_widget(app_xml, "npc_flags_table"));
					// Get flags.
	for (GList *list = g_list_first(ftable->children); list; 
						list = g_list_next(list))
		{
		GtkCheckButton *cbox;
		unsigned long *bits;
		int fnum;
		if (Get_flag_cbox(list, &oflags, &siflags, &type_flags, cbox,
						bits, fnum))
			if (gtk_toggle_button_get_active(
						GTK_TOGGLE_BUTTON(cbox)))
				*bits |= (1<<fnum);
		}
	int properties[12];		// Get properties.
	GtkTable *ptable = GTK_TABLE(
			glade_xml_get_widget(app_xml, "npc_props_table"));
	for (GList *list = g_list_first(ptable->children); list; 
						list = g_list_next(list))
		{
		GtkSpinButton *spin;
		int pnum;
		if (Get_prop_spin(list, spin, pnum))
			properties[pnum] = 
				gtk_spin_button_get_value_as_int(spin);
		}
	short num_schedules = 0;
	Serial_schedule schedules[8];
	for (int i = 0; i < 8; i++)
		if (Get_schedule_line(app_xml, i, schedules[num_schedules]))
			num_schedules++;
	if (Npc_actor_out(server_socket, addr, tx, ty, tz, shape, frame, face,
		name, npc_num, ident, usecode, 
			properties, attack_mode, alignment,
			oflags, siflags, type_flags, 
			num_schedules, schedules) == -1)
		{
		cout << "Error sending npc data to server" <<endl;
		return 0;
		}
	cout << "Sent npc data to server" << endl;
	if (!addr)
		{
		set_statusbar("npc_status", npc_ctx,
					"Click on map at place to insert npc");
					// Make 'apply' insensitive.
		gtk_widget_set_sensitive(glade_xml_get_widget(app_xml, 
						"npc_apply_btn"), false);
		waiting_for_server = Npc_response;
		return 1;		// Leave window open.
		}
	close_npc_window();
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
 *	Paint the face.
 */

void ExultStudio::show_npc_face
	(
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!npc_face_draw)
		return;
	npc_face_draw->configure();
	GtkWidget *frame = glade_xml_get_widget(app_xml, "npc_face_frame");
	int shnum = (int) gtk_object_get_user_data(GTK_OBJECT(frame));
	npc_face_draw->draw_shape_centered(shnum, 0);
	if (w != -1)
		npc_face_draw->show(x, y, w, h);
	else
		npc_face_draw->show();
	}

/*
 *	Set NPC face.
 */

void ExultStudio::set_npc_face
	(
	int shape,
	int frame
	)
	{
//	set_entry("npc_shape", shape);
//	set_entry("npc_frame", frame);
	if (shape < 0)
		shape = 1;		// Default to 1st after Avatar.
	GtkWidget *widget = glade_xml_get_widget(app_xml, "npc_face_frame");
	gtk_object_set_user_data(GTK_OBJECT(widget), (gpointer) shape);
	char *label = g_strdup_printf("Face #%d", shape);
	gtk_frame_set_label(GTK_FRAME(widget), label);
	g_free(label);
	show_npc_face();
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
C_EXPORT void on_npc_set_sched
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

/*
 *	Received game position for schedule.
 */
static void Game_loc_response
	(
	Exult_server::Msg_type id,
	unsigned char *data,
	int datalen,
	void *client
	)
	{
	if (id != Exult_server::game_pos)
		return;
					// Get box with loc. spin btns.
	GtkBox *box = (GtkBox *) client;
	int tx = Read2(data);
	int ty = Read2(data);
	int tz = Read2(data);
	if (tz != 0)
		{
		EStudio::Alert("Non-zero height (%d) not yet supported", tz);
		return;
		}
	GList *list = g_list_first(box->children);
	GtkWidget *spin = ((GtkBoxChild *) list->data)->widget;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), tx);
	list = g_list_next(list);
	spin = ((GtkBoxChild *) list->data)->widget;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), ty);
	list = g_list_next(list);
	spin = ((GtkBoxChild *) list->data)->widget;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), tz);
	}

/*
 *	One of the "Game" buttons to set location from current game position.
 */

C_EXPORT void on_sched_loc_clicked
	(
	GtkWidget *btn,			// One of the 'Game' buttons.
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
					// Get location box.
	GtkBox *box = GTK_BOX(gtk_widget_get_parent(btn));
	if (Send_data(studio->get_server_socket(), 
					Exult_server::game_pos) == -1)
		cout << "Error sending message to server" << endl;
	else
		studio->set_msg_callback(Game_loc_response, box);
	}
