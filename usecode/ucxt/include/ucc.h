/*
 *  Copyright (C) 2001-2011  The Exult Team
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

#ifndef UCC_H
#define UCC_H

#include <vector>
#include <string>

class UCc;

class UCc {
public:
	UCc(const unsigned int offset = 0, const unsigned int id = 0, const std::vector<unsigned char> &params = std::vector<unsigned char>())
		: _id(id), _offset(offset), _params(params), _tagged(false), _missing_args(0) {};
	UCc(const unsigned int id, const std::string &miscstr)
		: _id(id), _miscstr(miscstr), _missing_args(0) {};
	UCc(const std::vector<unsigned int> &params_parsed)
		: _id(0), _offset(0), _params_parsed(params_parsed), _tagged(false), _missing_args(0) {};

	unsigned int               _id;
	std::string                _miscstr;
	unsigned int               _offset;
	std::vector<unsigned char> _params;
	std::vector<unsigned int>  _params_parsed;
	bool                       _tagged;
	std::vector<unsigned int>  _jump_offsets;
	unsigned int               _missing_args;

	/* A temporary array to hold the items this opcode theoretically popped
	   from the stack. This should probably go in it's own wrapper class with
	   the current UCc. */
	std::vector<UCc *>    _popped;
};

class UCOptions {
public:
	UCOptions() : output_extern_header(false), noconf(false),
		rawops(false), autocomment(false),
		uselesscomment(false), verbose(false),
		very_verbose(false),
		ucdebug(false), basic(false),
		output_list(false), output_asm(false),
		output_ucs(false), output_flag(false),
		output_trans_table(false),
		mode_all(false), mode_dis(false),
		force_ext32(false),
		_game(GAME_BG)
	{};

	bool game_bg()      const {
		return _game == GAME_BG;
	};
	bool game_si()      const {
		return _game == GAME_SI;
	};
	bool game_fov()     const {
		return _game == GAME_FOV;
	};
	bool game_ss()      const {
		return _game == GAME_SS;
	};
	bool game_u7()      const {
		return game_bg() || game_si() || game_fov() || game_ss();
	};
	bool game_u8()      const {
		return _game == GAME_U8;
	};

	bool output_extern_header;

	bool noconf;
	bool rawops;
	bool autocomment;
	bool uselesscomment;
	bool verbose;
	bool very_verbose;
	bool ucdebug;
	bool basic;

	bool output_list;
	bool output_asm;
	bool output_ucs;
	bool output_flag;
	bool output_trans_table;

	bool mode_all;
	bool mode_dis;

	bool force_ext32; // force ext32 function format output for all functions

	//private:
	unsigned int _game;
	enum { GAME_BG = 1, GAME_FOV = 2, GAME_SI = 3, GAME_SS = 4, GAME_U8 = 5 };
};

#endif
