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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#include "server.h"
#include "config.h"
#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;
/*
 *	Sockets, etc.
 */
extern int xfd;				// X-windows fd.
int listen_socket = -1;			// Listen here for map-editor.
int client_socket = -1;			// Socket to the map-editor.
int highest_fd = -1;			// Largest fd + 1.

/*
 *	Set the 'highest_fd' value to 1 + <largest fd>.
 */
inline void Set_highest_fd
	(
	)
	{
	highest_fd = xfd;		// Figure highest to listen to.
	if (listen_socket > highest_fd)
		highest_fd = listen_socket;
	if (client_socket > highest_fd)
		highest_fd = client_socket;
	highest_fd++;			// Select wants 1+highest.
	}

/*
 *	Initialize server for map-editing.  Call this AFTER xfd has been set.
 */

void Server_init
	(
	)
	{
	listen_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (listen_socket < 0)
		perror("Failed to open map-editor socket");
	else 
		{
					// Get location of socket file.
		std::string servename = get_system_path("<GAMEDAT>/exultserver");
					// Make sure it isn't there.
		unlink(servename.c_str());
		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, servename.c_str());
		if (bind(listen_socket, (struct sockaddr *) &addr, 
		      sizeof(addr.sun_family) + strlen(addr.sun_path)) == -1 ||
		    listen(listen_socket, 1) == -1)
			{
			perror("Bind or listen on socket failed");
			close(listen_socket);
			listen_socket = -1;
			}
		else			// Set to be non-blocking.
			{
			cout << "Listening on socket " << listen_socket
								<< endl;
			fcntl(listen_socket, F_SETFL, 
				fcntl(listen_socket, F_GETFL) | O_NONBLOCK);
			}
		}
	Set_highest_fd();
	}

/*
 *	Close the server.
 */

void Server_close
	(
	)
	{
	// unlink socket file+++++++
	}

/*
 *	A message from a client is available, so handle it.
 */

static void Handle_client_message
	(
	int& fd				// Socket to client.  May be closed.
	)
	{
	unsigned char buf[256];		// +++++We should buffer reads.
	int len = read(fd, buf, 1);	// Get magic.
	cerr << "Client message read: " << len << " bytes" << endl;
	if (!len)			// Closed?
		{
		close(fd);
		fd = -1;
		Set_highest_fd();
		return;
		}
	if (len == -1)			// Nothing available?
		return;
	if (buf[0] != Exult_server::magic)
		{
		cout << "Bad magic read" << endl;
		return;
		}
	if (read(fd, buf, 3) != 3)
		{
		cout << "Couldn't read length+type" << endl;
		return;
		}
	int datalen = buf[0] | (buf[1]<<8);
	int type = buf[2];		// Message type.
	if (datalen > Exult_server::maxlength)
		{
		cout << "Length " << datalen << " exceeds max" << endl;
		//+++++++++Eat the chars.
		return;
		}
	datalen = read(fd, buf, datalen);// Read data.
	//+++++++++++
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
	if (listen_socket >= 0)
		FD_SET(listen_socket, &rfds);
	if (client_socket >= 0)
		FD_SET(client_socket, &rfds);
					// Wait for timeout or event.
	if (select(highest_fd, &rfds, 0, 0, &timer) > 0)
		{			// Something's come in.
		if (listen_socket >= 0 && FD_ISSET(listen_socket, &rfds))
			{		// New client connection.
					// For now, just one at a time.
			if (client_socket >= 0)
				close(client_socket);
			client_socket = accept(listen_socket, 0, 0);
			cout << "Accept returned client_socket = " <<
							client_socket << endl;
					// Non-blocking.
			fcntl(client_socket, F_SETFL, 
				fcntl(client_socket, F_GETFL) | O_NONBLOCK);
			Set_highest_fd();
			}
		if (client_socket >= 0 && FD_ISSET(client_socket, &rfds))
			Handle_client_message(client_socket);
		}
	}

#endif
