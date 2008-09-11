/**
 **	Shapeinf.cc: Info. about shapes read from various 'static' data files.
 **
 **	Written: 4/29/99 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman
Copyright (C) 1999-2002 The Exult Team

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

#include "shapeinf.h"
#include "ammoinf.h"
#include "aniinf.h"
#include "armorinf.h"
#include "bodyinf.h"
#include "continf.h"
#include "effhpinf.h"
#include "expinf.h"
#include "frnameinf.h"
#include "monstinf.h"
#include "npcdollinf.h"
#include "objdollinf.h"
#include "sfxinf.h"
#include "warminf.h"
#include "weaponinf.h"

#include "utils.h"
#include <vector>
#include <map>
#include <string>

using std::vector;
using std::cerr;
using std::endl;

Shape_info::Shape_info() : weight(0), volume(0),
	ready_type(0), occludes_flag(false), weapon_offsets(0), 
	armor(0), weapon(0), ammo(0), monstinf(0), sfxinf(0), aniinf(0),
	explosion(0), body(0), npcpaperdoll(0), container_gump(-1),
	monster_food(-1), actor_flags(0), shape_flags(0), modified_flags(0),
	frompatch_flags(0), have_static_flags(0)
	{
	tfa[0] = tfa[1] = tfa[2] = shpdims[0] = shpdims[1] = 0;
	dims[0] = dims[1] = dims[2] = 0;
	}

/*
 *	Not supported:
 */
Shape_info::Shape_info(const Shape_info & other) : weight(0), volume(0),
		ready_type(0), occludes_flag(false), weapon_offsets(0), 
		armor(0), weapon(0), ammo(0), monstinf(0), sfxinf(0), aniinf(0),
		explosion(0), body(0), npcpaperdoll(0), container_gump(-1),
		monster_food(-1), actor_flags(0), shape_flags(0), modified_flags(0),
		frompatch_flags(0), have_static_flags(0)
	{ copy(other); }
const Shape_info & Shape_info::operator = (const Shape_info & other)
	{ copy(other); return *this; }
/*
 *	Clean up.
 */

Shape_info::~Shape_info()
	{
	delete weapon;
	delete ammo;
	delete armor;
	if(weapon_offsets)
		delete [] weapon_offsets;
	delete monstinf;
	delete npcpaperdoll;
	delete sfxinf;
	delete aniinf;
	delete explosion;
	delete body;
	clear_paperdoll_info();
	clear_effective_hp_info();
	clear_frame_name_info();
	clear_warmth_info();
	}

/*
 *	Copy.
 */

void Shape_info::copy
	(
	const Shape_info& inf2,
	bool skip_dolls
	)
	{
	for (int i = 0; i < 3; ++i)
		{
		tfa[i] = inf2.tfa[i];
		dims[i] = inf2.dims[i];
		}
	weight = inf2.weight;
	volume = inf2.volume;
	shpdims[0] = inf2.shpdims[0];
	shpdims[1] = inf2.shpdims[1];
	ready_type = inf2.ready_type;
	occludes_flag = inf2.occludes_flag;
	container_gump = inf2.container_gump;
	shape_flags = inf2.shape_flags;
	monster_food = inf2.monster_food;
	actor_flags = inf2.actor_flags;
	modified_flags = inf2.modified_flags;
	frompatch_flags = inf2.frompatch_flags;
	have_static_flags = inf2.have_static_flags;
	// Allocated fields.
	delete [] weapon_offsets;
	if (inf2.weapon_offsets)
		{
		weapon_offsets = new unsigned char[64];
		memcpy(weapon_offsets, inf2.weapon_offsets, 64);
		}
	else
		weapon_offsets = 0;
	delete armor;
	armor = inf2.armor ? new Armor_info(*inf2.armor) : 0;
	delete ammo;
	ammo = inf2.ammo ? new Ammo_info(*inf2.ammo) : 0;
	delete weapon;
	weapon = inf2.weapon ? new Weapon_info(*inf2.weapon) : 0;
	delete monstinf;
	monstinf = inf2.monstinf ? new Monster_info(*inf2.monstinf) : 0;
	if (!skip_dolls || !npcpaperdoll)
		{
		delete npcpaperdoll;
		npcpaperdoll = inf2.npcpaperdoll ?
				new Paperdoll_npc(*inf2.npcpaperdoll) : 0;
		}
	if (!skip_dolls || objpaperdoll.empty())
		copy_vector_info(inf2.objpaperdoll, objpaperdoll);
	copy_vector_info(inf2.hpinf, hpinf);
	copy_vector_info(inf2.cntrules, cntrules);
	copy_vector_info(inf2.nameinf, nameinf);
	copy_vector_info(inf2.warminf, warminf);

	delete sfxinf;
	sfxinf = inf2.sfxinf ? new SFX_info(*inf2.sfxinf) : 0;
	delete explosion;
	explosion = inf2.explosion ? new Explosion_info(*inf2.explosion) : 0;
	delete aniinf;
	aniinf = inf2.aniinf ? new Animation_info(*inf2.aniinf) : 0;
	delete body;
	body = inf2.body ? new Body_info(*inf2.body) : 0;
	}

