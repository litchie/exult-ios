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

#include "game.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "misc_buttons.h"
#include "Paperdoll_gump.h"
#include "contain.h"
#include "actors.h"
#include "objiter.h"
#include "cheat.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif

using std::cerr;
using std::endl;
using std::size_t;
using std::snprintf;

//#define SHOW_USECODE_CONTAINER
//#define SHOW_NONREADIED_OBJECTS

/*
 *
 *	SERPENT ISLE PAPERDOLL GUMP
 *
 */

/*
 *	Statics:
 */

// Paperdoll is completely different to Actor
short Paperdoll_gump::diskx = 123, Paperdoll_gump::disky = 137;
short Paperdoll_gump::heartx = 97, Paperdoll_gump::hearty = 137;
short Paperdoll_gump::combatx = 51, Paperdoll_gump::combaty = 142;
short Paperdoll_gump::cstatx = 72, Paperdoll_gump::cstaty = 137;
short Paperdoll_gump::cmodex = 75, Paperdoll_gump::cmodey = 140;
short Paperdoll_gump::halox = 123, Paperdoll_gump::haloy = 120;

short Paperdoll_gump::coords[36] = {
	76, 20,		/* head */	115, 27,	/* back */
	92,  61,	/* belt */	38, 55,		/* lhand */
	55, 62,		/* lfinger */	80, 80,		/* legs */
	84, 105,	/* feet */	90, 50,		/* rfinger */
	117, 55,	/* rhand */	83, 43,		/* torso */
	44, 26,		/* neck */	69, 59,		/* ammo */
	59, 19,		/* back2 */	94, 20,		/* back 3 (shield) */
	76, 26,		/* ears */	76, 33,		/* cloak */
	73, 53,		/* gloves */	0, 0		/* usecode container */
};
short Paperdoll_gump::coords_blue[36] = {
	76, 20,		/* head */	64, 27,		/* back */
	84, 61,		/* belt */	30, 58,		/* lhand */
	55, 62,		/* lfinger */	80, 80,		/* legs */
	84, 105,	/* feet */	90, 50,		/* rfinger */
	68, 50,		/* rhand */	83, 43,		/* torso */
	22, 26,		/* neck */	69, 59,		/* ammo */
	59, 19,		/* back2 */	94, 20,		/* back 3 (shield) */
	76, 26,		/* ears */	76, 33,		/* cloak */
	73, 53,		/* gloves */	0, 0		/* usecode container */
};
short Paperdoll_gump::shapes_blue[36] = {
	54, 0,		/* head */	54, 0,		/* back */
	54, 0,		/* belt */	55, 0,		/* lhand */
	55, 0,		/* lfinger */	54, 0,		/* legs */
	54, 0,		/* feet */	54, 0,		/* rfinger */
	54, 0,		/* rhand */	54, 0,		/* torso */
	54, 0,		/* neck */	54, 0,		/* ammo */
	55, 0,		/* back2 */	54, 0,		/* back 3 (shield) */
	54, 0,		/* ears */	54, 0,		/* cloak */
	54, 0,		/* gloves */	54, 0		/* usecode container */
};
short Paperdoll_gump::coords_hot[36] = {
	76, 20,		/* head */	94, 27,		/* back */
	92,  61,	/* belt */	38, 55,		/* lhand */
	55, 62,		/* lfinger */	80, 80,		/* legs */
	84, 105,	/* feet */	90, 50,		/* rfinger */
	117, 55,	/* rhand */	83, 43,		/* torso */
	76, 41,		/* neck */	69, 59,		/* ammo */
	59, 19,		/* back2 */	94, 20,		/* back 3 (shield) */
	76, 26,		/* ears */	76, 33,		/* cloak */
	73, 53,		/* gloves */	0, 0		/* usecode container */
};



//
// Paperdoll Coords
//
	
short Paperdoll_gump::bodyx = 46, Paperdoll_gump::bodyy = 33;
short Paperdoll_gump::headx = 46, Paperdoll_gump::heady = 22;

short Paperdoll_gump::beltfx = 58, Paperdoll_gump::beltfy = 52;
short Paperdoll_gump::neckfx = 46, Paperdoll_gump::neckfy = 47;
short Paperdoll_gump::beltmx = 57, Paperdoll_gump::beltmy = 55;
short Paperdoll_gump::neckmx = 46, Paperdoll_gump::neckmy = 44;

