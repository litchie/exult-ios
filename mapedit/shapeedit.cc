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

using	std::cout;
using	std::endl;

/*
 *	Equip window's Cancel button.
 */
extern "C" void on_equip_cancel_clicked
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
extern "C" gboolean on_equip_window_delete_event
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
extern "C" gboolean on_equip_recnum_changed
	(
	GtkWidget *widget,
	GdkEventFocus *event,
	gpointer user_data
	)
	{
	int recnum = gtk_spin_button_get_value_as_int(
						GTK_SPIN_BUTTON(widget));
	ExultStudio::get_instance()->init_equip_window(recnum);
	return TRUE;
	}

/*
 *	Widgets in one row of the 'equipment' dialog:
 */
struct Equip_row_widgets
	{
	GtkWidget *draw, *shape, *name, *chance, *count;
	};

static Equip_row_widgets equip_rows[10];// Holds widgets from equip. dialog.

/*
 *	Set up 'equipment' dialog's table, which has 10 identical rows.
 */

static void Setup_equip
	(
	GtkTable *table,		// Table to fill.
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
		rows[row].draw = drawingarea;
		gtk_widget_show (drawingarea);
		gtk_container_add (GTK_CONTAINER(frame), drawingarea);
		gtk_widget_set_usize (drawingarea, 20, 40);
					// Shape #:
  		GtkWidget *spin = gtk_spin_button_new (GTK_ADJUSTMENT(
			gtk_adjustment_new (1, 0, 1023, 1, 50, 50)), 1, 0);
		rows[row].shape = spin;
		gtk_widget_show(spin);
		gtk_table_attach (table, spin, 1, 2, row + 2, row + 3,
                		    (GtkAttachOptions) (GTK_FILL),
		                    (GtkAttachOptions) (0), 0, 0);
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
		const char *nm = (names && shnum > 0) ? names[shnum] : "";
		gtk_label_set_text(GTK_LABEL(widgets.name), nm);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.chance),
						elem.get_probability());
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets.count),
						elem.get_quantity());
		}
	}

#if 0

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
		elem.set(gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(widgets.shape)), 
			 gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(widgets.chance)),
			 gtk_spin_button_get_value(
					GTK_SPIN_BUTTON(widgets.count)));
		}
	}
#endif
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
		Setup_equip(GTK_TABLE(table), equip_rows);
		}
					// This will cause the data to be set:
	set_spin("equip_recnum", recnum, 1, ecnt);
#if 0
	if (shape_draw)			// Ifile might have changed.
		delete shape_draw;
	shape_draw = 0;
	if (ifile && palbuf)
		{
		shape_draw = new Shape_draw(ifile, palbuf,
			    glade_xml_get_widget(app_xml, "shinfo_draw"));
//		shape_draw->enable_drop(Shape_shape_dropped, this);
		}
#endif
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
 *	Shape window's Cancel button.
 */
extern "C" void on_shinfo_cancel_clicked
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
extern "C" gboolean on_shape_window_delete_event
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
extern "C" gboolean on_shinfo_draw_expose_event
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
 *	Shape window's Equip-Edit button.
 */
