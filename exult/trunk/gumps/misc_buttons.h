/*
Copyright (C) 2000-2013 The Exult Team

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

#ifndef _MISC_BUTTONS_H_
#define _MISC_BUTTONS_H_

#include "Gump_button.h"

class Actor;

/*
 *	A checkmark for closing its parent:
 */
class Checkmark_button : public Gump_button
{
public:
	Checkmark_button(Gump *par, int px, int py);
	virtual Gump_widget *clone(Gump *par)
		{ return new Checkmark_button(par, x, y); }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
	virtual bool is_checkmark() const { return true; }
};

/*
 *	A 'heart' button for bringing up stats.
 */
class Heart_button : public Gump_button
{
public:
	Heart_button(Gump *par, int px, int py);
	virtual Gump_widget *clone(Gump *par)
		{ return new Heart_button(par, x, y); }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
};

/*
 *	A diskette for bringing up the 'save' box.
 */
class Disk_button : public Gump_button
{
public:
	Disk_button(Gump *par, int px, int py);
	virtual Gump_widget *clone(Gump *par)
		{ return new Disk_button(par, x, y); }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
};

/*
 *	The combat toggle button.
 */
class Combat_button : public Gump_button
{
public:
	Combat_button(Gump *par, int px, int py);
	virtual Gump_widget *clone(Gump *par)
		{ return new Combat_button(par, x, y); }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
	virtual void paint();
};

/*
 *	The halo button.
 */
class Halo_button : public Gump_button
{
	Actor *actor;			// Who this represents.
public:
	Halo_button(Gump *par, int px, int py, Actor *a);
	virtual Gump_widget *clone(Gump *par)
		{ return new Halo_button(par, x, y, actor); }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
};

/*
 *	Combat mode.  Has 10 frames corresponding to Actor::Attack_mode.
 */
class Combat_mode_button : public Gump_button
{
	Actor *actor;			// Who this represents.
public:
	Combat_mode_button(Gump *par, int px, int py, Actor *a);
	virtual Gump_widget *clone(Gump *par)
		{ return new Combat_mode_button(par, x, y, actor); }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
};

/*
 *	The Serpent Isle Combat Stats Button
 */
class Cstats_button : public Gump_button
{
public:
	Cstats_button(Gump *par, int px, int py);
	virtual Gump_widget *clone(Gump *par)
		{ return new Cstats_button(par, x, y); }

 	// What to do when 'clicked':
	virtual bool activate(int button=1);
};

#endif