short Paperdoll_gump::legsx = 57, Paperdoll_gump::legsy = 66;
short Paperdoll_gump::feetx = 46, Paperdoll_gump::feety = 99;

short Paperdoll_gump::handsx = 68, Paperdoll_gump::handsy = 44;
short Paperdoll_gump::rhandx = 68, Paperdoll_gump::rhandy = 44;
short Paperdoll_gump::lhandx = 34, Paperdoll_gump::lhandy = 65;

short Paperdoll_gump::ahandx = 28, Paperdoll_gump::ahandy = 59;
short Paperdoll_gump::ammox = 28, Paperdoll_gump::ammoy = 59;

short Paperdoll_gump::backfx = 68, Paperdoll_gump::backfy = 28;
short Paperdoll_gump::back2fx = 34, Paperdoll_gump::back2fy = 22;
short Paperdoll_gump::backmx = 68, Paperdoll_gump::backmy = 22;
short Paperdoll_gump::back2mx = 35, Paperdoll_gump::back2my = 22;
short Paperdoll_gump::shieldfx = 56, Paperdoll_gump::shieldfy = 25;
short Paperdoll_gump::shieldmx = 57, Paperdoll_gump::shieldmy = 22;


/*
 *	Find the index of the closest 'spot' to a mouse point.
 *
 *	Output:	Index, or -1 if unsuccessful.
 */

int Paperdoll_gump::find_closest
	(
	int mx, int my,			// Mouse point in window.
	int only_empty			// Only allow empty spots.
	)
{
	mx -= x; my -= y;		// Get point rel. to us.
	long closest_squared = 1000000;	// Best distance squared.
	int closest = -1;		// Best index.
	int spot;

	for (size_t i = 0; i < sizeof(coords_hot)/(2*sizeof(coords_hot[0])); i++)
	{
		if (Game::get_game_type() == BLACK_GATE && i > Actor::shield_spot)
			continue;
		else if (Game::get_game_type() == BLACK_GATE && i == Actor::shield_spot)
			spot = Actor::belt;
		else if (Game::get_game_type() == BLACK_GATE && i == Actor::back2h_spot)
			spot = Actor::belt;
		else
			spot = i;

		int dx = mx - coords_hot[spot*2], dy = my - coords_hot[spot*2+1];
		long dsquared = dx*dx + dy*dy;

		// Better than prev and free if required.?
		if (dsquared < closest_squared && !(only_empty && container->get_readied(spot)))
		{
			closest_squared = dsquared;
			closest = spot;
		}
	}

	return (closest);
}

/*
 *	Create the gump display for an actor.
 */

Paperdoll_gump::Paperdoll_gump
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum			// Shape #.
	) : Gump(cont, initx, inity, 123, SF_PAPERDOL_VGA)
{
	set_object_area(Rectangle(26, 0, 104, 140), 6, 145);

	// Create Heart button
	heart_button = new Heart_button(this, heartx, hearty);
	Actor *actor = (Actor *) cont;
	// Create Cstats button or Halo and Cmode
	if (Game::get_game_type() == BLACK_GATE)
	{
		if (cont->get_npc_num() == 0)
			halo_button = new Halo_button(this, halox, haloy, 
									actor);
		else
			halo_button = new Halo_button(this, diskx, disky,
									actor);

		cmode_button = new Combat_mode_button(this, cmodex, cmodey, 
									actor);
		cstats_button = NULL;
	}
	else
	{
		cstats_button = new Cstats_button(this, cstatx, cstaty);
		halo_button = NULL;
		cmode_button = NULL;
	}



	// If Avatar create Disk Button
	if (cont->get_npc_num() == 0)
		disk_button = new Disk_button(this, diskx, disky);
	else
		disk_button = NULL;
		

	// If Avtar create Combat Button
	if (cont->get_npc_num() == 0)
		combat_button = new Combat_button(this, combatx, combaty);
	else
		combat_button = NULL;


	// Put all the objects in the right place
	for (size_t i = 0; i < sizeof(coords)/2*sizeof(coords[0]); i++)
	{
		Game_object *obj = container->get_readied(i);
		if (obj)
			set_to_spot(obj, i);
	}
}

/*
 *	Delete actor display.
 */

Paperdoll_gump::~Paperdoll_gump
	(
	)
{
	if (heart_button) delete heart_button;
	if (disk_button) delete disk_button;
	if (combat_button) delete combat_button;
	if (cstats_button) delete cstats_button;
	if (halo_button) delete halo_button;
	if (cmode_button) delete cmode_button;
}

