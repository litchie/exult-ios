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

#ifndef _GUMPS_H_
#define _GUMPS_H_

#include "mouse.h"

class Modal_gump;
class Game_window;

/*
 *	Verify user wants to quit.
 *
 *	Output:	1 to quit.
 */
int Okay_to_quit();


/*
 *	Prompt for a numeric value using a slider.
 *
 *	Output:	Value, or 0 if user hit ESC.
 */

int Prompt_for_number(int minval, int maxval, int step, int def);


/*
 *	Handle a modal gump, like the range slider or the save box, until
 *	the gump self-destructs.
 *
 *	Output:	0 if user hit ESC.
 */

int Do_Modal_gump(Modal_gump *, Mouse::Mouse_shapes);


/*
 *	Show a number.
 */

void Paint_num(Game_window *gwin, int num, int x, int y);


/*
 *	Delay between animations.
 */

inline void Delay
	(
	)
{
#ifdef XWIN
	X_Delay();
#else					/* May use this for Linux too. */
	SDL_Delay(20);			// Try 1/50 second.
#endif
}


char *newstrdup(const char *s);


#endif
