/*
Copyright (C) 2001-2004 The Exult Team

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

#include "Face_stats.h"
#include "Face_button.h"
#include "Paperdoll_gump.h"
#include "actors.h"
#include "party.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "game.h"
#include "Gump_manager.h"


#define PALETTE_INDEX_RED	22
#define PALETTE_INDEX_GREEN	64
#define PALETTE_INDEX_BLUE	79

#define PORTRAIT_NUM_MODES	3

#define PORTRAIT_WIDTH		40

class Stat_bar : public Gump_button
{
	Actor		*actor;
	int		prop;
	int		prop_max;
	unsigned char	colour;

	int		val;
	int		max_val;
public:
	Stat_bar(Gump *par, int px, int py, Actor *a, int s, int m, unsigned char c);
	virtual void double_clicked(int x, int y);
	virtual void paint();

	virtual void activate() {}
	virtual void push() {}
	virtual void unpush() {}
					// update dirty region, if required
	virtual void update_widget();

	virtual bool is_draggable() { return false; }
};

Stat_bar::Stat_bar (Gump *par, int px, int py, Actor *a, int s, int m, unsigned char c)
	: Gump_button(par, EXULT_FLX_HP_BAR_SHP, px, py, SF_EXULT_FLX),
	actor(a), prop(s), prop_max(m), colour(c), val(-256), max_val(-256)
{

	gwin->add_dirty(get_rect());
	val = actor->get_effective_prop(prop);
	max_val = actor->get_effective_prop(prop_max);
}

void Stat_bar::double_clicked(int x, int y)
{
	gumpman->add_gump(actor, game->get_shape("gumps/statsdisplay"));
}



/*
 *	Paint on screen.
 */

void Stat_bar::paint
	(
	)
{
	Gump_button::paint();

	int width =  (val * 32) / (max_val ? max_val : 1);

	if (width > 0)
	{
		if (width > 32) width = 32;

		int px = x;
		int py = y;

		if (parent)
		{
			px += parent->get_x();
			py += parent->get_y();
		}

		gwin->get_win()->fill8(colour, width, 3, px, py);
	}
}

/*
 *	Update Dirty Region
 */

void Stat_bar::update_widget()
{
	if (val != actor->get_effective_prop(prop) || max_val != actor->get_effective_prop(prop_max))
		gwin->add_dirty(get_rect());

	val = actor->get_effective_prop(prop);
	max_val = actor->get_effective_prop(prop_max);
}

/*
 *	Portrait_button
 */

class Portrait_button : public Face_button
{
protected:
	Stat_bar	*hp;		// Bar for HP
	Stat_bar	*mana;		// Bar for MANA
	bool		hit;
	int		pois;
	int		prot;
public:
	Portrait_button(Gump *par, int px, int py, Actor *a);
	virtual ~Portrait_button();
	virtual void double_clicked(int x, int y);
	virtual void paint();

					// update dirty region, if required
	virtual void update_widget();

	virtual int on_button(int mx, int my);

	virtual Rectangle get_rect();

	virtual bool is_draggable() { return false; }
};


Portrait_button::Portrait_button(Gump *par, int px, int py, Actor *a)
	: Face_button(par, px+14, py-20, a), hp(0), mana(0)
{
	hp = new Stat_bar(par, px+4, py - 10, a, Actor::health, Actor::strength, PALETTE_INDEX_RED);

	if (actor->get_effective_prop(Actor::magic)>0) 
		mana = new Stat_bar(par, px+4, py - 5, a, Actor::mana, Actor::magic, PALETTE_INDEX_BLUE);

	hit = actor->was_hit();
	pois = actor->get_flag(Obj_flags::poisoned);
	prot = actor->get_flag(Obj_flags::protection);

	gwin->add_dirty(get_rect());
}

Portrait_button::~Portrait_button()
{
	if (hp) delete hp;
	if (mana) delete mana;
}

void Portrait_button::double_clicked(int x, int y)
{
	if (hp && hp->on_button(x, y))
		hp->double_clicked(x, y);
	else if (mana && mana->on_button(x, y)) 
		mana->double_clicked(x, y);
	else 
		actor->show_inventory();
}