/*
 *	Is a given screen point on one of our buttons?
 *
 *	Output: ->button if so.
 */

Gump_button *Paperdoll_gump::on_button
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
	else if (cstats_button && cstats_button->on_button(gwin, mx, my))
		return cstats_button;
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

int Paperdoll_gump::add
	(
	Game_object *obj,
	int mx, int my,			// Screen location of mouse.
	int sx, int sy,			// Screen location of obj's hotspot.
	bool dont_check,		// Skip volume check.
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
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

/*
 *	Set object's coords. to given spot.
 */

void Paperdoll_gump::set_to_spot
	(
	Game_object *obj,
	int index			// Spot index.
	)
{
	Game_window *gwin = Game_window::get_game_window();
	
	// Get shape.
	Shape_frame *shape = obj->get_shape();
	//if (!shape)			// Funny?  Try frame 0.
	//	shape = gwin->get_shape(obj->get_shapenum(), 0);
	if (!shape)
		return;

	// Height and width
	int w = shape->get_width(), h = shape->get_height();
	
	// Set object's position.
	obj->set_chunk(spotx(index) + shape->get_xleft() - w/2 - object_area.x,
		spoty(index) + shape->get_yabove() - h/2 - object_area.y);
}

/*
 *	Paint on screen.
 */

void Paperdoll_gump::paint
	(
	Game_window *gwin
	)
{
	Game_object *obj;

	// Paint Objects
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.

	gwin->paint_shape(x, y, *this);

	// Paint red "checkmark".
	check_button->paint(gwin);

	// Get the information required about ourself
	Actor *actor = dynamic_cast<Actor *> (container);
	Paperdoll_npc *info = GetCharacterInfo (container->get_shapenum());
	if (!info) info = GetCharacterInfo (actor->get_shape_real());
	if (!info && Game::get_game_type() != BLACK_GATE) info = Characters;
	else if (!info) info = Characters_BG;

	// Spots that are female/male specific
	int	shieldx, shieldy,
		back2x,  back2y,
		backx,   backy,
		neckx,   necky,
		beltx,   belty;

	if (info->is_female)		// Set the female spots
	{
		shieldx = shieldfx;
		shieldy = shieldfy;
		back2x = back2fx;
		back2y = back2fy;
		backx = backfx;
		backy = backfy;
		neckx = neckfx;
		necky = neckfy;
		beltx = beltfx;
		belty = beltfy;
	}
	else				// Set the male spots
	{
		shieldx = shieldmx;
		shieldy = shieldmy;
		back2x = back2mx;
		back2y = back2my;
		backx = backmx;
		backy = backmy;
		neckx = neckmx;
		necky = neckmy;
		beltx = beltmx;
		belty = beltmy;
	}
	
	// Now paint. Order is very specific


	if (Game::get_game_type() != BLACK_GATE)
	{
		paint_object      (gwin, box, info, Actor::shield_spot,shieldx,shieldy);
		paint_object      (gwin, box, info, Actor::back2h_spot, back2x, back2y);
		paint_object      (gwin, box, info, Actor::back,        backx,  backy);
		paint_object      (gwin, box, info, Actor::cloak_spot,  bodyx,  bodyy);
		paint_body        (gwin, box, info);
		paint_object      (gwin, box, info, Actor::legs,        legsx,  legsy);
		paint_object      (gwin, box, info, Actor::feet,        feetx,  feety);		
		paint_object      (gwin, box, info, Actor::ammo,        ammox,  ammoy);
		paint_object      (gwin, box, info, Actor::torso,       bodyx,  bodyy);
		paint_belt        (gwin, box, info);
		paint_head        (gwin, box, info);
		paint_object      (gwin, box, info, Actor::neck,        neckx,  necky);
		paint_object      (gwin, box, info, Actor::belt,        beltx,  belty);
		paint_arms        (gwin, box, info);
		paint_object_arms (gwin, box, info, Actor::torso,       bodyx,  bodyy, 1, Actor::torso);
		paint_object      (gwin, box, info, Actor::ears_spot,   headx,  heady);
		paint_object      (gwin, box, info, Actor::head,        headx,  heady);
		paint_object      (gwin, box, info, Actor::cloak_spot,  bodyx,  bodyy, 0, Actor::special_spot);
		paint_object_arms (gwin, box, info, Actor::rfinger,     lhandx, lhandy, 0);
		paint_object_arms (gwin, box, info, Actor::lfinger,     rhandx, rhandy, 0);
		paint_object_arms (gwin, box, info, Actor::hands2_spot, handsx, handsy, 0);
		paint_object      (gwin, box, info, Actor::lhand,       lhandx, lhandy);
		paint_object      (gwin, box, info, Actor::ammo,        ahandx, ahandy, 2, -1, Actor::lhand);
		paint_object      (gwin, box, info, Actor::rhand,       rhandx, rhandy);

		// if debugging show usecode container
#ifdef SHOW_USECODE_CONTAINER
		paint_object      (gwin, box, info, Actor::ucont_spot,  20,      20 );
#endif
	}
	else
	{
		Paperdoll_item *item1, *item2;

		paint_object      (gwin, box, info, Actor::belt,       shieldx,shieldy, 0, Actor::shield_spot);
		paint_object      (gwin, box, info, Actor::belt,        back2x, back2y, 0, Actor::back2h_spot);
		paint_object      (gwin, box, info, Actor::back,        backx,  backy);
		paint_object      (gwin, box, info, Actor::torso,       bodyx,  bodyy,  0, Actor::cloak_spot);
		paint_body        (gwin, box, info);
		paint_object      (gwin, box, info, Actor::legs,        legsx,  legsy);
		paint_object      (gwin, box, info, Actor::feet,        feetx,  feety);		
		paint_object      (gwin, box, info, Actor::ammo,        ammox,  ammoy);

		obj = container->get_readied(Actor::torso);
		item1 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::cloak_spot);
		item2 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::special_spot);
		if (!item1 && !item2)
			paint_object      (gwin, box, info, Actor::torso,       bodyx,  bodyy);
		
		paint_belt        (gwin, box, info);
		paint_head        (gwin, box, info);
		paint_object      (gwin, box, info, Actor::neck,        neckx,  necky);

		obj = container->get_readied(Actor::belt);
		item1 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::shield_spot);
		item2 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::back2h_spot);
		if (!item1 && !item2)
			paint_object      (gwin, box, info, Actor::belt,        beltx,  belty);

		paint_arms        (gwin, box, info);
		paint_object_arms (gwin, box, info, Actor::torso,       bodyx,  bodyy, 1, Actor::torso);
		paint_object      (gwin, box, info, Actor::head,        headx,  heady);
		paint_object      (gwin, box, info, Actor::torso,       bodyx,  bodyy, 0, Actor::special_spot);
		paint_object_arms (gwin, box, info, Actor::rfinger,     lhandx, lhandy, 0);

		obj = container->get_readied(Actor::lfinger);
		item1 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::hands2_spot);
		if (!item1)
			paint_object_arms (gwin, box, info, Actor::lfinger,     rhandx, rhandy, 0);
		else
			paint_object_arms (gwin, box, info, Actor::lfinger,     handsx, handsy, 0, Actor::hands2_spot);

		paint_object      (gwin, box, info, Actor::lhand,       lhandx, lhandy);
		paint_object      (gwin, box, info, Actor::ammo,        ahandx, ahandy, 2, -1, Actor::lhand);
		paint_object      (gwin, box, info, Actor::rhand,       rhandx, rhandy);
	}
	
