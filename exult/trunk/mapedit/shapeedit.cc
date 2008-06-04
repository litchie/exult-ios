/**
 **	Shapeedit.cc - Shape-editing methods.
 **
 **	Written: 6/8/01 - JSF
 **/

/*
Copyright (C) 2002 The Exult Team

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

#include <map>
#include <string>
#include "studio.h"
#include "servemsg.h"
#include "exult_constants.h"
#include "utils.h"
#include "shapeinf.h"
#include "ammoinf.h"
#include "aniinf.h"
#include "armorinf.h"
#include "bodyinf.h"
#include "continf.h"
#include "effhpinf.h"
#include "expinf.h"
#include "frnameinf.h"
#include "monstinf.h"
#include "npcdollinf.h"
#include "objdollinf.h"
#include "sfxinf.h"
#include "warminf.h"
#include "weaponinf.h"
#include "u7drag.h"
#include "shapefile.h"
#include "shapedraw.h"
#include "items.h"

using	std::cout;
using	std::endl;
using	std::string;
using	std::map;

// HP Info columns
enum
	{
	HP_FRAME_COLUMN = 0,
	HP_QUALITY_COLUMN,
	HP_HIT_POINTS,
	HP_FROM_PATCH,
	HP_MODIFIED,
	HP_COLUMN_COUNT
	};

// Warmth Info columns
enum
	{
	WARM_FRAME_COLUMN = 0,
	WARM_VALUE_COLUMN,
	WARM_FROM_PATCH,
	WARM_MODIFIED,
	WARM_COLUMN_COUNT
	};

// Obj paperdoll Info columns
enum
	{
	DOLL_WORLD_FRAME = 0,
	DOLL_SPOT,
	DOLL_TRANSLUCENT,
	DOLL_GENDER_BASED,
	DOLL_SPOT_TYPE,
	DOLL_SHAPE,
	DOLL_FRAME_0,
	DOLL_FRAME_1,
	DOLL_FRAME_2,
	DOLL_FRAME_3,
	DOLL_FROM_PATCH,
	DOLL_MODIFIED,
	DOLL_COLUMN_COUNT
	};

// Content rules columns
enum
	{
	CNT_SHAPE_COLUMN = 0,
	CNT_ACCEPT_COLUMN,
	CNT_FROM_PATCH,
	CNT_MODIFIED,
	CNT_COLUMN_COUNT
	};

// Frame name columns
enum
	{
	FNAME_FRAME = 0,
	FNAME_QUALITY,
	FNAME_MSGTYPE,
	FNAME_MSGSTR,
	FNAME_OTHERTYPE,
	FNAME_OTHERMSG,
	FNAME_FROM_PATCH,
	FNAME_MODIFIED,
	FNAME_COLUMN_COUNT
	};

/*
 *	Widgets in one row of the 'equipment' dialog:
 */
struct Equip_row_widgets
	{
	Shape_draw *draw;		// Where the shape is drawn.
	GtkWidget *shape, *name, *chance, *count;
	};
					// Holds widgets from equip. dialog.
static Equip_row_widgets equip_rows[10] = {0};

/*
 *	Equip window's Okay, Apply buttons.
 */
C_EXPORT void on_equip_okay_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->save_equip_window();
	ExultStudio::get_instance()->close_equip_window();
	}
C_EXPORT void on_equip_apply_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->save_equip_window();
	}

/*
 *	Equip window's Cancel button.
 */
C_EXPORT void on_equip_cancel_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_equip_window();
	}

/*
 *	Equip window's close button.
 */
C_EXPORT gboolean on_equip_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_equip_window();
	return TRUE;
	}

/*
 *	Record # changed, so update what's displayed.
 */
C_EXPORT gboolean on_equip_recnum_changed
	(
	GtkWidget *widget,
	gpointer user_data
	)
	{
	int recnum = gtk_spin_button_get_value_as_int(
						GTK_SPIN_BUTTON(widget));
	ExultStudio::get_instance()->init_equip_window(recnum);
	return TRUE;
	}

/*
 *	Add new record.
 */
C_EXPORT void on_equip_new_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->new_equip_record();
	}

/*
 *	Draw shape in one of the Equip dialog rows.
 */
C_EXPORT gboolean on_equip_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->row.
	)
	{
	ExultStudio::get_instance()->show_equip_shape(
		(Equip_row_widgets *) data,
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}
/*
 *	Shape # on one of the rows was changed.
 */
C_EXPORT gboolean on_equip_shape_changed
	(
	GtkWidget *widget,
	gpointer data			// ->row info.
	)
	{
	Equip_row_widgets *eq = (Equip_row_widgets *) data;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->show_equip_shape(eq);
	int shape = gtk_spin_button_get_value_as_int(
				GTK_SPIN_BUTTON(eq->shape));
	const char *nm = studio->get_shape_name(shape);
	gtk_label_set_text(GTK_LABEL(eq->name), nm ? nm : "");
	return (TRUE);
	}

/*
 *	Callback for when a shape is dropped on one of the Equip draw areas.
 */

static void Equip_shape_dropped
	(
	int file,			// U7_SHAPE_SHAPES.
	int shape,
	int frame,
	void *udata			// ->row.
	)
	{
	Equip_row_widgets *eq = (Equip_row_widgets *) udata;
	if (file == U7_SHAPE_SHAPES && shape >= 0 && shape < c_max_shapes)
		{			// Set shape #.
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(eq->shape), shape);
		}
	}

/*
 *	Set up 'equipment' dialog's table, which has 10 identical rows.
 */

static void Setup_equip
	(
	GtkTable *table,		// Table to fill.
	Vga_file *vgafile,		// Shapes.
	unsigned char *palbuf,		// Palette for drawing shapes.
	Equip_row_widgets rows[10]	// Filled in.
	)
	{
	gtk_table_resize(table, 12, 17);
	gtk_widget_show (GTK_WIDGET(table));
					// Labels at top:
	GtkWidget *label = gtk_label_new ("Shape");
	gtk_widget_show (label);
	gtk_table_attach (table, label, 0, 3, 0, 1,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new ("Chance (%)");
	gtk_widget_show (label);
	gtk_table_attach (table, label, 4, 5, 0, 1,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new ("Count");
	gtk_widget_show (label);
	gtk_table_attach (table, label, 6, 7, 0, 1,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (0), 0, 0);
					// Separators:
	GtkWidget *vsep = gtk_vseparator_new ();
	gtk_widget_show (vsep);
	gtk_table_attach (table, vsep, 3, 4, 0, 12,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (GTK_FILL), 2, 0);
	vsep = gtk_vseparator_new ();
	gtk_widget_show (vsep);
	gtk_table_attach (table, vsep, 5, 6, 0, 12,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (GTK_FILL), 2, 0);
	GtkWidget *hsep = gtk_hseparator_new ();
	gtk_widget_show (hsep);
	gtk_table_attach (table, hsep, 0, 7, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

					// Create the rows.
	for (int row = 0; row < 10; row++)
		{
					// Create frame, shape drawing area.
		GtkWidget *frame = gtk_frame_new (NULL);
		gtk_widget_show (frame);
		gtk_table_attach (table, frame, 0, 1, row + 2, row + 3,
	                      (GtkAttachOptions) (GTK_FILL),
	                      (GtkAttachOptions) (GTK_FILL), 3, 0);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

		GtkWidget *drawingarea = gtk_drawing_area_new ();
		gtk_widget_show (drawingarea);
		gtk_container_add (GTK_CONTAINER(frame), drawingarea);
		gtk_widget_set_usize (drawingarea, 20, 40);
		if (vgafile && palbuf)
			{
			rows[row].draw = new Shape_draw(vgafile, palbuf, 
								drawingarea);
			gtk_signal_connect(GTK_OBJECT(drawingarea), 
				"expose-event", 
				GTK_SIGNAL_FUNC(on_equip_draw_expose_event),
							&rows[row]);
			rows[row].draw->enable_drop(Equip_shape_dropped, 
							&rows[row]);
			}
					// Shape #:
  		GtkWidget *spin = gtk_spin_button_new (GTK_ADJUSTMENT(
			gtk_adjustment_new (1, 0, c_max_shapes-1, 1, 50, 50)), 1, 0);
		rows[row].shape = spin;
		gtk_widget_show(spin);
		gtk_table_attach (table, spin, 1, 2, row + 2, row + 3,
		                  (GtkAttachOptions) (GTK_FILL),
		                  (GtkAttachOptions) (0), 0, 0);
		gtk_signal_connect(GTK_OBJECT(spin), "changed",
				GTK_SIGNAL_FUNC(on_equip_shape_changed),
								&rows[row]);
					// Name:
		label = gtk_label_new("label1");
		rows[row].name = label;
		gtk_widget_show (label);
		gtk_table_attach (table, label, 2, 3, row + 2, row + 3,
		                  (GtkAttachOptions) (0),
		                  (GtkAttachOptions) (0), 0, 0);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_misc_set_alignment (GTK_MISC (label), 6.70552e-08, 0.5);
					// Chance:
		spin = gtk_spin_button_new(GTK_ADJUSTMENT(
			gtk_adjustment_new (1, 0, 100, 1, 10, 10)), 1, 0);
		rows[row].chance = spin;
		gtk_widget_show (spin);
		gtk_table_attach (table, spin, 4, 5, row + 2, row + 3,
		                  (GtkAttachOptions) (GTK_FILL),
		                  (GtkAttachOptions) (0), 0, 0);
					// Count:
		spin = gtk_spin_button_new (GTK_ADJUSTMENT(
			gtk_adjustment_new (1, 0, 100, 1, 10, 10)), 1, 0);
		rows[row].count = spin;
		gtk_widget_show(spin);
		gtk_table_attach (table, spin, 6, 7, row + 2, row + 3,
		                  (GtkAttachOptions) (GTK_FILL),
		                  (GtkAttachOptions) (0), 0, 0);
		}
	}

/*
 *	Set the fields to a given record.
 */

void ExultStudio::init_equip_window
	(
	int recnum			// Record # to start with (1-based).
	)
	{
					// Fill in the record.
	Equip_record& rec = Monster_info::get_equip(recnum - 1);
					// Go through rows.
	for (int row = 0; row < 10; row++)
		{
		Equip_element& elem = rec.get(row);
		Equip_row_widgets& widgets = equip_rows[row];
		int shnum = elem.get_shapenum();
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.shape), 
									shnum);
		const char *nm = get_shape_name(shnum);
		gtk_label_set_text(GTK_LABEL(widgets.name), nm);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.chance),
						elem.get_probability());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.count),
						elem.get_quantity());
		}
	}

/*
 *	Store the fields to a given record.
 */

void ExultStudio::save_equip_window
	(
	)
	{
	int recnum = get_spin("equip_recnum");
					// Get the record.
	Equip_record& rec = Monster_info::get_equip(recnum - 1);
					// Go through rows.
	for (int row = 0; row < 10; row++)
		{
		Equip_element& elem = rec.get(row);
		Equip_row_widgets& widgets = equip_rows[row];
		elem.set(gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(widgets.shape)), 
			 gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(widgets.chance)),
			 gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(widgets.count)));
		}
	shape_info_modified = true;
	}

/*
 *	Open the equip-editing window.
 */

void ExultStudio::open_equip_window
	(
	int recnum			// Record # to start with (1-based).
	)
	{
	int ecnt = Monster_info::get_equip_cnt();
	if (recnum <= 0 || recnum > ecnt)
		return;
	if (!equipwin)			// First time?
		{
		equipwin = glade_xml_get_widget( app_xml, "equip_window" );
		GtkWidget *table = glade_xml_get_widget(app_xml,
								"equip_table");
		Setup_equip(GTK_TABLE(table), vgafile->get_ifile(), 
							palbuf, equip_rows);
		}
					// This will cause the data to be set:
	set_spin("equip_recnum", recnum, 1, ecnt);
	gtk_widget_show(equipwin);
//	show_shinfo_shape();		// Be sure picture is updated.
	}

/*
 *	Close the equip window.
 */

void ExultStudio::close_equip_window
	(
	)
	{
	if (equipwin)
		gtk_widget_hide(equipwin);
	}

/*
 *	Paint the shape in one row of the Equip window draw area.
 */

void ExultStudio::show_equip_shape
	(
	Equip_row_widgets *eq,
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!eq->draw)
		return;
	eq->draw->configure();
	int shnum = gtk_spin_button_get_value_as_int(
						GTK_SPIN_BUTTON(eq->shape));

	if (!shnum)			// Don't draw shape 0.
		shnum = -1;
	eq->draw->draw_shape_centered(shnum, 0);
	if (w != -1)
		eq->draw->show(x, y, w, h);
	else
		eq->draw->show();
	}

/*
 *	Add a new equipment record.
 */

void ExultStudio::new_equip_record
	(
	)
	{
	Equip_record rec;
	Monster_info::add_equip(rec);
	int ecnt = Monster_info::get_equip_cnt();
	int recnum = ecnt;
					// Show new entry.
	set_spin("equip_recnum", recnum, 1, ecnt);
	shape_info_modified = true;
	}

/*
 *	Shape window's Okay, Apply buttons.
 */
C_EXPORT void on_shinfo_okay_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->save_shape_window();
	ExultStudio::get_instance()->close_shape_window();
	}
C_EXPORT void on_shinfo_apply_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->save_shape_window();
	}

/*
 *	Shape window's Cancel button.
 */
C_EXPORT void on_shinfo_cancel_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_shape_window();
	}

/*
 *	Shape window's close button.
 */
C_EXPORT gboolean on_shape_window_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	ExultStudio::get_instance()->close_shape_window();
	return TRUE;
	}

/*
 *	Draw shape in draw area.
 */
C_EXPORT gboolean on_shinfo_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	ExultStudio::get_instance()->show_shinfo_shape(
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Draw gump shape in container page.
 */
C_EXPORT gboolean on_shinfo_gump_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	ExultStudio::get_instance()->show_shinfo_gump(
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Gump shape # changed, so update shape displayed.
 */
C_EXPORT gboolean on_shinfo_gump_num_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->show_shinfo_gump();
	return TRUE;
	}

/*
 *	Draw gump shape in NPC paperdoll page.
 */
C_EXPORT gboolean on_shinfo_npcgump_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	ExultStudio::get_instance()->show_shinfo_npcgump(
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Gump shape # changed, so update shape displayed.
 */
C_EXPORT gboolean on_shinfo_npcpaperdoll_gump_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->show_shinfo_npcgump();
	return TRUE;
	}

/*
 *	Draw body shape in body page.
 */
C_EXPORT gboolean on_shinfo_body_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	ExultStudio::get_instance()->show_shinfo_body(
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Body shape # changed, so update shape displayed.
 */
C_EXPORT gboolean on_shinfo_body_shape_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->show_shinfo_body();
	return TRUE;
	}

/*
 *	Body frame # changed, so update shape displayed.
 */
C_EXPORT gboolean on_shinfo_body_frame_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->show_shinfo_body();
	return TRUE;
	}

/*
 *	Draw explosion sprite in explosion page.
 */
C_EXPORT gboolean on_shinfo_explosion_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	ExultStudio::get_instance()->show_shinfo_explosion(
		event->area.x, event->area.y, event->area.width,
							event->area.height);
	return (TRUE);
	}

/*
 *	Explosion sprite # changed, so update shape displayed.
 */
C_EXPORT gboolean on_shinfo_explosion_sprite_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->show_shinfo_explosion();
	return TRUE;
	}

/*
 *	Weapon ammo type changed.
 */
