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

#include "Actor_gump.h"
#include "actors.h"
#include "gamewin.h"
#include "misc_buttons.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif

using std::size_t;
using std::snprintf;

#define TWO_HANDED_BROWN_SHAPE	48
#define TWO_HANDED_BROWN_FRAME	0
#define TWO_FINGER_BROWN_SHAPE	48
#define TWO_FINGER_BROWN_FRAME	1

/*
 *	Statics:
 */

short Actor_gump::diskx = 124, Actor_gump::disky = 115;
short Actor_gump::heartx = 124, Actor_gump::hearty = 132;
short Actor_gump::combatx = 52, Actor_gump::combaty = 103;
short Actor_gump::halox = 47, Actor_gump::haloy = 110;
short Actor_gump::cmodex = 48, Actor_gump::cmodey = 132;
short Actor_gump::coords[24] = {
	114, 10,	/* head */	115, 24,	/* back */
	115, 37,	/* belt */	115, 55,	/* lhand */
	115, 71,	/* lfinger */	114, 85,	/* legs */
	76, 98,		/* feet */	35, 70,		/* rfinger */
	37, 56,		/* rhand */	37, 37,		/* torso */
	37, 24,		/* neck */	37, 11		/* ammo */
};

/*
 *	Find the index of the closest 'spot' to a mouse point.
 *
 *	Output:	Index, or -1 if unsuccessful.
 */

int Actor_gump::find_closest
	(
	int mx, int my,			// Mouse point in window.
	int only_empty			// Only allow empty spots.
	)
{
	mx -= x; my -= y;		// Get point rel. to us.
	long closest_squared = 1000000;	// Best distance squared.
	int closest = -1;		// Best index.
	for (size_t i = 0; i < sizeof(coords)/(2*sizeof(coords[0])); i++)
	{
		int dx = mx - spotx(i), dy = my - spoty(i);
		long dsquared = dx*dx + dy*dy;
					// Better than prev.?
		if (dsquared < closest_squared && (!only_empty ||
						!container->get_readied(i)))
		{
			closest_squared = dsquared;
			closest = i;
		}
	}
	return (closest);
}

/*
 *	Create the gump display for an actor.
 */

Actor_gump::Actor_gump
	(
	Container_game_object *cont,	// Container it represents.  MUST
					//   be an Actor.
	int initx, int inity, 		// Coords. on screen.
	int shnum			// Shape #.
	) : Gump(cont, initx, inity, shnum)
{
	set_object_area(Rectangle(26, 0, 104, 132), 6, 136);
	Actor *npc = cont->as_actor();
	heart_button = new Heart_button(this, heartx, hearty);
	if (npc->get_npc_num() == 0)
		disk_button = new Disk_button(this, diskx, disky);
	else
		disk_button = NULL;
	if (npc->get_npc_num() == 0)
		combat_button = new Combat_button(this, combatx, combaty);
	else
		combat_button = NULL;
	halo_button = new Halo_button(this, halox, haloy, npc);
	cmode_button = new Combat_mode_button(this, cmodex, cmodey, npc);
							
	for (size_t i = 0; i < sizeof(coords)/2*sizeof(coords[0]); i++)
	{			// Set object coords.
		Game_object *obj = container->get_readied(i);
		if (obj)
			set_to_spot(obj, i);
	}
}

/*
 *	Delete actor display.
 */

Actor_gump::~Actor_gump
	(
	)
{
	if (heart_button) delete heart_button;
	if (disk_button) delete disk_button;
	if (combat_button) delete combat_button;
	if (halo_button) delete halo_button;
	if (cmode_button) delete cmode_button;
}

/*
 *	Is a given screen point on one of our buttons?
 *
 *	Output: ->button if so.
 */

Gump_button *Actor_gump::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
{
	Gump_button *btn = Gump::on_button(gwin, mx, my);
	if (btn)
		return btn;
	else if (heart_button && heart_button->on_button(gwin, mx, my))
		return heart_button;
	else if (disk_button && disk_button->on_button(gwin, mx, my))
		return disk_button;
	else if (combat_button && combat_button->on_button(gwin, mx, my))
		return combat_button;
	else if (halo_button && halo_button->on_button(gwin, mx, my))
		return halo_button;
	else if (cmode_button && cmode_button->on_button(gwin, mx, my))
		return cmode_button;
	return 0;
}