#ifdef SHOW_NONREADIED_OBJECTS
	Object_iterator iter(container->get_objects());
	while ((obj = iter.get_next()) != 0)
		if (actor->find_readied(obj) == -1)
			gwin->paint_shape(box.x, box.y, *obj);
#endif


	// Paint buttons.
	if (heart_button) heart_button->paint(gwin);
	if (disk_button) disk_button->paint(gwin);
	if (combat_button) combat_button->paint(gwin);
	if (cstats_button) cstats_button->paint(gwin);
	if (halo_button) halo_button->paint(gwin);
	if (cmode_button) cmode_button->paint(gwin);

					// Show weight.
	int max_weight = container->get_max_weight();
	int weight = container->get_weight()/10;
	char text[20];
	snprintf(text, 20, "%d/%d", weight, max_weight);
	int twidth = gwin->get_text_width(2, text);
	const int boxw = 102;
	gwin->paint_text(2, text, x + 84 - (twidth/2), y + 114);
}


/*
 *	Paint a generic object on screen
 */

void Paperdoll_gump::paint_object 
	(
	Game_window *gwin,		// gwin
	const Rectangle &box,		// box
	Paperdoll_npc *info,		// info
	int spot,			// Actor::belt
	int sx, int sy,			// back2x, back2y
	int frame,			// 0
	int itemtype,			// Actor::back2h_spot
	int checkspot,			// -1
	int checktype			// -1
	)
{
	Game_object *obj = container->get_readied(spot);
	if (!obj) return;

	int old_it = itemtype;
	if (itemtype == -1) itemtype = spot;
	
	Paperdoll_item *item = GetItemInfo (obj->get_shapenum(), obj->get_framenum(), itemtype);
	if (!item || item->frame == -1 || item->shape == -1)
	{
		if ((old_it != -1 && !item)|| checkspot != -1) return;
		//if (!obj->get_cx() && !obj->get_cy()) return;

		set_to_spot(obj, spot);
	
		ShapeID s(shapes_blue[spot*2], shapes_blue[spot*2+1], SF_GUMPS_VGA);

		if (Game::get_game_type() == BLACK_GATE) s.set_file(SF_BG_SIGUMP_FLX);

		gwin->paint_shape(box.x + coords_blue[spot*2],
				box.y + coords_blue[spot*2+1],
				s);
		int ox = box.x + obj->get_cx(), oy = box.y + obj->get_cy();
		gwin->paint_shape(x, y, *obj);
		if (cheat.is_selected(obj))
					// Outline selected obj.
			gwin->paint_hit_outline(ox, oy, obj->get_shape());

		return;
	}
	else if (checkspot != -1)
	{
		Game_object *check = container->get_readied(checkspot);
		if (!check) return;
	
		if (checktype == -1) checktype = checkspot;
	
		Paperdoll_item *item_check = GetItemInfo (check->get_shapenum(), check->get_framenum(), checktype);
		if (!item_check || item_check->type != item->type)
			return;
	}


	int f = item->frame;

	if (frame == 1)
		f = item->frame2;
	else if (frame == 2)
		f = item->frame3;
	else if (frame == 3)
		f = item->frame4;
		
	if (item->gender && !info->is_female) f++;
 
	ShapeID s(item->shape, f, item->file);
	gwin->paint_shape(box.x + sx, box.y + sy, s, 1);
	if (cheat.is_selected(obj))	// Outline selected obj.
		gwin->paint_hit_outline(box.x + sx, box.y + sy, s.get_shape());
}