C_EXPORT gboolean on_shinfo_weapon_ammo_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int sel = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	bool on = sel == 0;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_weapon_ammo_shape", on);
	return (TRUE);
	}

/*
 *	Weapon projectile type changed.
 */
C_EXPORT gboolean on_shinfo_weapon_sprite_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int sel = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	bool on = sel == 0;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_weapon_proj", on);
	return (TRUE);
	}

/*
 *	Ammo projectile type changed.
 */
C_EXPORT gboolean on_shinfo_ammo_sprite_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int sel = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	bool on = sel == 0;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_ammo_proj", on);
	return (TRUE);
	}

/*
 *	Animation type changed.
 */
C_EXPORT gboolean on_shinfo_animation_type_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int sel = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	bool on = sel != (int)Animation_info::FA_HOURLY;
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_animation_ticks", on);
	studio->set_sensitive("shinfo_animation_sfxsynch", on);
	bool sfxon = !studio->get_toggle("shinfo_animation_sfxsynch");
	studio->set_sensitive("shinfo_animation_sfxdelay", on && sfxon);
	on = (sel == (int)Animation_info::FA_LOOPING);
	bool freezeon = studio->get_optmenu("shinfo_animation_freezefirst") == 2;
	studio->set_sensitive("shinfo_animation_freezefirst", on);
	studio->set_sensitive("shinfo_animation_freezechance", on && freezeon);
	studio->set_sensitive("shinfo_animation_recycle", on);
	return (TRUE);
	}

/*
 *	Animation frame count menu changed.
 */
C_EXPORT gboolean on_shinfo_animation_frtype_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_animation_frcount", on);
	return (TRUE);
	}

/*
 *	Animation SFX synchronization type changed.
 */
C_EXPORT gboolean on_shinfo_animation_sfxsynch_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_animation_sfxdelay", on);
	return (TRUE);
	}

/*
 *	Animation first frame freeze type changed.
 */
C_EXPORT gboolean on_shinfo_animation_freezefirst_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int sel = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_animation_freezechance", sel == 2);
	return (TRUE);
	}

/*
 *	Effective HPs frame type changed.
 */
C_EXPORT gboolean on_shinfo_effhps_frame_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_effhps_frame_num", on);
	return (TRUE);
	}

/*
 *	Effective HPs quality type changed.
 */
C_EXPORT gboolean on_shinfo_effhps_qual_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_effhps_qual_num", on);
	return (TRUE);
	}

/*
 *	Effective HPs HP type changed.
 */
C_EXPORT gboolean on_shinfo_effhps_hp_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_effhps_hp_val", on);
	return (TRUE);
	}

/*
 *	Frame names frame type changed.
 */
C_EXPORT gboolean on_shinfo_framenames_frame_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_framenames_frame_num", on);
	return (TRUE);
	}

/*
 *	Frame names quality type changed.
 */
C_EXPORT gboolean on_shinfo_framenames_qual_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_framenames_qual_num", on);
	return (TRUE);
	}

/*
 *	Frame name main name type changed.
 */
C_EXPORT gboolean on_shinfo_framenames_name_type_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int type = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	bool fon = type == 3;
	ExultStudio *studio = ExultStudio::get_instance();
	if (type == 0)
		studio->set_entry("shinfo_framenames_name_text",
				studio->get_text_entry("shinfo_name"), false);
	else if (type == 1)
		studio->set_entry("shinfo_framenames_name_text", "", false);
	else
		studio->set_sensitive("shinfo_framenames_name_text", true);
	if (type != 3)
		{
		studio->set_optmenu("shinfo_framenames_comp_pos", 0, false);
		studio->set_optmenu("shinfo_framenames_comp_type", 0, false);
		studio->set_optmenu("shinfo_framenames_comp_msg_type", 1, false);
		studio->set_entry("shinfo_framenames_comp_msg_text", "", false);
		}
	else
		{
		studio->set_sensitive("shinfo_framenames_comp_pos", true);
		studio->set_sensitive("shinfo_framenames_comp_type", true);
		int ot = studio->get_optmenu("shinfo_framenames_comp_type");
		if (!ot)
			studio->set_sensitive("shinfo_framenames_comp_msg_type", true);
		else
			studio->set_optmenu("shinfo_framenames_comp_msg_type", 0, false);
		ot = studio->get_optmenu("shinfo_framenames_comp_msg_type");
		studio->set_sensitive("shinfo_framenames_comp_msg_text", ot == 2);
		}
	return (TRUE);
	}

/*
 *	Framen name other kind changed.
 */
C_EXPORT gboolean on_shinfo_framenames_comp_msg_type_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int val = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_framenames_comp_msg_text", val == 2);
	return (TRUE);
	}

/*
 *	Framen name other type changed.
 */
C_EXPORT gboolean on_shinfo_framenames_comp_type_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int val = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	ExultStudio *studio = ExultStudio::get_instance();
	if (!val)
		studio->set_sensitive("shinfo_framenames_comp_msg_type", true);
	else
		studio->set_optmenu("shinfo_framenames_comp_msg_type", 0, false);
	val = studio->get_optmenu("shinfo_framenames_comp_msg_type");
	studio->set_sensitive("shinfo_framenames_comp_msg_text", val == 2);
	return (TRUE);
	}

/*
 *	Warmth frame type changed.
 */
C_EXPORT gboolean on_shinfo_warmth_frame_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_warmth_frame_num", on);
	return (TRUE);
	}

/*
 *	Content rules shape type changed.
 */
C_EXPORT gboolean on_shinfo_cntrules_shape_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_cntrules_shape_num", on);
	return (TRUE);
	}

/*
 *	Explosion SFX type changed.
 */
C_EXPORT gboolean on_shinfo_explosion_sfx_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_explosion_sfx_number", on);
	return (TRUE);
	}

/*
 *	Shape window's Equip-Edit button.
 */
C_EXPORT void on_open_equip_button_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *s = ExultStudio::get_instance();
	s->open_equip_window(s->get_spin("shinfo_monster_equip"));
	}

/*
 *	Frame # in shape-info window:
 */
C_EXPORT void
on_shinfo_frame_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
	ExultStudio *s = ExultStudio::get_instance();
	s->set_shape_notebook_frame(
				gtk_spin_button_get_value_as_int(button));
					// Force repaint of shape.
	gtk_widget_queue_draw(glade_xml_get_widget(s->get_xml(), 
							"shinfo_draw"));
}

/*
 *	Helper tree functions.
 */
template<typename T>
static inline int Find_unary_iter
	(
	GtkTreeModel *model,
	GtkTreeIter *&iter,
	T newval
	)
	{
	iter = new GtkTreeIter();
	if (!gtk_tree_model_get_iter_first(model, iter))
		{
		delete iter;
		iter = 0;
		return 1;
		}
	do
		{
		T val;
		gtk_tree_model_get(model, iter, 0, &val, -1);
		if (val == newval)
			return 0;
		else if (newval < val)
			return -1;
		} while (gtk_tree_model_iter_next(model, iter));
	return 1;
	}

template<typename T1, typename T2>
static inline int Find_binary_iter
	(
	GtkTreeModel *model,
	GtkTreeIter *&iter,
	T1 newval1,
	T2 newval2
	)
	{
	iter = new GtkTreeIter();
	if (!gtk_tree_model_get_iter_first(model, iter))
		{
		delete iter;
		iter = 0;
		return 1;
		}
	do
		{
		T1 val1;
		T2 val2;
		gtk_tree_model_get(model, iter, 0, &val1, 1, &val2, -1);
		if (val1 == newval1 && val2 == newval2)
			return 0;
		else if ((newval1 == val1 && newval2 < val2) || (newval1 < val1))
			return -1;
		} while (gtk_tree_model_iter_next(model, iter));
	return 1;
	}

/*
 *	Helper functions for effective HP data.
 */
static inline void Get_hp_fields
	(
	ExultStudio *studio,
	unsigned int& frnum,
	unsigned int& qual,
	unsigned int *hps = 0
	)
	{
	bool anyfr = studio->get_toggle("shinfo_effhps_frame_type");
	bool anyq = studio->get_toggle("shinfo_effhps_qual_type");
	bool nohps = studio->get_toggle("shinfo_effhps_hp_type");
	frnum = anyfr ? (unsigned int)(-1) :
			studio->get_spin("shinfo_effhps_frame_num");
	qual = anyq ? (unsigned int)(-1) :
			studio->get_spin("shinfo_effhps_qual_num");
	if (hps)
		*hps = nohps ? 0 : studio->get_spin("shinfo_effhps_hp_val");
	}

static inline bool Have_quality()
	{
	ExultStudio *studio = ExultStudio::get_instance();
	int c = studio->get_optmenu("shinfo_shape_class");
	return (c == 1 || c == 5 || c == 6 || c == 9 || c == 10 || c == 11);
	}

static inline void Set_hp_fields
	(
	int frnum = 0,
	int qual = 0,
	int hps = 0
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_toggle("shinfo_effhps_frame_type", frnum == -1);
	bool havequal = Have_quality();
	if (!havequal)
		qual = -1;
	studio->set_toggle("shinfo_effhps_qual_type", qual == -1, havequal);
	studio->set_toggle("shinfo_effhps_hp_type", hps == 0);
	studio->set_spin("shinfo_effhps_frame_num", frnum == -1 ? 0 : frnum, frnum != -1);
	studio->set_spin("shinfo_effhps_qual_num", qual == -1 ? 0 : qual, qual != -1);
	studio->set_spin("shinfo_effhps_hp_val", hps, hps != 0);
	}

/*
 *	Adding HP information.
 */
C_EXPORT void on_shinfo_effhps_update_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GtkTreeView *hptree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_effhps_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(hptree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	unsigned int newfrnum, newqual, newhps;
	Get_hp_fields(studio, newfrnum, newqual, &newhps);
	int cmp = Find_binary_iter(model, iter, newfrnum, newqual);
	if (cmp)
		{
		GtkTreeIter newiter;
		if (cmp > 0)
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		else
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		gtk_tree_store_set(store, &newiter, HP_FRAME_COLUMN, (int)newfrnum,
				HP_QUALITY_COLUMN, (int)newqual,
				HP_HIT_POINTS, (int)newhps,
				HP_FROM_PATCH, 1, HP_MODIFIED, 1, -1);
		}
	else
		{
		unsigned int hps;
		gtk_tree_model_get(model, iter, HP_HIT_POINTS, &hps, -1);
		if (hps != newhps)
			gtk_tree_store_set(store, iter, HP_HIT_POINTS, newhps,
					HP_MODIFIED, 1, -1);
		}
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Deleting HP information.
 */
C_EXPORT void on_shinfo_effhps_remove_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GtkTreeView *hptree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_effhps_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(hptree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	unsigned int newfrnum, newqual, newhps;
	Get_hp_fields(studio, newfrnum, newqual);
	if (!Find_binary_iter(model, iter, newfrnum, newqual))
		gtk_tree_store_remove(store, iter);
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Changed HP selection.
 */
C_EXPORT void on_shinfo_effhps_list_cursor_changed
	(
	GtkTreeView *treeview,
	gpointer     user_data
	)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;
	gtk_tree_view_get_cursor(treeview, &path, &col);
	if (!col)
		return;

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	int frnum, qual, hps;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, HP_FRAME_COLUMN, &frnum,
			HP_QUALITY_COLUMN, &qual, HP_HIT_POINTS, &hps, -1);
	Set_hp_fields(frnum, qual, hps);
}

/*
 *	Changed shape class.
 */
C_EXPORT gboolean on_shinfo_shape_class_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	if (!Have_quality())
		{
		studio->set_toggle("shinfo_effhps_qual_type", true, false);
		studio->set_spin("shinfo_effhps_qual_num", 0, false);
		studio->set_toggle("shinfo_framenames_qual_type", true, false);
		studio->set_spin("shinfo_framenames_qual_num", 0, false);
		}
	else
		{
		studio->set_sensitive("shinfo_effhps_qual_type", true);
		studio->set_sensitive("shinfo_effhps_qual_num", true);
		studio->set_sensitive("shinfo_framenames_qual_type", true);
		studio->set_sensitive("shinfo_framenames_qual_num", true);
		}
	return (TRUE);
	}

/*
 *	Helper functions for warmth data.
 */
static inline void Get_warmth_fields
	(
	ExultStudio *studio,
	unsigned int& frnum,
	unsigned int *warm = 0
	)
	{
	bool anyfr = studio->get_toggle("shinfo_warmth_frame_type");
	frnum = anyfr ? (unsigned int)(-1) :
			studio->get_spin("shinfo_warmth_frame_num");
	if (warm)
		*warm = studio->get_spin("shinfo_warmth_val");
	}

static void Set_warmth_fields
	(
	int frnum = 0,
	int warmth = 0
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_toggle("shinfo_warmth_frame_type", frnum == -1);
	studio->set_spin("shinfo_warmth_frame_num", frnum == -1 ? 0 : frnum, frnum != -1);
	studio->set_spin("shinfo_warmth_val", warmth);
	}

/*
 *	Adding warmth information.
 */
C_EXPORT void on_shinfo_warmth_update_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	unsigned int newfrnum, newwarm;
	Get_warmth_fields(studio, newfrnum, &newwarm);

	GtkTreeView *warmtree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_warmth_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(warmtree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	int cmp = Find_unary_iter(model, iter, newfrnum);
	if (cmp)
		{
		GtkTreeIter newiter;
		if (cmp > 0)
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		else
			gtk_tree_store_insert_before(store, &newiter, NULL, iter);
		gtk_tree_store_set(store, &newiter, WARM_FRAME_COLUMN, (int)newfrnum,
				WARM_VALUE_COLUMN, (int)newwarm, WARM_FROM_PATCH, 1,
				WARM_MODIFIED, 1, -1);
		}
	else
		{
		unsigned int warm;
		gtk_tree_model_get(model, iter, WARM_VALUE_COLUMN, &warm, -1);
		if (warm != newwarm)
			gtk_tree_store_set(store, iter, WARM_VALUE_COLUMN, newwarm,
					WARM_MODIFIED, 1, -1);
		}
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Deleting warmth information.
 */
C_EXPORT void on_shinfo_warmth_remove_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GtkTreeView *warmtree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_warmth_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(warmtree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	unsigned int newfrnum;
	Get_warmth_fields(studio, newfrnum);
	if (!Find_unary_iter(model, iter, newfrnum))
		gtk_tree_store_remove(store, iter);
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Changed warmth selection.
 */
C_EXPORT void on_shinfo_warmth_list_cursor_changed
	(
	GtkTreeView *treeview,
	gpointer     user_data
	)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;
	gtk_tree_view_get_cursor(treeview, &path, &col);
	if (!col)
		return;

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	int frnum, warmth;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, WARM_FRAME_COLUMN, &frnum,
			WARM_VALUE_COLUMN, &warmth, -1);
	Set_warmth_fields(frnum, warmth);
}

/*
 *	Helper functions for content rules.
 */
static inline void Get_cntrules_fields
	(
	ExultStudio *studio,
	unsigned int& shnum,
	bool *accept = false
	)
	{
	bool anysh = studio->get_toggle("shinfo_cntrules_shape_type");
	shnum = anysh ? (unsigned int)(-1) :
			studio->get_spin("shinfo_cntrules_shape_num");
	if (accept)
		*accept = studio->get_toggle("shinfo_cntrules_accept");
	}

static void Set_cntrules_fields
	(
	int shnum = 0,
	bool accept = false
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_toggle("shinfo_cntrules_shape_type", shnum == -1);
	studio->set_spin("shinfo_cntrules_shape_num", shnum == -1 ? 0 : shnum, shnum != -1);
	studio->set_toggle("shinfo_cntrules_accept", accept);
	}

