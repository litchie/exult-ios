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

#include "game.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "misc_buttons.h"
#include "Paperdoll_gump.h"
#include "contain.h"
#include "objiter.h"
#include "actors.h"

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
short Paperdoll_gump::coords[36] = {
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
short Paperdoll_gump::lhandx = 35, Paperdoll_gump::lhandy = 66;

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
	
	for (size_t i = 0; i < sizeof(coords)/(2*sizeof(coords[0])); i++)
	{
		int dx = mx - spotx(i), dy = my - spoty(i);
		long dsquared = dx*dx + dy*dy;

		// Better than prev and free if required.?
		if (dsquared < closest_squared && !(only_empty && container->get_readied(i)))
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

Paperdoll_gump::Paperdoll_gump
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum			// Shape #.
	) : Gump(cont, initx, inity, 123, true)
{
	// Create Heart button
	heart_button = new Heart_button(this, heartx, hearty);
			
	// Create Cstats button
	cstats_button = new Cstats_button(this, cstatx, cstaty);


	// If Avtar create Disk Button
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
	if (heart_button)
		delete heart_button;
	if (disk_button)
		delete disk_button;
	if (combat_button)
		delete combat_button;
	if (cstats_button)
		delete cstats_button;
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
	int sx, int sy			// Screen location of obj's hotspot.
	)
{
	Game_object *cont = find_object(mx, my);
	
	if (cont && cont->add(obj))
		return (1);
	
	int index = find_closest(mx, my, 1);
	
	if (index != -1 && container->add_readied(obj, index))
		return (1);

	if (container->add(obj))
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
	Shape_frame *shape = gwin->get_shape(*obj);
	
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
	// Paint Objects
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.

	// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum(), is_paperdoll());
	
	// Paint red "checkmark".
	paint_button(gwin, check_button);

	// Get the information required about ourself
	Paperdoll_npc *info = GetCharacterInfo (container->get_shapenum());

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
	paint_object_arms (gwin, box, info, Actor::torso,       bodyx,  bodyy, 1);
	paint_object      (gwin, box, info, Actor::ears_spot,   headx,  heady);
	paint_object      (gwin, box, info, Actor::head,        headx,  heady);
	paint_object      (gwin, box, info, Actor::cloak_spot,  bodyx,  bodyy, 0, Actor::special_spot);
	paint_object_arms (gwin, box, info, Actor::lfinger,     rhandx, rhandy, 0);
	paint_object_arms (gwin, box, info, Actor::rfinger,     lhandx, lhandy, 0);
	paint_object_arms (gwin, box, info, Actor::hands2_spot, handsx, handsy, 0);
	paint_object      (gwin, box, info, Actor::lhand,       lhandx, lhandy);
	paint_object      (gwin, box, info, Actor::ammo,        ahandx, ahandy, 2, -1, Actor::lhand);
	paint_object      (gwin, box, info, Actor::rhand,       rhandx, rhandy);

// if debugging show usecode container
#ifdef DEBUG
	paint_object      (gwin, box, info, Actor::ucont_spot,  0,      0,     0, Actor::special_spot);
#endif


	// Paint buttons.
	if (heart_button)
		paint_button(gwin, heart_button);
	if (disk_button)
		paint_button(gwin, disk_button);
	if (combat_button)
		paint_button(gwin, combat_button);
	if (cstats_button)
		paint_button(gwin, cstats_button);

					// Show weight.
	int max_weight = 2*container->get_property(Actor::strength);
	int weight = container->get_weight()/10;
	char text[20];
	sprintf(text, "%d/%d", weight, max_weight);
	int twidth = gwin->get_text_width(2, text);
	const int boxw = 102;
	gwin->paint_text(2, text, x + 84 - (twidth/2), y + 114);
}


/*
 *	Paint a generic object on screen
 */

void Paperdoll_gump::paint_object 
	(
	Game_window *gwin,
	const Rectangle &box,
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
	if (!obj) return;
	
	if (itemtype == -1) itemtype = spot;
	
	Paperdoll_item *item = GetItemInfo (obj->get_shapenum(), obj->get_framenum(), itemtype);
	if (!item)
	{
		if (spot != itemtype || checkspot != -1) return;
		
		if (!obj->get_cx() && !obj->get_cy()) set_to_spot(obj, spot);
		
		if (spot == Actor::lhand)	// Right hand line
			gwin->paint_gump (box.x + 30, box.y + 58, 55, 0);
		else if (spot == Actor::rhand)	// Left hand line
			gwin->paint_gump (box.x + 68, box.y + 50, 54, 0);
			
		gwin->paint_shape(box.x + obj->get_cx(),box.y + obj->get_cy(), 
			obj->get_shapenum(), obj->get_framenum());
			
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

	gwin->paint_gump (box.x + sx, box.y + sy, item->shape, f, true);
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
	gwin->paint_gump (box.x + bodyx, box.y + bodyy, info->body_shape, info->body_frame, true);
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
	if (info->is_female) gwin->paint_gump (box.x + beltfx, box.y + beltfy, 10, 0, true);
	else gwin->paint_gump (box.x + beltmx, box.y + beltmy, 10, 1, true);
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

	gwin->paint_gump (box.x + headx, box.y + heady, info->head_shape, f, true);
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

	switch (get_arm_type())
	{
		default:
		gwin->paint_gump (box.x + bodyx, box.y + bodyy, info->arms_shape, info->arms_frame, true);
		return;

		case OT_Double:
		gwin->paint_gump (box.x + bodyx, box.y + bodyy, info->arms_shape, info->arms_frame_2h, true);
		return;

		case OT_Staff:
		gwin->paint_gump (box.x + bodyx, box.y + bodyy, info->arms_shape, info->arms_frame_staff, true);
		return;
	}
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

	Paperdoll_npc *info = GetCharacterInfo (container->get_shapenum());

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

// if debugging show usecode container
#ifdef DEBUG
	if (obj = check_object      (gwin, mx, my, info, Actor::ucont_spot,  0,      0,      0, Actor::special_spot))
		return obj;
#endif
	
	// Must be done in this order
	if (obj = check_object      (gwin, mx, my, info, Actor::rhand,       rhandx, rhandy))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::ammo,        ahandx, ahandy, 2, -1, Actor::lhand))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::lhand,       lhandx, lhandy))
		return obj;
	if (obj = check_object_arms (gwin, mx, my, info, Actor::hands2_spot, handsx, handsy, 0))
		return obj;
	if (obj = check_object_arms (gwin, mx, my, info, Actor::rfinger,     lhandx, lhandy, 0))
		return obj;
	if (obj = check_object_arms (gwin, mx, my, info, Actor::lfinger,     rhandx, rhandy, 0))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::cloak_spot,  bodyx,  bodyy,  0, Actor::special_spot))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::head,        headx,  heady))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::ears_spot,   headx,  heady))
		return obj;
	if (obj = check_object_arms (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy,  1))
		return obj;
	if (check_arms              (gwin, mx, my, info))
		return NULL;
	if (obj = check_object      (gwin, mx, my, info, Actor::belt,        beltx,  belty))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::neck,        neckx,  necky))
		return obj;
	if (check_head              (gwin, mx, my, info))
		return NULL;
	if (check_belt              (gwin, mx, my, info))
		return NULL;
	if (obj = check_object      (gwin, mx, my, info, Actor::torso,       bodyx,  bodyy))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::ammo,        ammox,  ammoy))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::feet,        feetx,  feety))		
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::legs,        legsx,  legsy))
		return obj;

	if (check_body        (gwin, mx, my, info))
		return NULL;
		
	if (obj = check_object      (gwin, mx, my, info, Actor::cloak_spot,  bodyx,  bodyy))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::back,        backx,  backy))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::back2h_spot, back2x, back2y))
		return obj;
	if (obj = check_object      (gwin, mx, my, info, Actor::shield_spot,shieldx,shieldy))
		return obj;
	
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
	
	if (itemtype == -1) itemtype = spot;
	
	Paperdoll_item *item = GetItemInfo (obj->get_shapenum(), obj->get_framenum(), itemtype);
	if (!item)
	{
		if (spot != itemtype || checkspot != -1) return 0;
		
		if (!obj->get_cx() && !obj->get_cy()) set_to_spot(obj, spot);
		
		if (check_shape (gwin, mx - obj->get_cx(), my - obj->get_cy(),
			obj->get_shapenum(), obj->get_framenum(), true))
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

	if (check_shape (gwin, mx - sx, my - sy, item->shape, f))
	{
		Shape_frame *shape = gwin->get_shape(*obj);
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
	return check_shape (gwin, mx - bodyx, my - bodyy, info->body_shape, info->body_frame);
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
	if (info->is_female) return check_shape (gwin, mx - beltfx, my - beltfy, 10, 0);
	else return check_shape (gwin, mx - beltmx, my - beltmy, 10, 1);

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

	return check_shape (gwin, mx - headx, my - heady, info->head_shape, f);
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
		return check_shape (gwin, mx - bodyx, my - bodyy, info->arms_shape, info->arms_frame);

		case OT_Double:
		return check_shape (gwin, mx - bodyx, my - bodyy, info->arms_shape, info->arms_frame_2h);

		case OT_Staff:
		return check_shape (gwin, mx - bodyx, my - bodyy, info->arms_shape, info->arms_frame_staff);
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
	bool notpd
	)
{
	Shape_frame *s;
	
	if (notpd) s = gwin->get_shape(shape, frame);
	else s = gwin->get_gump_shape(shape, frame, true);
	
	// If no shape, return
	if (!s) return false;

	Rectangle r = gwin->get_shape_rect (s, 0, 0);
	
	// If point not in rectangle, return
	if (!r.has_point (px, py)) return false;
	
	// If point not in shape, return
	if (!s->has_point (px, py)) return false;
	
	return true;
}
