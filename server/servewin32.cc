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

namespace Exult_server
{
static HANDLE hPipe = INVALID_HANDLE_VALUE;
static OVERLAPPED oOverlap;			// Overlap structure
static LPOVERLAPPED lpOverlap = 0;		// Pointer to overlap structure
static OVERLAPPED oOverlapCon;			// Overlap structure
static LPOVERLAPPED lpOverlapCon = 0;		// Pointer to overlap structure
static BOOL bWinNT = FALSE;
static BOOL bClient = FALSE;

static const char * const pipe_name = "\\\\.\\pipe\\exultserver-";

// static Functions

static char *get_pipe_name(const char *static_path)
{
	// Check to make sure that the pipe name hasn't been passed to us
	if (!std::strncmp(static_path, "\\\\.\\pipe\\", std::strlen("\\\\.\\pipe\\"))) {
		char *path = new char[std::strlen(static_path)+1];
		return std::strcpy(path, static_path);
	}

	int num_chars = GetLongPathName(static_path, NULL, 0);
	int head_len = std::strlen(Exult_server::pipe_name);
	char *actual_path;

	if (num_chars) {
		char *temp = new char[num_chars+1];
		GetLongPathName(static_path, temp, num_chars+1);
		num_chars = GetFullPathName(temp, 0, NULL, NULL);
		actual_path = new char[num_chars+head_len+1];
		char *temp2;
		GetFullPathName(temp, num_chars+1, actual_path+head_len, &temp2);
	}
	else {
		cout << "Unable to find actual static path" << endl;
		return 0;
	}

	std::memcpy (actual_path, Exult_server::pipe_name,head_len);

	for (char *temp = actual_path+head_len; *temp; temp++)
		if (*temp == '\\') *temp = '/';
//		if ((*temp == '\\') || (*temp == ':') || (*temp == '/')) *temp = '_';

	if (std::strlen(actual_path) > 255)actual_path[255] = 0;

	while (actual_path[std::strlen(actual_path)-1] == '/') actual_path[std::strlen(actual_path)-1] = 0;

	return actual_path;
}

static bool setup_overlap()
{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof (info);
	GetVersionEx (&info);

	memset(&oOverlap,0, sizeof (oOverlap));
	memset(&oOverlapCon,0, sizeof (oOverlapCon));

	// Platform is NT
	if (info.dwPlatformId == 2)
	{
		// Create the overlap event
		oOverlap.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL);
		lpOverlap = &oOverlap;
		oOverlapCon.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL);
		lpOverlapCon = &oOverlapCon;

		bWinNT = TRUE;

		return true;
	}

	return false;
}

static void free_overlap()
{
	if (oOverlap.hEvent) CloseHandle(oOverlap.hEvent);
	oOverlap.hEvent = 0;
	lpOverlap = 0;

	if (oOverlapCon.hEvent) CloseHandle(oOverlapCon.hEvent);
	oOverlapCon.hEvent = 0;
	lpOverlapCon = 0;

}

// Hack functions
int write(int file, const void *v, unsigned int len)
{
	DWORD num_written;

	WriteFile(hPipe, v, len, &num_written, lpOverlap);

	const unsigned char *data = (unsigned char *)v;
	//cout << "write = " << len << endl;
	//for (int q = 0 ; q < len; q++) cout << ((int) data [q]) << " ";
	//cout << endl;


	if (GetLastError() == ERROR_BROKEN_PIPE) return 0;

	if (num_written == 0) return -1;

	return num_written;
}

int read(int file, void *v, unsigned int len)
{
	DWORD num_read;
	int total = peek_pipe();

	if (total == -1) return 0;
	else if (len == 0) return 0;
	else if (total == 0) return -1;
	else if (total < len) len = total;

	ReadFile(hPipe, v, len, &num_read, NULL);

	const unsigned char *data = (unsigned char *)v;
	//cout << "read = " << num_read << endl;
	//for (int q = 0 ; q < num_read; q++) cout << ((int) data[q]) << " ";
	//cout << endl;

	if (GetLastError() == ERROR_BROKEN_PIPE) return 0;

	return num_read;
}

int close(int file)
{	
	return 0;
}

// Server Functions

bool create_pipe (const char *static_path)
{
	char *pipe_name = Exult_server::get_pipe_name(static_path);

	// Must have overlap for now
	if(!setup_overlap()) {
		cerr << "Server not supported on Win9x." << endl;
		return false;
	}

	//cout << "Exult Server pipe name: " << pipe_name << endl;

	// Create the Win32 Named pipe
	hPipe = CreateNamedPipe(pipe_name,				// pipe name
				PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,// pipe open mode
				PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,	// pipe-specific modes
				1,					// maximum number of instances
				32768,					// output buffer size
				32768,					// input buffer size
				0,					// time-out interval
				NULL);					// Security Descriptor

	delete [] pipe_name;

	if (hPipe == INVALID_HANDLE_VALUE) {
		cerr << "Unable to create pipe. Reason: " << GetLastError() << endl;
		return false;
	}

	cout << "Created Pipe for Exult Server" << endl;

	setup_connect();

	return true;
}

void setup_connect()
{
	// Attempt to connect to the pipe
        BOOL fConnected = ConnectNamedPipe(hPipe, lpOverlapCon) ? 
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

	// If it's connected, signal the event
	if (fConnected) SetEvent(oOverlapCon.hEvent);
}

bool try_connect_to_client()
{
	DWORD num_bytes;
	BOOL fConnected = GetOverlappedResult (hPipe, lpOverlapCon, &num_bytes, FALSE);
			
	if (!fConnected) return false;

	return true;
}

void disconnect_from_client()
{
	FlushFileBuffers (hPipe);
	DisconnectNamedPipe(hPipe);
}

void close_pipe()
{
	if (hPipe != INVALID_HANDLE_VALUE) {
		disconnect_from_client();
		free_overlap();
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

// Client Functions
int try_connect_to_server (const char *static_path)
{
	char *pipe_name = Exult_server::get_pipe_name(static_path);

	// Must have overlap
	if(!setup_overlap()) {
		cerr << "Client not supported on Win9x." << endl;
		return -1;
	}

	// No server
	if (!WaitNamedPipe(pipe_name, 0)) return 0;

	//cout << "Attempting to connect to server: " << pipe_name << endl;

	hPipe = CreateFile (pipe_name,				// Pipe Name
				GENERIC_READ|GENERIC_WRITE,	// Desired Access
				0,				// Share Mode
				NULL,				// Security Att
				OPEN_EXISTING,			// Creation Disposition
				FILE_FLAG_OVERLAPPED,		// Flags and Attributes
				NULL);				// TemplateFile


	if (hPipe == INVALID_HANDLE_VALUE)
	{
		ERROR_ALREADY_EXISTS;
		cout << "Pipe handle was invalid! : " << GetLastError () << endl;
		return 0;
	}

	bClient = TRUE;

	return 1;
}

void disconnect_from_server()
{
	if (hPipe != INVALID_HANDLE_VALUE) {
		FlushFileBuffers (hPipe);
		free_overlap();
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

int peek_pipe()
{
	DWORD to_get;
	PeekNamedPipe(hPipe, 0, 0, 0, &to_get, 0);

	if (GetLastError() == ERROR_BROKEN_PIPE)
		return -1;

	//if (to_get > 0) cout << "Exult_server::peek_pipe() " << to_get << endl;

	return to_get;
}


}

#endif
