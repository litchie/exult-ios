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
static HANDLE hPipeRead = INVALID_HANDLE_VALUE;
static HANDLE hPipeWrite = INVALID_HANDLE_VALUE;

static OVERLAPPED oOverlap;			// Overlap structure
static LPOVERLAPPED lpOverlap = 0;		// Pointer to overlap structure
static OVERLAPPED oOverlapCon;			// Overlap structure
static LPOVERLAPPED lpOverlapCon = 0;		// Pointer to overlap structure
static BOOL bWinNT = FALSE;
static BOOL bClient = FALSE;
static BOOL bBroken = FALSE;
static BOOL bNotifyDis = FALSE;

static const char * const pipe_name = "\\\\.\\pipe\\exultserver-";
static const char * const mail_server = "\\\\.\\mailslot\\exultserver-";
static const char * const mail_client = "\\\\.\\mailslot\\exultclient-";

// static Functions

static char *get_pipe_name(const char *static_path, const char * const pipe_name = Exult_server::pipe_name, bool ms = false)
{
	// Check to make sure that the pipe name hasn't been passed to us
	if (!std::strncmp(static_path, "\\\\.\\pipe\\", std::strlen("\\\\.\\pipe\\"))) {
		char *path = new char[std::strlen(static_path)+1];
		return std::strcpy(path, static_path);
	}

	// Check to make sure that the mailslot name hasn't been passed to us
	if (!std::strncmp(static_path, "\\\\.\\mailslot\\", std::strlen("\\\\.\\mailslot\\"))) {
		char *path = new char[std::strlen(static_path)+1];
		return std::strcpy(path, static_path);
	}

	int num_chars = GetLongPathName(static_path, NULL, 0);
	int head_len = std::strlen(pipe_name);
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

	std::memcpy (actual_path, pipe_name,head_len);

	if (!ms){
		for (char *temp = actual_path+head_len; *temp; temp++)
			if (*temp == '\\') *temp = '/';
	}
	else {
		for (char *temp = actual_path+head_len; *temp; temp++)
			if (*temp == '/') *temp = '\\';
			else if (*temp == ':') *temp = '_';
	}

	if (std::strlen(actual_path) > 255)actual_path[255] = 0;

	while (actual_path[std::strlen(actual_path)-1] == '/') actual_path[std::strlen(actual_path)-1] = 0;
	while (actual_path[std::strlen(actual_path)-1] == '\\') actual_path[std::strlen(actual_path)-1] = 0;

	return actual_path;
}

static void detect_winnt()
{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof (info);
	GetVersionEx (&info);

	// Platform is NT
	if (info.dwPlatformId == 2) bWinNT = TRUE;
}


