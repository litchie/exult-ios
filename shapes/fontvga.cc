/**
 **	Fontvga.cc - Handle the 'fonts.vga' file and text rendering.
 **
 **	Written: 4/29/99 - JSF
 **/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <fstream>
#include <iostream>
#ifdef MACOS
  #include <cassert>
#endif
#include "fontvga.h"
#include "fnames.h"
#ifndef ALPHA_LINUX_CXX
#  include <cctype>
#endif
#include "utils.h"
#include "Flex.h"

using std::cout;
using std::endl;
// using std::string;

/*
 *	Fonts in 'fonts.vga':
 *
 *	0 = Normal yellow.
 *	1 = Large runes.
 *	2 = small black (as in zstats).
 *	3 = runes.
 *	4 = tiny black, used in books.
 *	5 = little white, glowing, for spellbooks.
 *	6 = runes.
 *	7 = normal red.
 *	8 = Serpentine (books)
 *	9 = Serpentine (signs)
 *	10 = Serpentine (gold signs)
 */

/*
 *	Horizontal leads, by fontnum:
 *
 *	This must include the Endgame fonts (currently 32-35)!!
 *      And the MAINSHP font (36)
 *	However, their values are set elsewhere
 */
// +++++TAG: This shouldn't be hard-coded.
static int hlead[] = {-2, -1, 0, -1, 0, 0, -1, -2, -1, -1};
/*
 *	Initialize.
 */

void Fonts_vga_file::init
	(
	)
	{
	int cnt = sizeof(hlead)/sizeof(hlead[0]);

	FlexFile sfonts(FONTS_VGA);
	FlexFile pfonts(PATCH_FONTS);
	int sn = sfonts.number_of_objects();
	int pn = pfonts.number_of_objects();
	int numfonts = pn > sn ? pn : sn;

	for (int i = 0; i < numfonts; i++)
		fonts[i].load(FONTS_VGA, PATCH_FONTS, i, i < cnt ? hlead[i] : 0, 0);
	}