/*
 *	Paint with arms frame
 */
void Paperdoll_gump::paint_object_arms
	(
	Game_window *gwin,
	const Rectangle &box,
	Paperdoll_npc *info,
	int spot,
	int sx, int sy,
	int start,
	int itemtype
	)
{
	switch (get_arm_type())
	{
		default:
		paint_object (gwin, box, info, spot, sx, sy, start, itemtype);
		break;
		
		case OT_Double:
		paint_object (gwin, box, info, spot, sx, sy, start+1, itemtype);
		break;
		
		case OT_Staff:
		paint_object (gwin, box, info, spot, sx, sy, start+2, itemtype);
		break;
	}	
}

/*
 *	Paint the body
 */
void Paperdoll_gump::paint_body 
	(
	Game_window *gwin,
	const Rectangle &box,
	Paperdoll_npc *info
	)
{
	ShapeID s(info->body_shape, info->body_frame, SF_PAPERDOL_VGA);
	gwin->paint_shape (box.x + bodyx, box.y + bodyy, s);
}

/*
 *	Paint the belt
 */
void Paperdoll_gump::paint_belt 
	(
	Game_window *gwin,
	const Rectangle &box,
	Paperdoll_npc *info
	)
{
	ShapeID s(10, 0, SF_PAPERDOL_VGA);
	if (!info->is_female) s.set_frame(1);
	gwin->paint_shape (box.x + beltmx, box.y + beltmy, s);
}

/*
 *	Paint the head
 */
void Paperdoll_gump::paint_head 
	(
	Game_window *gwin,
	const Rectangle &box,
	Paperdoll_npc *info
	)
{
	Game_object *obj = container->get_readied(Actor::head);

	Paperdoll_item *item = NULL;
	if (obj) item = GetItemInfo (obj->get_shapenum(), obj->get_framenum());

	int f = info->head_frame;

	if (item && item->type == OT_Helm)
		f = info->head_frame_helm;

	ShapeID s(info->head_shape, f, info->file);
	gwin->paint_shape(box.x + headx, box.y + heady, s);
}

/*
 *	Paint the arms
 */