/*
 *	Adding content rules.
 */
C_EXPORT void on_shinfo_cntrules_update_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	unsigned int newshnum;
	bool newaccept;
	Get_cntrules_fields(studio, newshnum, &newaccept);

	GtkTreeView *cnttree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_cntrules_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(cnttree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	int cmp = Find_unary_iter(model, iter, newshnum);
	if (cmp)
		{
		GtkTreeIter newiter;
		if (cmp > 0)
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		else
			gtk_tree_store_insert_before(store, &newiter, NULL, iter);
		gtk_tree_store_set(store, &newiter, CNT_SHAPE_COLUMN, (int)newshnum,
				CNT_ACCEPT_COLUMN, (int)newaccept, CNT_FROM_PATCH, 1,
				CNT_MODIFIED, 1, -1);
		}
	else
		{
		unsigned int accept;
		gtk_tree_model_get(model, iter, CNT_ACCEPT_COLUMN, &accept, -1);
		if (accept != newaccept)
			gtk_tree_store_set(store, iter, CNT_ACCEPT_COLUMN, newaccept,
					CNT_MODIFIED, 1, -1);
		}
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Deleting content rules.
 */
C_EXPORT void on_shinfo_cntrules_remove_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GtkTreeView *cnttree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_cntrules_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(cnttree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	unsigned int newshnum;
	Get_cntrules_fields(studio, newshnum);
	if (!Find_unary_iter(model, iter, newshnum))
		gtk_tree_store_remove(store, iter);
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Changed warmth selection.
 */
C_EXPORT void on_shinfo_cntrules_list_cursor_changed
	(
	GtkTreeView *treeview,
	gpointer     user_data
	)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;
	gtk_tree_view_get_cursor(treeview, &path, &col);
	if (!col)
		return;

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	int shnum, accept;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, CNT_SHAPE_COLUMN, &shnum,
			CNT_ACCEPT_COLUMN, &accept, -1);
	Set_cntrules_fields(shnum, accept);
}

/*
 *	Helper functions for frame names.
 */
static inline void Get_framenames_fields
	(
	ExultStudio *studio,
	unsigned int &frnum,
	unsigned int &qual,
	int *type = 0,
	const char **str = 0,
	int *othertype = 0,
	const char **othermsg = 0
	)
	{
	bool anyfr = studio->get_toggle("shinfo_framenames_frame_type");
	frnum = anyfr ? (unsigned int)(-1) :
			studio->get_spin("shinfo_framenames_frame_num");
	bool anyq = studio->get_toggle("shinfo_framenames_qual_type");
	qual = anyq ? (unsigned int)(-1) :
			studio->get_spin("shinfo_framenames_qual_num");
	if (type)
		{
		switch (studio->get_optmenu("shinfo_framenames_name_type"))
			{
			case 0:
				*type = -255;	break;
			case 1:
				*type = -1;		break;
			case 2:
				*type = 0;		break;
			default:
				{
				int b0 = studio->get_optmenu("shinfo_framenames_comp_pos") + 1,
					b1 = studio->get_optmenu("shinfo_framenames_comp_type") * 2;
				*type = b0 + b1;
				break;
				}
			}
		}
	if (str)
		*str = studio->get_text_entry("shinfo_framenames_name_text");
	if (othertype)
		{
		switch (studio->get_optmenu("shinfo_framenames_comp_msg_type"))
			{
			case 0:
				*othertype = -255;	break;
			case 1:
				*othertype = -1;	break;
			default:
				*othertype = 1;		break;
			}
		}
	if (othermsg)
		*othermsg = studio->get_text_entry("shinfo_framenames_comp_msg_text");
	}

static void Set_framenames_fields
	(
	int frnum = 0,
	int qual = 0,
	int type = 0,
	const char *str = 0,
	int othertype = 0,
	const char *othermsg = 0
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_toggle("shinfo_framenames_frame_type", frnum == -1);
	studio->set_spin("shinfo_framenames_frame_num", frnum == -1 ? 0 : frnum, frnum != -1);
	bool havequal = Have_quality();
	if (!havequal)
		qual = -1;
	studio->set_toggle("shinfo_framenames_qual_type", qual == -1, havequal);
	studio->set_spin("shinfo_framenames_qual_num", qual == -1 ? 0 : qual, qual != -1);
	studio->set_entry("shinfo_framenames_name_text", str ? str : "", type >= 0);
	if (type <= 0)
		{
		studio->set_optmenu("shinfo_framenames_name_type",
				type == -255 ? 0 : (type == -1 ? 1 : 2));
		studio->set_optmenu("shinfo_framenames_comp_pos", 0, false);
		studio->set_optmenu("shinfo_framenames_comp_type", 0, false);
		studio->set_optmenu("shinfo_framenames_comp_msg_type", 1, false);
		studio->set_entry("shinfo_framenames_comp_msg_text", "", false);
		}
	else
		{
		--type;
		int flag = (type&2)/2;
		studio->set_optmenu("shinfo_framenames_name_type", 3);
		studio->set_optmenu("shinfo_framenames_comp_pos", (type&1));
		studio->set_optmenu("shinfo_framenames_comp_type", flag);
		int compindex = !flag ? (othertype == -255 ? 0 :
				(othertype == -1 ? 1 : 2)) : 0;
		studio->set_optmenu("shinfo_framenames_comp_msg_type", compindex, !flag);
		studio->set_entry("shinfo_framenames_comp_msg_text",
			othermsg ? othermsg : 0, othertype >= 0);
		}
	}

/*
 *	Adding frame names.
 */
C_EXPORT void on_shinfo_framenames_update_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	unsigned int newfrnum, newqual;
	int newtype, newothertype;
	const char *newstr, *newothermsg;
	Get_framenames_fields(studio, newfrnum, newqual, &newtype, &newstr, 
			&newothertype, &newothermsg);

	GtkTreeView *nametree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_framenames_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(nametree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	int cmp = Find_binary_iter(model, iter, newfrnum, newqual);
	if (cmp)
		{
		GtkTreeIter newiter;
		if (cmp > 0)
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		else
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		gtk_tree_store_set(store, &newiter, FNAME_FRAME, (int)newfrnum,
				FNAME_QUALITY, (int)newqual, FNAME_MSGTYPE, newtype,
				FNAME_MSGSTR, newstr, FNAME_OTHERTYPE, newothertype,
				FNAME_OTHERMSG, newothermsg,
				FNAME_FROM_PATCH, 1, FNAME_MODIFIED, 1, -1);
		}
	else
		{
		int type, othertype;
		const char *str, *othermsg;
		gtk_tree_model_get(model, iter, FNAME_MSGTYPE, &type,
				FNAME_MSGSTR, &str, FNAME_OTHERTYPE, &othertype,
				FNAME_OTHERMSG, &othermsg, -1);
		if (type != newtype || othertype != newothertype
				|| !strcmp(str, newstr) || !strcmp(othermsg, newothermsg))
			gtk_tree_store_set(store, iter, FNAME_FRAME, (int)newfrnum,
					FNAME_QUALITY, (int)newqual, FNAME_MSGTYPE, newtype,
					FNAME_MSGSTR, newstr, FNAME_OTHERTYPE, newothertype,
					FNAME_OTHERMSG, newothermsg,
					FNAME_MODIFIED, 1, -1);
		}
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Deleting frame name.
 */
C_EXPORT void on_shinfo_framenames_remove_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GtkTreeView *nametree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_framenames_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(nametree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	unsigned int newfrnum, newqual;
	Get_framenames_fields(studio, newfrnum, newqual);
	if (!Find_binary_iter(model, iter, newfrnum, newqual))
		gtk_tree_store_remove(store, iter);
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Changed frame name selection.
 */
void on_shinfo_framenames_list_cursor_changed
	(
	GtkTreeView *treeview,
	gpointer     user_data
	)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;
	gtk_tree_view_get_cursor(treeview, &path, &col);
	if (!col)
		return;

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	int frnum, qual, type, othertype;
	const char *str, *othermsg;
	gtk_tree_model_get(model, &iter, FNAME_FRAME, &frnum,
			FNAME_QUALITY, &qual, FNAME_MSGTYPE, &type,
			FNAME_MSGSTR, &str, FNAME_OTHERTYPE, &othertype,
			FNAME_OTHERMSG, &othermsg, -1);
	Set_framenames_fields(frnum, qual, type, str, othertype, othermsg);
}

/*
 *	Helper functions for object paperdoll data.
 */
static inline void Get_objpaperdoll_fields
	(
	ExultStudio *studio,
	unsigned int& frnum,
	int& spot,
	int *shape = 0,
	int *frame0 = 0,
	int *frame1 = 0,
	int *frame2 = 0,
	int *frame3 = 0,
	int *type = 0,
	bool *trans = 0,
	bool *gender = 0
	)
	{
	int ready = studio->get_optmenu("shinfo_ready_spot");
	bool fron = ready == 3 || ready == 9 || ready == 10 || ready == 11;
	spot = studio->get_optmenu("shinfo_objpaperdoll_spot");
	int max;
	if (spot == 0 && ready == 4)
		max = 1;
	else if (spot == 3 && ready == 2)
		max = 2;
	else
		max = 0;
	spot = spot == 18 ? 102 : spot;

	bool anyfr = studio->get_toggle("shinfo_objpaperdoll_wframe_type");
	frnum = anyfr ? (unsigned int)(-1) :
			studio->get_spin("shinfo_objpaperdoll_wframe");
	if (shape)
		*shape = studio->get_spin("shinfo_objpaperdoll_shape");
	if (frame0)
		*frame0 = studio->get_spin("shinfo_objpaperdoll_frame0");
	if (frame1)
		*frame1 = fron ? studio->get_spin("shinfo_objpaperdoll_frame1") : -1;
	if (frame2)
		*frame2 = fron ? studio->get_spin("shinfo_objpaperdoll_frame2") : -1;
	if (frame3)
		*frame3 = ready == 9 ? studio->get_spin("shinfo_objpaperdoll_frame3") : -1;
	if (type)
		*type = max > 0 ? studio->get_spin("shinfo_objpaperdoll_spotframe") : 0;
	if (trans)
		*trans = studio->get_toggle("shinfo_objpaperdoll_trans");
	if (gender)
		*gender = studio->get_toggle("shinfo_objpaperdoll_gender");
	}

static void Set_objpaperdoll_fields
	(
	int frnum = 0, int spot = 0,
	bool trans = false, bool gender = false, short type = 0,
	int shape = 0,
	int frame0 = 0, int frame1 = 0, int frame2 = 0, int frame3 = 0
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	int ready = studio->get_optmenu("shinfo_ready_spot");
	bool fron = ready == 3 || ready == 9 || ready == 10 || ready == 11;
	int max;
	if (spot == 0 && ready == 4)
		max = 1;
	else if (spot == 3 && ready == 2)
		max = 2;
	else
		max = 0;
	studio->set_sensitive("shinfo_objpaperdoll_spotframe", max > 0);
	if (type > max)
		type = 0;
	studio->set_toggle("shinfo_objpaperdoll_wframe_type", frnum == -1);
	studio->set_spin("shinfo_objpaperdoll_wframe",
			frnum == -1 ? 0 : frnum, frnum != -1);
	studio->set_optmenu("shinfo_objpaperdoll_spot", spot == 102 ? 18 : spot);
	studio->set_toggle("shinfo_objpaperdoll_trans", trans != 0);
	studio->set_toggle("shinfo_objpaperdoll_gender", gender != 0);
	studio->set_spin("shinfo_objpaperdoll_spotframe", type, 0, max);
	studio->set_spin("shinfo_objpaperdoll_shape", shape);
	studio->set_spin("shinfo_objpaperdoll_frame0", frame0);
	studio->set_spin("shinfo_objpaperdoll_frame1",
			fron ? frame1 : 0, fron);
	studio->set_spin("shinfo_objpaperdoll_frame2",
			fron ? frame2 : 0, fron);
	studio->set_spin("shinfo_objpaperdoll_frame3",
			ready == 9 ? frame3 : 0, ready == 9);
	}

static inline void Set_objpaperdoll_sensitivity
	(
	ExultStudio *studio,
	int spot,
	int ready
	)
	{
	int max;
	if (spot == 0 && ready == 4)
		max = 1;
	else if (spot == 3 && ready == 2)
		max = 2;
	else
		max = 0;
	studio->set_spin("shinfo_objpaperdoll_spotframe", 0, max);
	studio->set_sensitive("shinfo_objpaperdoll_spotframe", max > 0);
	bool fron = ready == 3 || ready == 9 || ready == 10 || ready == 11;
	studio->set_sensitive("shinfo_objpaperdoll_frame1", fron);
	studio->set_sensitive("shinfo_objpaperdoll_frame2", fron);
	studio->set_sensitive("shinfo_objpaperdoll_frame3", ready == 9);
	}

/*
 *	Paperdoll selection changed.
 */
C_EXPORT void on_shinfo_objpaperdoll_list_cursor_changed
	(
	GtkTreeView *treeview,
	gpointer     user_data
	)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeViewColumn *col;
	gtk_tree_view_get_cursor(treeview, &path, &col);
	if (!col)
		return;

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	int frnum, spot, trans, gender, type, shape,
		frame0, frame1, frame2, frame3, patch;
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	gtk_tree_model_get(model, &iter, DOLL_WORLD_FRAME, &frnum,
			DOLL_SPOT, &spot,
			DOLL_TRANSLUCENT, &trans,
			DOLL_GENDER_BASED, &gender,
			DOLL_SPOT_TYPE, &type,
			DOLL_SHAPE, &shape,
			DOLL_FRAME_0, &frame0,
			DOLL_FRAME_1, &frame1,
			DOLL_FRAME_2, &frame2,
			DOLL_FRAME_3, &frame3, -1);
	Set_objpaperdoll_fields(frnum, spot, trans, gender, type, shape,
			frame0, frame1, frame2, frame3);
}

/*
 *	Paperdoll world frame type changed.
 */
C_EXPORT gboolean on_shinfo_objpaperdoll_frame_type_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio *studio = ExultStudio::get_instance();
	studio->set_sensitive("shinfo_objpaperdoll_wframe", on);
	return (TRUE);
	}

/*
 *	Adding paperdoll information.
 */
