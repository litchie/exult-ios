/*
Copyright (C) 2000 The Exult Team

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

#include "gamewin.h"
#include "Gump.h"
#include "Gump_widget.h"


/*
 *	Is a given screen point on this widget?
 */

int Gump_widget::on_widget
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
{
	mx -= parent->get_x() + x;	// Get point rel. to gump.
	my -= parent->get_y() + y;
	Shape_frame *cshape = gwin->get_gump_shape(shapenum, 0);
	return (cshape->has_point(mx, my));
}
