/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Server.cc - Server functions.
 **
 **	Written: 5/2/2001 - JSF
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

/*
 *	For the time being, we'll only inflict this on X users.
 */
#ifdef XWIN

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#include "config.h"
#include "utils.h"

/*
 *	Sockets, etc.
 */
extern int xfd;				// X-windows fd.
int mapedit_listen_socket = -1;		// Listen here for map-editor.
int mapedit_socket = -1;		// Socket to the map-editor.
int highest_fd = -1;			// Largest fd + 1.
/*
 *	Initialize server for map-editing.  Call this AFTER xfd has been set.
 */

void Server_init
	(
	)
	{
	mapedit_listen_socket = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (mapedit_listen_socket < 0)
		cerr << "Failed to open map-editor socket" << endl;
	else 
		{
					// Get location of socket file.
		string servename = get_system_path("<GAMEDAT>/exultserver");
		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, servename.c_str());
		if (bind(mapedit_listen_socket, (struct sockaddr *) &addr, 
		      sizeof(addr.sun_family) + strlen(addr.sun_path)) == -1 ||
		    listen(mapedit_listen_socket, 1) == -1)
			{
			perror("Bind or listen failed");
			close(mapedit_listen_socket);
			mapedit_listen_socket = -1;
			}
		}
	int highest_fd = xfd;		// Figure highest to listen to.
	if (mapedit_listen_socket > highest_fd)
		highest_fd = mapedit_listen_socket;
	highest_fd++;			// Select wants 1+highest.
	}

/*
 *	Delay for a fraction of a second, or until there's data available.
 *	If a server request comes, it's handled here.
 */

void Server_delay
	(
	)
	{
	extern int xfd;
	fd_set rfds;
	struct timeval timer;
	timer.tv_sec = 0;
	timer.tv_usec = 50000;		// Try 1/50 second.
	FD_ZERO(&rfds);
	FD_SET(xfd, &rfds);
	if (mapedit_listen_socket > 0)
		FD_SET(mapedit_listen_socket, &rfds);
	if (mapedit_socket > 0)
		FD_SET(mapedit_socket, &rfds);
					// Wait for timeout or event.
	if (select(highest_fd, &rfds, 0, 0, &timer) > 0)
		{			// Something's come in.
//+++++++++++++++++++
		}
	}

#endif
