/*
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifndef	_ARGS_H_
#define	_ARGS_H_

// Handy argument processor. I'm certain the implementation could be better
// but it suffices quite well at the moment.

#include "exult_types.h"

#ifdef __DECCXX
#  undef declare
#endif

#ifndef ALPHA_LINUX_CXX
#  include <string>
#endif
#include <vector>

class	Args
{
	struct Opts
	{
		std::string	option;
		bool	*bval;
		std::string  *sval;
		int	*ival;
		uint32 *uval;

		bool	dbval;
		std::string	dsval;
		int	dival;
		uint32 duval;
		enum { no_type=0,type_bool,type_string,type_int,type_unsigned } valuetype;
		Opts() :option(""),valuetype(no_type) {};
		~Opts() {};
	};
	std::vector<Opts> options;
	public:
	Args() {};
	~Args() {};
	void	declare(const char *s,bool *b,bool defval=true);
	void	declare(const char *s,std::string *b,const char *defval=0);
	void	declare(const char *s,int *b,int defval=0);
	void	declare(const char *s,uint32 *b,uint32 defval=0);
	void	process(int argc,char **argv);
};

#endif