C_EXPORT void on_shinfo_objpaperdoll_update_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	unsigned int newfrnum;
	int newspot, newshape, newframe0, newframe1, newframe2, newframe3, newtype;
	bool newtrans, newgender;
	Get_objpaperdoll_fields(studio, newfrnum, newspot, &newshape, &newframe0,
			&newframe1, &newframe2, &newframe3, &newtype, &newtrans, &newgender);

	GtkTreeView *dolltree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_objpaperdoll_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(dolltree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	int cmp = Find_binary_iter(model, iter, newfrnum, newspot);
	if (cmp)
		{
		GtkTreeIter newiter;
		if (cmp > 0)
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		else
			gtk_tree_store_insert_after(store, &newiter, NULL, iter);
		gtk_tree_store_set(store, &newiter,
				DOLL_WORLD_FRAME, (int)newfrnum, DOLL_SPOT, newspot,
				DOLL_TRANSLUCENT, newtrans, DOLL_GENDER_BASED, newgender,
				DOLL_SPOT_TYPE, newtype, DOLL_SHAPE, newshape,
				DOLL_FRAME_0, newframe0, DOLL_FRAME_1, newframe1,
				DOLL_FRAME_2, newframe2, DOLL_FRAME_3, newframe3,
				DOLL_FROM_PATCH, 1, DOLL_MODIFIED, 1, -1);
		}
	else
		{
		int trans, gender, type, shape,
			frame0, frame1, frame2, frame3, patch, modded;
		gtk_tree_model_get(model, iter, DOLL_TRANSLUCENT, &trans,
				DOLL_GENDER_BASED, &gender, DOLL_SPOT_TYPE, &type,
				DOLL_SHAPE, &shape, DOLL_FRAME_0, &frame0,
				DOLL_FRAME_1, &frame1, DOLL_FRAME_2, &frame2,
				DOLL_FRAME_3, &frame3, DOLL_FROM_PATCH, &patch,
				DOLL_MODIFIED, &modded, -1);
		if ((trans != newtrans) || (gender != newgender)
				|| (type != newtype) || (shape != newshape)
				|| (frame0 != newframe0) || (frame1 != newframe1)
				|| (frame2 != newframe2) || (frame3 != newframe3))
			gtk_tree_store_set(store, iter,
					DOLL_TRANSLUCENT, newtrans, DOLL_GENDER_BASED, newgender,
					DOLL_SPOT_TYPE, newtype, DOLL_SHAPE, newshape,
					DOLL_FRAME_0, newframe0, DOLL_FRAME_1, newframe1,
					DOLL_FRAME_2, newframe2, DOLL_FRAME_3, newframe3,
					DOLL_MODIFIED, 1, -1);
		}
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Deleting object paperdoll information.
 */
C_EXPORT void on_shinfo_objpaperdoll_remove_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GtkTreeView *hptree = GTK_TREE_VIEW(
			glade_xml_get_widget(studio->get_xml(), "shinfo_objpaperdoll_list"));
	GtkTreeModel *model = gtk_tree_view_get_model(hptree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter *iter;
	unsigned int newfrnum;
	int newspot;
	Get_objpaperdoll_fields(studio, newfrnum, newspot);
	if (!Find_binary_iter(model, iter, newfrnum, newspot))
		gtk_tree_store_remove(store, iter);
	if (iter)
		gtk_tree_iter_free(iter);
	}

/*
 *	Paperdoll spot changed.
 */
C_EXPORT gboolean on_shinfo_objpaperdoll_spot_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int spot = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	ExultStudio *studio = ExultStudio::get_instance();
	int ready = studio->get_optmenu("shinfo_ready_spot");
	Set_objpaperdoll_sensitivity(studio, spot, ready);
	return (TRUE);
	}

/*
 *	Ready spot changed.
 */
C_EXPORT gboolean on_shinfo_ready_spot_changed
	(
	GtkWidget *widget,
	gpointer data
	)
	{
	GtkWidget *menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
	GtkWidget *active = gtk_menu_get_active(GTK_MENU(menu));
	int ready = g_list_index(GTK_MENU_SHELL(menu)->children, active);
	ExultStudio *studio = ExultStudio::get_instance();
	int spot = studio->get_optmenu("shinfo_objpaperdoll_spot");
	Set_objpaperdoll_sensitivity(studio, spot, ready);
	return (TRUE);
	}

/*
 *	Special-shapes toggles:
 */
C_EXPORT void on_shinfo_weapon_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_weapon_box", on);
	}
C_EXPORT void on_shinfo_ammo_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_ammo_box", on);
	}
C_EXPORT void on_shinfo_armor_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_armor_box", on);
	}
C_EXPORT void on_shinfo_monster_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_monster_box", on);
	}
C_EXPORT void on_shinfo_container_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_container_box", on);
	}
C_EXPORT void on_shinfo_body_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_coffin", on);
	}
C_EXPORT void on_shinfo_animation_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_animation_box", on);
	}
C_EXPORT void on_shinfo_explosion_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_explosion_box", on);
	}
C_EXPORT void on_shinfo_effhps_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_effhps_box", on);
	}
C_EXPORT void on_shinfo_npcflags_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_npcflags_box", on);
	}
C_EXPORT void on_shinfo_sound_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_sfx_box", on);
	}
C_EXPORT void on_shinfo_warmth_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_warmth_box", on);
	}
C_EXPORT void on_shinfo_cntrules_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_cntrules_box", on);
	}
C_EXPORT void on_shinfo_framenames_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_framenames_box", on);
	}
C_EXPORT void on_shinfo_npcpaperdoll_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_npcpaperdoll_box", on);
	}
C_EXPORT void on_shinfo_objpaperdoll_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_visible("shinfo_objpaperdoll_box", on);
	}
C_EXPORT void on_shinfo_ammo_special_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_sensitive("shinfo_ammo_drop", !on);
	}
C_EXPORT void on_shinfo_single_sfx_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_sensitive("shinfo_sfx_type", on);
	ExultStudio::get_instance()->set_sensitive("shinfo_sfx_count", on);
	}
C_EXPORT void on_shinfo_sfx_clock_check_toggled
	(
	GtkToggleButton *btn,
	gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_sensitive("shinfo_sfx_clock_sfx", on);
	}

/*
 *	Browse for usecode.
 */
C_EXPORT void on_shinfo_weapon_uc_browse_clicked          
	(
	GtkButton *button,
	gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	const char *uc = studio->browse_usecode(true);
	if (*uc)
		studio->set_entry("shinfo_weapon_uc", uc, true);
	}

/*
 *	Callback for when a gump is dropped in the container page's draw area.
 */

static void Gump_shape_dropped
	(
	int file,
	int shape,
	int frame,
	void *udata
	)
	{
	if (file == U7_SHAPE_GUMPS && shape >= 0)
		{			// Set shape #.
		ExultStudio::get_instance()->set_spin(
						"shinfo_gump_num", shape);
		}
	}

/*
 *	Callback for when a gump is dropped in the NPC paperdoll page's draw area.
 */

static void NPC_Gump_shape_dropped
	(
	int file,
	int shape,
	int frame,
	void *udata
	)
	{
	if (file == U7_SHAPE_GUMPS && shape >= 0)
		{			// Set shape #.
		ExultStudio::get_instance()->set_spin(
						"shinfo_npcpaperdoll_gump", shape);
		}
	}

/*
 *	Callback for when a body is dumped in the body page's draw area.
 */

static void Body_shape_dropped
	(
	int file,
	int shape,
	int frame,
	void *udata
	)
	{
	if (file == U7_SHAPE_SHAPES && shape >= 0)
		{			// Set shape #.
		ExultStudio::get_instance()->set_spin(
						"shinfo_body_shape", shape);
		ExultStudio::get_instance()->set_spin(
						"shinfo_body_frame", frame);
		}
	}

/*
 *	Callback for when a sprite is dropped in the explosion page's draw area.
 */

static void Explosion_shape_dropped
	(
	int file,
	int shape,
	int frame,
	void *udata
	)
	{
	if (file == U7_SHAPE_SPRITES && shape >= 0)
		{			// Set shape #.
		ExultStudio::get_instance()->set_spin(
						"shinfo_explosion_sprite", shape);
		}
	}
/*
 *	Set frame-dependent fields in the shape-editing notebook.
 */

void ExultStudio::set_shape_notebook_frame
	(
	int frnum			// Frame # to set.
	)
	{
	Shape_file_info *file_info = (Shape_file_info *)
		gtk_object_get_data(GTK_OBJECT(shapewin), "file_info");
	int shnum = get_num_entry("shinfo_shape");
	Vga_file *ifile = file_info->get_ifile();
	Shape_frame *shape = ifile->get_shape(shnum, frnum);
	set_spin("shinfo_originx", shape->get_xright(), -shape->get_width(),
							shape->get_width());
	set_spin("shinfo_originy", shape->get_ybelow(), -shape->get_height(),
							shape->get_height());

	Shape_info *info = (Shape_info *)
			gtk_object_get_user_data(GTK_OBJECT(shapewin));
	if (!info)
		return;
	set_spin("shinfo_xtiles", info->get_3d_xtiles(frnum));
	set_spin("shinfo_ytiles", info->get_3d_ytiles(frnum));
	unsigned char wx, wy;		// Weapon-in-hand offset.
	info->get_weapon_offset(frnum, wx, wy);
	set_spin("shinfo_wihx", wx, 0, 255);	// Negative???
	set_spin("shinfo_wihy", wy, 0, 255);
	}

inline short get_bg_spots(short spot)
	{
	switch (spot)
		{
		case 0x00: return 0;
		case 0x01: return 1;
		case 0x14: return 2;
		case 0x08: return 3;
		case 0x09: return 4;
		case 0x04: return 6;
		case 0x05: return 9;
		case 0x15: return 10;
		case 0x06: return 11;
		case 0x0A: return 13;
		case 0x0B: return 14;
		case 0x12: return 15;
		case 0x0F: return 16;
			// For Usecode Container in BG:
		case 0x07: return 19;
		default: return 0;
		}
	}

inline short get_si_spots(short spot)
	{
	switch (spot)
		{
		case 0x00: return 0;
		case 0x01: return 1;
		case 0x14: return 2;
		case 0x0A: return 3;
		case 0x04: return 4;
		case 0x09: return 5;
		case 0x03: return 6;
		case 0x02: return 7;
		case 0x0F: return 8;
		case 0x0C: return 9;
		case 0x05: return 10;
		case 0x08: return 11;
		case 0x0B: return 12;
		case 0x0E: return 13;
		case 0x0D: return 14;
		case 0x06: return 15;
		default: return 0;
		}
	}

/*
 *	Fill in the shape-editing notebook.
 */

