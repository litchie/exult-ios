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
/*
                              Artaxerxes
  Hi,
  here is the newest (Oct 4 2000 11am EST) bodies.cc file
  hope it helps
*/

  154, 762, 22, 			// x mage
  155, 762, 2, 				// X Ferryman
  226, 892, 29, 			// X flying gargoyle
  227, 400, 5, 				// X monk male
  228, 414, 28, 			// X naked man
  229, 414, 27, 			// X naked woman
  230, 762, 0, 				// x ethereal monster -- there should
					//    not be a body.. just blood
  247, 762, 23, 			// x paladin
  259, 400, 20, 			// x fighter
  265, 400, 27, 			// X townsman
  274, 414, 2, 				// X gargoyle -- Healer (?)
  299, 762, 0, 				// x ghost
  304, 400, 28, 			// X forger
  317, 762, 0, 				// x ghost
  318, 400, 5, 				// x monk
  319, 400, 9, 				// x male peasant
  337, 762, 0, 				// x ghost (invisible)
  354, 762, 8, 				// x liche
  375, 778, 2, 				// X unicorn
  380, 778, 4, 				// X cyclop
  381, 778, 3, 				// x three headed hydra
  382, 414, 7, 				// x Kissme
  394, 892, 10, 			// x guard
  401, 762, 25, 			// x pirate
  401, 400, 16, 			// x pirate.
  403, 414, 16, 			// x Batlin
  445, 400, 3, 				// X mage male
  446, 400, 4, 				// X mage female
  447, 892, 11, 			// x wounded man
  448, 400, 6, 				// X monk female
  449, 400, 7, 				// X beggar
  450, 400, 8, 				// X beggar
  451, 400, 13, 			// x male noble.
  452, 400, 10, 			// x female peasant.
  454, 400, 12, 			// x female shopkeeper.
  455, 400, 11, 			// x male shopkeeper.
  456, 400, 14, 			// x female noble.
  457, 400, 15, 			// x male gypsy 
  458, 400, 16, 			// x pirate.
  459, 400, 17, 			// x female gypsy
  460, 400, 18, 			// x male ranger
  461, 400, 19, 			// x female ranger
  462, 400, 20, 			// x male fighter.
  463, 400, 21, 			// x female fighter.
  464, 400, 22, 			// X paladin
  465, 414, 18, 			// x Iolo.
  466, 414, 12, 			// x Lord British
  467, 400, 24, 			// X chuck jester
  468, 400, 25, 			// x male entertainer.
  469, 400, 26, 			// x female entertainer.
  471, 400, 30, 			// X kid
  472, 400, 31, 			// X noble Kid
  473, 414, 1, 				// X noble Gargoyle
  475, 892, 4, 				// X Gargoyle -- Forksis (?)
  476, 778, 1, 				// X Smith (?) -- Iolo's Horse
  477, 414, 6, 				// X Honest fox
  478, 414, 10, 			// X Sherry
  479, 414, 11, 			// X emp
  480, 414, 14, 			// X noble Gargoyle
  482, 414, 16, 			// x Batlin
  485, 414, 17, 			// X thief -- Isle of the Avatar
  487, 414, 19, 			// X Shamino
  488, 414, 20, 			// X Dupre
  489, 414, 21, 			// x Spark.
  490, 414, 22, 			// X Jaana
  491, 892, 12, 			// x acid slug
  492, 892, 13, 			// x alligator
  493, 762, 0, 				// x bat -- there is no body for 
					//   bat... just blood
  494, 892, 15, 			// x bee
  495, 892, 18, 			// x cat
  496, 892, 24, 			// x dog 
  498, 892, 20, 			// x chicken
  499, 892, 21, 			// x corpser
  500, 778, 11, 			// x cow
  501, 778, 6, 				// x cyclops
  502, 778, 12, 			// x deer
  504, 778, 7, 				// x dragon
  505, 778, 8, 				// x drake
  506, 892, 19,				// x Hook.
  509, 892, 26, 			// x fish
  510, 892, 27, 			// x fox
  511, 762, 2, 				// x gazer
  513, 762, 4, 				// x gremlin
  514, 762, 6, 				// x headless
  517, 762, 7, 				// X insects 
  519, 762, 8, 				// X black shadow dead
  521, 762, 10, 			// x mouse
  523, 762, 11, 			// x rat
  524, 762, 12, 			// x reaper
  525, 762, 13, 			// X sea serpent -- should disappear
  528, 762, 16, 			// x skeleton
  529, 762, 17, 			// X slime
  530, 762, 18, 			// x snake
  532, 762, 5, 				// x harpie
  533, 778, 5, 				// x troll
  534, 762, 0, 				// x wisp
  536, 762, 0, 				// x tentacles (invisible).
  537, 762, 21, 			// x wolf
  617, 414, 23, 			// X Time Lord (live shape = 617:16)
  661, 762, 9, 				// x mongbat
  706, 762, 26, 			// x scorpion
  716, 892, 16, 			// x bird
  720, 400, 23, 			// x Guard
  721, 400, 1, 				// X Avatar male
  727, 778, 9, 				// x horse
  753, 762, 20, 			// x stone harpie
  784, 892, 25, 			// x emp
  805, 892, 4, 				// x Forksis
  806, 892, 7, 				// x guard
  811, 892, 17, 			// x rabbit
  861, 778, 5, 				// X troll
  864, 400, 29, 			// X Baby
  865, 762, 19, 			// x spider
  881, 892, 2, 				// x Elizabeth
  882, 892, 1, 				// x Abraham
  883, 762, 1, 				// x wingless Gargoyle
  884, 892, 5, 				// x Fellowship member
  929, 400, 10, 			// x Fellowship member
  946, 892, 8, 				// x guard
  952, 762, 2, 				// X Skara Brae Liche
  957, 400, 11, 			// X shopkeeper
  965, 400, 30, 			// X little boy
  970, 762, 14, 			// x sheep
  989, 400, 2, 				// X avatar female
  1015, 414, 4	 			// x Stone Golem
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
