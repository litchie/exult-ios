/*
 *  Copyright (C) 2001-2002  The Exult Team
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

#ifndef UCC_H
#define UCC_H

#include <vector>
#include <string>

class UCc;
using std::vector;
using std::string;

class UCc
{
	public:
		UCc(const unsigned int offset=0, const unsigned int id=0, const vector<unsigned char> &params=vector<unsigned char>())
		   : _id(id), _offset(offset), _params(params), _tagged(false) {};
		UCc(const unsigned int id, const string &miscstr)
		   : _id(id), _miscstr(miscstr) {};
        //UCc() : _id(0), _offset(0), _tagged(false) {};

		unsigned int          _id;
		string                _miscstr;
		unsigned int          _offset;
		vector<unsigned char> _params;
		vector<unsigned int>  _params_parsed;
		bool                  _tagged;
		vector<unsigned int>  _jump_offsets;
		
		/* A temporary array to hold the items this opcode theoretically popped
		   from the stack. This should probably go in it's own wrapper class with
		   the current UCc. */
		vector<UCc *>         _popped;
};

#endif