void Paperdoll_gump::paint_arms 
	(
	Game_window *gwin,
	const Rectangle &box,
	Paperdoll_npc *info
	)
{
	Game_object *obj = container->get_readied(Actor::rhand);

	Paperdoll_item *item = NULL;
	if (obj) item = GetItemInfo (obj->get_shapenum(), obj->get_framenum());

	ShapeID s(info->arms_shape, info->arms_frame, SF_PAPERDOL_VGA);

	switch (get_arm_type())
	{
		case OT_Double:
		s.set_frame(info->arms_frame_2h);
		break;

		case OT_Staff:
		s.set_frame(info->arms_frame_staff);
		break;

		default:
		break;
	}

	gwin->paint_shape (box.x + bodyx, box.y + bodyy, s);
}


/*
 *	Gets which arm frame to use
 */

Paperdoll_gump::Object_type Paperdoll_gump::get_arm_type(void)
{
	Game_object *obj = container->get_readied(Actor::lhand);
	if (!obj) return OT_Normal;
	
	Paperdoll_item *item = GetItemInfo (obj->get_shapenum(), obj->get_framenum());

	if (item) return item->type;

	return OT_Normal;
}


/*
 *	Find object a screen point is on.
 *
 *	Output:	Object found, or null.
 */

Game_object * Paperdoll_gump::find_object
	(
	int mx, int my			// Mouse pos. on screen.
	)
{

	Game_window *gwin = Game_window::get_game_window();
	
	// Check Objects
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.
	mx -= box.x;
	my -= box.y;

	// Get the information required about ourself
	Actor *actor = dynamic_cast<Actor *> (container);
	Paperdoll_npc *info = GetCharacterInfo (container->get_shapenum());
	if (!info) info = GetCharacterInfo (actor->get_shape_real());
	if (!info && Game::get_game_type() != BLACK_GATE) info = Characters;
	else if (!info) info = Characters_BG;

	int	shieldx, shieldy,
		back2x,  back2y,
		backx,   backy,
		neckx,   necky,
		beltx,   belty;

	if (info->is_female)
	{
		shieldx = shieldfx;
		shieldy = shieldfy;
		back2x = back2fx;
		back2y = back2fy;
		backx = backfx;
		backy = backfy;
		neckx = neckfx;
		necky = neckfy;
		beltx = beltfx;
		belty = beltfy;
	}
	else
	{
		shieldx = shieldmx;
		shieldy = shieldmy;
		back2x = back2mx;
		back2y = back2my;
		backx = backmx;
		backy = backmy;
		neckx = neckmx;
		necky = neckmy;
		beltx = beltmx;
		belty = beltmy;
	}

	Game_object *obj;
	
	
	// Must be done in this order (reverse of rendering)
	if (Game::get_game_type() != BLACK_GATE)
	{
		// if debugging show usecode container
#ifdef SHOW_USECODE_CONTAINER
		if (obj = check_object      (gwin, mx, my, info, Actor::ucont_spot,  0,      0 ))
			return obj;
#endif

		if ((obj = check_object      (gwin, mx, my, info, Actor::rhand,       rhandx, rhandy)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::ammo,        ahandx, ahandy, 2, -1, Actor::lhand)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::lhand,       lhandx, lhandy)))
			return obj;
		if ((obj = check_object_arms (gwin, mx, my, info, Actor::hands2_spot, handsx, handsy, 0)))
			return obj;
		if ((obj = check_object_arms (gwin, mx, my, info, Actor::rfinger,     lhandx, lhandy, 0)))
			return obj;
		if ((obj = check_object_arms (gwin, mx, my, info, Actor::lfinger,     rhandx, rhandy, 0)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::cloak_spot,  bodyx,  bodyy,  0, Actor::special_spot)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::head,        headx,  heady)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::ears_spot,   headx,  heady)))
			return obj;
		if ((obj = check_object_arms (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy,  1, Actor::torso)))
			return obj;
		if (check_arms              (gwin, mx, my, info))
			return NULL;
		if ((obj = check_object      (gwin, mx, my, info, Actor::belt,        beltx,  belty)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::neck,        neckx,  necky)))
			return obj;
		if (check_head              (gwin, mx, my, info))
			return NULL;
		if (check_belt              (gwin, mx, my, info))
			return NULL;
		if ((obj = check_object      (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::ammo,        ammox,  ammoy)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::feet,        feetx,  feety)))	
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::legs,        legsx,  legsy)))
			return obj;
		if (check_body        (gwin, mx, my, info))
			return NULL;
		if ((obj = check_object      (gwin, mx, my, info, Actor::cloak_spot,  bodyx,  bodyy)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::back,        backx,  backy)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::back2h_spot, back2x, back2y)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::shield_spot,shieldx,shieldy)))
			return obj;
	}
	else
	{
		Paperdoll_item *item1, *item2;

		if ((obj = check_object      (gwin, mx, my, info, Actor::rhand,       rhandx, rhandy)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::ammo,        ahandx, ahandy, 2, -1, Actor::lhand)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::lhand,       lhandx, lhandy)))
			return obj;

		obj = container->get_readied(Actor::lfinger);
		item1 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::hands2_spot);
		if (!item1 && (obj = check_object_arms (gwin, mx, my, info, Actor::lfinger,     rhandx, rhandy, 0)))
			return obj;
		else if ((obj = check_object_arms (gwin, mx, my, info, Actor::lfinger,     rhandx, rhandy, 0, Actor::hands2_spot)))
			return obj;

		if ((obj = check_object_arms (gwin, mx, my, info, Actor::rfinger,     lhandx, lhandy, 0)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy,  0, Actor::special_spot)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::head,        headx,  heady)))
			return obj;
		if ((obj = check_object_arms (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy,  1, Actor::torso)))
			return obj;
		if (check_arms              (gwin, mx, my, info))
			return NULL;

		obj = container->get_readied(Actor::belt);
		item1 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::shield_spot);
		item2 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::back2h_spot);
		if (!item1 && !item2 && (obj = check_object      (gwin, mx, my, info, Actor::belt,        beltx,  belty)))
			return obj;

		if ((obj = check_object      (gwin, mx, my, info, Actor::neck,        neckx,  necky)))
			return obj;
		if (check_head              (gwin, mx, my, info))
			return NULL;
		if (check_belt              (gwin, mx, my, info))
			return NULL;

		obj = container->get_readied(Actor::torso);
		item1 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::cloak_spot);
		item2 = !obj?NULL:GetItemInfo (obj->get_shapenum(), obj->get_framenum(), Actor::special_spot);
		if (!item1 && !item2 && (obj = check_object      (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy)))
			return obj;

		if ((obj = check_object      (gwin, mx, my, info, Actor::ammo,        ammox,  ammoy)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::feet,        feetx,  feety)))	
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::legs,        legsx,  legsy)))
			return obj;

		if (check_body        (gwin, mx, my, info))
			return NULL;
			
		if ((obj = check_object      (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy, 0, Actor::cloak_spot)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::back,        backx,  backy)))
			return obj;

		if ((obj = check_object      (gwin, mx, my, info, Actor::belt,        back2x, back2y, 0, Actor::back2h_spot)))
			return obj;
		if ((obj = check_object      (gwin, mx, my, info, Actor::belt,       shieldx,shieldy, 0, Actor::shield_spot)))
			return obj;
	}
	return NULL;
}

