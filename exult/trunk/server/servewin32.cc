/*
Copyright (C) 2002 The Exult Team

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

// Win32 Only file

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if defined(WIN32) && defined(USE_EXULTSTUDIO)

#include <iostream>			/* For debugging msgs. */
#include <cstring>			/* For debugging msgs. */
#include "servemsg.h"
#include "servewin32.h"

using std::cout;
using std::cerr;
using std::endl;

#include <winsock2.h>

namespace Exult_server
{
bool gWSInitialized = false;
SOCKET gDataSocket = INVALID_SOCKET;
SOCKET gServerSocket = INVALID_SOCKET;
HANDLE hPortFile = INVALID_HANDLE_VALUE;

// Statics
static bool InitializeWinsock()
{
	if (gWSInitialized) return true;

	WSADATA gWsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &gWsaData );
   if ( iResult != NO_ERROR ) 
		cout << "Error at WSAStartup()" << std::endl;
	else
		gWSInitialized = true;

	return gWSInitialized;
}

bool OpenPortFile(const char *gamedat_path, bool writing)
{
	// Creates a file to store the port number in that is created
	// If this fails on the client, there isn't a server active
	char filename[MAX_PATH];
	strcpy(filename, gamedat_path);
	strcat(filename, "\\studio.port");

	// The locking is setup to prevent two servers from running for the same gamedat dir
	// it's also setup so the file is deleted when the server shuts down
    #ifndef UNDER_CE
	hPortFile = CreateFile (
		 filename,
		 writing?GENERIC_WRITE:GENERIC_READ,
		 FILE_SHARE_READ|FILE_SHARE_DELETE|(!writing?FILE_SHARE_WRITE:0),
		 NULL,
		 writing?CREATE_ALWAYS:OPEN_EXISTING,
		 FILE_ATTRIBUTE_TEMPORARY|(writing?FILE_FLAG_DELETE_ON_CLOSE:0),
		 NULL
	);
	#else
	wchar_t *Wfilename = new wchar_t[strlen(filename) + 1];
	mbstowcs(Wfilename, filename, strlen(filename));
	hPortFile = CreateFile (
		 Wfilename,
		 writing?GENERIC_WRITE:GENERIC_READ,
		 FILE_SHARE_READ|FILE_SHARE_DELETE|(!writing?FILE_SHARE_WRITE:0),
		 NULL,
		 writing?CREATE_ALWAYS:OPEN_EXISTING,
		 FILE_ATTRIBUTE_TEMPORARY|(writing?FILE_FLAG_DELETE_ON_CLOSE:0),
		 NULL
	);
	#endif
	if (hPortFile == INVALID_HANDLE_VALUE) return false;
	return true;
}

// Hack functions
int write(int file, const void *v, unsigned int len)
{
	return send(gDataSocket,(const char*)v,len, 0);
}

int read(int file, void *v, unsigned int len)
{
	if (len == 0) return 0;
	return recv(gDataSocket,(char*)v,len,0);;
	/*
	WSABUF buffer;
	buffer.buf = (char*)v;
	buffer.len = len;
	DWORD flags = 0;//MSG_PARTIAL;
	DWORD numberRead = 0;
	WSARecv(gDataSocket,&buffer,1,&numberRead,&flags,0,0);
	return numberRead;
	*/
}

int close(int file)
{	
	return 0;
}


// Server Functions

