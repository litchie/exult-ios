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
#include "exult_constants.h"
#include "shapevga.h"
#include "shapefile.h"
#include "ibuf8.h"

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
	studio->open_combo_window();
	}
void ExultStudio::open_combo_window
	(
	)
	{
	if (!vgafile)
		{
		EStudio::Alert("'shapes.vga' file isn't present");
		return;
		}
	Shapes_vga_file *svga = (Shapes_vga_file *) vgafile->get_ifile();
	delete combowin;		// Delete old (svga may have changed).
	combowin = new Combo_editor(svga, palbuf);
	combowin->show(true);
	}

/*
 *	Callbacks for "Combo" creation window.
 */
C_EXPORT gint on_combo_draw_expose_event
	(
	GtkWidget *widget,		// The view window.
	GdkEventExpose *event,
	gpointer data			// ->Shape_chooser.
	)
	{
	Combo_editor *combo = (Combo_editor *) gtk_object_get_user_data(
		GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(widget))));
	combo->render(&event->area);
	return TRUE;
	}

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
	Shape_draw *draw,
	int selected			// Index of 'selected' item, or -1.
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
		if (it - members.begin() == selected)
					// Outline selected.
					// FOR NOW, use color #1 ++++++++
			draw->draw_shape_outline(m->shapenum, m->framenum,
						x, y, 1);
		}
	}

/*
 *	Create combo editor.
 */

Combo_editor::Combo_editor
	(
	Shapes_vga_file *svga,		// File containing shapes.
	unsigned char *palbuf		// Palette for drawing shapes.
	) : Shape_draw(svga, palbuf, glade_xml_get_widget(
		ExultStudio::get_instance()->get_xml(), "combo_draw")),
	    selected(-1)
	{
	static bool first = true;
	combo = new Combo(svga);
	GladeXML *app_xml = ExultStudio::get_instance()->get_xml();
	win = glade_xml_get_widget(app_xml, "combo_win");
	gtk_object_set_user_data(GTK_OBJECT(win), this);
	if (first)			// Indicate the events we want.
		{
		gtk_widget_set_events(draw, GDK_EXPOSURE_MASK | 
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		    GDK_BUTTON1_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
		first = false;
		}
			//++++++++++Testing:
	combo->add(3, 3, 0, 162, 0);
	combo->add(5, 5, 0, 161, 0);
	selected = 1;
	}

/*
 *	Clean up.
 */

Combo_editor::~Combo_editor
	(
	)
	{
	}

/*
 *	Show/hide.
 */

void Combo_editor::show
	(
	bool tf
	)
	{
	if (tf)
		gtk_widget_show(win);
	else
		gtk_widget_hide(win);
	}

/*
 *	Display.
 */

void Combo_editor::render
	(
	GdkRectangle *area		// 0 for whole draw area.
	)
	{
	Shape_draw::configure();	// Setup the first time.
					// Get dims.
	int draww = draw->allocation.width,
	    drawh = draw->allocation.height;
	GdkRectangle all;
	if (!area)
		{
		all.x = all.y = 0;
		all.width = draww;
		all.height = drawh;
		area = &all;
		}
	gdk_gc_set_clip_rectangle(drawgc, area);
	iwin->fill8(255);		// Fill with background color.
	combo->draw(this, selected);	// Draw shapes.
	Shape_draw::show(area->x, area->y, area->width, area->height);
	}