/*
 *	Checks for a generic object on screen
 */

Game_object * Paperdoll_gump::check_object 
	(
	Game_window *gwin,
	int mx, int my,
	Paperdoll_npc *info,
	int spot,
	int sx, int sy,
	int frame,
	int itemtype,
	int checkspot,
	int checktype
	)
{
	Game_object *obj = container->get_readied(spot);
	if (!obj) return NULL;
	
	int old_it = itemtype;
	if (itemtype == -1) itemtype = spot;
	
	Paperdoll_item *item = GetItemInfo (obj->get_shapenum(), obj->get_framenum(), itemtype);
	if (!item || item->frame == -1 || item->shape == -1)
	{
		if ((old_it != -1 &&!item) || checkspot != -1) return 0;
		
		if (!obj->get_cx() && !obj->get_cy()) set_to_spot(obj, spot);
		
		if (check_shape (gwin, mx - obj->get_cx(), my - obj->get_cy(),
			obj->get_shapenum(), obj->get_framenum(), obj->get_shapefile()))
		{
			return obj;
		}
				
		return NULL;
	}
	else if (checkspot != -1)
	{
		Game_object *check = container->get_readied(checkspot);
		if (!check) return NULL;
	
		if (checktype == -1) checktype = checkspot;
	
		Paperdoll_item *item_check = GetItemInfo (check->get_shapenum(), check->get_framenum(), checktype);
		if (!item_check || item_check->type != item->type)
			return NULL;
	}


	int f = item->frame;

	if (frame == 1)
		f = item->frame2;
	else if (frame == 2)
		f = item->frame3;
	else if (frame == 3)
		f = item->frame4;
		
	if (item->gender && !info->is_female) f++;

	if (check_shape (gwin, mx - sx, my - sy, item->shape, f, item->file))
	{
		Shape_frame *shape = obj->get_shape();
		int w = shape->get_width(), h = shape->get_height();
					// Set object's position.
		obj->set_chunk(mx + shape->get_xleft() - w/2, my + shape->get_yabove() - h/2);
		
		return obj;
	}
	
	return NULL;
}