/*
 *	Get (safely) a specified information set.
 */
// Get armor protection.
int Shape_info::get_armor() const
	{
	return armor ? armor->prot : 0;
	}

// Get armor-granted immunities.
int Shape_info::get_armor_immunity() const
	{
	return armor ? armor->immune : 0;
	}

// Get sprite of explosion.
int Shape_info::get_explosion_sprite() const
	{
	return explosion ? explosion->sprite : 5;
	}

// Get sfx of explosion.
int Shape_info::get_explosion_sfx() const
	{
	return explosion ? explosion->sfxnum : -1;
	}

int Shape_info::get_body_shape() const
	{
	return body ? body->bshape : 400;
	}

int Shape_info::get_body_frame() const
	{
	return body ? body->bframe : 3;
	}

Weapon_info *Shape_info::get_weapon_info_safe() const
	{
	return weapon ? weapon :
			const_cast<Weapon_info *>(Weapon_info::get_default());
	}

Ammo_info *Shape_info::get_ammo_info_safe() const
	{
	return ammo ? ammo :
			const_cast<Ammo_info *>(Ammo_info::get_default());
	}

Monster_info *Shape_info::get_monster_info_safe() const
	{
	return monstinf ? monstinf :
			const_cast<Monster_info *>(Monster_info::get_default());
	}

Animation_info *Shape_info::get_animation_info_safe
	(
	int shnum,
	int nframes
	)
	{
	if (!aniinf)
		aniinf = Animation_info::create_from_tfa(0, nframes);
	return aniinf;
	}

bool Shape_info::has_paperdoll_info() const
	{
	return objpaperdoll.size() != 0;
	}

std::vector<Paperdoll_item>& Shape_info::set_paperdoll_info(bool tf)
	{
	return set_vector_info(tf, objpaperdoll);
	}

void Shape_info::clean_invalid_paperdolls()
	{
	return clean_vector(objpaperdoll);
	}

void Shape_info::clear_paperdoll_info()
	{
	objpaperdoll.clear();
	}

void Shape_info::add_paperdoll_info(Paperdoll_item& add)
	{
	add_vector_info(add, objpaperdoll);
	}

bool Shape_info::has_content_rules() const
	{
	return cntrules.size() != 0;
	}

std::vector<Content_rules>& Shape_info::set_content_rules(bool tf)
	{
	return set_vector_info(tf, cntrules);
	}

void Shape_info::clean_invalid_content_rules()
	{
	return clean_vector(cntrules);
	}

void Shape_info::clear_content_rules()
	{
	cntrules.clear();
	}

void Shape_info::add_content_rule(Content_rules& add)
	{
	add_vector_info(add, cntrules);
	}

bool Shape_info::has_effective_hp_info() const
	{
	return hpinf.size() != 0;
	}

std::vector<Effective_hp_info>& Shape_info::set_effective_hp_info(bool tf)
	{
	return set_vector_info(tf, hpinf);
	}

void Shape_info::clean_invalid_hp_info()
	{
	return clean_vector(hpinf);
	}

void Shape_info::clear_effective_hp_info()
	{
	hpinf.clear();
	}