bool create_pipe (const char *gamedat_path)
{
	if (!InitializeWinsock()) return false;

	// Create a socket.
	gServerSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( gServerSocket == INVALID_SOCKET ) {
		cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
		close_pipe();
		return false;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	service.sin_port = 0;	// Random port so we can have multiple instances of exult studio and exult running... in theory

	if ( bind( gServerSocket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) {
		cerr << "bind() failed." << std::endl;
		close_pipe();
		return false;
	}

	// Listen on the socket.
	if ( listen( gServerSocket, 1 ) == SOCKET_ERROR )
	{
		cerr << "Error listening on socket." << std::endl;
		close_pipe();
		return false;
	}

	int socksize = sizeof(service);
	getsockname (gServerSocket, (SOCKADDR*) &service, &socksize);

	if (!OpenPortFile(gamedat_path,true))
	{
		cerr << "Error creating temporary file in gamedat dir for port number." << endl;
		close_pipe();
		return false;
	}

	DWORD numWriten;
	WriteFile(hPortFile,&service.sin_port,2,&numWriten,NULL);

	cout << "Opened socket for Exult Server on port " << ntohs(service.sin_port) << endl;

	return true;
}

void setup_connect()
{
	// Not required
}

bool try_connect_to_client(const char *gamedat_path)
{
	// returns size of data waiting, -1 if disconnected
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(gServerSocket, &rfds);
	TIMEVAL tv;
	tv.tv_sec = 0;
	tv.tv_usec = 20000;
	if (!select(0,&rfds,0,0,&tv)) return false;

	// Does accept
	gDataSocket = accept(gServerSocket,NULL,NULL);
	if (gDataSocket == INVALID_SOCKET) return false;

	return true;
}

void disconnect_from_client()
{
	// Does cleanup after client has disconnected, don't need to do anything really...
}

void close_pipe()
{
	// Close socket
	if (hPortFile != INVALID_HANDLE_VALUE) CloseHandle(hPortFile);
	if (gDataSocket != INVALID_SOCKET) closesocket(gDataSocket);
	if (gServerSocket != INVALID_SOCKET) closesocket(gServerSocket);
	if (gWSInitialized) WSACleanup();

	gDataSocket = INVALID_SOCKET;
	gServerSocket = INVALID_SOCKET;
	hPortFile = INVALID_HANDLE_VALUE;
	gWSInitialized = false;

}

// Client Functions
int try_connect_to_server (const char *gamedat_path)
{
	if (!InitializeWinsock()) return false;

	// Create a socket.
	gDataSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( gDataSocket == INVALID_SOCKET ) {
		cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
		close_pipe();
		return false;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	service.sin_port = 0;

	if (!OpenPortFile(gamedat_path,false))
	{
		cerr << "Error opening temporary file in gamedat dir with port number." << endl;
		close_pipe();
		return false;
	}

	DWORD numRead;
	ReadFile(hPortFile,&service.sin_port,2,&numRead,NULL);
	CloseHandle(hPortFile);
	hPortFile = INVALID_HANDLE_VALUE;
	if (numRead != 2) {
		cerr << "Error read temporary file in gamedat dir with port number." << endl;
		close_pipe();
		return false;
	}

	if ( connect( gDataSocket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR) {
		cerr << "Failed to connect." << std::endl;
		close_pipe();
		return false;
	}

	// Is exult studio opening socket and connecting to Exult
	return 1;
}

void disconnect_from_server()
{
	// Does cleanup after server has gone
	close_pipe();
}

// General stuff
int peek_pipe()
{
	// returns size of data waiting, -1 if disconnected
	if (gDataSocket == INVALID_SOCKET) return -1;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(gDataSocket, &rfds);
	TIMEVAL tv;
	tv.tv_sec = 0;
	tv.tv_usec = 20000;
	if (select(0,&rfds,0,0,&tv))
	{
		char c;
		int to_get = recv(gDataSocket,&c,1,MSG_PEEK);
		if (to_get == 0 || to_get == SOCKET_ERROR) 
		{
			closesocket(gDataSocket);
			gDataSocket = INVALID_SOCKET;
			return -1;
		}
		return to_get;
	}
	return 0;
}

bool is_broken() 
{
	if (gDataSocket == INVALID_SOCKET) return true;
	peek_pipe();
	return (gDataSocket == INVALID_SOCKET);
}

bool notify_connection_lost()
{
	return true;
}

bool is_win9x() 
{
	return false;
}

}

#endif