/*
 *	Add an object.
 *
 *	Output:	0 if cannot add it.
 */

int Actor_gump::add
	(
	Game_object *obj,
	int mx, int my,			// Screen location of mouse.
	int sx, int sy,			// Screen location of obj's hotspot.
	bool dont_check,		// Skip volume check.
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
#if 1
{
	Game_object *cont = find_object(mx, my);
	
	if (cont && cont->add(obj, false, combine))
		return (1);
	
	int index = find_closest(mx, my, 1);
	
	if (index != -1 && container->add_readied(obj, index))
		return (1);

	if (container->add(obj, dont_check, combine))
		return (1);

	return (0);
}
#else
{
					// Find index of closest spot.
	int index = find_closest(mx, my);
	if (!container->add_readied(obj, index))
	{			// Can't add it there?
					// Try again for an empty spot.
		index = find_closest(mx, my, 1);
		if (index < 0 || !container->add_readied(obj, index))
					// Just try to add it.
			if (!container->add(obj))
				return (0);
	}
					// In case it went in another obj:
	index = container->find_readied(obj);
	if (index >= 0)
		set_to_spot(obj, index);// Set obj. coords.
	return (1);
}
#endif

/*
 *	Set object's coords. to given spot.
 */

void Actor_gump::set_to_spot
	(
	Game_object *obj,
	int index			// Spot index.
	)
{
					// Get shape info.
	Shape_frame *shape = obj->get_shape();
	if (!shape)
		return;			// Not much we can do.
	int w = shape->get_width(), h = shape->get_height();
					// Set object's position.
	obj->set_chunk(spotx(index) + shape->get_xleft() - w/2 - object_area.x,
		spoty(index) + shape->get_yabove() - h/2 - object_area.y);
					// Shift if necessary.
	int x0 = obj->get_cx() - shape->get_xleft(), 
	    y0 = obj->get_cy() - shape->get_yabove();
	int newcx = obj->get_cx(), newcy = obj->get_cy();
	if (x0 < 0)
		newcx -= x0;
	if (y0 < 0)
		newcy -= y0;
	int x1 = x0 + w, y1 = y0 + h;
	if (x1 > object_area.w)
		newcx -= x1 - object_area.w;
	if (y1 > object_area.h)
		newcy -= y1 - object_area.h;
	obj->set_chunk(newcx, newcy);
}

/*
 *	Paint on screen.
 */

void Actor_gump::paint
	(
	)
{
					// Watch for any newly added objs.
	for (size_t i = 0; i < sizeof(coords)/2*sizeof(coords[0]); i++)
	{			// Set object coords.
		Game_object *obj = container->get_readied(i);
		if (obj)//&& !obj->get_cx() && !obj->get_cy())
			set_to_spot(obj, i);
	}

	Gump::paint();			// Paint gump & objects.

	// Paint over blue lines for 2 handed
	Actor *actor = container->as_actor();
	if (actor) {
		if (actor->is_two_fingered()) {
			int sx = x + 36,	// Note this is the right finger slot shifted slightly
				sy = y + 70;
			ShapeID sid(TWO_FINGER_BROWN_SHAPE, TWO_FINGER_BROWN_FRAME, SF_GUMPS_VGA);
			sid.paint_shape (sx,sy);
		}
		if (actor->is_two_handed()) {
			int sx = x + 36,	// Note this is the right hand slot shifted slightly
				sy = y + 55;
			ShapeID sid(TWO_HANDED_BROWN_SHAPE, TWO_HANDED_BROWN_FRAME, SF_GUMPS_VGA);
			sid.paint_shape (sx,sy);
		}
	}
					// Paint buttons.
	if (heart_button) heart_button->paint();
	if (disk_button) disk_button->paint();
	if (combat_button) combat_button->paint();
	if (halo_button) halo_button->paint();
	if (cmode_button) cmode_button->paint();
					// Show weight.
	int max_weight = container->get_max_weight();
	int weight = container->get_weight()/10;
	char text[20];
	snprintf(text, 20, "%d/%d", weight, max_weight);
	int twidth = gwin->get_text_width(2, text);
	const int boxw = 102;
	gwin->paint_text(2, text, x + 28 + (boxw - twidth)/2, y + 120);
}

Container_game_object * Actor_gump::find_actor(int mx, int my)
{
	return container;
}