void ExultStudio::init_shape_notebook
	(
	Shape_info& info,
	GtkWidget *book,		// The notebook.
	int shnum,			// Shape #.
	int frnum			// Frame #.
	)
	{
	static int classes[] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 
							9, 10, 11, 12, 0};
	const int numclasses = sizeof(classes)/sizeof(classes[0]);
	int shclass = (unsigned int) info.get_shape_class();
	set_optmenu("shinfo_shape_class", shclass < numclasses ?
					classes[shclass] : 0);
	set_shape_notebook_frame(frnum);
	set_spin("shinfo_ztiles", info.get_3d_height());
	int spot = info.get_ready_type();
	if (game_type == BLACK_GATE)
		spot = get_bg_spots(spot);
	else
		spot = get_si_spots(spot);
	set_optmenu("shinfo_ready_spot", spot);
	set_toggle("shinfo_is_spell_check", info.is_spell());
	
	set_spin("shinfo_weight", info.get_weight(), 0, 255);
	set_spin("shinfo_volume", info.get_volume(), 0, 255);
					// Bunch of flags:
	set_toggle("shinfo_sfx_check", info.has_sfx());
	set_toggle("shinfo_strange_check", info.has_strange_movement());
	set_toggle("shinfo_animated_check", info.is_animated());
	set_toggle("shinfo_solid_check", info.is_solid());
	set_toggle("shinfo_water_check", info.is_water());
			// ++++Set poison for flats, field for non-flats.
	set_toggle("shinfo_poison_check", info.is_poisonous());
	set_toggle("shinfo_field_check", info.is_field());
	set_toggle("shinfo_door_check", info.is_door());
	set_toggle("shinfo_barge_check", info.is_barge_part());
	set_toggle("shinfo_transp_check", info.is_transparent());
	set_toggle("shinfo_light_check", info.is_light_source());
	set_toggle("shinfo_transl_check", info.has_translucency());
	set_toggle("shinfo_obstaclex_check", info.is_xobstacle());
	set_toggle("shinfo_obstacley_check", info.is_yobstacle());
	set_toggle("shinfo_occludes_check", info.occludes());
					// More flags, originally hard-coded:
	static char *shpflags[] = {
				"shinfo_shape_flag0",
				"shinfo_shape_flag1",
				"shinfo_shape_flag2",
				"shinfo_shape_flag3",
				"shinfo_shape_flag4",
				"shinfo_shape_flag5",
				"shinfo_shape_flag6" };
	set_bit_toggles(&shpflags[0], 
		sizeof(shpflags)/sizeof(shpflags[0]), info.get_shape_flags());
					// Extras.
	Weapon_info *winfo = info.get_weapon_info();
	set_toggle("shinfo_weapon_check", winfo != 0);
	set_visible("shinfo_weapon_box", winfo != 0);
	if (winfo)			// Setup weapon page.
		{
		set_spin("shinfo_weapon_damage", winfo->get_damage());
		set_spin("shinfo_weapon_range", winfo->get_range());
		set_optmenu("shinfo_weapon_type", winfo->get_damage_type());
		int ammo = winfo->get_ammo_consumed();
		int ammo_use = ammo >= 0 ? 0 : -ammo;
		set_optmenu("shinfo_weapon_ammo", ammo_use);
		set_spin("shinfo_weapon_ammo_shape", ammo >= 0 ? ammo : 0, ammo >= 0);
		int proj = winfo->get_projectile();
		int proj_type = proj >= 0 ? 0 : (proj == -1 ? 1 : 2);
		set_optmenu("shinfo_weapon_sprite", proj_type);
		set_spin("shinfo_weapon_proj", proj >= 0 ? proj : 0, proj >= 0);
		set_optmenu("shinfo_weapon_uses", winfo->get_uses());
		set_spin("shinfo_weapon_sfx", winfo->get_sfx(), 0, 255);
		set_spin("shinfo_weapon_hitsfx", winfo->get_hitsfx(), 0, 255);
					// Show usecode in hex.
		set_entry("shinfo_weapon_uc", winfo->get_usecode(), true);
		static char *powers[] = {
					"shinfo_weapon_pow0",
					"shinfo_weapon_pow1",
					"shinfo_weapon_pow2",
					"shinfo_weapon_pow3",
					"shinfo_weapon_pow4",
					"shinfo_weapon_pow5",
					"shinfo_weapon_pow6",
					"shinfo_weapon_pow7" };
		set_bit_toggles(&powers[0], 
			sizeof(powers)/sizeof(powers[0]), winfo->get_powers());
		set_toggle("shinfo_weapon_flag0", winfo->lucky());
		set_toggle("shinfo_weapon_flag1", winfo->explodes());
		set_toggle("shinfo_weapon_flag2", winfo->no_blocking());
		set_toggle("shinfo_weapon_flag3", winfo->delete_depleted());
		set_toggle("shinfo_weapon_always", winfo->autohits());
		set_toggle("shinfo_weapon_target", winfo->needs_target(), false);
		set_toggle("shinfo_weapon_returns", winfo->returns());
		set_optmenu("shinfo_weapon_frames", winfo->get_actor_frames(false));
		set_optmenu("shinfo_proj_frames", winfo->get_actor_frames(true));
		set_spin("shinfo_proj_rotation", winfo->get_rotation_speed());
		set_optmenu("shinfo_proj_speed", winfo->get_missile_speed()-1);
		}
	Ammo_info *ainfo = info.get_ammo_info();
	set_toggle("shinfo_ammo_check", ainfo != 0);
	set_visible("shinfo_ammo_box", ainfo != 0);
	if (ainfo)			// Setup ammo page.
		{
		set_spin("shinfo_ammo_damage", ainfo->get_damage());
		set_spin("shinfo_ammo_family", ainfo->get_family_shape());
		int proj = ainfo->get_sprite_shape();
		int proj_type = proj >= 0 ? 0 : (proj == -1 ? 1 : 2);
		set_optmenu("shinfo_ammo_sprite", proj_type);
		set_spin("shinfo_ammo_proj", proj >= 0 ? proj : 0, proj >= 0);
		set_optmenu("shinfo_ammo_type", ainfo->get_damage_type());
		static char *powers[] = {
					"shinfo_ammo_pow0",
					"shinfo_ammo_pow1",
					"shinfo_ammo_pow2",
					"shinfo_ammo_pow3",
					"shinfo_ammo_pow4",
					"shinfo_ammo_pow5",
					"shinfo_ammo_pow6",
					"shinfo_ammo_pow7" };
		set_bit_toggles(&powers[0], 
			sizeof(powers)/sizeof(powers[0]), ainfo->get_powers());
					// 'Explode'???
		set_toggle("shinfo_ammo_flag0", ainfo->lucky());
		set_toggle("shinfo_ammo_flag1", ainfo->autohits());
		set_toggle("shinfo_ammo_flag2", ainfo->returns());
		set_toggle("shinfo_ammo_flag3", ainfo->no_blocking());
		set_toggle("shinfo_ammo_flag6", ainfo->explodes());
		set_toggle("shinfo_ammo_special", ainfo->is_homing());
		set_optmenu("shinfo_ammo_drop", ainfo->get_drop_type(), !ainfo->is_homing());
		}
	Armor_info *arinfo = info.get_armor_info();
	set_toggle("shinfo_armor_check", arinfo != 0);
	set_visible("shinfo_armor_box", arinfo != 0);
	if (arinfo)			// Setup armor page.
		{
		static char *immun[] = {"shinfo_armor_immun0",
					"shinfo_armor_immun1",
					"shinfo_armor_immun2",
					"shinfo_armor_immun3",
					"shinfo_armor_immun4",
					"shinfo_armor_immun5",
					"shinfo_armor_immun6",
					"shinfo_armor_immun7" };
		set_spin("shinfo_armor_value", arinfo->get_prot());
		set_bit_toggles(&immun[0], 
			sizeof(immun)/sizeof(immun[0]), arinfo->get_immune());
		}
	unsigned char aflags = info.get_actor_flags();
	set_toggle("shinfo_npcflags_check", aflags != 0);
	set_visible("shinfo_npcflags_box", aflags != 0);
	if (aflags)
		{
		static char *flags[] = {	"shinfo_actor_flag0",
					"shinfo_actor_flag1",
					"shinfo_actor_flag2",
					"shinfo_actor_flag3",
					"shinfo_actor_flag4" };
		set_bit_toggles(&flags[0], sizeof(flags)/sizeof(flags[0]), aflags);
		}
	Monster_info *minfo = info.get_monster_info();
	set_toggle("shinfo_monster_check", minfo != 0);
	set_visible("shinfo_monster_box", minfo != 0);
	if (minfo)			// Setup monster page.
		{
		set_spin("shinfo_monster_str", minfo->get_strength());
		set_spin("shinfo_monster_dex", minfo->get_dexterity());
		set_spin("shinfo_monster_intel", minfo->get_intelligence());
		set_spin("shinfo_monster_cmb", minfo->get_combat());
		set_spin("shinfo_monster_armor", minfo->get_armor());
		set_spin("shinfo_monster_wpn", minfo->get_weapon());
		set_spin("shinfo_monster_reach", minfo->get_reach());
		set_spin("shinfo_monster_equip", minfo->get_equip_offset());
		set_spin("shinfo_monster_sfx", minfo->get_hitsfx());
		set_spin("shinfo_monster_food", info.get_monster_food());
		set_optmenu("shinfo_monster_align", minfo->get_alignment());
		set_optmenu("shinfo_monster_attackmode", minfo->get_attackmode());
		static char *vuln[] = {	"shinfo_monster_vuln0",
					"shinfo_monster_vuln1",
					"shinfo_monster_vuln2",
					"shinfo_monster_vuln3",
					"shinfo_monster_vuln4",
					"shinfo_monster_vuln5",
					"shinfo_monster_vuln6",
					"shinfo_monster_vuln7" };
		static char *immun[] = {"shinfo_monster_immun0",
					"shinfo_monster_immun1",
					"shinfo_monster_immun2",
					"shinfo_monster_immun3",
					"shinfo_monster_immun4",
					"shinfo_monster_immun5",
					"shinfo_monster_immun6",
					"shinfo_monster_immun7" };
		set_bit_toggles(&vuln[0], sizeof(vuln)/sizeof(vuln[0]),
						minfo->get_vulnerable());
		set_bit_toggles(&immun[0], 
			sizeof(immun)/sizeof(immun[0]), minfo->get_immune());
		set_toggle("shinfo_monster_splits", minfo->splits());
		set_toggle("shinfo_monster_cant_yell", minfo->cant_yell());
		set_toggle("shinfo_monster_cant_bleed", minfo->cant_bleed());
		set_toggle("shinfo_monster_sleep_safe", minfo->sleep_safe());
		set_toggle("shinfo_monster_charm_safe", minfo->charm_safe());
		set_toggle("shinfo_monster_curse_safe", minfo->curse_safe());
		set_toggle("shinfo_monster_paralysis_safe", minfo->paralysis_safe());
		set_toggle("shinfo_monster_poison_safe", minfo->poison_safe());
		set_toggle("shinfo_monster_death_safe", minfo->death_safe());
		set_toggle("shinfo_monster_power_safe", minfo->power_safe());
		set_toggle("shinfo_monster_cant_die", minfo->cant_die());
		static char *flags[] = {"shinfo_monster_flag0",
					"shinfo_monster_flag1",
					"shinfo_monster_flag2",
					"shinfo_monster_flag3",
					"shinfo_monster_flag4",
					"shinfo_monster_flag5",
					"shinfo_monster_flag6",
					"shinfo_monster_flag7" };
		set_bit_toggles(&flags[0],
			sizeof(flags)/sizeof(flags[0]), minfo->get_flags());
		}
	int container_gump = info.get_container_gump();
	set_toggle("shinfo_container_check", container_gump >= 0);
	set_visible("shinfo_container_box", container_gump >= 0);
	set_spin("shinfo_gump_num", container_gump >= 0 ? container_gump : 0);

	bool hasbody = info.has_body_info();
	set_toggle("shinfo_body_check", hasbody);
	set_visible("shinfo_coffin", hasbody);
	if (hasbody)
		{
		int shnum = info.get_body_shape();
		set_spin("shinfo_body_shape", shnum);
		set_spin("shinfo_body_frame", info.get_body_frame(),
				0, vgafile->get_ifile()->get_num_frames(shnum)-1);
		}
	bool hasexp = info.has_explosion_info();
	set_toggle("shinfo_explosion_check", hasexp);
	set_visible("shinfo_explosion_box", hasexp);
	if (hasexp)
		{
		set_spin("shinfo_explosion_sprite", info.get_explosion_sprite());
		int sfxnum = info.get_explosion_sfx();
		if (sfxnum < 0)
			{
			set_toggle("shinfo_explosion_sfx_type", true);
			set_spin("shinfo_explosion_sfx_number", 0, false);
			}
		else
			{
			set_toggle("shinfo_explosion_sfx_type", false);
			set_spin("shinfo_explosion_sfx_number", sfxnum, true);
			}
		}
	SFX_info *sfxinf = info.get_sfx_info();
	set_toggle("shinfo_sound_check", sfxinf != 0);
	set_visible("shinfo_sfx_box", sfxinf != 0);
	if (sfxinf)
		{
		int range = sfxinf->get_sfx_range();
		set_toggle("shinfo_single_sfx", range == 1);
		set_spin("shinfo_sfx_first", sfxinf->get_sfx());
		set_spin("shinfo_sfx_chance", sfxinf->get_chance());
		set_optmenu("shinfo_sfx_type", sfxinf->play_randomly(), range != 1);
		set_spin("shinfo_sfx_count", range, range != 1);
		bool hourly = sfxinf->play_horly_ticks();
		set_toggle("shinfo_sfx_clock_check", hourly);
		set_spin("shinfo_sfx_clock_sfx", sfxinf->get_extra_sfx(), hourly);
		}
	set_sensitive("shinfo_npcpaperdoll_gump", game_type == BLACK_GATE);
	set_sensitive("shinfo_npcgump_draw", game_type == BLACK_GATE);
	Paperdoll_npc *npcinf = info.get_npc_paperdoll();
	set_toggle("shinfo_npcpaperdoll_check", npcinf != 0);
	set_visible("shinfo_npcpaperdoll_box", npcinf != 0);
	if (npcinf)
		{
		set_toggle("shinfo_npcpaperdoll_female", npcinf->is_npc_female());
		set_toggle("shinfo_npcpaperdoll_trans", npcinf->is_translucent());
		set_spin("shinfo_npcpaperdoll_bshape", npcinf->get_body_shape());
		set_spin("shinfo_npcpaperdoll_bframe", npcinf->get_body_frame());
		set_spin("shinfo_npcpaperdoll_hshape", npcinf->get_head_shape());
		set_spin("shinfo_npcpaperdoll_hframe", npcinf->get_head_frame());
		set_spin("shinfo_npcpaperdoll_hhelm", npcinf->get_head_frame_helm());
		set_spin("shinfo_npcpaperdoll_ashape", npcinf->get_arms_shape());
		static char *arm_names[] = {"shinfo_npcpaperdoll_aframe",
			"shinfo_npcpaperdoll_atwohanded",
			"shinfo_npcpaperdoll_astaff"
			};
		for (int i = 0; i < sizeof(arm_names)/sizeof(arm_names[0]); i++)
			set_spin(arm_names[i], npcinf->get_arms_frame(i));
		bool bg = game_type == BLACK_GATE;
		set_spin("shinfo_npcpaperdoll_gump",
				bg ? npcinf->get_gump_shape() : -1, bg);
		}
	Animation_info *aniinf = info.get_animation_info();
	set_toggle("shinfo_animation_check", aniinf != 0);
	set_visible("shinfo_animation_box", aniinf != 0);
	if (aniinf)
		{
		Animation_info::AniType type = aniinf->get_type();
		bool on = (type != Animation_info::FA_NON_LOOPING);
		set_optmenu("shinfo_animation_type", (int)type);
		Vga_file *ifile = curfile->get_ifile();
		int nframes = ifile->get_num_frames(shnum);
		int count = aniinf->get_frame_count();
		if (count == nframes || count < 0)
			{
			set_toggle("shinfo_animation_frtype", true);
			set_spin("shinfo_animation_frcount", nframes, false);
			}
		else
			{
			set_toggle("shinfo_animation_frtype", false);
			set_spin("shinfo_animation_frcount", nframes, true);
			}
		set_optmenu("shinfo_animation_ticks", aniinf->get_frame_delay(), on);
		int sfxdelay = aniinf->get_sfx_delay();
		set_toggle("shinfo_animation_sfxsynch", sfxdelay < 0, on);
		if (sfxdelay == 0)
			sfxdelay = 1;
		set_spin("shinfo_animation_sfxdelay", sfxdelay < 0 ? 1 : sfxdelay,
				on && sfxdelay > 0);
		on = (type == Animation_info::FA_LOOPING);
		int chance = aniinf->get_freeze_first_chance();
		set_optmenu("shinfo_animation_freezefirst", chance == 100 ? 0 :
				(chance == 0 ? 1 : 2), on);
		bool usespin = (chance > 0) && (chance < 100);
		set_spin("shinfo_animation_freezechance",
				usespin ? chance : 1, on && usespin);
		set_spin("shinfo_animation_recycle", aniinf->get_recycle(), on);
		}
	std::vector<Effective_hp_info>& hpinf = info.get_effective_hp_info();
	set_toggle("shinfo_effhps_check", !hpinf.empty());
	set_visible("shinfo_effhps_box", !hpinf.empty());
	if (!hpinf.empty())
		{
		GtkTreeView *hptree = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "shinfo_effhps_list"));
		GtkTreeModel *model = gtk_tree_view_get_model(hptree);
		GtkTreeStore *store = GTK_TREE_STORE(model);
		GtkTreeIter iter;
		Effective_hp_info *first = 0;
		for (std::vector<Effective_hp_info>::iterator it = hpinf.begin();
			it != hpinf.end(); ++it)
			{
			gtk_tree_store_append(store, &iter, NULL);
			Effective_hp_info& hps = *it;
			if (hps.is_invalid())
				continue;
			if (!first)
				first = &*it;
			gtk_tree_store_set(store, &iter, HP_FRAME_COLUMN, hps.get_frame(),
					HP_QUALITY_COLUMN, hps.get_quality(),
					HP_HIT_POINTS, hps.get_hps(),
					HP_FROM_PATCH, hps.from_patch(),
					HP_MODIFIED, hps.was_modified(), -1);
			}
		GtkTreeSelection *sel = gtk_tree_view_get_selection(hptree);
		gtk_tree_model_get_iter_first(model, &iter);
		gtk_tree_selection_select_iter(sel, &iter);
		GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_set_cursor(hptree, path, NULL, false);
		gtk_tree_path_free(path);
		Set_hp_fields(first->get_frame(), first->get_quality(),
				first->get_hps());
		}
	else
		Set_hp_fields();
	std::vector<Warmth_info>& warminf = info.get_warmth_info();
	set_toggle("shinfo_warmth_check", !warminf.empty());
	set_visible("shinfo_warmth_box", !warminf.empty());
	if (!warminf.empty())
		{
		GtkTreeView *warmtree = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "shinfo_warmth_list"));
		GtkTreeModel *model = gtk_tree_view_get_model(warmtree);
		GtkTreeStore *store = GTK_TREE_STORE(model);
		GtkTreeIter iter;
		Warmth_info *first = 0;
		for (std::vector<Warmth_info>::iterator it = warminf.begin();
			it != warminf.end(); ++it)
			{
			gtk_tree_store_append(store, &iter, NULL);
			Warmth_info& warm = *it;
			if (warm.is_invalid())
				continue;
			if (!first)
				first = &*it;
			gtk_tree_store_set(store, &iter, WARM_FRAME_COLUMN, warm.get_frame(),
					WARM_VALUE_COLUMN, warm.get_warmth(),
					WARM_FROM_PATCH, warm.from_patch(),
					WARM_MODIFIED, warm.was_modified(), -1);
			}
		GtkTreeSelection *sel = gtk_tree_view_get_selection(warmtree);
		gtk_tree_model_get_iter_first(model, &iter);
		gtk_tree_selection_select_iter(sel, &iter);
		GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_set_cursor(warmtree, path, NULL, false);
		gtk_tree_path_free(path);
		Set_warmth_fields(first->get_frame(), first->get_warmth());
		}
	else
		Set_warmth_fields();
	std::vector<Content_rules>& cntinf = info.get_content_rules();
	set_toggle("shinfo_cntrules_check", !cntinf.empty());
	set_visible("shinfo_cntrules_box", !cntinf.empty());
	if (!cntinf.empty())
		{
		GtkTreeView *cnttree = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "shinfo_cntrules_list"));
		GtkTreeModel *model = gtk_tree_view_get_model(cnttree);
		GtkTreeStore *store = GTK_TREE_STORE(model);
		GtkTreeIter iter;
		Content_rules *first = 0;
		for (std::vector<Content_rules>::iterator it = cntinf.begin();
			it != cntinf.end(); ++it)
			{
			gtk_tree_store_append(store, &iter, NULL);
			Content_rules& cntr = *it;
			if (cntr.is_invalid())
				continue;
			if (!first)
				first = &*it;
			gtk_tree_store_set(store, &iter, CNT_SHAPE_COLUMN, cntr.get_shape(),
					CNT_ACCEPT_COLUMN, cntr.accepts_shape(),
					CNT_FROM_PATCH, cntr.from_patch(),
					CNT_MODIFIED, cntr.was_modified(), -1);
			}
		GtkTreeSelection *sel = gtk_tree_view_get_selection(cnttree);
		gtk_tree_model_get_iter_first(model, &iter);
		gtk_tree_selection_select_iter(sel, &iter);
		GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_set_cursor(cnttree, path, NULL, false);
		gtk_tree_path_free(path);
		Set_cntrules_fields(first->get_shape(), first->accepts_shape());
		}
	else
		Set_cntrules_fields();
	std::vector<Frame_name_info>& nmvec = info.get_frame_name_info();
	set_toggle("shinfo_framenames_check", !nmvec.empty());
	set_visible("shinfo_framenames_box", !nmvec.empty());
	if (!nmvec.empty())
		{
		GtkTreeView *nametree = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "shinfo_framenames_list"));
		GtkTreeModel *model = gtk_tree_view_get_model(nametree);
		GtkTreeStore *store = GTK_TREE_STORE(model);
		GtkTreeIter iter;
		Frame_name_info *first = 0;
		for (std::vector<Frame_name_info>::iterator it = nmvec.begin();
			it != nmvec.end(); ++it)
			{
			gtk_tree_store_append(store, &iter, NULL);
			Frame_name_info& nmit = *it;
			if (nmit.is_invalid())
				continue;
			if (!first)
				first = &*it;
			int type = nmit.get_type(), msgid = nmit.get_msgid();
			const char *sname = get_text_entry("shinfo_name");
			const char *msgstr = type == -255 ? sname :
				(type == -1 || msgid >= num_misc_names ? 0 : misc_names[msgid]);
			int otmsg = nmit.get_othermsg();
			int otype = type <= 0 ? -1 : (otmsg < 0 ? otmsg : 2);
			const char *otmsgstr = otype == -255 ? sname :
				(otype == -1 || otmsg >= num_misc_names ? 0 : misc_names[otmsg]);
			gtk_tree_store_set(store, &iter, FNAME_FRAME, nmit.get_frame(),
				FNAME_QUALITY, nmit.get_quality(), FNAME_MSGTYPE, type,
				FNAME_MSGSTR, msgstr, FNAME_OTHERTYPE, otype,
				FNAME_OTHERMSG, otmsgstr, FNAME_FROM_PATCH, nmit.from_patch(),
				FNAME_MODIFIED, nmit.was_modified(), -1);
			}
		GtkTreeSelection *sel = gtk_tree_view_get_selection(nametree);
		gtk_tree_model_get_iter_first(model, &iter);
		gtk_tree_selection_select_iter(sel, &iter);
		GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_set_cursor(nametree, path, NULL, false);
		gtk_tree_path_free(path);
		int type = first->get_type(), msgid = first->get_msgid();
		const char *sname = get_text_entry("shinfo_name");
		const char *msgstr = type == -255 ? sname :
			(type == -1 || msgid >= num_misc_names ? "" : misc_names[msgid]);
		int otmsg = first->get_othermsg();
		int otype = type <= 0 ? -1 : (otmsg < 0 ? otmsg : 2);
		const char *otmsgstr = otype == -255 ? sname :
			(otype == -1 || otmsg >= num_misc_names ? "" : misc_names[otmsg]);
		Set_framenames_fields(first->get_frame(), first->get_quality(),
				type, msgstr, otype, otmsgstr);
		}
	else
		Set_framenames_fields();
	std::vector<Paperdoll_item>& dollinf = info.get_paperdoll_info();
	set_toggle("shinfo_objpaperdoll_check", !dollinf.empty());
	set_visible("shinfo_objpaperdoll_box", !dollinf.empty());
	if (!dollinf.empty())
		{
		GtkTreeView *dolltree = GTK_TREE_VIEW(
				glade_xml_get_widget(app_xml, "shinfo_objpaperdoll_list"));
		GtkTreeModel *model = gtk_tree_view_get_model(dolltree);
		GtkTreeStore *store = GTK_TREE_STORE(model);
		GtkTreeIter iter;
		Paperdoll_item *first = 0;
		for (std::vector<Paperdoll_item>::iterator it = dollinf.begin();
			it != dollinf.end(); ++it)
			{
			gtk_tree_store_append(store, &iter, NULL);
			Paperdoll_item& doll = *it;
			if (doll.is_invalid())
				continue;
			if (!first)
				first = &*it;
			gtk_tree_store_set(store, &iter,
					DOLL_WORLD_FRAME, doll.get_world_frame(),
					DOLL_SPOT, doll.get_object_spot(),
					DOLL_TRANSLUCENT, doll.is_translucent(),
					DOLL_GENDER_BASED, doll.is_gender_based(),
					DOLL_SPOT_TYPE, doll.get_spot_frame(),
					DOLL_SHAPE, doll.get_paperdoll_shape(),
					DOLL_FRAME_0, doll.get_paperdoll_frame(0),
					DOLL_FRAME_1, doll.get_paperdoll_frame(1),
					DOLL_FRAME_2, doll.get_paperdoll_frame(2),
					DOLL_FRAME_3, doll.get_paperdoll_frame(3),
					DOLL_FROM_PATCH, doll.from_patch(),
					DOLL_MODIFIED, doll.was_modified(), -1);
			}
		GtkTreeSelection *sel = gtk_tree_view_get_selection(dolltree);
		gtk_tree_model_get_iter_first(model, &iter);
		gtk_tree_selection_select_iter(sel, &iter);
		GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_set_cursor(dolltree, path, NULL, false);
		gtk_tree_path_free(path);
		Paperdoll_item& doll = *first;
		Set_objpaperdoll_fields(doll.get_world_frame(), doll.get_object_spot(),
				doll.is_translucent(), doll.is_gender_based(),
				doll.get_spot_frame(), doll.get_paperdoll_shape(),
				doll.get_paperdoll_frame(0), doll.get_paperdoll_frame(1),
				doll.get_paperdoll_frame(2), doll.get_paperdoll_frame(3));
		}
	else
		Set_objpaperdoll_fields();

	gtk_widget_show(book);
	}

