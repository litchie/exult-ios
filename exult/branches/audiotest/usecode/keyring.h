/*
 *  Copyright (C) 2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _KEYRING_H
#define _KEYRING_H

#include <set>

class Keyring {
 public:
	Keyring() { }

	void read();  // read from KEYRING.DAT
	void write(); // write to KEYRING.DAT

	void clear();            // remove all keys
	void addkey(int qual);   // add key to keyring
	bool checkkey(int qual); // is key on keyring?

 private:
	std::set<int> keys;
};

#endif
