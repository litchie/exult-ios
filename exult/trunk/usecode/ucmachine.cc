/**
 **	ucmachine.cc - Interpreter for usecode.
 **
 **	Written: 8/12/99 - JSF
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ucmachine.h"
#include "conversation.h"
#include "keyring.h"


Usecode_machine::Usecode_machine
	(
	Game_window *gw
	) : gwin(gw), call_depth(0), keyring(0), conv(0), party_count(0)
	{
					// Clear global flags.
	std::memset(gflags, 0, sizeof(gflags));
					// Clear party list.
	std::memset((char *) &party[0], 0, sizeof(party));
	conv = new Conversation;
	keyring = new Keyring;
	}

/*
 *	Delete.
 */

Usecode_machine::~Usecode_machine
	(
	)
	{
	delete conv;
	delete keyring;
	}

void Usecode_machine::init_conversation()
{
	conv->init_faces();
}

int Usecode_machine::get_num_faces_on_screen() const
{
	 return conv->get_num_faces_on_screen();
}