extern "C" void on_open_equip_button_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	ExultStudio *s = ExultStudio::get_instance();
	s->open_equip_window(s->get_spin("shinfo_monster_equip"));
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
	set_spin("shinfo_xtiles", info.get_3d_xtiles());
	set_spin("shinfo_ytiles", info.get_3d_ytiles());
	set_spin("shinfo_ztiles", info.get_3d_height());
	int spot = info.get_ready_type();
	if (spot < 0)
		spot = 3;		// Left hand if looks invalid.
	else if (spot == 100)
		spot = 18;		// LR Hand.  Not sure about this.
	else if (spot > 17)
		spot = 3;
	set_optmenu("shinfo_ready_spot", spot);
	set_spin("shinfo_weight", info.get_weight());
	set_spin("shinfo_volume", info.get_volume());
	unsigned char wx, wy;		// Weapon-in-hand offset.
	info.get_weapon_offset(frnum, wx, wy);
	set_spin("shinfo_wihx", wx);
	set_spin("shinfo_wihy", wy);
					// Bunch of flags:
	set_toggle("shinfo_sfx_check", info.has_sfx());
	set_toggle("shinfo_strange_check", info.has_strange_movement());
	set_toggle("shinfo_animated_check", info.is_animated());
	set_toggle("shinfo_solid_check", info.is_solid());
	set_toggle("shinfo_water_check", info.is_water());
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
		set_spin("shinfo_weapon_proj", winfo->get_projectile());
		set_optmenu("shinfo_weapon_uses", winfo->get_uses());
		set_spin("shinfo_weapon_sfx", winfo->get_sfx());
		set_spin("shinfo_weapon_hitsfx", winfo->get_hitsfx());
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
					// 'Explode'???
		set_toggle("shinfo_weapon_returns", winfo->returns());
		}
	Ammo_info *ainfo = info.get_ammo_info();
	set_toggle("shinfo_ammo_check", ainfo != 0);
	set_visible("shinfo_ammo_box", ainfo != 0);
	if (ainfo)			// Setup ammo page.
		{
		set_spin("shinfo_ammo_damage", ainfo->get_damage());
		set_spin("shinfo_ammo_family", ainfo->get_family_shape());
		set_optmenu("shinfo_ammo_type", ainfo->get_damage());
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
		}
	Armor_info *arinfo = info.get_armor_info();
	set_toggle("shinfo_armor_check", arinfo != 0);
	set_visible("shinfo_armor_box", arinfo != 0);
	if (arinfo)			// Setup armor page.
		{
		static char *immun[] = {"shinfo_armor_immun0",
					"shinfo_armor_immun1",
					"shinfo_armor_immun2",
					"shinfo_armor_immun3" };
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
					"shinfo_monster_vuln3" };
		static char *immun[] = {"shinfo_monster_immun0",
					"shinfo_monster_immun1",
					"shinfo_monster_immun2",
					"shinfo_monster_immun3" };
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
		static char *flags[] = {"shinfo_monster_flag0",
					"shinfo_monster_flag1",
					"shinfo_monster_flag2",
					"shinfo_monster_flag3",
					"shinfo_monster_flag4" };
		set_bit_toggles(&flags[0],
			sizeof(flags)/sizeof(flags[0]), minfo->get_flags());
		}
	gtk_widget_show(book);
	}
#if 0
/*
 *	Save the shape-editing notebook.
 */

void ExultStudio::save_shape_notebook
	(
	Shape_info& info,
	GtkWidget *book,		// The notebook.
	int shnum,			// Shape #.
	int frnum			// Frame #.
	)
	{
	static int classes[] = {0, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14};
	int shclass = classes[get_optmenu("shinfo_shape_class")];
					classes[shclass] : 0);
	int xtiles = get_spin("shinfo_xtiles");
	int ytiles = get_spin("shinfo_ytiles");
	int ztiles = set_spin("shinfo_ztiles");
	int spot = get_optment("shinfo_ready_spot");
	if (spot == 18)			// LR hand.
		spot = 100;
	int weight = get_spin("shinfo_weight");
	int volume = get_spin("shinfo_volume");
	unsigned char wx = get_spin("shinfo_wihx"),
		      wy = get_spin("shinfo_wihy");
					// Bunch of flags:
	info.set_sfx(get_toggle("shinfo_sfx_check"));
	info.set_strange_movement(get_toggle("shinfo_strange_check"));
	info.set_animated(get_toggle("shinfo_animated_check"));
	info.set_solid(get_toggle("shinfo_solid_check"));
	info.set_water(get_toggle("shinfo_water_check"));
	info.set_poisonous(get_toggle("shinfo_poison_check"));
	info.set_field(get_toggle("shinfo_field_check"));
	info.set_door(get_toggle("shinfo_door_check"));
	info.set_barge_part(get_toggle("shinfo_barge_check"));
	info.set_transparent(get_toggle("shinfo_transp_check"));
	info.set_light_source(get_toggle("shinfo_light_check"));
	info.set_translucency(get_toggle("shinfo_transl_check"));
	info.set_obstacle(get_toggle("shinfo_obstaclex_check"), 
			get_toggle("shinfo_obstacley_check"));
	info.set_occludes(get_toggle("shinfo_occludes_check"));
					// Extras.