struct Update_hps
	{
	void operator() (Shape_info& info, GtkTreeModel *model, GtkTreeIter *iter)
		{
		unsigned int frnum, qual, hps, patch, modded;
		gtk_tree_model_get(model, iter, HP_FRAME_COLUMN, &frnum,
				HP_QUALITY_COLUMN, &qual, HP_HIT_POINTS, &hps,
				HP_FROM_PATCH, &patch, HP_MODIFIED, &modded, -1);
		if (!info.has_quality() && qual != -1)	// Ignore these.
			return;
		Effective_hp_info hpinf(frnum, qual, hps, patch, modded);
		info.add_effective_hp_info(hpinf);
		}
	};

struct Update_warmth
	{
	void operator() (Shape_info& info, GtkTreeModel *model, GtkTreeIter *iter)
		{
		unsigned int frnum, warm, patch, modded;
		gtk_tree_model_get(model, iter, WARM_FRAME_COLUMN, &frnum,
				WARM_VALUE_COLUMN, &warm,
				WARM_FROM_PATCH, &patch, WARM_MODIFIED, &modded, -1);
		Warmth_info warminf(frnum, warm, patch, modded);
		info.add_warmth_info(warminf);
		}
	};

struct Update_cntrules
	{
	void operator() (Shape_info& info, GtkTreeModel *model, GtkTreeIter *iter)
		{
		unsigned int shnum, accept, patch, modded;
		gtk_tree_model_get(model, iter, CNT_SHAPE_COLUMN, &shnum,
				CNT_ACCEPT_COLUMN, &accept,
				CNT_FROM_PATCH, &patch, CNT_MODIFIED, &modded, -1);
		Content_rules cntinf(shnum, accept, patch, modded);
		info.add_content_rule(cntinf);
		}
	};

struct Update_paperdolls
	{
	void operator() (Shape_info& info, GtkTreeModel *model, GtkTreeIter *iter)
		{
		unsigned int frnum;
		int spot, trans, gender, type, shape,
			frame0, frame1, frame2, frame3, patch, modded;
		gtk_tree_model_get(model, iter, DOLL_WORLD_FRAME, &frnum,
				DOLL_SPOT, &spot,
				DOLL_TRANSLUCENT, &trans,
				DOLL_GENDER_BASED, &gender,
				DOLL_SPOT_TYPE, &type,
				DOLL_SHAPE, &shape,
				DOLL_FRAME_0, &frame0,
				DOLL_FRAME_1, &frame1,
				DOLL_FRAME_2, &frame2,
				DOLL_FRAME_3, &frame3,
				DOLL_FROM_PATCH, &patch,
				DOLL_MODIFIED, &modded, -1);
		Paperdoll_item dollinf(frnum, spot, trans, gender, type, shape,
				frame0, frame1, frame2, frame3, patch, modded);
		info.add_paperdoll_info(dollinf);
		}
	};

int ExultStudio::find_misc_name(const char *id) const
	{
	map<string, int>::const_iterator it = misc_name_map.find(id);
	if (it != misc_name_map.end())
		return it->second;
	return -1;
	}

int ExultStudio::add_misc_name(const char *id)
	{
	int num = num_misc_names++;
	char **newlist = new char*[num_misc_names];
	memcpy(newlist, misc_names, num*sizeof(char *));
	delete [] misc_names;
	misc_names = newlist;
	misc_names[num] = newstrdup(id);
	misc_name_map.insert(std::pair<string, int>(string(id), num));
	shape_names_modified = true;
	return num;
	}

struct Update_framenames
	{
private:
	int Find_name_id(const char *msg)
		{
		ExultStudio *studio = ExultStudio::get_instance();
		int idnum = studio->find_misc_name(msg);
		if (idnum < 0)
			return studio->add_misc_name(msg);
		return idnum;
		}
public:
	void operator() (Shape_info& info, GtkTreeModel *model, GtkTreeIter *iter)
		{
		unsigned int frnum, qual;
		int type, othertype, patch, modded;
		const char *str, *othermsg;
		gtk_tree_model_get(model, iter, FNAME_FRAME, &frnum,
				FNAME_QUALITY, &qual, FNAME_MSGTYPE, &type,
				FNAME_MSGSTR, &str, FNAME_OTHERTYPE, &othertype,
				FNAME_OTHERMSG, &othermsg,
				FNAME_FROM_PATCH, &patch, FNAME_MODIFIED, &modded, -1);
		int msgid, otherid;
		msgid = type < 0 ? -1 : Find_name_id(str);
		otherid = type <= 0 ? -1 :
				(othertype < 0 ? othertype : Find_name_id(othermsg));
		Frame_name_info nminf((int)frnum, (int)qual, type, msgid, otherid,
				patch, modded);
		info.add_frame_name_info(nminf);
		}
	};

template <typename Functor>
static inline void update_shape_vector
	(
	const char *treename,
	Shape_info& info
	)
	{
	Functor update;
	GtkTreeView *tree = GTK_TREE_VIEW(
		glade_xml_get_widget(ExultStudio::get_instance()->get_xml(), treename));
	GtkTreeModel *model = gtk_tree_view_get_model(tree);
	GtkTreeStore *store = GTK_TREE_STORE(model);
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first(model, &iter))
			// Tree has contents.
		do
			{
			update(info, model, &iter);
			} while (gtk_tree_model_iter_next(model, &iter));
	}

inline Weapon_info *Shape_info::set_weapon_info(bool tf)
	{ return set_info(tf, weapon); }
inline Ammo_info *Shape_info::set_ammo_info(bool tf)
	{ return set_info(tf, ammo); }
inline Armor_info *Shape_info::set_armor_info(bool tf)
	{ return set_info(tf, armor); }
inline Monster_info *Shape_info::set_monster_info(bool tf)
	{ return set_info(tf, monstinf); }
inline Paperdoll_npc *Shape_info::set_npc_paperdoll_info(bool tf)
	{ return set_info(tf, npcpaperdoll); }
inline SFX_info *Shape_info::set_sfx_info(bool tf)
	{ return set_info(tf, sfxinf); }
inline Explosion_info *Shape_info::set_explosion_info(bool tf)
	{ return set_info(tf, explosion); }
inline Animation_info *Shape_info::set_animation_info(bool tf)
	{ return set_info(tf, aniinf); }
inline Body_info *Shape_info::set_body_info(bool tf)
	{ return set_info(tf, body); }

/*
 *	Save the shape-editing notebook.
 */

