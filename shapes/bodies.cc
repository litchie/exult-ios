/*
 *  bodies.cc - Associate bodies with 'live' shapes.
 *
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "bodies.h"
#include "hash_utils.h"
#include "game.h"

using std::size_t;

/*
 *	Each triple here is <live shape>, <body shape>, <body frame>.
 */

short Body_lookup::bg_table[] = {	// BLACK GATE.
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
  466, 414, 13, 			// x Lord British
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

short Body_lookup::si_table[] = {	// SERPENT_ISLE.
 /*
 Artaxerxes

 Hi,
 here again another set of body frame numbers for Exult.
 Took me a few hours but now it's done...
 Hope that helps.
 */

 /*
 the dead shapes
 */

 //0, 400, 0, // dead bodies
//0, 402, 0, // more dead bodies
//0, 414, 0, // more more dead bodies
//0, 762, 0, // dead bodies
//0, 778, 0, // more dead bodies
//0, 892, 0, // more dead bodies

 /*
 living shapes, dead shape, frame number
 */

 179, 402, 23, // black robe mage no face
214, 402, 3, // male gwani
226, 414, 10, // old red robe mage
228, 402, 11, // pikeman
229, 414, 8, // naked white man brown hair
230, 414, 27, // naked white woman blonde hair
247, 400, 24, // jester
250, 414, 23, // xenka monk (male?) no face gold belt
259, 402, 12, // male ranger (winemaker) with mask
265, 400, 27, // male shopkeeper (red outfit)
274, 414, 15, // female noble (green dress) wth green crown
298, 892, 16, // pinguin (Linux?)
299, 414, 21, // Cantra (little girl from Monitor)
302, 762, 3, // grizzly bear (brown)
304, 400, 28, // male forger
312, 892, 19, // kid goblins
317, 402, 13, // possessed Shamino
318, 414, 16, // Batlin
319, 400, 9, // male peasant (brown outfit)
329, 892, 25, // boar
334, 762, 22, // green monster with long neck
337, 762, 15, // phantom
354, 762, 11, // Gurnodir (king Goblin)
363, 892, 14, // magic gremlins
372, 778, 1, // ice troll
373, 402, 31, // skeletton with ripped robe
375, 778, 2, // unicorn
381, 414, 17, // male thief (black band on eyes)
382, 762, 10, // ratman
401, 402, 15, // male noble-mage with orange-red coat and blue undies
446, 400, 4, // old female wild mage
447, 402, 18, // red eye wolf
449, 762, 29, // polar bear (white)
451, 400, 13, // male mayor (blue outfit)
454, 400, 12, // female citizen (orange outfit)
455, 400, 11, // male citizen (orange outfit)
457, 400, 15, // male gypsy
458, 400, 16, // male pirate
460, 778, 7, // skeletal dragon
461, 400, 19, // female ranger (no mask)
462, 400, 20, // male fighter (mail arms and legs)
464, 400, 22, // paladin (square helm)
465, 414, 18, // Iolo
468, 400, 25, // male shopkeeper (beige outfit)
469, 400, 26, // female gypsie
472, 892, 29, // noble little boy
478, 414, 4, // male goblins
480, 414, 1, // noble winged gargoyle
487, 414, 19, // Shamino
488, 414, 20, // Dupre
491, 892, 12, // acid slug
492, 762, 1, // alligator
493, 892, 9, // bat
494, 402, 19, // Goul (?)
495, 892, 18, // cat
496, 892, 24, // dog
498, 892, 11, // chicken
499, 892, 13, // corpser
500, 778, 11, // cow
501, 778, 4, // cyclop
502, 778, 12, // deer
509, 892, 26, // trout
510, 892, 27, // fox
511, 892, 1, // gazer
512, 400, 0, // red worm/snake (whack-a-snake)
514, 762, 6, // headless
517, 892, 2, // insect
521, 400, 0, // flying serpent/dragon (dreamland)
523, 892, 4, // rat
524, 762, 12, // reaper
528, 762, 16, // skelet
529, 892, 8, // slime
530, 892, 6, // serpent
532, 762, 5, // harpie
533, 778, 5, // Troll
536, 400, 0, // sea tentacle
537, 892, 7, // wolf (normal)
550, 762, 7, // brass automaton
560, 400, 3, // female beauty priestess
565, 400, 6, // Caladin
581, 400, 7, // old male mage with cane (green outfit)
588, 762, 8, // male vampire (very blue outfit)
647, 400, 5, // female fighter
652, 400, 17, // female mayor (blue outfit)
658, 400, 30, // Petra
660, 400, 10, // no skin gwani (?)
661, 762, 9, // Mongbat
665, 402, 22, // female mage (black outfit, red belt)
669, 400, 8, // trapper (Gwenno)
691, 414, 4, // male goblins
702, 892, 20, // phoenix
703, 400, 0, // ice corpser
706, 762, 26, // scorpio
716, 892, 10, // bird
720, 400, 23, // male guard (purple/red outfit + chain helm)
721, 400, 1, // male bonde avatar
725, 414, 4, // male goblins
734, 402, 21, // gold automaton
742, 400, 21, // female worker (black top, white skirt)
744, 414, 5, // female goblins
747, 762, 27, // iron automaton
753, 762, 20, // stone harpie
754, 762, 2, // snow leopard
763, 892, 28, // ice snake
766, 402, 2, // female spanish dancer
772, 892, 31, // parrot
798, 402, 16, // small ice dragon
805, 402, 4, // black male fighter
809, 414, 2, // young male mage (dark blue coat + gold trim)
811, 892, 17, // rabbit
814, 402, 5, // male artist (blue outfit)
815, 402, 6, // tall male individual (ripped clothes)
816, 402, 7, // male fighter (orange outfit + chain mail)
817, 402, 14, // young male mage (brown coat + grey undies)
818, 402, 9, // Zulith (?)
830, 402, 8, // Chancellor (Monitor)
832, 778, 8, // ice sea serpent
846, 762, 24, // snake woman
855, 414, 6, // Great Captain
861, 762, 23, // fire elemental
862, 414, 7, // male gwani
865, 762, 19, // spider
867, 892, 30, // baby gwani
874, 762, 17, // St-Bernard (Hound)
877, 778, 6, // Big ice dragon
880, 762, 18, // possessed Iolo
882, 892, 21, // flying red snake
883, 414, 3, // wingless gargoyle
885, 778, 3, // magic black horse
888, 414, 11, // female spanish dancer
906, 762, 21, // possessed Dupre
915, 400, 0, // brown tentacle
916, 414, 12, // male artist (purple coat + blue undies)
917, 402, 10, // male trapper (?)
945, 402, 29, // dressed iron automaton
946, 402, 27, // male mage (very blue outfit)
947, 402, 28, // gold male fighter (Silver Seed)
957, 402, 30, // catman creature (SS)
968, 402, 17, // ice golem
970, 892, 5, // sheep
975, 400, 20, // male fighter (brown outfit + chain mail)
978, 778, 9, // green dragon
979, 778, 10, // man spider
981, 402, 25, // liche
989, 400, 2, // white female avatar
1015, 402, 26, // female citizen (blue dress + green sleeves)
1024, 414, 28, // black male avatar
1025, 414, 29, // black female avatar
1026, 414, 30, // brown male avatar
1027, 414, 31, // brown female avatar
1028, 400, 1, // blonde male avatar
1029, 400, 2 // blonde female avatar

 /*
 the missing shapes
 */

 //392, 0, 0, // old female mage with gold belt
//482, 0, 0, // insivible woman
//793, 0, 0, // Lord British
//853, 0, 0, // invisible man
//891, 0, 0, // naked male brown avatar
//893, 0, 0, // naked male black avatar
//921, 0, 0, // naked male blonde avatar
//930, 0, 0, // naked female bonde avatar
//933, 0, 0, // naked female black avatar
//938, 0, 0, // naked female brown avatar
//1030, 0, 0, // naked black male avatar
//1031, 0, 0, // naked black female avatar
//1032, 0, 0, // naked brown male avatar
//1033, 0, 0, // naked brown female avatar
//1034, 0, 0, // naked blonde male avatar
//1035, 0, 0, // naked blonde female avatar

	};

#ifndef DONT_HAVE_HASH_SET

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

#else

/*
 *	For testing whether one shape is less than another
 */
class Less_shapes
	{
public:
     	bool operator() (const short *a, const short *b) const
     		{ return a[0] < b[0]; }
	};

#endif

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
#ifndef DONT_HAVE_HASH_SET
	static hash_set<short *, Hash_shapes, Equal_shapes> *htable = 0;
#else
	static std::set<short *, Less_shapes> *htable = 0;
#endif

	if (!htable)			// First time?
		{
#ifndef DONT_HAVE_HASH_SET
		htable = new hash_set<short *, Hash_shapes, Equal_shapes>(300);
#else
		htable = new std::set<short *, Less_shapes>();
#endif
		short *ptr;
		int cnt;
		if (Game::get_game_type() == BLACK_GATE)
			{
			cnt = sizeof(bg_table)/(3*sizeof(bg_table[0]));
			ptr = &bg_table[0];
			}
		else
			{
			cnt = sizeof(si_table)/(3*sizeof(si_table[0]));
			ptr = &si_table[0];
			}
		while (cnt--)		// Add values.
			{
			htable->insert(ptr);
			ptr += 3;
			}
		}
	short key = (short) liveshape;
#ifndef DONT_HAVE_HASH_SET
	hash_set<short *, Hash_shapes, Equal_shapes>::iterator it =
							htable->find(&key);
#else
	std::set<short *, Less_shapes>::iterator it = htable->find(&key);
#endif
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


/*
 *	Recognize dead body shapes.  +++++++Hopefully, there's a better way.
 */

int Is_body
	(
	int shapeid
	)
	{
	switch (shapeid)
		{
	case 400:
	case 414:
	case 762:
	case 778:
	case 892:
		return 1;
	case 402:
		return Game::get_game_type() == SERPENT_ISLE;
	default:
		return 0;
		}
	}
