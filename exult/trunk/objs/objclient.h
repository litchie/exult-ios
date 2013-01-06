/*
 *	objclient.h - Game objects that have been removed, but need deleting.
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

#ifndef OBJCLIENT_H
#define OBJCLIENT_H	1

class Game_object;
/*
 *	A class (ie, Schedule) that needs to be notified when an object
 *	is about to be deleted.
 */
class Object_client {
	Game_object_vector objs;
protected:
					// Notify that schedule's obj. has been moved or deleted.
	virtual void notify_object_gone(Game_object *obj) = 0;
	void add_client(Game_object *obj) {
		if (obj && obj->add_client(this))
		    objs.push_back(obj);
	}
	void add_client(Game_object_vector& objs);
	void remove_clients();
public:
	void object_gone(Game_object *obj);
	virtual ~Object_client() {  }
	void kill_client_list();
};
#endif	/* OBJCLIENT_H */
