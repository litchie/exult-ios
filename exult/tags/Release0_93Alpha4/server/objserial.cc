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
	Serial_out& operator<<(int v)
		{ Write4(buf, v); return *this; }
	Serial_out& operator<<(unsigned long v)
		{ Write4(buf, v); return *this; }
	Serial_out& operator<<(short v)
		{ Write2(buf, v); return *this; }
	Serial_out& operator<<(bool v)
		{ *buf++ = (v ? 1 : 0); return *this; }
	Serial_out& operator<<(std::string& s);
	};

/*
 *	Write out a string.
 */
Serial_out& Serial_out::operator<<
	(
	std::string& s
	)
	{
	const char *str = s.c_str();
	int len = std::strlen(str);		// Get length.
	*this << len;			// First the length.
	std::memcpy(buf, str, len);		// Then the bytes.
	buf += len;
	return *this;
	}

/*
 *	Decode.
 */
class Serial_in
	{
	unsigned char *& buf;
public:
	Serial_in(unsigned char *& b) : buf(b)
		{  }
	Serial_in& operator<<(int& v)
		{ v = Read4(buf); return *this; }
	Serial_in& operator<<(unsigned long& v)
		{ v = Read4(buf); return *this; }
	Serial_in& operator<<(short v)
		{ v = Read2(buf); return *this; }
	Serial_in& operator<<(bool &v)
		{ v = *buf++ ? true : false; return *this; }
	Serial_in& operator<<(std::string& s);
	};

/*
 *	Read in a string.
 */
Serial_in& Serial_in::operator<<
	(
	std::string& s
	)
	{
	int len;
	(*this) << len;			// Get length.
	s.assign((char *) buf, len);	// Set string.
	buf += len;
	return *this;
	}

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
	io << addr << tx << ty << tz << shape << frame;
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
	int& type,
	int& criteria,
	int& probability,
	int& distance,
	bool& nocturnal,
	bool& once,
	bool& hatched,
	bool& auto_reset,
	int& data1, int& data2
	)
	{
	Serial io(buf);
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io << type << criteria << probability << distance << 
		nocturnal << once << hatched << auto_reset << data1 << data2;
	}

/*
 *	Low-level serialization for use both by Exult and ExultStudio (so
 *	don't put in anything that will pull in all of Exult).
 *
 *	Output:	1 if successful, else 0.
 */
template <class Serial> 
void Npc_actor_io
	(
	unsigned char *& buf,		// Where to store data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	std::string& name,
	short& ident,
	int& usecode,
	short properties[12],
	short& attack_mode,
	short& alignment,
	unsigned long& oflags,		// Object flags.
	unsigned long& siflags,		// Extra flags for SI.
	unsigned long& type_flags	// Movement flags.
	//+++++++++Schedule changes.
	)
	{
	Serial io(buf);
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io << name << ident << usecode;
	for (int i = 0; i < sizeof(properties)/sizeof(properties[0]); i++)
		io << properties[i];
	io << attack_mode << alignment << oflags << siflags << type_flags;
	}

/*
 *	Send out an egg object.
 *
 *	Output:	-1 if unsuccessful.  0 if okay.
 */

int Egg_object_out
	(
	int fd,				// Socket.
	unsigned long addr,		// Address.
	int tx, int ty, int tz,	// Absolute tile coords.
	int shape, int frame,
	int type,
	int criteria,
	int probability,
	int distance,
	bool nocturnal,
	bool once,
	bool hatched,
	bool auto_reset,
	int data1, int data2
	)
	{
	unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Egg_object_io<Serial_out>(ptr, addr, tx, ty, tz, shape, frame,
		type, criteria, probability, distance, 
		nocturnal, once, hatched, auto_reset,
		data1, data2);
	return Exult_server::Send_data(fd, Exult_server::egg, buf, ptr - buf);
	}

/*
 *	Decode an egg object.
 *
 *	Output:	0 if unsuccessful.
 */

int Egg_object_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	int& type,
	int& criteria,
	int& probability,
	int& distance,
	bool& nocturnal,
	bool& once,
	bool& hatched,
	bool& auto_reset,
	int& data1, int& data2
	)
	{
	unsigned char *ptr = data;
	Egg_object_io<Serial_in>(ptr, addr, tx, ty, tz, shape, frame,
		type, criteria, probability, distance, 
		nocturnal, once, hatched, auto_reset, data1, data2);
	return (ptr - data) == datalen;
	}

/*
 *	Send out an npc object.
 *
 *	Output:	-1 if unsuccessful.  0 if okay.
 */

int Npc_actor_out
	(
	int fd,				// Socket.
	unsigned long addr,		// Address.
	int tx, int ty, int tz,		// Absolute tile coords.
	int shape, int frame,
	std::string name,
	short ident,
	int usecode,
	short properties[12],
	short attack_mode,
	short alignment,
	unsigned long oflags,		// Object flags.
	unsigned long siflags,		// Extra flags for SI.
	unsigned long type_flags	// Movement flags.
	//+++++++++Schedule changes.
	)
	{
	unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Npc_actor_io<Serial_out>(ptr, addr, tx, ty, tz, shape, frame,
		name, ident, usecode, properties, attack_mode, alignment,
		oflags, siflags, type_flags);
	return Exult_server::Send_data(fd, Exult_server::npc, buf, ptr - buf);
	}

/*
 *	Decode an npc object.
 *
 *	Output:	0 if unsuccessful.
 */

int Npc_actor_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	std::string& name,
	short& ident,
	int& usecode,
	short properties[12],
	short& attack_mode,
	short& alignment,
	unsigned long& oflags,		// Object flags.
	unsigned long& siflags,		// Extra flags for SI.
	unsigned long& type_flags	// Movement flags.
	//+++++++++Schedule changes.
	)
	{
	unsigned char *ptr = data;
	Npc_actor_io<Serial_in>(ptr, addr, tx, ty, tz, shape, frame,
		name, ident, usecode, properties, attack_mode, alignment,
		oflags, siflags, type_flags);
	return (ptr - data) == datalen;
	}