int Portrait_button::on_button(int x, int y)
{
	if (hp && hp->on_button(x, y))
		return true;
	else if (mana && mana->on_button(x, y)) 
		return true;
	else if (Face_button::on_button(x, y))
		return true;

	return false;
}

void Portrait_button::update_widget()
{
	Face_button::update_widget();
	if (hp)	hp->update_widget();
	if (mana) mana->update_widget();

	if (hit != actor->was_hit() ||
		pois != actor->get_flag(Obj_flags::poisoned) ||
		prot != actor->get_flag(Obj_flags::protection))
	{
		Rectangle r = get_rect();
		gwin->add_dirty(r);
		hit = actor->was_hit();
		pois = actor->get_flag(Obj_flags::poisoned);
		prot = actor->get_flag(Obj_flags::protection);
		r = get_rect();
		gwin->add_dirty(r);
	}
}

/*
 *	Paint on screen.
 */

void Portrait_button::paint()
{
	Face_button::paint();

	Shape_frame *s = get_shape();

	if (s)
	{
		int px = x;
		int py = y;

		if (parent)
		{
			px += parent->get_x();
			py += parent->get_y();
		}

		if (hit)
		{
			sman->paint_outline(px, py, s, HIT_PIXEL);
		}
		else if (pois)
		{
			sman->paint_outline(px, py, s, POISON_PIXEL);
		}
		else if (prot)
		{
			sman->paint_outline(px, py, s, PROTECT_PIXEL);
		}
	}

	if (hp)	hp->paint();
	if (mana) mana->paint();
}

Rectangle Portrait_button::get_rect()
{
	Rectangle rect = Face_button::get_rect();
	if (hit || pois || prot) rect.enlarge(2);

	if (hp)
	{
		Rectangle r = hp->get_rect();
		rect.add(r);
	}
	if (mana)
	{
		Rectangle r = mana->get_rect();
		rect.add(r);
	}

	return rect;
}

/*
 *	Face_stats
 */

Face_stats::Face_stats() : Gump(0, 0, 0, 0, SF_GUMPS_VGA)
{
	if (self) throw exult_exception("Only 1 Set of Party Portraits Allowed!");

	for (int i = 1; i < 8; i++)
	{
		npc_nums[i] = -1;
		party[i] = 0;
	}


	create_buttons();

	self = this;
}

Face_stats::~Face_stats()
{
	delete_buttons();

	gwin->set_all_dirty();
	self = 0;
}

/*
 *	Paint on screen.
 */

void Face_stats::paint
	(
	)
{
	for (int i = 0; i < 8; i++)
		if (party[i]) party[i]->paint();
}

/*
 *	On a Button?
 */

Gump_button *Face_stats::on_button(int mx, int my)
{
	for (int i = 0; i < 8; i++)
		if (party[i] && party[i]->on_button(mx, my))
			return party[i];


	return 0;
}

// add dirty region, if dirty
void Face_stats::update_gump()
{
	if (has_changed())
	{
		delete_buttons();
		create_buttons();
	}
	else
	{
		for (int i = 0; i < 8; i++)
			if (party[i]) party[i]->update_widget();
	}
}

// Has this changed?
bool Face_stats::has_changed()
{
	if (resx != gwin->get_width() || resy != gwin->get_height())
		return true;

	if (party_size != partyman->get_count()) return true;

	for (int i = 0; i < party_size; i++)
		if (npc_nums[i+1] != partyman->get_member(i)) 
			return true;

	return false;
}

// Delete all the buttons
void Face_stats::delete_buttons()
{
	gwin->add_dirty(get_rect());

	for (int i = 0; i < 8; i++)
	{
		if (party[i])
		{
			delete party[i];
			party[i] = 0;
		}
		npc_nums[i] = -1;
	}

	resx = 0;
	resy = 0;
}