void ExultStudio::save_shape_notebook
	(
	Shape_info& info,
	int shnum,			// Shape #.
	int frnum			// Frame #.
	)
	{
	static int classes[] = {0, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14};
	info.set_shape_class((Shape_info::Shape_class)
			classes[get_optmenu("shinfo_shape_class")]);
	info.set_3d(get_spin("shinfo_xtiles"), get_spin("shinfo_ytiles"),
						get_spin("shinfo_ztiles"));
	int spot = get_optmenu("shinfo_ready_spot");
	static const signed char bg_spots[] = {0x00, 0x01, 0x14, 0x08, 0x09,
			0x00, 0x04, 0x00, 0x00, 0x05, 0x15, 0x06, 0x00, 0x0A, 0x0B, 0x12,
			0x0F, 0x07};
	static const signed char si_spots[] = {0x00, 0x01, 0x14, 0x0A, 0x04,
			0x09, 0x03, 0x02, 0x0F, 0x0C, 0x05, 0x08, 0x0B, 0x0E, 0x0D, 0x00,
			0x00, 0x06};
	if (game_type == BLACK_GATE)
		spot = bg_spots[spot];
	else
		spot = si_spots[spot];
	info.set_ready_type(spot);
	info.set_is_spell(get_toggle("shinfo_is_spell_check"));
	info.set_weight_volume(get_spin("shinfo_weight"),
						get_spin("shinfo_volume"));
	info.set_weapon_offset(frnum, get_spin("shinfo_wihx"),
	                       get_spin("shinfo_wihy"));
					// Bunch of flags:
	info.set_sfx(get_toggle("shinfo_sfx_check"));
	info.set_strange_movement(get_toggle("shinfo_strange_check"));
	info.set_animated(get_toggle("shinfo_animated_check"));
	info.set_solid(get_toggle("shinfo_solid_check"));
	info.set_water(get_toggle("shinfo_water_check"));
			// ++++Set poison for flats, field for non-flats.
	info.set_field(get_toggle("shinfo_field_check") ||
			get_toggle("shinfo_poison_check"));
	info.set_door(get_toggle("shinfo_door_check"));
	info.set_barge_part(get_toggle("shinfo_barge_check"));
	info.set_transparent(get_toggle("shinfo_transp_check"));
	info.set_light_source(get_toggle("shinfo_light_check"));
	info.set_translucency(get_toggle("shinfo_transl_check"));
	info.set_obstacle(get_toggle("shinfo_obstaclex_check"), 
			get_toggle("shinfo_obstacley_check"));
	info.set_occludes(get_toggle("shinfo_occludes_check"));
					// More flags, originally hard-coded:
	static char *shpflags[] = {
				"shinfo_shape_flag0",
				"shinfo_shape_flag1",
				"shinfo_shape_flag2",
				"shinfo_shape_flag3",
				"shinfo_shape_flag4",
				"shinfo_shape_flag5",
				"shinfo_shape_flag6" };
	info.set_shape_flags(get_bit_toggles(&shpflags[0], 
				sizeof(shpflags)/sizeof(shpflags[0])));
					// Extras.
	if (!get_toggle("shinfo_weapon_check"))
		info.set_weapon_info(false);	// Not a weapon.
	else
		{
		Weapon_info *winfo = info.set_weapon_info(true);
		winfo->set_damage(
			get_spin("shinfo_weapon_damage"),
			get_optmenu("shinfo_weapon_type"));
		winfo->set_range(get_spin("shinfo_weapon_range"));
		int ammo_use = get_optmenu("shinfo_weapon_ammo");
		int ammo = ammo_use == 0 ? 
				get_spin("shinfo_weapon_ammo_shape")
			:  ammo_use == 2 ? -2		// Charges.
			:  ammo_use == 3 ? -3		// Thrown.
			:  -1;				// None.
		winfo->set_ammo(ammo);
		int proj_type = get_optmenu("shinfo_weapon_sprite");
		int proj = proj_type == 0 ? 
				get_spin("shinfo_weapon_proj")
			:  ammo_use == 2 ? -3		// Weapon.
			:  -1;				// None.
		winfo->set_projectile(proj);
		winfo->set_uses(get_optmenu("shinfo_weapon_uses"));
		winfo->set_sfxs(get_spin("shinfo_weapon_sfx"),
				get_spin("shinfo_weapon_hitsfx"));
					// Get usecode in hex.
		winfo->set_usecode(get_num_entry("shinfo_weapon_uc"));
		static char *powers[] = {
					"shinfo_weapon_pow0",
					"shinfo_weapon_pow1",
					"shinfo_weapon_pow2",
					"shinfo_weapon_pow3",
					"shinfo_weapon_pow4",
					"shinfo_weapon_pow5",
					"shinfo_weapon_pow6",
					"shinfo_weapon_pow7" };
		winfo->set_powers(get_bit_toggles(&powers[0], 
					sizeof(powers)/sizeof(powers[0])));
		winfo->set_lucky(get_toggle("shinfo_weapon_flag0"));
		winfo->set_explodes(get_toggle("shinfo_weapon_flag1"));
		winfo->set_no_blocking(get_toggle("shinfo_weapon_flag2"));
		winfo->set_delete_depleted(get_toggle("shinfo_weapon_flag3"));
		winfo->set_autohits(get_toggle("shinfo_weapon_always"));
		winfo->set_needs_target(get_toggle("shinfo_weapon_target"));
		winfo->set_returns(get_toggle("shinfo_weapon_returns"));
		winfo->set_actor_frames(get_optmenu("shinfo_weapon_frames") |
				(get_optmenu("shinfo_proj_frames")<<2));
		winfo->set_rotation_speed(get_spin("shinfo_proj_rotation"));
		winfo->set_missile_speed(get_optmenu("shinfo_proj_speed")+1);
		}
	if (!get_toggle("shinfo_ammo_check"))
		info.set_ammo_info(false);	// Not ammo.
	else
		{
		Ammo_info *ainfo = info.set_ammo_info(true);
		ainfo->set_family_shape(get_spin("shinfo_ammo_family"));
		ainfo->set_damage(get_spin("shinfo_ammo_damage"),
				get_optmenu("shinfo_ammo_type"));
		int proj_type = get_optmenu("shinfo_ammo_sprite");
		int proj = proj_type == 0 ? 
				get_spin("shinfo_ammo_proj")
			:  proj_type == 2 ? -3		// Weapon.
			:  -1;				// None.
		ainfo->set_sprite_shape(proj);
		static char *powers[] = {
					"shinfo_ammo_pow0",
					"shinfo_ammo_pow1",
					"shinfo_ammo_pow2",
					"shinfo_ammo_pow3",
					"shinfo_ammo_pow4",
					"shinfo_ammo_pow5",
					"shinfo_ammo_pow6",
					"shinfo_ammo_pow7"};
		ainfo->set_powers(get_bit_toggles(&powers[0], 
					sizeof(powers)/sizeof(powers[0])));
		ainfo->set_lucky(get_toggle("shinfo_ammo_flag0"));
		ainfo->set_autohits(get_toggle("shinfo_ammo_flag1"));
		ainfo->set_returns(get_toggle("shinfo_ammo_flag2"));
		ainfo->set_no_blocking(get_toggle("shinfo_ammo_flag3"));
		ainfo->set_explodes(get_toggle("shinfo_ammo_flag6"));
		ainfo->set_homing(get_toggle("shinfo_ammo_special"));

		if (get_toggle("shinfo_ammo_special"))
			{
			ainfo->set_homing(true);
			ainfo->set_drop_type(0);
			}
		else
			{
			ainfo->set_homing(false);
			ainfo->set_drop_type(get_optmenu("shinfo_ammo_drop"));
			}
		}
	if (!get_toggle("shinfo_armor_check"))
		info.set_armor_info(false);	// Not armor.
	else
		{
		Armor_info *arinfo = info.set_armor_info(true);
		static char *immun[] = {"shinfo_armor_immun0",
					"shinfo_armor_immun1",
					"shinfo_armor_immun2",
					"shinfo_armor_immun3",
					"shinfo_armor_immun4",
					"shinfo_armor_immun5" };
		arinfo->set_prot(get_spin("shinfo_armor_value"));
		arinfo->set_immune(get_bit_toggles(&immun[0], 
					sizeof(immun)/sizeof(immun[0])));
		}
	if (!get_toggle("shinfo_npcflags_check"))
		info.set_actor_flags(0);
	else
		{
		static char *flags[] = {	"shinfo_actor_flag0",
					"shinfo_actor_flag1",
					"shinfo_actor_flag2",
					"shinfo_actor_flag3",
					"shinfo_actor_flag4" };
		info.set_actor_flags(get_bit_toggles(&flags[0], 
					sizeof(flags)/sizeof(flags[0])));
		}
	if (!get_toggle("shinfo_monster_check"))
		info.set_monster_info(false);
	else
		{
		Monster_info *minfo = info.set_monster_info(true);
		minfo->set_stats(
			get_spin("shinfo_monster_str"),
			get_spin("shinfo_monster_dex"),
			get_spin("shinfo_monster_intel"),
			get_spin("shinfo_monster_cmb"),
			get_spin("shinfo_monster_armor"),
			get_spin("shinfo_monster_wpn"),
			get_spin("shinfo_monster_reach"));
		minfo->set_equip_offset(get_spin("shinfo_monster_equip"));
		minfo->set_hitsfx(get_spin("shinfo_monster_sfx"));
		minfo->set_alignment(get_optmenu("shinfo_monster_align"));
		minfo->set_attackmode(get_optmenu("shinfo_monster_attackmode"));
		static char *vuln[] = {	"shinfo_monster_vuln0",
					"shinfo_monster_vuln1",
					"shinfo_monster_vuln2",
					"shinfo_monster_vuln3",
					"shinfo_monster_vuln4",
					"shinfo_monster_vuln5",
					"shinfo_monster_vuln6",
					"shinfo_monster_vuln7" };
		static char *immun[] = {"shinfo_monster_immun0",
					"shinfo_monster_immun1",
					"shinfo_monster_immun2",
					"shinfo_monster_immun3",
					"shinfo_monster_immun4",
					"shinfo_monster_immun5",
					"shinfo_monster_immun6",
					"shinfo_monster_immun7" };
		minfo->set_vulnerable(get_bit_toggles(&vuln[0], 
						sizeof(vuln)/sizeof(vuln[0])));
		minfo->set_immune(get_bit_toggles(&immun[0], 
						sizeof(immun)/sizeof(immun[0])));
		minfo->set_splits(get_toggle("shinfo_monster_splits"));
		minfo->set_cant_yell(get_toggle("shinfo_monster_cant_yell"));
		minfo->set_cant_bleed(get_toggle("shinfo_monster_cant_bleed"));
		minfo->set_sleep_safe(get_toggle("shinfo_monster_sleep_safe"));
		minfo->set_charm_safe(get_toggle("shinfo_monster_charm_safe"));
		minfo->set_curse_safe(get_toggle("shinfo_monster_curse_safe"));
		minfo->set_paralysis_safe(get_toggle("shinfo_monster_paralysis_safe"));
		minfo->set_poison_safe(get_toggle("shinfo_monster_poison_safe"));
		minfo->set_death_safe(get_toggle("shinfo_monster_death_safe"));
		minfo->set_power_safe(get_toggle("shinfo_monster_power_safe"));
		minfo->set_cant_die(get_toggle("shinfo_monster_cant_die"));
		static char *flags[] = {"shinfo_monster_flag0",
					"shinfo_monster_flag1",
					"shinfo_monster_flag2",
					"shinfo_monster_flag3",
					"shinfo_monster_flag4",
					"shinfo_monster_flag5",
					"shinfo_monster_flag6",
					"shinfo_monster_flag7" };
		minfo->set_flags(get_bit_toggles(&flags[0],
					sizeof(flags)/sizeof(flags[0])));
		info.set_monster_food(get_spin("shinfo_monster_food"));
		}
	if (!get_toggle("shinfo_container_check"))
		info.set_container_gump(-1);
	else
		{
		info.set_container_gump(get_spin("shinfo_gump_num"));
		}
	if (!get_toggle("shinfo_body_check"))
		info.set_body_info(false);
	else
		{
		Body_info *binfo = info.set_body_info(true);
		binfo->set(get_spin("shinfo_body_shape"), get_spin("shinfo_body_frame"));
		}
	if (!get_toggle("shinfo_explosion_check"))
		info.set_explosion_info(false);
	else
		{
		Explosion_info *einfo = info.set_explosion_info(true);
		int sfx = get_toggle("shinfo_explosion_sfx_type") ? -1
			: get_spin("shinfo_explosion_sfx_number");
		einfo->set(get_spin("shinfo_explosion_sprite"), sfx);
		}
	if (!get_toggle("shinfo_sound_check"))
		info.set_sfx_info(false);
	else
		{
		SFX_info *sfxinf = info.set_sfx_info(true);
		sfxinf->set_sfx(get_spin("shinfo_sfx_first"));
		sfxinf->set_play_randomly(get_optmenu("shinfo_sfx_type"));
		sfxinf->set_chance(get_spin("shinfo_sfx_chance"));
		int extra = get_toggle("shinfo_sfx_clock_check") ?
				get_spin("shinfo_sfx_clock_sfx") : -1;
		sfxinf->set_extra_sfx(extra);
		int range = get_toggle("shinfo_single_sfx") ? 1 :
				get_spin("shinfo_sfx_count");
		sfxinf->set_sfx_range(range);
		}
	if (!get_toggle("shinfo_npcpaperdoll_check"))
		info.set_npc_paperdoll_info(false);
	else
		{
		Paperdoll_npc *npcinf = info.set_npc_paperdoll_info(true);
		npcinf->set_is_female(get_toggle("shinfo_npcpaperdoll_female"));
		npcinf->set_translucent(get_toggle("shinfo_npcpaperdoll_trans"));
		npcinf->set_body_shape(get_spin("shinfo_npcpaperdoll_bshape"));
		npcinf->set_body_frame(get_spin("shinfo_npcpaperdoll_bframe"));
		npcinf->set_head_shape(get_spin("shinfo_npcpaperdoll_hshape"));
		npcinf->set_head_frame(get_spin("shinfo_npcpaperdoll_hframe"));
		npcinf->set_head_frame_helm(get_spin("shinfo_npcpaperdoll_hhelm"));
		npcinf->set_arms_shape(get_spin("shinfo_npcpaperdoll_ashape"));
		static char *arm_names[] = {"shinfo_npcpaperdoll_aframe",
			"shinfo_npcpaperdoll_atwohanded",
			"shinfo_npcpaperdoll_astaff"
			};
		for (int i = 0; i < sizeof(arm_names)/sizeof(arm_names[0]); i++)
			npcinf->set_arms_frame(i, get_spin(arm_names[i]));
		if (game_type == BLACK_GATE)
			npcinf->set_gump_shape(get_spin("shinfo_npcpaperdoll_gump"));
		}
	if (!get_toggle("shinfo_animation_check"))
		info.set_animation_info(false);
	else
		{
		Animation_info *aniinf = info.set_animation_info(true);
		Animation_info::AniType type =
				(Animation_info::AniType)get_optmenu("shinfo_animation_type");
		aniinf->set_type(type);
		int count = get_spin("shinfo_animation_frcount");
		Vga_file *ifile = curfile->get_ifile();
		int nframes = ifile->get_num_frames(shnum);
		if (get_toggle("shinfo_animation_frtype") || count == nframes)
			aniinf->set_frame_count(-1);
		else
			aniinf->set_frame_count(count);
		if (type != Animation_info::FA_NON_LOOPING)
			{
			aniinf->set_frame_delay(get_spin("shinfo_animation_ticks"));
			int sfxdelay = get_toggle("shinfo_animation_sfxsynch") ? -1
				: get_spin("shinfo_animation_sfxdelay");
			aniinf->set_sfx_delay(sfxdelay);
			if (type == Animation_info::FA_LOOPING)
				{
				int menu = get_optmenu("shinfo_animation_freezefirst");
				int chance = menu == 0 ? 100
					: (menu == 1 ? 0 : get_spin("shinfo_animation_freezechance"));
				aniinf->set_freeze_first_chance(chance);
				aniinf->set_recycle(get_spin("shinfo_animation_recycle"));
				}
			}
		}
	if (!get_toggle("shinfo_effhps_check"))
		info.set_effective_hp_info(false);
	else
		{
		info.set_effective_hp_info(true);
		update_shape_vector<Update_hps>("shinfo_effhps_list", info);
		info.clean_invalid_hp_info();
		}
	if (!get_toggle("shinfo_warmth_check"))
		info.set_warmth_info(false);
	else
		{
		info.set_warmth_info(true);
		update_shape_vector<Update_warmth>("shinfo_warmth_list", info);
		info.clean_invalid_warmth_info();
		}
	if (!get_toggle("shinfo_objpaperdoll_check"))
		info.set_paperdoll_info(false);
	else
		{
		info.set_paperdoll_info(true);
		update_shape_vector<Update_paperdolls>("shinfo_objpaperdoll_list", info);
		info.clean_invalid_paperdolls();
		}
	if (!get_toggle("shinfo_cntrules_check"))
		info.set_content_rules(false);
	else
		{
		info.set_content_rules(true);
		update_shape_vector<Update_cntrules>("shinfo_cntrules_list", info);
		info.clean_invalid_content_rules();
		}
	if (!get_toggle("shinfo_framenames_check"))
		info.set_frame_name_info(false);
	else
		{
		info.set_frame_name_info(true);
		update_shape_vector<Update_framenames>("shinfo_framenames_list", info);
		info.clean_invalid_name_info();
		}

	shape_info_modified = true;
	}

static inline void add_terminal_columns
	(
	GtkTreeView *tree,
	int patch_index,
	int modified_index
	)
	{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
			"Patch", renderer, "text", patch_index, NULL);
	gtk_tree_view_column_set_visible(col, false);
	gtk_tree_view_append_column(tree, col);
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes(
			"Modified", renderer, "text", modified_index, NULL);
	gtk_tree_view_column_set_visible(col, false);
	gtk_tree_view_append_column(tree, col);
	}


/*
 *	Open the shape-editing window.
 */