/*
 *	Checks for object with arms frame
 */
Game_object * Paperdoll_gump::check_object_arms
	(
	Game_window *gwin,
	int mx, int my,
	Paperdoll_npc *info,
	int spot,
	int sx, int sy,
	int start,
	int itemtype
	)
{
	switch (get_arm_type())
	{
		default:
		return check_object (gwin, mx, my, info, spot, sx, sy, start, itemtype);
		
		case OT_Double:
		return check_object (gwin, mx, my, info, spot, sx, sy, start+1, itemtype);
		
		case OT_Staff:
		return check_object (gwin, mx, my, info, spot, sx, sy, start+2, itemtype);
	}	
	return NULL;
}

/*
 *	Checks for the body
 */
bool Paperdoll_gump::check_body 
	(
	Game_window *gwin,
	int mx, int my,
	Paperdoll_npc *info
	)
{
	return check_shape (gwin, mx - bodyx, my - bodyy, info->body_shape, info->body_frame, SF_PAPERDOL_VGA);
}

/*
 *	Checks for the belt
 */
bool Paperdoll_gump::check_belt 
	(
	Game_window *gwin,
	int mx, int my,
	Paperdoll_npc *info
	)
{
	if (info->is_female) return check_shape (gwin, mx - beltfx, my - beltfy, 10, 0, SF_PAPERDOL_VGA);
	else return check_shape (gwin, mx - beltmx, my - beltmy, 10, 1, SF_PAPERDOL_VGA);

	return false;
}

/*
 *	Checks for the head
 */
bool Paperdoll_gump::check_head 
	(
	Game_window *gwin,
	int mx, int my,
	Paperdoll_npc *info
	)
{
	Game_object *obj = container->get_readied(Actor::head);

	Paperdoll_item *item = NULL;
	if (obj) item = GetItemInfo (obj->get_shapenum(), obj->get_framenum());

	int f = info->head_frame;

	if (item && item->type == OT_Helm)
		f = info->head_frame_helm;

	return check_shape (gwin, mx - headx, my - heady, info->head_shape, f, info->file);
}

/*
 *	Checks for the arms
 */
bool Paperdoll_gump::check_arms 
	(
	Game_window *gwin,
	int mx, int my,
	Paperdoll_npc *info
	)
{
	Game_object *obj = container->get_readied(Actor::rhand);

	Paperdoll_item *item = NULL;
	if (obj) item = GetItemInfo (obj->get_shapenum(), obj->get_framenum());

	switch (get_arm_type())
	{
		default:
		return check_shape (gwin, mx - bodyx, my - bodyy, info->arms_shape, info->arms_frame, SF_PAPERDOL_VGA);

		case OT_Double:
		return check_shape (gwin, mx - bodyx, my - bodyy, info->arms_shape, info->arms_frame_2h, SF_PAPERDOL_VGA);

		case OT_Staff:
		return check_shape (gwin, mx - bodyx, my - bodyy, info->arms_shape, info->arms_frame_staff, SF_PAPERDOL_VGA);
	}
	return false;
}

/*
 *	Generic Shaper checking
 */
bool Paperdoll_gump::check_shape
	(
	Game_window *gwin,
	int px, int py,
	int shape, int frame,
	ShapeFile file
	)
{
	ShapeID sid(shape, frame, file);
	Shape_frame *s = sid.get_shape();
	
	// If no shape, return
	if (!s) return false;

	Rectangle r = gwin->get_shape_rect (s, 0, 0);
	
	// If point not in rectangle, return
	if (!r.has_point (px, py)) return false;
	
	// If point not in shape, return
	if (!s->has_point (px, py)) return false;
	
	return true;
}

Container_game_object *Paperdoll_gump::find_actor(int mx, int my)
{
	return container;
}
