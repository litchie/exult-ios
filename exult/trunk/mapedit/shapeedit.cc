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

#include "studio.h"
#include "servemsg.h"
#include "exult_constants.h"
#include "utils.h"
#include "shapeinf.h"
#include "monstinf.h"
#include "u7drag.h"
#include "shapefile.h"
#include "shapedraw.h"
#include "items.h"

using	std::cout;
using	std::endl;

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
	if (file == U7_SHAPE_SHAPES && shape >= 0 && shape < 1024)
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
			gtk_adjustment_new (1, 0, 1023, 1, 50, 50)), 1, 0);
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
C_EXPORT void on_shinfo_ammo_special_check_toggled
	(
	GtkToggleButton *btn,
        gpointer user_data
	)
	{
	bool on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
	ExultStudio::get_instance()->set_sensitive("shinfo_ammo_drop", !on);
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
 *	Set frame-dependent fields in the shape-editing notebook.
 */

void ExultStudio::set_shape_notebook_frame
	(
	int frnum			// Frame # to set.
	)
	{
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
		case 0x08: return 1;
		case 0xA0: return 2;
		case 0x01: return 3;
		case 0xA1: return 4;
		case 0x40: return 5;
		case 0x48: return 6;
		case 0x20: return 8;
		case 0x28: return 11;
		case 0xA8: return 12;
		case 0x30: return 13;
		case 0x50: return 15;
		case 0x58: return 16;
		case 0x90: return 17;
		case 0x78: return 18;
			// For Usecode Container in BG:
		case 0x38: return 19;
		default: return 0;
		}
	}

