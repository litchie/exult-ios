/*
 *
 *  Copyright (C) 2006  The Exult Team
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

Frigidazzi object#(0x414) ()
{
	if (gflags[KNOWS_FURCAP_OWNER])
	{
		// When you give the fur cap back to Frigidazzi,
		// she clears a flag she "shouldn't". This hack is
		// so that we don't have to rewrite her entire
		// usecode to fix that...
		// I ordinarily would simply cache the flag state,
		// call the original function and set the new flag...
		// but there are a couple hidden aborts when
		// Rotoluncia's automaton is *not* summoned which
		// prevents that...
		script AVATAR after 5 ticks
		{	nohalt;					call setFurcapFlag;}
	}

	Frigidazzi.original();
}
