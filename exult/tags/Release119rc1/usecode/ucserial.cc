/*
 *	ucserial.cc - Serialization of usecode objects
 *
 *  Copyright (C) 2002  The Exult Team
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

#include "servemsg.h"
#include "ucserial.h"
#include "useval.h"
#include "utils.h"
#include "debugmsg.h"

/*

/*
 *	Read/write out data common to all objects.
 *
 *	Output:	1 if successful, else 0.
 */

template <class Serial> 
void Stack_frame_io
	(
	Serial& io,
	int& functionid,
	int& ip,
	int& call_chain,
	int& call_depth,
	int& eventid,
	int& caller_item,
	int& num_args,
	int& num_vars
	)
{
	unsigned char c = (unsigned char)Exult_server::dbg_stackframe;
	io << c << functionid << ip << call_chain << call_depth
	   << eventid << caller_item << num_args << num_vars;
	// locals!
}

int Stack_frame_out
	(
	int fd,				// Socket.
	int functionid,
	int ip,
	int call_chain,
	int call_depth,
	int eventid,
	int caller_item,
	int num_args,
	int num_vars,	
	Usecode_value* locals
	)
{
	static unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Serial_out io(ptr);
	Stack_frame_io<Serial_out>(io, functionid, ip, call_chain, call_depth,
							   eventid, caller_item, num_args, num_vars);

	int remaining = Exult_server::maxlength - (ptr - buf);
	for (int i = 0; i < num_args + num_vars; i++) {
		int vallen = locals[i].save(ptr, remaining);
		// error checking...
		ptr += vallen;
		remaining -= vallen;
	}

	return Exult_server::Send_data(fd, Exult_server::usecode_debugging, 
								   buf, ptr - buf);
	// locals!
}


bool Stack_frame_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	int& functionid,
	int& ip,
	int& call_chain,
	int& call_depth,
	int& eventid,
	int& caller_item,
	int& num_args,
	int& num_vars,
	Usecode_value*& locals
	)
{
	unsigned char *ptr = data;
	Serial_in io(ptr);
	Stack_frame_io<Serial_in>(io, functionid, ip, call_chain, call_depth,
							  eventid, caller_item, num_args, num_vars);

	int remaining = datalen - (ptr - data);
	locals = new Usecode_value[num_args + num_vars];
	for (int i = 0; i < num_args + num_vars; i++) {
		unsigned char *tmpptr = ptr;
		locals[i].restore(ptr, remaining);
		// error checking...
		remaining -= (ptr - tmpptr);
	}


	return (ptr - data) == datalen;
	// locals!
}
