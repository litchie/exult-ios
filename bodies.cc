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

using std::hash_set;
using std::size_t;

/*
 *	Each triple here is <live shape>, <body shape>, <body frame>.
 */

short Body_lookup::table[] = {
				// Monsters:
	491,	892,	12,	// acid slug
	492,	892,	13,	// alligator
	493,	762,	9,	// bat   (no body!)
	494,	892,	15,	// bee
	716,	892,	16,	// bird
	495,	892,	18,	// cat
	498,	892,	20,	// chicken
	499,	892,	21,	// corpser
	500,	778,	11,	// cow
	501,	778,	6,	// cyclops
	502,	778,	12,	// deer
	496,	892,	24,	// dog
	504,	778,	7,	// dragon
	505,	778,	8,	// drake
	784,	892,	25,	// emp
	230,	762,	2,	// ethereal monster (no body!)
	884,	892,	5,	// Fellowship member
	929,	400,	10,	// Fellowship member
	155,	762,	2,	// Ferryman
	259,	400,	20,	// fighter
	509,	892,	26,	// fish
	510,	892,	27,	// fox
	883,	762,	1,	// Gargoyle, wingless
	511,	762,	2,	// gazer
	337,	762,	0,	// ghost	(invisible).
	299,	762,	0,	// ghost
	317,	762,	0,	// ghost
	1015,	414,	4,	// Golem
	513,	762,	4,	// gremlin
	394,	892,	10,	// guard
	806,	892,	7,	// guard
	946,	892,	8,	// guard
	532,	762,	5,	// harpie
	514,	762,	6,	// headless
	727,	778,	1,	// horse
	381,	778,	3,	// hydra, three headed
	382,	414,	7,	// Kissme
	517,	892,	15,	// insects
	354,	762,	8,	// liche
	519,	762,	8,	// liche (Horatius?)
	466,	414,	12,	// Lord British
	154,	762,	22,	// mage
	661,	762,	9,	// mongbat
	521,	762,	10,	// mouse
	247,	762,	23,	// paladin
	401,	762,	25,	// pirate
	811,	892,	17,	// rabbit
	523,	762,	11,	// rat
	524,	762,	12,	// reaper
	706,	762,	26,	// scorpion
	525,	762,	2,	// sea serpent (no body!)
	970,	762,	14,	// sheep
	528,	762,	16,	// skeleton
	529,	762,	0,	// slime (invisible).
	530,	762,	18,	// snake
	865,	762,	19,	// spider
	753,	762,	20,	// stone harpie
	536,	762,	0,	// tentacles (invisible).
	533,	778,	5,	// troll
	534,	762,	0,	// wisp
	537,	762,	21,	// wolf
	447,	892,	11,	// wounded man
				// NCPs:
	318,	400,	5,	// Sage.
	319,	400,	9,	// Male peasant.
//	403,	892,	3,	// Batlin.
	403,	414,	16,	// Batlin.
//	482,	892,	3,	// Batlin.
	403,	414,	16,	// Batlin.
	451,	400,	13,	// Male noble.
	452,	400,	10,	// Female peasant.
	454,	400,	12,	// Female shopkeeper.
	455,	400,	11,	// Male shopkeeper.
	456,	400,	14,	// Female noble.
	457,	400,	15,	// Male gypsy.
	458,	400,	16,	// Pirate.
	459,	400,	17,	// Female gypsy.
	401,	400,	16,	// Pirate.
	460,	400,	18,	// Male ranger
	461,	400,	19,	// Female ranger
	462,	400,	20,	// Male fighter.
	463,	400,	21,	// Female fighter.
	465,	400,	3,	// Iolo.
	468,	400,	25,	// Male entertainer.
	469,	400,	26,	// Female entertainer.
	489,	414,	21,	// Spark.
	506,	892,	19,	// Hook.
	720,	400,	23,	// Guard.
	805,	892,	4,	// Forksis.
	881,	892,	2,	// Elizabeth.
	882,	892,	1,	// Abraham.
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
