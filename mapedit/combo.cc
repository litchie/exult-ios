/**
 **	Combo.cc - A combination of multiple objects.
 **
 **	Written: 4/26/02 - JSF
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
#include "combo.h"
#include "shapedraw.h"
#include "exult_constants.h"
#include "shapevga.h"


/*
 *	Open combo window.
 */

C_EXPORT void on_new_combo1_activate
	(
	GtkMenuItem     *menuitem,
        gpointer         user_data
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
//	studio->open_locator_window();
	}

/*
 *	Callbacks for "Combo" creation window.
 */
C_EXPORT void
on_combo_apply_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	//+++++++++++++
}

C_EXPORT void
on_combo_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(button));
	//+++++++++++++
	gtk_widget_hide(win);
}

C_EXPORT void
on_combo_locx_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
//+++++++	ExultStudio::get_instance()->set_edit_lift(
//+++++++			gtk_spin_button_get_value_as_int(button));
}

C_EXPORT void
on_combo_locy_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
//+++++++	ExultStudio::get_instance()->set_edit_lift(
//+++++++			gtk_spin_button_get_value_as_int(button));
}

C_EXPORT void
on_combo_locz_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
//+++++++	ExultStudio::get_instance()->set_edit_lift(
//+++++++			gtk_spin_button_get_value_as_int(button));
}

C_EXPORT void
on_combo_order_changed			(GtkSpinButton *button,
					 gpointer	  user_data)
{
//+++++++	ExultStudio::get_instance()->set_edit_lift(
//+++++++			gtk_spin_button_get_value_as_int(button));
}

/*
 *	Create empty combo.
 */

Combo::Combo
	(
	Shapes_vga_file *svga
	) : shapes_file(svga),
	    starttx(0), startty(0), xtiles(0), ytiles(0), ztiles(0), 
	    hot_index(-1)
	{
	}

/*
 *	Clean up.
 */

Combo::~Combo
	(
	)
	{
	for (vector<Combo_member *>::iterator it = members.begin();
					it != members.end(); ++it)
		delete *it;
	}

/*
 *	Add a new object.
 */

void Combo::add
	(
	int tx, int ty, int tz,		// Location rel. to top-left.
	int shnum, int frnum		// Shape.
	)
	{
	Combo_member *memb = new Combo_member(tx, ty, tz, shnum, frnum);
	members.push_back(memb);
					// Figure visible top-left tile.
	Shape_info& info = shapes_file->get_info(shnum);
	int xtiles = info.get_3d_xtiles(frnum),
	    ytiles = info.get_3d_ytiles(frnum),
	    ztiles = info.get_3d_height();
	int vtx = tx - xtiles + 1 - (tz + ztiles)/2, 
	    vty = ty - ytiles + 1 - (tz + ztiles)/2;
	if (vtx < starttx)		// Adjust our starting point.
		starttx = vtx;
	if (vty < startty)
		startty = vty;
	if (hot_index == -1)		// First one?
		hot_index = 0;
	}

/*
 *	Remove i'th object.
 */

void Combo::remove
	(
	int i
	)
	{
	//+++++++++++
	}

/*
 *	Paint in a drawing area.
 */

void Combo::draw
	(
	Shape_draw *draw
	)
	{
	for (vector<Combo_member *>::iterator it = members.begin();
					it != members.end(); ++it)
		{
		Combo_member *m = *it;
					// Figure pixels up-left for lift.
		int lft = m->tz*(c_tilesize/2);
					// Figure relative tile.
		int mtx = m->tx - starttx,
		    mty = m->ty - startty;
		int x = mtx*c_tilesize - lft,
		    y = mty*c_tilesize - lft;
		draw->draw_shape(m->shapenum, m->framenum, x, y);
		}
	}
