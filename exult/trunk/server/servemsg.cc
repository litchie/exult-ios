/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Servemsg.cc - Server msgs.
 **	NOTE:	This is for inclusion by both client and server.
 **
 **	Written: 5/28/2001 - JSF
 **/

/*
Copyright (C) 2000-2001 The Exult Team

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

#ifdef XWIN

#include <unistd.h>
#include <iostream.h>			/* For debugging msgs. */
#include "servemsg.h"
#include <cstring>

namespace Exult_server
{

/*
 *	Send data.
 *
 *	Output:	0 if error.
 */

int Send_data
	(
	int socket,
	Msg_type id,
	unsigned char *data,
	int datalen
	)
	{
	unsigned char buf[maxlength + hdrlength];
	buf[0] = magic&0xff;		// Store magic (low-byte first).
	buf[1] = (magic>>8)&0xff;
	buf[2] = datalen&0xff;		// Data length.
	buf[3] = (datalen>>8)&0xff;
	buf[4] = id;
	std::memcpy(&buf[5], data, datalen);	// The data itself.
	int len = datalen + hdrlength;
	return (write(socket, buf, len) == len);
	}

/*
 *	Read message from client.
 *
 *	Output:	Length of data, else 0.
 */

int Receive_data
	(
	int& socket,			// Closed, set to -1 if disconnected.
	Msg_type& id,			// ID returned.
	unsigned char *data,
	int datalen
	)
	{
	unsigned char buf[maxlength];
	int len = read(socket, buf, 2);	// Get magic.
	if (!len)			// Closed?
		{
		close(socket);
		socket = -1;
		return 0;
		}
	if (len == -1)			// Nothing available?
		return 0;
	int magic = buf[0] + (buf[1]<<8);
	if (magic != Exult_server::magic)
		{
		cout << "Bad magic read" << endl;
		return 0;
		}
	if (read(socket, buf, 3) != 3)
		{
		cout << "Couldn't read length+type" << endl;
		return 0;
		}
	int dlen = buf[0] | (buf[1]<<8);
					// Message type.
	id = (Exult_server::Msg_type) buf[2];
	if (dlen > Exult_server::maxlength || dlen > datalen)
		{
		cout << "Length " << datalen << " exceeds max" << endl;
		//+++++++++Eat the chars.
		return 0;
		}
	datalen = read(socket, buf, dlen);	// Read data.
	if (datalen < dlen)
		{
		cout << "Failed to read all " << dlen << " bytes" << endl;
		return 0;
		}
	return datalen;
	}




}

#endif	/* XWIN */

