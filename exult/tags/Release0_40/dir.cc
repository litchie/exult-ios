/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Dir.cc - Directions.
 **
 **	Written: 10/1/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include "dir.h"

/*
 *	Return the direction for a given slope (0-7).
 *	NOTE:  Assumes cartesian coords, NOT screen coords. (which have y
 *		growing downwards).
 */

Direction Get_direction
	(
	int deltay,
	int deltax
	)
	{
	if (deltax == 0)
		return deltay > 0 ? north : south;
	int dydx = (1024*deltay)/deltax;// Figure 1024*tan.
	if (dydx >= 0)
		if (deltax >= 0)	// Top-right quadrant?
			return dydx <= 424 ? east : dydx <= 2472 ? northeast
								: north;
		else			// Lower-left.
			return dydx <= 424 ? west : dydx <= 2472 ? southwest
								: south;
	else
		if (deltax >= 0)	// Lower-right.
			return dydx >= -424 ? east : dydx >= -2472 ? southeast
								: south;
		else			// Top-left?
			return dydx >= -424 ? west : dydx >= -2472 ? northwest
								: north;
	}

/*
 *	Return the direction for a given slope (0-7), rounded to NSEW.
 *	NOTE:  Assumes cartesian coords, NOT screen coords. (which have y
 *		growing downwards).
 */

Direction Get_direction4
	(
	int deltay,
	int deltax
	)
	{
	if (deltax >= 0)		// Right side?
		return (deltay > deltax ? north : deltay < -deltax ? south
								: east);
	else				// Left side.
		return (deltay > -deltax ? north : deltay < deltax ? south
								: west);
	}

/*
 *	Return the direction for a given slope (0-15).
 *	NOTE:  Assumes cartesian coords, NOT screen coords. (which have y
 *		growing downwards).
 */

int Get_direction16
	(
	int deltay,
	int deltax
	)
	{
	if (deltax == 0)
		return deltay > 0 ? 0 : 8;
	int dydx = (1024*deltay)/deltax;// Figure 1024*tan.
	int adydx = dydx < 0 ? -dydx : dydx;
	int angle = 0;
	if (adydx < 1533)		// atan(5*11.25)
		{
		if (adydx < 204)	// atan(11.25).
			angle = 4;
		else if (adydx < 684)	// atan(3*11.25).
			angle = 3;
		else
			angle = 2;
		}
	else
		{
		if (adydx < 5148)	// atan(7*11.25).
			angle = 1;
		else
			angle = 0;
		}
	if (deltay < 0)			// Check quadrants.
		if (deltax > 0)
			angle = 8 - angle;
		else
			angle += 8;
	else if (deltax < 0)
		angle = 16 - angle;
	return angle % 16;
	}

#if 0
/*
 *	Lookup arctangent in a table for degrees 0-85.
 */

static unsigned Lookup_atan
	(
	unsigned dydx
	)
	{
					// 1024*tan(x), where x ranges from
					//   5 deg to 85.
	static unsigned tans[18] = {0, 90, 181, 274, 373, 477, 591, 717, 859,
			1024, 1220, 1462, 1774, 2196, 2813, 3822, 5807, 11704};
	static int cnt = sizeof(tans)/sizeof(tans[0]);
	for (int i = 1; i < cnt; i++)	// Don't bother with 0.
		if (dydx < tans[i])
			return (5*(i - 1));
	return (5*(cnt - 1));
	}

/*
 *	Return the arctangent, rounded to 5-degree increments as an 
 *	angle counter-clockwise from the east.
 *
 *	Output: Arctangent in degrees.
 */

unsigned Arctangent
	(
	int deltay,
	int deltax
	)
	{
	unsigned angle;			// Gets angle in degrees.
	int absx = deltax >= 0 ? deltax : -deltax;
	int absy = deltay >= 0 ? deltay : -deltay;
	if (absy > 23*absx)		// Vertical?
		angle = 90;
	else
		angle = Lookup_atan((1024*absy)/absx);
	if (deltay >= 0)
		if (deltax >= 0)	// Top-right quadrant?
			return angle;
		else			// Top-left?
			return 180 - angle;
	else
		if (deltax >= 0)	// Lower-right.
			return 360 - angle;
		else			// Lower-left.
			return 180 + angle;
	}
#endif
