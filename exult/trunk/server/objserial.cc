/**
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "utils.h"
#include "objserial.h"
#include "servemsg.h"
#include <iostream>

using std::cout;
using std::endl;

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
void Object_io
	(
	Serial &io,			// Where to store data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	int& quality,
	std::string& name
	)
	{
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io << quality << name;
	}

/*
 *	Low-level serialization for use both by Exult and ExultStudio (so
 *	don't put in anything that will pull in all of Exult).
 *
 *	Output:	1 if successful, else 0.
 */
template <class Serial> 
void Container_io
	(
	Serial &io,			// Where to store data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,		// Absolute tile coords.
	int& shape, int& frame,
	int& quality,
	std::string& name,
	unsigned char& resistance,
	bool& invisible, bool& okay_to_take
	)
	{
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io << quality << name << resistance << invisible << okay_to_take;
	}

/*
 *	Low-level serialization for use both by Exult and ExultStudio (so
 *	don't put in anything that will pull in all of Exult).
 *
 *	Output:	1 if successful, else 0.
 */
template <class Serial> 
void Barge_object_io
	(
	Serial &io,			// Where to store data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
 	int& shape, int& frame,
	int& xtiles,
	int& ytiles,
	int& dir
	)
	{
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io << xtiles << ytiles << dir;
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
	Serial &io,			// Where to store data.
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
	int& data1, int& data2, int& data3,
	std::string& str1
	)
	{
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io << type << criteria << probability << distance << 
		nocturnal << once << hatched << auto_reset << 
		data1 << data2 << data3 << str1;
	}

/*
 *	Low-level serialization for use both by Exult and ExultStudio (so
 *	don't put in anything that will pull in all of Exult).
 *
 *	Output:	1 if successful, else 0.
 */
template <class Serial> 
static void Npc_actor_io
	(
	Serial &io,			// Where to store data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame, int& face,
	std::string& name,
	short& npc_num,
	short& ident,
	int& usecode,
	int *properties,		// Must have room for 12.
	short& attack_mode,
	short& alignment,
	unsigned long& oflags,		// Object flags.
	unsigned long& siflags,		// Extra flags for SI.
	unsigned long& type_flags,	// Movement flags.
	short& num_schedules,		// # of schedule changes.
	Serial_schedule *schedules	// Schedule changes.  Room for 8.
	)
	{
	Common_obj_io<Serial>(io, addr, tx, ty, tz, shape, frame);
	io << face << name << npc_num << ident << usecode;
	int i;
	for (i = 0; i < 12; i++)
		io << properties[i];
	io << attack_mode << alignment << oflags << siflags << type_flags;
	io << num_schedules;
	for (i = 0; i < num_schedules; i++)
		io << schedules[i].time << schedules[i].type <<
				schedules[i].tx << schedules[i].ty <<
				schedules[i].tz;
	}

/*
 *	Send out an object.
 *
 *	Output:	-1 if unsuccessful.  0 if okay.
 */

int Object_out
	(
	int fd,				// Socket.
	Exult_server::Msg_type id,	// Message id.
	unsigned long addr,		// Address.
	int tx, int ty, int tz,		// Absolute tile coords.
	int shape, int frame,
	int quality,
	std::string name
	)
	{
	static unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Serial_out io(ptr);
	Object_io(io, addr, tx, ty, tz, shape, frame, quality,
		name);
	return Exult_server::Send_data(fd, id, buf, ptr - buf);
	}

/*
 *	Decode an object.
 *
 *	Output:	0 if unsuccessful.
 */

int Object_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	int& quality,
	std::string& name
	)
	{
	unsigned char *ptr = data;
	Serial_in io(ptr);
	Object_io(io, addr, tx, ty, tz, shape, frame, quality,
		name);
	return (ptr - data) == datalen;
	}

/*
 *	Send out a container object.
 *
 *	Output:	-1 if unsuccessful.  0 if okay.
 */

int Container_out
	(
	int fd,				// Socket.
	unsigned long addr,		// Address.
	int tx, int ty, int tz,		// Absolute tile coords.
	int shape, int frame,
	int quality,
	std::string name,
	unsigned char resistance,
	bool invisible, bool okay_to_take
	)
	{
	static unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Serial_out io(ptr);
	Container_io(io, addr, tx, ty, tz, shape, frame, quality,
		name, resistance, invisible, okay_to_take);
	return Exult_server::Send_data(fd, Exult_server::container, buf, ptr - buf);
	}

/*
 *	Decode a container object.
 *
 *	Output:	0 if unsuccessful.
 */

int Container_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,		// Absolute tile coords.
	int& shape, int& frame,
	int& quality,
	std::string& name,
	unsigned char& resistance,
	bool& invisible, bool& okay_to_take
	)
	{
	unsigned char *ptr = data;
	Serial_in io(ptr);
	Container_io(io, addr, tx, ty, tz, shape, frame, quality,
		name, resistance, invisible, okay_to_take);
	return (ptr - data) == datalen;
	}

/*
 *	Send out an barge object.
 *
 *	Output:	-1 if unsuccessful.  0 if okay.
 */