void Face_stats::create_buttons()
{
	int i;
	int pos = 0;
	int width = PORTRAIT_WIDTH;

	resx = gwin->get_width();
	resy = gwin->get_height();
	x = 0;
	y = resy;

	party_size = partyman->get_count();

	int num_to_paint = 0;

	for (i = 0; i < party_size; i++) {
		int num = partyman->get_member(i);
		// Show faces if in SI, or if paperdolls are allowed
		if (GAME_SI || sman->can_use_paperdolls())
			++num_to_paint;
		else
			{
			// Otherwise, show faces also if the character
			// has paperdoll information (*risky*, as the
			// specified face may not exist in bg_paperdoll
			// file (or a patch), leading to a crash)
			Actor *act = gwin->get_npc(num);
			if (Paperdoll_gump::GetCharacterInfo(act->get_shapenum()))
				++num_to_paint;
			}
		}

	if (mode == 0) pos = 0;
	else if (mode == 1) pos = (resx - (num_to_paint+1)*PORTRAIT_WIDTH)/2;
	else if (mode == 2)
	{
		pos = resx - PORTRAIT_WIDTH;
		width = - PORTRAIT_WIDTH;
	}

	std::memset (party, 0, sizeof(party));

	party[0] = new Portrait_button(this, pos, 0, gwin->get_main_actor());

	for (i = 0; i < party_size; i++)
	{
		npc_nums[i+1] = partyman->get_member(i);
		Actor *act = gwin->get_npc(npc_nums[i+1]);
		// Show faces if in SI, or if paperdolls are allowed
		if (GAME_SI || sman->can_use_paperdolls() ||
				// Otherwise, show faces also if the character
				// has paperdoll information (*risky*, as the
				// specified face may not exist in bg_paperdoll
				// file (or a patch), leading to a crash)
				Paperdoll_gump::GetCharacterInfo(act->get_shapenum())) {
			pos += width;
			party[i+1] = new Portrait_button(this, pos, 0, gwin->get_npc(npc_nums[i+1]));
		}
		else {
			party[i+1] = 0;
		}
	}

	region.x = region.y = region.w = region.h = 0;

	for (i = 0; i < 8; i++)
		if (party[i])
		{
			Rectangle r = party[i]->get_rect();
			region.add(r);
		}
}

bool Face_stats::has_point(int x, int y)
{

	for (int i = 0; i < 8; i++)
		if (party[i] && party[i]->on_button(x, y)) return true;

	return false;
}

/*
 *	Add an object.  If mx, my, sx, sy are all -1, the object's position
 *	is calculated by 'paint()'.  If they're all -2, it's assumed that
 *	obj->cx, obj->cy are already correct.
 *
 *	Output:	0 if cannot add it.
 */

int Face_stats::add
	(
	Game_object *obj,
	int mx, int my,			// Mouse location.
	int sx, int sy,			// Screen location of obj's hotspot.
	bool dont_check,		// Skip volume check.
	bool combine			// True to try to combine obj.  MAY
					//   cause obj to be deleted.
	)
{
	if (sx < 0 && sy < 0 && my < 0 && mx < 0) return 0;


	for (int i = 0; i < 8; i++)
		if (party[i] && party[i]->on_button(mx, my))
			return party[i]->get_actor()->add(obj, dont_check,
								combine);

	return (0);
}

Container_game_object *Face_stats::find_actor(int mx, int my)
{

	for (int i = 0; i < 8; i++) if (party[i] && party[i]->on_button(mx, my))
		return party[i]->get_actor();

	return 0;
}

// Statics

int Face_stats::mode = 0;
Face_stats *Face_stats::self = 0;

// Creates if doesn't already exist
void Face_stats::CreateGump()
{
	if (!self)
	{
		new Face_stats();
		gumpman->add_gump(self);
	}
}

// Removes is exists
void Face_stats::RemoveGump()
{
	if (self) gumpman->close_gump(self);
	//delete self;
}

// Increments the state of the gump
void Face_stats::AdvanceState()
{
	if (!self) CreateGump();
	else
	{
		RemoveGump();

		mode++;
		mode %=PORTRAIT_NUM_MODES;
		if (mode) CreateGump();
	}
}

void Face_stats::save_config(Configuration *config)
{
	if(self)
		config->set("config/gameplay/facestats",mode,true);
	else
		config->set("config/gameplay/facestats",-1,true);
}

void Face_stats::load_config(Configuration *config)
{
	int nmode;
	if (Game::get_game_type() == EXULT_DEVEL_GAME)
		return;			// FOR NOW, skip if new game.
	config->value("config/gameplay/facestats",nmode,-1);
	if(self)
		RemoveGump();
	if(nmode>=0) {
		mode = nmode%PORTRAIT_NUM_MODES;
		CreateGump();
	}
}