void ExultStudio::open_shape_window
	(
	int shnum,			// Shape #.
	int frnum,			// Frame #.
	Shape_file_info *file_info,	// For image file shape is in.
	char *shname,			// ->shape name, or null.
	Shape_info *info		// Info. if in main object shapes.
	)
	{
	if (!shapewin)			// First time?
		shapewin = glade_xml_get_widget( app_xml, "shape_window" );
	if (shape_draw)			// Ifile might have changed.
		delete shape_draw;
	shape_draw = 0;
	if (gump_draw)
		delete gump_draw;
	gump_draw = 0;
	if (npcgump_draw)
		delete npcgump_draw;
	npcgump_draw = 0;
	if (body_draw)
		delete body_draw;
	body_draw = 0;
	if (explosion_draw)
		delete explosion_draw;
	explosion_draw = 0;
	Vga_file *ifile = file_info->get_ifile();
	if (ifile && palbuf)
		{
		shape_draw = new Shape_draw(ifile, palbuf,
		             glade_xml_get_widget(app_xml, "shinfo_draw"));
//		shape_draw->enable_drop(Shape_shape_dropped, this);
		}
	if (gumpfile && palbuf)
		{
		gump_draw = new Shape_draw(gumpfile->get_ifile(), palbuf,
		            glade_xml_get_widget(app_xml, "shinfo_gump_draw"));
		gump_draw->enable_drop(Gump_shape_dropped, this);
		npcgump_draw = new Shape_draw(gumpfile->get_ifile(), palbuf,
		            glade_xml_get_widget(app_xml, "shinfo_npcgump_draw"));
		npcgump_draw->enable_drop(NPC_Gump_shape_dropped, this);
		}
	if (vgafile && palbuf)
		{
		body_draw = new Shape_draw(vgafile->get_ifile(), palbuf,
		            glade_xml_get_widget(app_xml, "shinfo_body_draw"));
		body_draw->enable_drop(Body_shape_dropped, this);
		}
	if (spritefile && palbuf)
		{
		explosion_draw = new Shape_draw(spritefile->get_ifile(), palbuf,
		            glade_xml_get_widget(app_xml, "shinfo_explosion_draw"));
		explosion_draw->enable_drop(Explosion_shape_dropped, this);
		}
					// Store ->'s.
	gtk_object_set_user_data(GTK_OBJECT(shapewin), info);
	gtk_object_set_data(GTK_OBJECT(shapewin), "file_info", file_info);
					// Shape/frame.
	set_entry("shinfo_shape", shnum, false, false);
	int nframes = ifile->get_num_frames(shnum);
	set_spin("shinfo_frame", frnum, 0, nframes - 1);
	set_spin("shinfo_effhps_frame_num", 0, nframes - 1);
	set_spin("shinfo_warmth_frame_num", 0, nframes - 1);
	set_spin("shinfo_framenames_frame_num", 0, nframes - 1);
					// Store name, #frames.
	set_entry("shinfo_name", shname ? shname : "");
//	set_spin("shinfo_num_frames", nframes);
					// Show xright, ybelow.
	Shape_frame *shape = ifile->get_shape(shnum, frnum);
	set_spin("shinfo_originx", shape->get_xright(), -shape->get_width(),
							shape->get_width());
	set_spin("shinfo_originy", shape->get_ybelow(), -shape->get_height(),
							shape->get_height());
					// Get info. notebook.
	// Ensure proper bounds for shapes:
	set_spin("shinfo_weapon_ammo_shape", 0, c_max_shapes-1);
	set_spin("shinfo_weapon_proj", 0, c_max_shapes-1);
	set_spin("shinfo_ammo_family", 0, c_max_shapes-1);
	set_spin("shinfo_body_shape", 0, c_max_shapes-1);
	set_spin("shinfo_cntrules_shape_num", 0, c_max_shapes-1);
	set_spin("shinfo_animation_frcount", 1, nframes);
	if (gumpfile)
		{
		set_spin("shinfo_gump_num", 0,
				gumpfile->get_ifile()->get_num_shapes()-1);
		set_spin("shinfo_npcpaperdoll_gump", -1,
				gumpfile->get_ifile()->get_num_shapes()-1);
		}
	if (spritefile)
		set_spin("shinfo_explosion_sprite", 0,
				spritefile->get_ifile()->get_num_shapes()-1);
	GtkTreeView *hptree = GTK_TREE_VIEW(
			glade_xml_get_widget(app_xml, "shinfo_effhps_list"));
	gtk_signal_connect(GTK_OBJECT(hptree), "cursor_changed",
				GTK_SIGNAL_FUNC(on_shinfo_effhps_list_cursor_changed),
				0L);
	GtkTreeModel *model = gtk_tree_view_get_model(hptree);
	if (!model)
		{
		GtkTreeStore *store = gtk_tree_store_new(
				HP_COLUMN_COUNT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
				G_TYPE_INT, G_TYPE_INT);
		gtk_tree_view_set_model(hptree, GTK_TREE_MODEL(store));
		g_object_unref(store);
		// Create each column.
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
				"Frame", renderer, "text", HP_FRAME_COLUMN, NULL);
		gtk_tree_view_append_column(hptree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Quality", renderer, "text", HP_QUALITY_COLUMN, NULL);
		gtk_tree_view_append_column(hptree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Hit Points", renderer, "text", HP_HIT_POINTS, NULL);
		gtk_tree_view_append_column(hptree, col);
		add_terminal_columns(hptree, HP_FROM_PATCH, HP_MODIFIED);
		}
	else
		{
		// Clear it.
		GtkTreeModel *model = gtk_tree_view_get_model(
							GTK_TREE_VIEW(hptree));
		GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);
		}

	GtkTreeView *warmtree = GTK_TREE_VIEW(
			glade_xml_get_widget(app_xml, "shinfo_warmth_list"));
	gtk_signal_connect(GTK_OBJECT(warmtree), "cursor_changed",
				GTK_SIGNAL_FUNC(on_shinfo_warmth_list_cursor_changed),
				0L);
	model = gtk_tree_view_get_model(warmtree);
	if (!model)
		{
		GtkTreeStore *store = gtk_tree_store_new(
				WARM_COLUMN_COUNT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
		gtk_tree_view_set_model(warmtree, GTK_TREE_MODEL(store));
		g_object_unref(store);
		// Create each column.
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
				"Frame", renderer, "text", WARM_FRAME_COLUMN, NULL);
		gtk_tree_view_append_column(warmtree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Warmth", renderer, "text", WARM_VALUE_COLUMN, NULL);
		gtk_tree_view_append_column(warmtree, col);
		add_terminal_columns(warmtree, WARM_FROM_PATCH, WARM_MODIFIED);
		}
	else
		{
		// Clear it.
		GtkTreeModel *model = gtk_tree_view_get_model(
							GTK_TREE_VIEW(warmtree));
		GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);
		}
	GtkTreeView *dolltree = GTK_TREE_VIEW(
			glade_xml_get_widget(app_xml, "shinfo_objpaperdoll_list"));
	gtk_signal_connect(GTK_OBJECT(dolltree), "cursor_changed",
				GTK_SIGNAL_FUNC(on_shinfo_objpaperdoll_list_cursor_changed),
				0L);
	model = gtk_tree_view_get_model(dolltree);
	if (!model)
		{
		GtkTreeStore *store = gtk_tree_store_new(
				DOLL_COLUMN_COUNT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
				G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
				G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
		gtk_tree_view_set_model(dolltree, GTK_TREE_MODEL(store));
		g_object_unref(store);
		// Create each column.
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
				"Frame", renderer, "text", DOLL_WORLD_FRAME, NULL);
		gtk_tree_view_append_column(dolltree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Spot", renderer, "text", DOLL_SPOT, NULL);
		gtk_tree_view_append_column(dolltree, col);
		char *columns[] = {"Trans", "Gender", "Spot frame", "DShape",
			"DFrame0", "DFrame1", "DFrame2", "DFrame3"};
		for (int i = 0; i < sizeof(columns)/sizeof(columns[0]); i++)
			{
			renderer = gtk_cell_renderer_text_new();
			col = gtk_tree_view_column_new_with_attributes(
					columns[i], renderer, "text", i + DOLL_TRANSLUCENT, NULL);
			if (i + DOLL_TRANSLUCENT > DOLL_SPOT_TYPE)
				gtk_tree_view_column_set_visible(col, false);
			gtk_tree_view_append_column(dolltree, col);
			}
		add_terminal_columns(warmtree, WARM_FROM_PATCH, WARM_MODIFIED);
		}
	else
		{
		// Clear it.
		GtkTreeModel *model = gtk_tree_view_get_model(
							GTK_TREE_VIEW(dolltree));
		GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);
		}

	GtkTreeView *cnttree = GTK_TREE_VIEW(
			glade_xml_get_widget(app_xml, "shinfo_cntrules_list"));
	gtk_signal_connect(GTK_OBJECT(cnttree), "cursor_changed",
				GTK_SIGNAL_FUNC(on_shinfo_cntrules_list_cursor_changed),
				0L);
	model = gtk_tree_view_get_model(cnttree);
	if (!model)
		{
		GtkTreeStore *store = gtk_tree_store_new(
				CNT_COLUMN_COUNT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
		gtk_tree_view_set_model(cnttree, GTK_TREE_MODEL(store));
		g_object_unref(store);
		// Create each column.
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
				"Shape", renderer, "text", CNT_SHAPE_COLUMN, NULL);
		gtk_tree_view_append_column(cnttree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Accept", renderer, "text", CNT_ACCEPT_COLUMN, NULL);
		gtk_tree_view_append_column(cnttree, col);
		add_terminal_columns(cnttree, CNT_FROM_PATCH, CNT_MODIFIED);
		}
	else
		{
		// Clear it.
		GtkTreeModel *model = gtk_tree_view_get_model(
							GTK_TREE_VIEW(cnttree));
		GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);
		}

	GtkTreeView *nametree = GTK_TREE_VIEW(
			glade_xml_get_widget(app_xml, "shinfo_framenames_list"));
	gtk_signal_connect(GTK_OBJECT(nametree), "cursor_changed",
				GTK_SIGNAL_FUNC(on_shinfo_framenames_list_cursor_changed),
				0L);
	model = gtk_tree_view_get_model(nametree);
	if (!model)
		{
		GtkTreeStore *store = gtk_tree_store_new(
				FNAME_COLUMN_COUNT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
				G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT,
				G_TYPE_INT);
		gtk_tree_view_set_model(nametree, GTK_TREE_MODEL(store));
		g_object_unref(store);
		// Create each column.
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
				"Frame", renderer, "text", FNAME_FRAME, NULL);
		gtk_tree_view_append_column(nametree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Quality", renderer, "text", FNAME_QUALITY, NULL);
		gtk_tree_view_append_column(nametree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Type", renderer, "text", FNAME_MSGTYPE, NULL);
		gtk_tree_view_column_set_visible(col, false);
		gtk_tree_view_append_column(nametree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Name", renderer, "text", FNAME_MSGSTR, NULL);
		gtk_tree_view_append_column(nametree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"Other Type", renderer, "text", FNAME_OTHERTYPE, NULL);
		gtk_tree_view_column_set_visible(col, false);
		gtk_tree_view_append_column(nametree, col);
		renderer = gtk_cell_renderer_text_new();
		col = gtk_tree_view_column_new_with_attributes(
				"(Pre|Suf)fix/Default", renderer, "text", FNAME_OTHERMSG, NULL);
		gtk_tree_view_append_column(nametree, col);
		add_terminal_columns(nametree, FNAME_FROM_PATCH, FNAME_MODIFIED);
		}
	else
		{
		// Clear it.
		GtkTreeModel *model = gtk_tree_view_get_model(
							GTK_TREE_VIEW(nametree));
		GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);
		}

	GtkWidget *notebook = glade_xml_get_widget(app_xml, "shinfo_notebook");
	if (info)
		init_shape_notebook(*info, notebook, shnum, frnum);
	else
		gtk_widget_hide(notebook);
	gtk_widget_show(shapewin);
	show_shinfo_shape();		// Be sure picture is updated.
	}

/*
 *	Save the shape-editing window.
 */

void ExultStudio::save_shape_window
	(
	)
	{
	int shnum = get_num_entry("shinfo_shape");
	int frnum = get_num_entry("shinfo_frame");
	Shape_info *info = (Shape_info *)
			gtk_object_get_user_data(GTK_OBJECT(shapewin));
	Shape_file_info *file_info = (Shape_file_info *)
		gtk_object_get_data(GTK_OBJECT(shapewin), "file_info");
	Vga_file *ifile = file_info->get_ifile();
	if (info)			// If 'shapes.vga', get name.
		{
		const gchar *nm = get_text_entry("shinfo_name");
		if (!nm)
			nm = "";
		char *oldname = get_shape_name(shnum);
		if (!oldname)
			oldname = "";
		if (strcmp(nm, oldname) != 0)
			{		// Name changed.
			Set_item_name(shnum, nm);
			shape_names_modified = true;
			}
		}
					// Update origin.
	Shape_frame *shape = ifile->get_shape(shnum, frnum);
	int xright = get_spin("shinfo_originx"),
	    ybelow = get_spin("shinfo_originy");
	if (xright != shape->get_xright() || ybelow != shape->get_ybelow())
		{			// It changed.
		file_info->set_modified();
		shape->set_offset(xright, ybelow);
		}
	if (info)
		save_shape_notebook(*info, shnum, frnum);
	}

/*
 *	Close the shape-editing window.
 */

void ExultStudio::close_shape_window
	(
	)
	{
	if (shapewin)
		gtk_widget_hide(shapewin);
	}

/*
 *	Paint the shape in the draw area.
 */

void ExultStudio::show_shinfo_shape
	(
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!shape_draw)
		return;
	shape_draw->configure();
					// Yes, this is kind of redundant...
	int shnum = get_num_entry("shinfo_shape");
	int frnum = get_num_entry("shinfo_frame");
	shape_draw->draw_shape_centered(shnum, frnum);
	if (w != -1)
		shape_draw->show(x, y, w, h);
	else
		shape_draw->show();
	}

/*
 *	Paint the gump shape on the container page.
 */

void ExultStudio::show_shinfo_gump
	(
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!gump_draw)
		return;
	gump_draw->configure();
					// Yes, this is kind of redundant...
	int shnum = get_spin("shinfo_gump_num");
	gump_draw->draw_shape_centered(shnum, 0);
	if (w != -1)
		gump_draw->show(x, y, w, h);
	else
		gump_draw->show();
	}

/*
 *	Paint the gump shape on the NPC paperdoll page.
 */

void ExultStudio::show_shinfo_npcgump
	(
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!npcgump_draw || game_type != BLACK_GATE)
		return;
	npcgump_draw->configure();
					// Yes, this is kind of redundant...
	int shnum = get_spin("shinfo_npcpaperdoll_gump");
	if (shnum < 0)
		return;
	npcgump_draw->draw_shape_centered(shnum, 0);
	if (w != -1)
		npcgump_draw->show(x, y, w, h);
	else
		npcgump_draw->show();
	}

/*
 *	Paint the body shape on the body page.
 */

void ExultStudio::show_shinfo_body
	(
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!body_draw)
		return;
	body_draw->configure();
					// Yes, this is kind of redundant...
	int shnum = get_spin("shinfo_body_shape");
	int frnum = get_spin("shinfo_body_frame");
	set_spin("shinfo_body_frame", frnum, 0, vgafile->get_ifile()->get_num_frames(shnum)-1);
	body_draw->draw_shape_centered(shnum, frnum);
	if (w != -1)
		body_draw->show(x, y, w, h);
	else
		body_draw->show();
	}

/*
 *	Paint the explosion sprite on the body page.
 */

void ExultStudio::show_shinfo_explosion
	(
	int x, int y, int w, int h	// Rectangle. w=-1 to show all.
	)
	{
	if (!explosion_draw)
		return;
	explosion_draw->configure();
					// Yes, this is kind of redundant...
	int shnum = get_spin("shinfo_explosion_sprite");
	Vga_file *ifile = spritefile->get_ifile();
	int frnum = ifile->get_num_frames(shnum)/2;
	explosion_draw->draw_shape_centered(shnum, frnum);
	if (w != -1)
		explosion_draw->show(x, y, w, h);
	else
		explosion_draw->show();
	}
