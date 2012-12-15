/*
 *  Copyright (C) 2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdio>

#include "objs.h"
#include "game.h"
#include "items.h"
#include "gamewin.h"
#include "actors.h"
#include "shapeinf.h"
#include "frnameinf.h"

using std::string;
#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
namespace std {
using ::snprintf;
}
#else
#endif
#ifndef UNDER_EMBEDDED_CE
using std::strchr;
#endif

/*
 *	For objects that can have a quantity, the name is in the format:
 *		%1/%2/%3/%4
 *	Where
 *		%1 : singular prefix (e.g. "a")
 *		%2 : main part of name
 *		%3 : singular suffix
 *		%4 : plural suffix (e.g. "s")
 */

/*
 *	Extracts the first, second and third parts of the name string
 */
static void get_singular_name
	(
	const char *name,		// Raw name string from TEXT.FLX
	string& output_name		// Output string
	)
{
	if(*name != '/')		// Output the first part
		{
		do
			output_name += *name++;
		while(*name != '/' && *name != '\0');
		if(*name == '\0')	// should not happen
			{
			output_name = "?";
			return;
			}
		// If there is a first part it is followed by a space
		output_name += ' ';
		}
	name++;

					// Output the second part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;

					// Output the third part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
}

/*
 *	Extracts the second and fourth parts of the name string
 */
static void get_plural_name
	(
	const char *name,
	int quantity,
	string& output_name
	)
{
	char buf[20];

	snprintf(buf, 20, "%d ", quantity);	// Output the quantity
	output_name = buf;

					// Skip the first part
	while(*name != '/' && *name != '\0')
		name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
					// Output the second part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
					// Skip the third part
	while(*name != '/' && *name != '\0')
		name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
	while(*name != '\0')		// Output the last part
		output_name += *name++;
}

/*
 *	Returns the string to be displayed when the item is clicked on
 */
string Game_object::get_name
	(
	) const
	{
	Shape_info& info = get_info();
	int qual = info.has_quality() && !info.is_npc() ? get_quality() : -1;
	Frame_name_info *nminf = info.get_frame_name(get_framenum(), qual);
	int shnum = get_shapenum();
	const char *name;
	const char *shpname = (shnum >= 0 && shnum < num_item_names) ?
						item_names[shnum] : 0;
	int type = nminf ? nminf->get_type() : -255;
	int msgid;
	if (type == -255 || (msgid = nminf->get_msgid()) >= num_misc_names)
		name = shpname;
	else if (type < 0)
		return "";	// None.
	else if (!type)
		name = misc_names[msgid];
	else if (!info.has_quality() && !info.is_body_shape())
		name = shpname;		// Use default name for these.
	else
		{
		int othermsg = nminf->get_othermsg();
		bool defname = false;
		string msg;
		string other;
		if (type >= 3)
			{	// Special names (in SI, corpse, urn).
			int npcnum = -1;
			if (!info.is_body_shape())
				npcnum = get_quality();
			else if (qual == 1)
				npcnum = get_live_npc_num();
			Actor *npc = gwin->get_npc(npcnum);
			if (npc && !npc->is_unused() &&
					(!info.is_body_shape() || npc->get_flag(Obj_flags::met)))
				{
				other = npc->get_npc_name_string();
				if (other.empty())	// No name.
					defname = true;
				else
					msg = misc_names[msgid];
				}
			else	// Default name.
				defname = true;
			}
		else
			{
			msg = misc_names[msgid];
			other = (othermsg >= 0 && othermsg < num_misc_names) ?
						misc_names[othermsg] : shpname;
			}
		if (defname)
			{
			if (othermsg >= 0 && othermsg < num_misc_names)
				name = misc_names[othermsg];
			else if (othermsg < 0 && othermsg != -255)	// None.
				return "";
			else	// Use shape's.
				name = shpname;
			}
		else if (type & 1)
			return other + msg;
		else
			return msg + other;
		}
	int quantity;
	string display_name;
	if (name == 0)
		return "";

	if (ShapeID::get_info(shnum).has_quantity())
		quantity = quality & 0x7f;
	else
		quantity = 1;

	// If there are no slashes then it is simpler
	if(strchr(name, '/') == 0)
		{
		if(quantity <= 1)
			display_name = name;
		else
			{
			char buf[50];

			snprintf(buf, 50, "%d %s", quantity, name);
			display_name = buf;
			}
		}
	else if(quantity <= 1)		// quantity might be zero?
		get_singular_name(name, display_name);
	else
		get_plural_name(name, quantity, display_name);
	return display_name;
	}