void Shape_info::add_effective_hp_info(Effective_hp_info& add)
	{
	add_vector_info(add, hpinf);
	}

bool Shape_info::has_frame_name_info() const
	{
	return nameinf.size() != 0;
	}

std::vector<Frame_name_info>& Shape_info::set_frame_name_info(bool tf)
	{
	return set_vector_info(tf, nameinf);
	}

void Shape_info::clean_invalid_name_info()
	{
	return clean_vector(nameinf);
	}

void Shape_info::clear_frame_name_info()
	{
	nameinf.clear();
	}

void Shape_info::add_frame_name_info(Frame_name_info& add)
	{
	add_vector_info(add, nameinf);
	}

bool Shape_info::has_warmth_info() const
	{
	return warminf.size() != 0;
	}

std::vector<Warmth_info>& Shape_info::set_warmth_info(bool tf)
	{
	return set_vector_info(tf, warminf);
	}

void Shape_info::clean_invalid_warmth_info()
	{
	return clean_vector(warminf);
	}

void Shape_info::clear_warmth_info()
	{
	warminf.clear();
	}

void Shape_info::add_warmth_info(Warmth_info& add)
	{
	add_vector_info(add, warminf);
	}

template <class T>
static T *Search_vector_data_double_wildcards
	(
	vector<T>& vec,
	int frame, int quality,
	short T::*fr, short T::*qual
	)
	{
	if (!vec.size())
		return 0;	// No name.
	T inf;
	inf.*fr = frame;
	inf.*qual = quality;
	typename vector<T>::iterator it;
		// Try finding exact match first.
	it = std::lower_bound(vec.begin(), vec.end(), inf);
	if (it == vec.end())	// Nowhere to be found.
		return 0;
	else if (*it == inf)	// Have it already.
		return &*it;
		// We only have to search forward for a match.
	if (quality != -1)
		{
		if ((*it).*fr == frame)
			{	// Maybe quality is to blame. Try wildcard quality.
			inf.*qual = -1;
			it = std::lower_bound(it, vec.end(), inf);
			if (it == vec.end())	// Nowhere to be found.
				return 0;
			else if (*it == inf)	// We got it!
				return &*it;
			}
		// Maybe frame is to blame? Try search for specific
		// quality with wildcard frame.
		inf.*qual = quality;
		inf.*fr = -1;
		it = std::lower_bound(it, vec.end(), inf);
		if (it == vec.end())	// Nowhere to be found.
			return 0;
		else if (*it == inf)	// We got it!
			return &*it;
		inf.*qual = -1;
		}
		// *Still* haven't found it. Last try: wildcard frame *and* quality.
	inf.*fr = -1;
	it = std::lower_bound(it, vec.end(), inf);
	if (it == vec.end() || *it != inf)	// It just isn't there.
		return 0;
	else	// At last!
		return &*it;
	}

Frame_name_info *Shape_info::get_frame_name(int frame, int quality)
	{
	return Search_vector_data_double_wildcards(nameinf,
			frame, quality,
			&Frame_name_info::frame, &Frame_name_info::quality);
	}

int Shape_info::get_effective_hps(int frame, int quality)
	{
	Effective_hp_info *inf = Search_vector_data_double_wildcards(hpinf,
			frame, quality,
			&Effective_hp_info::frame, &Effective_hp_info::quality);
	return inf ? inf->hps : 0;	// Default to indestructible.
	}

Paperdoll_item *Shape_info::get_item_paperdoll(int frame, int spot)
	{
	if (!objpaperdoll.size())
		return 0;	// No warmth.
	Paperdoll_item inf;
	inf.world_frame = frame;
	inf.spot = spot;
	vector<Paperdoll_item>::iterator it;
		// Try finding exact match first.
	it = std::lower_bound(objpaperdoll.begin(), objpaperdoll.end(), inf);
	if (it == objpaperdoll.end())	// Nowhere to be found.
		return 0;
	else if (*it == inf && !it->is_invalid())	// Have it already.
		return &*it;
		// Time for wildcard world frame.
	inf.world_frame = -1;
	it = std::lower_bound(it, objpaperdoll.end(), inf);
	if (it == objpaperdoll.end() || *it != inf	// It just isn't there...
			 || it->is_invalid())	// ... or it is invalid.
		return 0;
	else	// At last!
		return &*it;
	}