inline short get_si_spots(short spot)
	{
	switch (spot)
		{
		case 0x00: return 0;
		case 0x08: return 1;
		case 0xA0: return 2;
		case 0x01: return 3;
		case 0xA1: return 4;
		case 0x50: return 5;
		case 0x20: return 6;
		case 0x48: return 7;
		case 0x18: return 8;
		case 0x10: return 9;
		case 0x78: return 10;
		case 0x60: return 11;
		case 0x28: return 12;
		case 0x40: return 13;
		case 0x58: return 14;
		case 0x70: return 15;
		case 0x68: return 16;
		case 0x30: return 19;
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
		int ammo_use = ammo > 0 ? 0 :
			winfo->uses_charges() ? 2 :
			winfo->is_thrown() ? 3 : 1;	// 1 == "None".
		set_optmenu("shinfo_weapon_ammo", ammo_use);
		set_spin("shinfo_weapon_ammo_shape", ammo, ammo > 0);
		set_spin("shinfo_weapon_proj", winfo->get_projectile(),
								0, 2047);
		set_optmenu("shinfo_weapon_uses", winfo->get_uses());
		set_spin("shinfo_weapon_sfx", winfo->get_sfx(), 0, 1023);
		set_spin("shinfo_weapon_hitsfx", winfo->get_hitsfx(), 0, 1023);
					// Show usecode in hex.
		set_entry("shinfo_weapon_uc", winfo->get_usecode(), true);
		static char *powers[] = {
					"shinfo_weapon_pow0",
					"shinfo_weapon_pow1",
					"shinfo_weapon_pow2",
					"shinfo_weapon_pow3",
					"shinfo_weapon_pow4",
					"shinfo_weapon_pow5" };
		set_bit_toggles(&powers[0], 
			sizeof(powers)/sizeof(powers[0]), winfo->get_powers());
		set_toggle("shinfo_weapon_explodes", winfo->explodes());
		set_toggle("shinfo_weapon_returns", winfo->returns());
		set_optmenu("shinfo_weapon_frames", winfo->get_actor_frames(false));
		set_optmenu("shinfo_proj_frames", winfo->get_actor_frames(true));
		set_spin("shinfo_proj_rotation", winfo->get_rotation_speed());
		set_spin("shinfo_proj_speed", winfo->get_missile_speed());
		set_spin("shinfo_proj_delay", winfo->get_cycle_delay());
		}
	Ammo_info *ainfo = info.get_ammo_info();
	set_toggle("shinfo_ammo_check", ainfo != 0);
	set_visible("shinfo_ammo_box", ainfo != 0);
	if (ainfo)			// Setup ammo page.
		{
		set_spin("shinfo_ammo_damage", ainfo->get_damage());
		set_spin("shinfo_ammo_family", ainfo->get_family_shape());
		set_optmenu("shinfo_ammo_type", ainfo->get_damage_type());
		static char *powers[] = {
					"shinfo_ammo_pow0",
					"shinfo_ammo_pow1",
					"shinfo_ammo_pow2",
					"shinfo_ammo_pow3",
					"shinfo_ammo_pow4",
					"shinfo_ammo_pow5" };
		set_bit_toggles(&powers[0], 
			sizeof(powers)/sizeof(powers[0]), ainfo->get_powers());
					// 'Explode'???
		set_toggle("shinfo_ammo_bursts", ainfo->bursts());
		set_toggle("shinfo_ammo_special", ainfo->has_special_behaviour());
		set_optmenu("shinfo_ammo_drop", ainfo->get_drop_type());
		ExultStudio::get_instance()->set_sensitive("shinfo_ammo_drop",
				!ainfo->has_special_behaviour());
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
					"shinfo_armor_immun5" };
		set_spin("shinfo_armor_value", arinfo->get_prot());
		set_bit_toggles(&immun[0], 
			sizeof(immun)/sizeof(immun[0]), arinfo->get_immune());
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
		set_optmenu("shinfo_monster_align", minfo->get_alignment());
		static char *vuln[] = {	"shinfo_monster_vuln0",
					"shinfo_monster_vuln1",
					"shinfo_monster_vuln2",
					"shinfo_monster_vuln3",
					"shinfo_monster_vuln4",
					"shinfo_monster_vuln5" };
		static char *immun[] = {"shinfo_monster_immun0",
					"shinfo_monster_immun1",
					"shinfo_monster_immun2",
					"shinfo_monster_immun3",
					"shinfo_monster_immun4",
					"shinfo_monster_immun5" };
		set_bit_toggles(&vuln[0], sizeof(vuln)/sizeof(vuln[0]),
						minfo->get_vulnerable());
		set_bit_toggles(&immun[0], 
			sizeof(immun)/sizeof(immun[0]), minfo->get_immune());
		set_toggle("shinfo_monster_splits", minfo->splits());
		set_toggle("shinfo_monster_cant_die", minfo->cant_die());
		set_toggle("shinfo_monster_cant_yell", minfo->cant_yell());
		set_toggle("shinfo_monster_cant_bleed", minfo->cant_bleed());
		set_toggle("shinfo_monster_poison_safe",
						minfo->poison_safe());
		set_toggle("shinfo_monster_teleports", minfo->can_teleport());
		set_toggle("shinfo_monster_summons", minfo->can_summon());
		set_toggle("shinfo_monster_be_invisible", 
						minfo->can_be_invisible());
		static char *flags[] = {"shinfo_monster_flag0",
					"shinfo_monster_flag1",
					"shinfo_monster_flag2",
					"shinfo_monster_flag3",
					"shinfo_monster_flag4" };
		set_bit_toggles(&flags[0],
			sizeof(flags)/sizeof(flags[0]), minfo->get_flags());
		}
	int container_gump = info.get_container_gump();
	set_toggle("shinfo_container_check", container_gump >= 0);
	set_visible("shinfo_container_box", container_gump >= 0);
	set_spin("shinfo_gump_num", container_gump >= 0 ? container_gump : 0);
	gtk_widget_show(book);
	}

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
	// In both cases, 0xFF means that it should not be saved
	const signed char bg_spots[] = {0xFF, 0x08, 0xA0, 0x01, 0xA1, 0x40, 0x48,
			0xFF, 0x20, 0xFF, 0xFF, 0x28, 0xA8, 0x30, 0xFF, 0x50, 0x58, 0x90,
			0x78, 0x38};
	const signed char si_spots[] = {0xFF, 0x08, 0xa0, 0x01, 0xa1, 0x50, 0x20,
			0x48, 0x18, 0x10, 0x78, 0x60, 0x28, 0x40, 0x58, 0x70, 0x68, 0xFF,
			0xFF, 0x30};
	if (game_type == BLACK_GATE)
		spot = bg_spots[spot];
	else
		spot = si_spots[spot];
	info.set_ready_type(spot);
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
		winfo->set_projectile(get_spin("shinfo_weapon_proj"));
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
					"shinfo_weapon_pow5" };
		winfo->set_powers(get_bit_toggles(&powers[0], 
					sizeof(powers)/sizeof(powers[0])));
					// 'Explode'???
		winfo->set_returns(get_toggle("shinfo_weapon_returns"));
		winfo->set_explodes(get_toggle("shinfo_weapon_explodes"));
		winfo->set_actor_frames(get_optmenu("shinfo_weapon_frames") |
				(get_optmenu("shinfo_proj_frames")<<2));
		winfo->set_rotation_speed(get_spin("shinfo_proj_rotation"));
		winfo->set_missile_speed(get_spin("shinfo_proj_speed"));
		winfo->set_cycle_delay(get_spin("shinfo_proj_delay"));
		}
	if (!get_toggle("shinfo_ammo_check"))
		info.set_ammo_info(false);	// Not ammo.
	else
		{
		Ammo_info *ainfo = info.set_ammo_info(true);
		ainfo->set_family_shape(get_spin("shinfo_ammo_family"));
		ainfo->set_damage(get_spin("shinfo_ammo_damage"),
				get_optmenu("shinfo_ammo_type"));
		static char *powers[] = {
					"shinfo_ammo_pow0",
					"shinfo_ammo_pow1",
					"shinfo_ammo_pow2",
					"shinfo_ammo_pow3",
					"shinfo_ammo_pow4",
					"shinfo_ammo_pow5" };
		ainfo->set_powers(get_bit_toggles(&powers[0], 
					sizeof(powers)/sizeof(powers[0])));
					// 'Explode'???
		ainfo->set_bursts(get_toggle("shinfo_ammo_bursts"));
		if (get_toggle("shinfo_ammo_special"))
			{
			ainfo->set_has_special_behaviour(true);
			ainfo->set_drop_type(0);
			}
		else
			{
			ainfo->set_has_special_behaviour(false);
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
		minfo->set_alignment(get_optmenu("shinfo_monster_align"));
		static char *vuln[] = {	"shinfo_monster_vuln0",
					"shinfo_monster_vuln1",
					"shinfo_monster_vuln2",
					"shinfo_monster_vuln3",
					"shinfo_monster_vuln4",
					"shinfo_monster_vuln5" };
		static char *immun[] = {"shinfo_monster_immun0",
					"shinfo_monster_immun1",
					"shinfo_monster_immun2",
					"shinfo_monster_immun3",
					"shinfo_monster_immun4",
					"shinfo_monster_immun5" };
		minfo->set_vulnerable(get_bit_toggles(&vuln[0], 
						sizeof(vuln)/sizeof(vuln[0])));
		minfo->set_immune(get_bit_toggles(&immun[0], 
			sizeof(immun)/sizeof(immun[0])));
		minfo->set_splits(get_toggle("shinfo_monster_splits"));
		minfo->set_cant_die(get_toggle("shinfo_monster_cant_die"));
		minfo->set_cant_yell(get_toggle("shinfo_monster_cant_yell"));
		minfo->set_cant_bleed(get_toggle("shinfo_monster_cant_bleed"));
		minfo->set_poison_safe(
				get_toggle("shinfo_monster_poison_safe"));
		minfo->set_can_teleport(
				get_toggle("shinfo_monster_teleports"));
		minfo->set_can_summon(
				get_toggle("shinfo_monster_summons"));
		minfo->set_can_be_invisible(
				get_toggle("shinfo_monster_be_invisible"));
		static char *flags[] = {"shinfo_monster_flag0",
					"shinfo_monster_flag1",
					"shinfo_monster_flag2",
					"shinfo_monster_flag3",
					"shinfo_monster_flag4" };
		minfo->set_flags(get_bit_toggles(&flags[0],
					sizeof(flags)/sizeof(flags[0])));
		}
	if (!get_toggle("shinfo_container_check"))
		info.set_container_gump(-1);
	else
		{
		info.set_container_gump(get_spin("shinfo_gump_num"));
		}
	shape_info_modified = true;
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
		}
					// Store ->'s.
	gtk_object_set_user_data(GTK_OBJECT(shapewin), info);
	gtk_object_set_data(GTK_OBJECT(shapewin), "file_info", file_info);
					// Shape/frame.
	set_entry("shinfo_shape", shnum, false, false);
	int nframes = ifile->get_num_frames(shnum);
	set_spin("shinfo_frame", frnum, 0, nframes - 1);
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
