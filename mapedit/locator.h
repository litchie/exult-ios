/**
 **	Locator.h - Locate game positions.
 **
 **	Written: March 2, 2002 - JSF
 **/

/*
Copyright (C) 2001-2002 The Exult Team

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

#ifndef INCL_LOCATOR
#define INCL_LOCATOR

/*
 *	The 'locator' window:
 */
class Locator
	{
	GtkWidget *win;			// Main window.
	GtkWidget *draw;		// GTK draw area to display.
	GdkGC *drawgc;			// For drawing in 'draw'.
	GtkAdjustment *hadj, *vadj;	// For horiz., vert. scales.
	int tx, ty, txs, tys, scale;	// Current Exult win. info. in tiles.
	void send_location();		// Send location/size to Exult.
public:
	Locator();
	~Locator();
	void show(bool tf);		// Show/hide.
					// Configure when created/resized.
	void configure(GtkWidget *widget);
	void render(GdkRectangle *area = 0);
					// Message from exult.
	void view_changed(unsigned char *data, int datalen);
					// Handle scrollbar.
	static void vscrolled(GtkAdjustment *adj, gpointer data);
	static void hscrolled(GtkAdjustment *adj, gpointer data);
	};

#endif