template <class T, typename U>
static T *Search_vector_data_single_wildcard
	(
	vector<T>& vec,
	int src,
	U T::*dat
	)
	{
	if (!vec.size())	// Not found.
		return 0;
	T inf;
	inf.*dat = src;
	typename vector<T>::iterator it;
		// Try finding exact match first.
	it = std::lower_bound(vec.begin(), vec.end(), inf);
	if (it == vec.end())	// Nowhere to be found.
		return 0;
	else if (*it == inf)	// Have it already.
		return &*it;
		// Try wildcard shape.
	inf.*dat = -1;
	it = std::lower_bound(it, vec.end(), inf);
	if (it == vec.end() || *it != inf)	// It just isn't there.
		return 0;
	else	// At last!
		return &*it;
	}

bool Shape_info::is_shape_accepted(int shape)
	{
	Content_rules *inf = Search_vector_data_single_wildcard(cntrules,
			shape, &Content_rules::shape);
#if DEBUG
	if (inf && !inf->accept)
		cerr << "Shape '" << shape << "' was REJECTED" << endl;
#endif
	return inf ? inf->accept : true;	// Default to true.
	}

int Shape_info::get_object_warmth(int frame)
	{
	Warmth_info *inf = Search_vector_data_single_wildcard(warminf,
			frame, &Warmth_info::frame);
	return inf ? inf->warmth : 0;	// Default to no warmth.
	}

bool Shape_info::was_modified()
	{
	if (modified_flags)
		return true;
	return was_changed(armor) || was_changed(weapon) ||
			was_changed(ammo) || was_changed(monstinf) ||
			was_changed(sfxinf) || was_changed(aniinf) ||
			was_changed(explosion) || was_changed(body) ||
			was_changed(npcpaperdoll) || was_changed(objpaperdoll) ||
			was_changed(hpinf) || was_changed(nameinf) ||
			was_changed(warminf) || was_changed(cntrules);
	}


/*
 *	Set 3D dimensions.
 */

void Shape_info::set_3d
	(
	int xt, int yt, int zt		// In tiles.
	)
	{
	xt = (xt - 1) & 7;		// Force legal values.
	yt = (yt - 1) & 7;
	zt &= 7;
	tfa[2] = (tfa[2]&~63)|xt|(yt<<3);
	tfa[0] = (tfa[0]&~(7<<5))|(zt<<5);
	dims[0] = xt + 1;
	dims[1] = yt + 1;
	dims[2] = zt;
	}

/*
 *	Set weapon offsets for given frame.
 */

void Shape_info::set_weapon_offset
	(
	int frame, 			// 0-31.
	unsigned char x, unsigned char y// 255 means "dont' draw".
	)
	{
	if (frame < 0 || frame > 31)
		return;
	if (x == 255 && y == 255)
		{
		if (weapon_offsets)	// +++Could delete if all 255's now.
			weapon_offsets[frame*2] =
			weapon_offsets[frame*2 + 1] = 255;
		return;
		}
	if (!weapon_offsets)
		{
		weapon_offsets = new unsigned char[64];
		std::memset(weapon_offsets, 255, sizeof(weapon_offsets));
		}
	weapon_offsets[frame*2] = x;
	weapon_offsets[frame*2 + 1] = y;
	}

/*
 *	Get rotated frame (algorithmically).
 */

int Shape_info::get_rotated_frame
	(
	int curframe,
	int quads			// 1=90, 2=180, 3=270.
	)
	{
	if (is_barge_part())		// Piece of a barge?
		switch (quads)
			{
		case 1:
			return (curframe^32)^((curframe&32) ? 3 : 1);
		case 2:
			return curframe^2;
		case 3:
			return (curframe^32)^((curframe&32) ? 1 : 3);
		default:
			return curframe;
			}
	else
					// Reflect.  Bit 32==horizontal.
		return curframe ^ ((quads%2)<<5);
	}
