/**
 **	Objserial.h - Object serialization.
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

#ifndef OBJSERIAL_H
#define OBJSERIAL_H	1

#include <string>

extern int Object_out
	(
	int fd,				// Socket.
	unsigned long addr,		// Address.
	int tx, int ty, int tz,		// Absolute tile coords.
	int shape, int frame,
	int quality,
	std::string name
	);
extern int Object_in
	(
	unsigned char *data,		// Data that was read.
	int datalen,			// Length of data.
	unsigned long& addr,		// Address.
	int& tx, int& ty, int& tz,	// Absolute tile coords.
	int& shape, int& frame,
	int& quality,
	std::string& name
	);

extern int Egg_object_out
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
	);
extern int Egg_object_in
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
	);

struct Serial_schedule			// For passing a schedule change.
	{
	short time;			// 0-7 (3-hour period).
	short type;			// Schedule type (mostly 0-31).
	int tx, ty;			// Tile to go to.
	};

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
	int *properties,		// 12 entries.
	short attack_mode,
	short alignment,
	unsigned long oflags,		// Object flags.
	unsigned long siflags,		// Extra flags for SI.
	unsigned long type_flags,	// Movement flags.
	short num_schedules,		// # of schedule changes.
	Serial_schedule *schedules	// Schedule changes.
	);
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
	int *properties,		// Must have room for 12.
	short& attack_mode,
	short& alignment,
	unsigned long& oflags,		// Object flags.
	unsigned long& siflags,		// Extra flags for SI.
	unsigned long& type_flags,	// Movement flags.
	short& num_schedules,		// # of schedule changes.
	Serial_schedule *schedules	// Schedule changes.  Room for 8.
	);
#endif