static bool setup_overlap()
{
	memset(&oOverlap,0, sizeof (oOverlap));
	memset(&oOverlapCon,0, sizeof (oOverlapCon));

	// Platform is NT
	if (bWinNT)
	{
		// Create the overlap event
		oOverlap.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL);
		lpOverlap = &oOverlap;
		oOverlapCon.hEvent = CreateEvent (NULL, TRUE, TRUE, NULL);
		lpOverlapCon = &oOverlapCon;

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

// This sets up mailslots for the interprocess coms on Win9x
static bool setup_server_mailslots(const char *static_path, bool both)
{
	// Create read frst
	if (hPipeRead == INVALID_HANDLE_VALUE)
	{
		char *server_name = Exult_server::get_pipe_name(static_path,mail_server, true);
		hPipeRead = CreateMailslot(server_name, 1024, 0, NULL);

		if (hPipeRead == INVALID_HANDLE_VALUE) {
			std::cout << "Unable to create mailslot : " << server_name << " Code " << GetLastError() << std::endl;
			delete [] server_name ;
			return false;
		}

		
		//cout << "Created mailslot. Name " << server_name << std::endl;
		delete [] server_name ;
		if (!both) return true;
	}

	// Now attempt to create read
	if (hPipeWrite == INVALID_HANDLE_VALUE)
	{
		char *client_name = Exult_server::get_pipe_name(static_path,mail_client, true);

		hPipeWrite = CreateFile (client_name,		// Pipe Name
				GENERIC_WRITE,			// Desired Access
				FILE_SHARE_READ,		// Share Mode
				NULL,				// Security Att
				OPEN_EXISTING,			// Creation Disposition
				FILE_ATTRIBUTE_NORMAL,		// Flags and Attributes
				NULL);				// TemplateFile

		if (hPipeWrite == INVALID_HANDLE_VALUE) {
			//std::cout << "Unable to connect to mailslot: " << client_name <<" Code " << GetLastError() << std::endl;
			delete [] client_name;
			return false;
		}
		//std::cout << "Connected to mailslot. Name " << client_name << std::endl;
		delete [] client_name ;
	}

	return true;
}

// This sets up mailslots for the interprocess coms on Win9x
static bool setup_client_mailslots(const char *static_path)
{
	// Create write frst
	if (hPipeWrite == INVALID_HANDLE_VALUE)
	{
		char *server_name = Exult_server::get_pipe_name(static_path,mail_server, true);

		hPipeWrite = CreateFile (server_name,		// Pipe Name
				GENERIC_WRITE,			// Desired Access
				FILE_SHARE_READ,		// Share Mode
				NULL,				// Security Att
				OPEN_EXISTING,			// Creation Disposition
				FILE_ATTRIBUTE_NORMAL,		// Flags and Attributes
				NULL);				// TemplateFile

		if (hPipeWrite == INVALID_HANDLE_VALUE) {
			//std::cout << "Unable to connect to mailslot: " << server_name <<" Code " << GetLastError() << std::endl;
			delete [] server_name;
			return false;
		}
		//std::cout << "Connected to mailslot. Name " << server_name << std::endl;
		delete [] server_name ;
	}

	// Now attempt to create read
	if (hPipeRead == INVALID_HANDLE_VALUE)
	{
		char *client_name = Exult_server::get_pipe_name(static_path,mail_client, true);
		hPipeRead = CreateMailslot(client_name, 1024, 0, NULL);

		if (hPipeRead == INVALID_HANDLE_VALUE) {
			std::cout << "Unable to create mailslot : " << client_name << " Code " << GetLastError() << std::endl;
			delete [] client_name ;
			CloseHandle(hPipeWrite);
			hPipeWrite = INVALID_HANDLE_VALUE;
			return false;
		}

		//std::cout << "Created mailslot. Name " << client_name << std::endl;
		delete [] client_name ;
	}

	return true;
}

// Hack functions
int write(int file, const void *v, unsigned int len)
{
	DWORD num_written;

	BOOL failed = !WriteFile(hPipeWrite, v, len, &num_written, lpOverlap);

	const unsigned char *data = (unsigned char *)v;
	//cout << "write = " << len << endl;
	//for (int q = 0 ; q < len; q++) cout << ((int) data [q]) << " ";
	//cout << endl;

	if ((!bWinNT && failed) || GetLastError() == ERROR_BROKEN_PIPE) {
		std::cout << "Broken!" << std::endl;
		bBroken = TRUE;
		return 0;
	}

	if (num_written == 0) return -1;

	return num_written;
}

static unsigned char buf[2048];
static unsigned int offset = 2048;
static unsigned int num_left = 0;

int read_win9x(int file, unsigned char *v, unsigned int len)
{
	DWORD num_read;

	//std::cout << "The Num wanted is " << len << std::endl;

	// Buffer is empty, fill it
	if (num_left == 0)
	{
		int total = peek_pipe();

		if (total >= 2048) {
			std::cerr << "Buffer too small. Read failed" << std::endl;
			return 0;
		}
		else if (total == 0) {
			return 0;
		}
		else if (total == -1) {
			return -1;
		}

		ReadFile(hPipeRead, buf, total, &num_read, NULL);
		offset = 0;
		num_left = total;

		//std::cout << "Read " << num_read << " bytes from mailslot" << std::endl;
	}

	// Now copy the amount of bytes, if we can, if we can't call the func recursively
	num_read = 0;

	// Buffer contains enough
	if (num_left >= len) {
		std::memcpy (v, buf+offset, len);
		num_left -= len;
		num_read  = len;
		offset += len;
	} // Uh oh, the buffer doesn't contain enough
	else {
		std::memcpy (v, buf+offset, num_left);
		len -= num_left;
		num_read += num_left;
		offset += num_left;
		num_left = 0;

		num_left += read_win9x(file, v+num_read, len);
	}

	//std::cout << "The Num read is " << num_read << std::endl;
	//std::cout << num_left << " byte left in buffer with offset " << offset<< std::endl;

	bNotifyDis = TRUE;
	return num_read;
}

int read(int file, void *v, unsigned int len)
{
	if (!bWinNT) return read_win9x(file, (unsigned char*) v, len);

	DWORD num_read;
	int total = peek_pipe();

	if (total == -1) return 0;
	else if (len == 0) return 0;
	else if (total == 0) return -1;
	else if (total < len) len = total;

	ReadFile(hPipeRead, v, len, &num_read, NULL);

	const unsigned char *data = (unsigned char *)v;
	//cout << "read = " << num_read << endl;
	//for (int q = 0 ; q < num_read; q++) cout << ((int) data[q]) << " ";
	//cout << endl;

	if (GetLastError() == ERROR_BROKEN_PIPE) {
		bBroken = TRUE;
		return 0;
	}

	return num_read;
}

int close(int file)
{	
	return 0;
}

// Server Functions

bool create_pipe (const char *static_path)
{
	bBroken = FALSE;

	detect_winnt();

	if (!bWinNT)
	{
		std::cout << "Using mailslots" << std::endl;
		hPipeRead = hPipeWrite= INVALID_HANDLE_VALUE;	
		return setup_server_mailslots(static_path, false);
	}

	char *pipe_name = Exult_server::get_pipe_name(static_path);

	// Must have overlap for now
	DWORD oflag = FILE_FLAG_OVERLAPPED;
	if(!setup_overlap()) {
		cerr << "Pipe Server not supported on Win9x." << endl;
		return true;
	}

	//cout << "Exult Server pipe name: " << pipe_name << endl;

	// Create the Win32 Named pipe
	hPipeRead = hPipeWrite = CreateNamedPipe(pipe_name,		// pipe name
				PIPE_ACCESS_DUPLEX|oflag,		// pipe open mode
				PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,	// pipe-specific modes
				1,					// maximum number of instances
				32768,					// output buffer size
				32768,					// input buffer size
				0,					// time-out interval
				NULL);					// Security Descriptor

	delete [] pipe_name;

	if (hPipeWrite  == INVALID_HANDLE_VALUE) {
		cerr << "Unable to create pipe. Reason: " << GetLastError() << endl;
		free_overlap();
		return false;
	}

	cout << "Created Pipe for Exult Server" << endl;

	setup_connect();

	return true;
}

void setup_connect()
{
	bBroken = FALSE;

	if (!bWinNT) return;

	// Attempt to connect to the pipe
        BOOL fConnected = ConnectNamedPipe(hPipeWrite, lpOverlapCon) ? 
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

	// If it's connected, signal the event
	if (fConnected) SetEvent(oOverlapCon.hEvent);
}

bool try_connect_to_client(const char *static_path)
{
	bBroken = FALSE;

	if (bWinNT)
	{
		DWORD num_bytes;

		BOOL fConnected = GetOverlappedResult (hPipeWrite, lpOverlapCon, &num_bytes, FALSE);
			
		if (!fConnected) return false;
	}
	else {
		return setup_server_mailslots(static_path, true);
	}

	return true;
}

void disconnect_from_client()
{
	bBroken = FALSE;
	if (bWinNT) {
		FlushFileBuffers (hPipeWrite);
		DisconnectNamedPipe(hPipeWrite);
	}
	else {
		if (hPipeWrite != INVALID_HANDLE_VALUE) {
			FlushFileBuffers (hPipeWrite);
			CloseHandle(hPipeWrite);
		}
		if (hPipeRead != INVALID_HANDLE_VALUE) {
			CloseHandle(hPipeRead);
		}
		hPipeRead = hPipeWrite= INVALID_HANDLE_VALUE;
	}
}

void close_pipe()
{
	bBroken = FALSE;
	if (bWinNT) {
		if (hPipeWrite != INVALID_HANDLE_VALUE) {
			disconnect_from_client();
			free_overlap();
			CloseHandle(hPipeWrite);
			hPipeRead = hPipeWrite = INVALID_HANDLE_VALUE;
		}
	}
	else {
		disconnect_from_client();
	}
}

// Client Functions
int try_connect_to_server (const char *static_path)
{
	bBroken = FALSE;
	detect_winnt();

	if (!bWinNT)
	{
		hPipeRead = hPipeWrite= INVALID_HANDLE_VALUE;	
		return setup_client_mailslots(static_path);
	}

	char *pipe_name = Exult_server::get_pipe_name(static_path);

	// No server
	if (!WaitNamedPipe(pipe_name, 0)) return 0;

	// Must have overlap
	DWORD oflag = FILE_FLAG_OVERLAPPED;
	if(!setup_overlap()) {
		cerr << "Pipe Client not supported on Win9x." << endl;
		return -1;
	}

	//cout << "Attempting to connect to server: " << pipe_name << endl;

	hPipeRead = hPipeWrite = CreateFile (pipe_name,		// Pipe Name
				GENERIC_READ|GENERIC_WRITE,	// Desired Access
				0,				// Share Mode
				NULL,				// Security Att
				OPEN_EXISTING,			// Creation Disposition
				oflag,				// Flags and Attributes
				NULL);				// TemplateFile


	if (hPipeWrite == INVALID_HANDLE_VALUE)
	{
		ERROR_ALREADY_EXISTS;
		cout << "Pipe handle was invalid! : " << GetLastError () << endl;
		free_overlap();
		return 0;
	}

	bClient = TRUE;

	return 1;
}

void disconnect_from_server()
{
	//CreateMailslot
	bBroken = FALSE;
	if (bWinNT) {
		if (hPipeWrite != INVALID_HANDLE_VALUE) {
			FlushFileBuffers (hPipeWrite);
			free_overlap();
			CloseHandle(hPipeWrite);
			hPipeRead = hPipeWrite= INVALID_HANDLE_VALUE;
		}
	}
	else
	{
		if (hPipeWrite != INVALID_HANDLE_VALUE) {
			FlushFileBuffers (hPipeWrite);
			CloseHandle(hPipeWrite);
		}
		if (hPipeRead != INVALID_HANDLE_VALUE) {
			CloseHandle(hPipeRead);
		}
		hPipeRead = hPipeWrite= INVALID_HANDLE_VALUE;
	}
}

#define WIN9X_KEEPALIVE_SIZE 1
int peek_pipe()
{
	if (bBroken) return -1;

	DWORD to_get;

	if (bWinNT) {
		PeekNamedPipe(hPipeRead, 0, 0, 0, &to_get, 0);

		if (GetLastError() == ERROR_BROKEN_PIPE) {
			std::cout << "Broken!" << std::endl;
			bBroken = TRUE;
			return -1;
		}

	}
	else {
		DWORD num_read;
		static DWORD last_time = 0;

		// This is a keep alive. It doesn't send anything,
		// But, if the mailslot closes, this will fail.
		// That is the theory, but win9x doesn't quite
		// work that way
#ifdef USE_WIN9X_KEEPALIVE
		if (last_time < GetTickCount()) {
			last_time = GetTickCount() + 5000;
			char buf[WIN9X_KEEPALIVE_SIZE];
			if (!WriteFile(hPipeWrite, buf, WIN9X_KEEPALIVE_SIZE, &num_read, 0)) {
				std::cout << "Broken!" << std::endl;
				bBroken = TRUE;
				return -1;
			}
		}
#endif

		GetMailslotInfo(hPipeRead, NULL, &to_get, NULL, NULL);
		
#ifdef USE_WIN9X_KEEPALIVE
		while (to_get <= WIN9X_KEEPALIVE_SIZE) {
			char buf[WIN9X_KEEPALIVE_SIZE];
			ReadFile(hPipeRead, buf, to_get, &num_read, 0);
			GetMailslotInfo(hPipeRead, NULL, &to_get, NULL, NULL);
		}
#endif
		

		if (to_get == MAILSLOT_NO_MESSAGE) return 0;
	}

	return to_get;
}

bool is_broken() 
{
	return bBroken != FALSE;
}

bool notify_connection_lost()
{
	if (bNotifyDis == TRUE) {
		bNotifyDis = FALSE;
		return true;
	}
	return bWinNT != FALSE;
}

bool is_win9x() 
{
	return bWinNT == FALSE;
}

}

#endif
