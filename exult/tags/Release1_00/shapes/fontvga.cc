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
#include "vgafile.h"
#include "fontvga.h"
#include "fnames.h"
#ifndef ALPHA_LINUX_CXX
#  include <cctype>
#endif
#include "ibuf8.h"

using std::cout;
using std::endl;
using std::isspace;
// using std::string;

/*
 *	Fonts in 'fonts.vga':
 *
 *	0 = Normal yellow.
 *	1 = Large runes.
 *	2 = small black (as in zstats).
 *	3 = runes.
 *	4 = tiny black, used in books.
 *	5 = little white.
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
//static int hlead[NUM_FONTS] = {-1, 0, 1, 0, 1, 0, 0, -1, 0, 0};
// For scrolls (12/6/00):
static int hlead[NUM_FONTS] = {-2, -1, 0, -1, 0, -1, -1, -2, -1, -1, 0, 0};
/*
 *	Initialize.
 */

void Fonts_vga_file::init
	(
	)
	{
	int cnt = sizeof(fonts)/sizeof(fonts[0]);
	for (int i = 0; i < cnt; i++)
		fonts[i].load(FONTS_VGA, i, hlead[i], 0);
	}

