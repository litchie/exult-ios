/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objserial.cc - Object serialization.
 **
 **	Written: 5/25/2001 - JSF
 **/

/*
Copyright (C) 2001  The Exult Team

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

#include <config.h>
#include "utils.h"
#include "objserial.h"
#include "servemsg.h"

/*
 *	Encode.
 */
class Serial_out
	{
	unsigned char *& buf;
public:
	Serial_out(unsigned char *& b) : buf(b)
		{  }
	void trans(int v)
		{ Write4(buf, v); }
	void trans(unsigned long v)
		{ Write4(buf, v); }
	void trans(bool v)
		{ *buf++ = (v ? 1 : 0); }
	};

/*
 *	Decode.
 */
class Serial_in
	{
	unsigned char *& buf;
public:
	Serial_in(unsigned char *& b) : buf(b)
		{  }
	void trans(int& v)
		{ v = Read4(buf); }
	void trans(unsigned long& v)
		{ v = Read4(buf); }
	void trans(bool &v)
		{ v = *buf++ ? true : false; }
	};

/*
 *	Read/write out data common to all objects.
 *
 *	Output:	1 if successful, else 0.
 */

template <class Serial> 
void Common_obj_io
	(
	Serial& io,
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame
	)
	{
	io.trans(addr);
	io.trans(tx);
	io.trans(ty);
	io.trans(tz);
	io.trans(shape);
	io.trans(frame);
	}

/*
 *	Low-level serialization for use both by Exult and ExultStudio (so
 *	don't put in anything that will pull in all of Exult).
 *
 *	Output:	1 if successful, else 0.
 */
template <class Serial> 
void Egg_object_io
	(
	unsigned char *& buf,		// Where to store data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	int& criteria,
	int& probability,
	int& distance,
	bool& nocturnal,
	bool& once,
	bool& auto_reset
	)
	{
	Serial io(buf);
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io.trans(criteria);
	io.trans(probability);
	io.trans(distance);
	io.trans(nocturnal);
	io.trans(once);
	io.trans(auto_reset);
	}

/*
 *	Send out an egg object.
 *
 *	Output:	0 if unsuccessful.
 */

int Egg_object_out
	(
	int fd,				// Socket.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	int& criteria,
	int& probability,
	int& distance,
	bool& nocturnal,
	bool& once,
	bool& auto_reset
	)
	{
	unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Egg_object_io<Serial_out>(ptr, addr, tx, ty, tz, shape, frame,
		criteria, probability, distance, nocturnal, once, auto_reset);
	return Exult_server::Send_data(fd, Exult_server::egg, buf, ptr - buf);
	}

