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

using	std::cout;
using	std::endl;

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

	void open_shape_window(int shnum, int frnum, Shape_info *info = 0);
	void close_shape_window();
/*
 *	Open the shape-editing window.
 */

void ExultStudio::open_shape_window
	(
	int shnum,			// Shape #.
	int frmnum,			// Frame #.
	Shape_info *info		// Info. if in main object shapes.
	)
	{
	if (!shapewin)			// First time?
		{
		shapewin = glade_xml_get_widget( app_xml, "shape_window" );
#if 0
		if (vgafile && palbuf)
			{
			shape_draw = new Shape_draw(vgafile, palbuf,
			    glade_xml_get_widget(app_xml, "shape_draw"));
			shape_draw->enable_drop(Shape_shape_dropped, this);
			}
#endif
		}
					// Init. shape address to info.
	gtk_object_set_user_data(GTK_OBJECT(shapewin), info);
	gtk_widget_show(shapewin);
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