++++++++++++++++++
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
		set_spin("shinfo_weapon_proj", winfo->get_projectile());
		set_optmenu("shinfo_weapon_uses", winfo->get_uses());
		set_spin("shinfo_weapon_sfx", winfo->get_sfx());
		set_spin("shinfo_weapon_hitsfx", winfo->get_hitsfx());
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
					// 'Explode'???
		set_toggle("shinfo_weapon_returns", winfo->returns());
		}
	Ammo_info *ainfo = info.get_ammo_info();
	set_toggle("shinfo_ammo_check", ainfo != 0);
	set_visible("shinfo_ammo_box", ainfo != 0);
	if (ainfo)			// Setup ammo page.
		{
		set_spin("shinfo_ammo_damage", ainfo->get_damage());
		set_spin("shinfo_ammo_family", ainfo->get_family_shape());
		set_optmenu("shinfo_ammo_type", ainfo->get_damage());
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
		}
	Armor_info *arinfo = info.get_armor_info();
	set_toggle("shinfo_armor_check", arinfo != 0);
	set_visible("shinfo_armor_box", arinfo != 0);
	if (arinfo)			// Setup armor page.
		{
		static char *immun[] = {"shinfo_armor_immun0",
					"shinfo_armor_immun1",
					"shinfo_armor_immun2",
					"shinfo_armor_immun3" };
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
					"shinfo_monster_vuln3" };
		static char *immun[] = {"shinfo_monster_immun0",
					"shinfo_monster_immun1",
					"shinfo_monster_immun2",
					"shinfo_monster_immun3" };
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
		static char *flags[] = {"shinfo_monster_flag0",
					"shinfo_monster_flag1",
					"shinfo_monster_flag2",
					"shinfo_monster_flag3",
					"shinfo_monster_flag4" };
		set_bit_toggles(&flags[0],
			sizeof(flags)/sizeof(flags[0]), minfo->get_flags());
		}
	gtk_widget_show(book);
	}
#endif
/*
 *	Open the shape-editing window.
 */

void ExultStudio::open_shape_window
	(
	int shnum,			// Shape #.
	int frnum,			// Frame #.
	Vga_file *ifile,		// File it's in.
	char *shname,			// ->shape name, or null.
	Shape_info *info		// Info. if in main object shapes.
	)
	{
	if (!shapewin)			// First time?
		shapewin = glade_xml_get_widget( app_xml, "shape_window" );
	if (shape_draw)			// Ifile might have changed.
		delete shape_draw;
	shape_draw = 0;
	if (ifile && palbuf)
		{
		shape_draw = new Shape_draw(ifile, palbuf,
			    glade_xml_get_widget(app_xml, "shinfo_draw"));
//		shape_draw->enable_drop(Shape_shape_dropped, this);
		}
					// Init. shape address to info.
	gtk_object_set_user_data(GTK_OBJECT(shapewin), info);
					// Shape/frame.
	set_entry("shinfo_shape", shnum, false, false);
	set_entry("shinfo_frame", frnum);
					// Store name, #frames.
	set_entry("shinfo_name", shname ? shname : "");
	set_spin("shinfo_num_frames", ifile->get_num_frames(shnum));
					// Show xright, ybelow.
	Shape_frame *shape = ifile->get_shape(shnum, frnum);
	set_spin("shinfo_originx", shape->get_xright());
	set_spin("shinfo_originy", shape->get_ybelow());
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
	if (!shnum)			// Don't draw shape 0.
		shnum = -1;
	shape_draw->draw_shape_centered(shnum, frnum);
	if (w != -1)
		shape_draw->show(x, y, w, h);
	else
		shape_draw->show();
	}

