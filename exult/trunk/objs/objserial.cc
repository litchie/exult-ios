/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objserial.cc - Object serialization.
 **
 **	Written: 5/25/2001 - JSF
 **/

/*
Copyright (C) 2001  The Exult Team

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

//#include <rpc/types.h>
#include <rpc/xdr.h>

/*
 *	Read/write out data common to all objects.
 *
 *	Output:	1 if successful, else 0.
 */

bool Common_obj_io
	(
	XDR *xdrs,
	unsigned long *addr,		// Address.
	int *tx, int *ty, int *tz,	// Absolute tile coords.
	int *shape, int *frame
	)
	{
	bool okay = 
		xdr_u_long(xdrs, addr) &&
		xdr_int(xdrs, tx) &&
		xdr_int(xdrs, ty) &&
		xdr_int(xdrs, tz) &&
		xdr_int(xdrs, shape) &&
		xdr_int(xdrs, frame);
	return okay;
	}

/*
 *	Low-level serialization for use both by Exult and ExultStudio (so
 *	don't put in anything that will pull in all of Exult).
 *
 *	Output:	1 if successful, else 0.
 */
bool Egg_object_io
	(
	XDR *xdrs,
	unsigned long *addr,		// Address.
	int *tx, int *ty, int *tz,	// Absolute tile coords.
	int *shape, int *frame,
	int *criteria,
	int *probability,
	int *distance,
	char *nocturnal,
	char *once,
	char *auto_reset
	)
	{
	if (!Common_obj_io(xdrs, addr, tx, ty, tz, shape, frame))
		return false;
	if (!xdr_int(xdrs, criteria) || !xdr_int(xdrs, probability) ||
	    !xdr_int(xdrs, distance))
		return false;
	if (!xdr_char(xdrs, nocturnal) || !xdr_char(xdrs, once) ||
	    !xdr_char(xdrs, auto_reset))
		return false;
	return true;
	}

