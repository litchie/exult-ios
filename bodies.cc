/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Bodies.cc - Associate bodies with 'live' shapes.
 **
 **	Written: 6/28/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "bodies.h"
#ifdef MACOS
  #include <hashset.h>
#else
  #include <hash_set>
#endif

/*
 *	Each triple here is <live shape>, <body shape>, <body frame>.
 */

short Body_lookup::table[] = {
	491,	762,	13,	 // acid slug
	492,	892,	13,	 // alligator
	493,	762,	9,	 // bat   ??Unsure
	403,	762,	2,	 // Batlin
	482,	762,	2,	 // Batlin
	494,	762,	2,	 // bee
	716,	762,	2,	 // bird
	495,	892,	18,	 // cat
	498,	762,	2,	 // chicken
	499,	762,	2,	 // corpser
	500,	778,	11,	 // cow
	259,	762,	2,	 // fighter
	154,	762,	22,	 // mage
	247,	762,	24,	 // paladin
	401,	762,	25,	 // pirate
	501,	762,	2,	 // cyclops
	502,	778,	12,	 // deer
	496,	892,	24,	 // dog
	504,	778,	7,	 // dragon
	505,	778,	8,	 // drake
	784,	762,	2,	 // emp
	230,	762,	2,	 // ethereal monster
	929,	762,	2,	 // Fellowship member
	382,	892,	15,	 // Kissme
	952,	762,	2,	 // The Ferryman
	509,	892,	26,	 // fish
	510,	892,	27,	 // fox
	883,	762,	1,	 // wingless Gargoyle
	511,	762,	2,	 // gazer
	337,	762,	2,	 // ghost
	299,	762,	2,	 // ghost
	317,	762,	15,	 // ghost
	1015,	414,	4,	 // Golem
	513,	762,	4,	 // gremlin
	946,	762,	23,	 // guard
	532,	762,	5,	 // harpie
	514,	762,	6,	 // headless
	506,	762,	2,	 // Hook
	727,	778,	1,	 // horse
	381,	762,	2,	 // three headed hydra
	517,	892,	15,	 // insects
	354,	762,	8,	 // liche
	519,	762,	8,	 // liche
	466,	414,	12,	 // Lord British
	884,	762,	2,	 // Fellowship member
	661,	762,	2,	 // mongbat
	521,	762,	2,	 // mouse
	394,	762,	23,	 // guard
	811,	762,	10,	 // rabbit
	523,	762,	11,	 // rat
	524,	762,	12,	 // reaper
	706,	762,	26,	 // scorpion
	525,	762,	2,	 // sea serpent
	970,	762,	14,	 // sheep
	528,	762,	16,	 // skeleton
	529,	762,	17,	 // slime
	530,	762,	18,	 // snake
	865,	762,	19,	 // spider
	753,	762,	20,	 // stone harpie
	536,	778,	3,	 // tentacles
	806,	762,	23,	 // guard
	533,	778,	4,	 // troll
	534,	762,	2,	 // wisp
	537,	762,	21,	 // wolf
	447,	762,	2	 // wounded man
	};

/*
 *	Hash function for triples in table:
 */
class Hash_shapes
	{
public:
					// Return 'live' shape.
	size_t operator() (const short *t) const
		{ return t[0]; }
	};

/*
 *	For testing if two triples match.
 */
class Equal_shapes
	{
public:
     	bool operator() (const short *a, const short *b) const
     		{ return a[0] == b[0]; }
	};

/*
 *	Lookup a shape's body.
 *
 *	Output:	0 if not found.
 */

int Body_lookup::find
	(
	int liveshape,			// Live actor's shape.
	int& deadshape,			// Dead shape returned.
	int& deadframe			// Dead frame returned.
	)
	{
	static hash_set<short *, Hash_shapes, Equal_shapes> *htable = 0;

	if (!htable)			// First time?
		{
		htable = new hash_set<short *, Hash_shapes, Equal_shapes>(300);
		int cnt = sizeof(table)/(3*sizeof(table[0]));
		short *ptr = &table[0];	// Add values.
		while (cnt--)
			{
			htable->insert(ptr);
			ptr += 3;
			}
		}
	short key = (short) liveshape;
	hash_set<short *, Hash_shapes, Equal_shapes>::iterator it =
							htable->find(&key);
	if (it != htable->end())
		{
		short *triple = *it;
		deadshape = triple[1];
		deadframe = triple[2];
		return 1;
		}
	else
		return 0;
	}