int Barge_object_out
	(
	int fd,				// Socket.
	unsigned long addr,		// Address.
	int tx, int ty, int tz,	// Absolute tile coords.
	int shape, int frame,
	int xtiles,
	int ytiles,
	int dir
	)
	{
	static unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Serial_out io(ptr);
	Barge_object_io(io, addr, tx, ty, tz, shape, frame,
		xtiles, ytiles, dir);
	return Exult_server::Send_data(fd, Exult_server::barge, buf,ptr - buf);
	}

/*
 *	Decode an barge object.
 *
 *	Output:	0 if unsuccessful.
 */

int Barge_object_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	int &xtiles,
	int &ytiles,
	int &dir
	)
	{
	unsigned char *ptr = data;
	Serial_in io(ptr);
	Barge_object_io(io, addr, tx, ty, tz, shape, frame,
		xtiles, ytiles, dir);
	return (ptr - data) == datalen;
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
	int data1, int data2, int data3,
	std::string str1
	)
	{
	static unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Serial_out io(ptr);
	Egg_object_io(io, addr, tx, ty, tz, shape, frame,
		type, criteria, probability, distance, 
		nocturnal, once, hatched, auto_reset,
		data1, data2, data3, str1);
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
	int& data1, int& data2, int& data3,
	std::string& str1
	)
	{
	unsigned char *ptr = data;
	Serial_in io(ptr);
	Egg_object_io(io, addr, tx, ty, tz, shape, frame,
		type, criteria, probability, distance, 
		nocturnal, once, hatched, auto_reset, 
		data1, data2, data3, str1);
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
	int shape, int frame, int face,
	std::string name,
	short npc_num,
	short ident,
	int usecode,
	int properties[12],
	short attack_mode,
	short alignment,
	unsigned long oflags,		// Object flags.
	unsigned long siflags,		// Extra flags for SI.
	unsigned long type_flags,	// Movement flags.
	short num_schedules,		// # of schedule changes.
	Serial_schedule *schedules	// Schedule changes.
	)
	{
	static unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Serial_out io(ptr);
	Npc_actor_io(io, addr, tx, ty, tz, shape, frame, face,
		name, npc_num, ident, usecode, 
		properties, attack_mode, alignment,
		oflags, siflags, type_flags,
		num_schedules, schedules);
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
	int& shape, int& frame, int& face,
	std::string& name,
	short& npc_num,
	short& ident,
	int& usecode,
	int properties[12],
	short& attack_mode,
	short& alignment,
	unsigned long& oflags,		// Object flags.
	unsigned long& siflags,		// Extra flags for SI.
	unsigned long& type_flags,	// Movement flags.
	short& num_schedules,		// # of schedule changes.
	Serial_schedule *schedules	// Schedule changes.  Room for 8.
	)
	{
	unsigned char *ptr = data;
	Serial_in io(ptr);
	Npc_actor_io(io, addr, tx, ty, tz, shape, frame, face,
		name, npc_num, ident, usecode, 
		properties, attack_mode, alignment,
		oflags, siflags, type_flags,
		num_schedules, schedules);
	return (ptr - data) == datalen;
	}

/*
 *	Game info. IO.
 */

template <class Serial> 
void Game_info_io
	(
	Serial &io,			// Where to store data.
	int& version,			// Server/client version.
	int& edit_lift,			// Lift being edited.
	int& hide_lift,			// Lift being hidden.
	bool& map_editing,		// In 'map-editing' mode.
	bool& tile_grid,		// Showing tile grid.
	bool& map_modified,		// Map was changed.
	int& edit_mode			// Mode we're in.
	)
	{
	io << version << edit_lift << hide_lift << 
			map_editing << tile_grid << map_modified << edit_mode;
	}

/*
 *	Send out game info.
 *
 *	Output:	-1 if unsuccessful.  0 if okay.
 */

int Game_info_out
	(
	int fd,				// Socket.
	int version,			// Server/client version.
	int edit_lift,			// Lift being edited.
	int hide_lift,			// Lift being hidden.
	bool map_editing,		// In 'map-editing' mode.
	bool tile_grid,			// Showing tile grid.
	bool map_modified,		// Map was changed.
	int edit_mode			// Mode we're in.
	)
	{
	static unsigned char buf[Exult_server::maxlength];
	unsigned char *ptr = &buf[0];
	Serial_out io(ptr);
	Game_info_io(io, version, edit_lift, hide_lift, 
			map_editing, tile_grid, map_modified, edit_mode);
	return Exult_server::Send_data(fd, Exult_server::info, buf, ptr - buf);
	}

/*
 *	Decode game info.
 *
 *	Output:	0 if unsuccessful.
 */

int Game_info_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	int& version,			// Server/client version.
	int& edit_lift,			// Lift being edited.
	int& hide_lift,			// Lift being hidden.
	bool& map_editing,		// In 'map-editing' mode.
	bool& tile_grid,		// Showing tile grid.
	bool& map_modified,		// Map was changed.
	int& edit_mode			// Mode we're in.
	)
	{
	unsigned char *ptr = data;
	Serial_in io(ptr);
	Game_info_io(io, version, edit_lift, hide_lift, map_editing, 
					tile_grid, map_modified, edit_mode);
	return (ptr - data) == datalen;
	}

