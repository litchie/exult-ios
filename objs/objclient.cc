/*
 *  objclient.cc - Game objects that have been removed, but need deleting.
 *
 *  Copyright (C) 2000-2013  The Exult Team
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <vector>
#include "objs.h"
#include "objclient.h"

/*
 *      Add a list of clients.
 */
void Object_client::add_client(Game_object_vector &objs) {
	for (Game_object_vector::iterator it = objs.begin();
	        it != objs.end(); ++it)
		add_client(*it);
}

/*
 *      Remove all clients.
 */
void Object_client::remove_clients() {
	for (Game_object_vector::iterator it = objs.begin();
	        it != objs.end(); ++it)
		(*it)->remove_client(this);
	objs.clear();
}

/*
 *  Remove all clients without notifying them. Do not use unless
 *  you know what you are doing.
 */
void Object_client::kill_client_list() {
	objs.clear();
}

/*
 *      An object should call this when it's about to be removed.
 */
void Object_client::object_gone(Game_object *obj) {
	for (Game_object_vector::iterator it = objs.begin();
	        it != objs.end(); ++it) {
		if ((*it) == obj) {
			objs.erase(it);
			notify_object_gone(obj);
			return;
		}
	}
}
