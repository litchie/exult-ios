/**
 **	Server.cc - Server functions for Exult (NOT ExultStudio).
 **
 **	Written: 5/2/2001 - JSF
 **/

/*
Copyright (C) 2000-2002 The Exult Team

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
#  include <config.h>	// All the ifdefs aren't useful if we don't do this
#endif

// only if compiled with "exult studio support"
#ifdef USE_EXULTSTUDIO

#include <unistd.h>
#include <fcntl.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <cstdio>

#ifdef _AIX
#include <strings.h>
#endif

#if HAVE_NETDB_H
#include <netdb.h>
#endif

#ifndef WIN32
#include <sys/un.h>
#endif

#include "server.h"
#include "servemsg.h"
#include "utils.h"
#include "egg.h"
#include "actors.h"
#include "gamewin.h"
#include "gamemap.h"
#include "chunkter.h"
#include "cheat.h"
#include "objserial.h"

#ifdef USECODE_DEBUGGER
#include "debugserver.h"
#endif

#ifdef WIN32
#include "servewin32.h"
#include "cheat.h"
#endif

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

#ifdef __sun__
// Solaris doesn't know PF_LOCAL
#define PF_LOCAL PF_UNIX
#endif

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
#ifndef WIN32
	// Get location of socket file.
	std::string servename("");
	char *home = getenv("HOME");
	if (home)			// Use $HOME/.exult/exultserver
		{			//   if possible.
		servename = home;
		servename += "/.exult";
		if (U7exists(servename.c_str()))
			servename += "/exultserver";
		else
			servename = "";
		}
	if (servename == "")
		servename = get_system_path("<GAMEDAT>/exultserver");
	// Make sure it isn't there.
	unlink(servename.c_str());
#if HAVE_GETADDRINFOX
	// Don't use the old deprecated network API
	int r;
	struct addrinfo hints,*ai;

	memset(&hints,0,sizeof(hints));
	hints.ai_flags=AI_PASSIVE;
	r=getaddrinfo(	0,
			servename.c_str(),
			&hints,
			&ai);
	if(r!=0)
		{
		cerr << "getaddrinfo(): "<<gai_strerror(r) << endl;
		return;
		}

	listen_socket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
#else
	// Deprecated
	listen_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
#endif
	if (listen_socket < 0)
		perror("Failed to open map-editor socket");
	else 
		{
#if HAVE_GETADDRINFOX
		if(bind(listen_socket,ai->ai_addr,ai->ai_addrlen) == -1 ||
			listen(listen_socket,1) == -1)
#else
		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, servename.c_str());
		if (bind(listen_socket, (struct sockaddr *) &addr, 
		      sizeof(addr.sun_family) + strlen(addr.sun_path)) == -1 ||
		    listen(listen_socket, 1) == -1)
#endif
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
#if HAVE_GETADDRINFOX
		freeaddrinfo(ai);
#endif
		}
	Set_highest_fd();
#else

	listen_socket = client_socket = -1;

	std::string servename = get_system_path("<STATIC>/");

	if (Exult_server::create_pipe(servename.c_str())) listen_socket = 1;

#endif
	}

/*
 *	Close the server.
 */

void Server_close
	(
	)
	{
#ifdef WIN32
	Exult_server::close_pipe();
	listen_socket = client_socket = -1;
#else
	// unlink socket file+++++++
#endif
	}


/*
 *	A message from a client is available, so handle it.
 */

static void Handle_client_message
	(
	int& fd				// Socket to client.  May be closed.
	)
	{
	unsigned char data[Exult_server::maxlength];
	Exult_server::Msg_type id;
	int datalen = Exult_server::Receive_data(fd, id, data, sizeof(data));
	if (datalen < 0)
		return;
	unsigned char *ptr = &data[0];
	Game_window *gwin = Game_window::get_game_window();
	switch (id)
		{
	case Exult_server::obj:
		Game_object::update_from_studio(&data[0], datalen);
		break;
	case Exult_server::egg:
		Egg_object::update_from_studio(&data[0], datalen);
		break;
	case Exult_server::npc:
		Actor::update_from_studio(&data[0], datalen);
		break;
	case Exult_server::info:
		{
		unsigned char data[Exult_server::maxlength];
		unsigned char *ptr = &data[0];
		Game_info_out(client_socket, gwin->get_num_npcs(),
			cheat.get_edit_lift(),
			gwin->skip_lift,
			cheat.in_map_editor(),
			cheat.show_tile_grid(),
			gwin->get_map()->was_map_modified());
		break;
		}
	case Exult_server::write_map:
		gwin->write_map();	// Send feedback?+++++
		break;
	case Exult_server::read_map:
		gwin->read_map();
		break;
	case Exult_server::map_editing_mode:
		{
		int onoff = Read2(ptr);
		if ((onoff != 0) != cheat.in_map_editor())
			cheat.toggle_map_editor();
		break;
		}
	case Exult_server::tile_grid:
		{
		int onoff = Read2(ptr);
		if ((onoff != 0) != cheat.show_tile_grid())
			cheat.toggle_tile_grid();
		break;
		}
	case Exult_server::edit_lift:
		{
		int lift = Read2(ptr);
		cheat.set_edit_lift(lift);
		break;
		}
	case Exult_server::reload_usecode:
		gwin->reload_usecode();
		break;
	case Exult_server::locate_terrain:
		{
		int tnum = Read2(ptr);
		int cx = (short) Read2(ptr);
		int cy = (short) Read2(ptr);
		bool up = *ptr++ ? true : false;
		bool okay = gwin->get_map()->locate_terrain(tnum, cx, cy, up);
		ptr = &data[2];		// Set back reply.
		Write2(ptr, cx);
		Write2(ptr, cy);
		ptr++;			// Skip 'up' flag.
		*ptr++ = okay ? 1 : 0;
		Exult_server::Send_data(client_socket, Exult_server::locate_terrain, data,
							ptr - data);
		break;
		}
	case Exult_server::swap_terrain:
		{
		int tnum = Read2(ptr);
		bool okay = gwin->get_map()->swap_terrains(tnum);
		*ptr++ = okay ? 1 : 0;
		Exult_server::Send_data(client_socket, Exult_server::swap_terrain, data,
							ptr - data);
		break;
		}
	case Exult_server::insert_terrain:
		{
		int tnum = (short) Read2(ptr);
		bool dup = *ptr++ ? true : false;
		bool okay = gwin->get_map()->insert_terrain(tnum, dup);
		*ptr++ = okay ? 1 : 0;
		Exult_server::Send_data(client_socket, 
			Exult_server::insert_terrain, data, ptr - data);
		break;
		}
	case Exult_server::send_terrain:
		{			// Send back #, total, 512-bytes data.
		int tnum = (short) Read2(ptr);
		Write2(ptr, gwin->get_map()->get_num_chunk_terrains());
		Chunk_terrain *ter = gwin->get_map()->get_terrain(tnum);
		ter->write_flats(ptr);	// Serialize it.
		ptr += 512;		// I just happen to know the length...
		Exult_server::Send_data(client_socket, 
			Exult_server::send_terrain, data, ptr - data);
		break;
		}
	case Exult_server::terrain_editing_mode:
		{			// 1=on, 0=off, -1=undo.
		int onoff = (short) Read2(ptr);
					// skip_lift==0 <==> terrain-editing.
		gwin->skip_lift = onoff == 1 ? 0 : 16;
		static char *msgs[3] = {"Terrain-Editing Aborted",
					"Terrain-Editing Done",
					"Terrain-Editing Enabled"};
		if (onoff == 0)		// End/commit.
			gwin->get_map()->commit_terrain_edits();
		else if (onoff == -1)
			gwin->get_map()->abort_terrain_edits();
		if (onoff >= -1 && onoff <= 1)
			gwin->center_text(msgs[onoff + 1]);
		gwin->set_all_dirty();
		break;
		}
	case Exult_server::set_edit_shape:
		{
		int shnum = (short) Read2(ptr);
		int frnum = (short) Read2(ptr);
		cheat.set_edit_shape(shnum, frnum);
		break;
		}
	case Exult_server::view_pos:
		{
		int tx = Read4(ptr);
		if (tx == -1)		// This is a query?
			{
			gwin->send_location();
			break;
			}
		int ty = Read4(ptr);
		// +++Later int txs = Read4(ptr);
		// int tys = Read4(ptr);
		// int scale = Read4(ptr);
					// Only set if chunk changed.
		if (tx/c_tiles_per_chunk != 
			gwin->get_scrolltx()/c_tiles_per_chunk || 
		    ty/c_tiles_per_chunk != 
			gwin->get_scrollty()/c_tiles_per_chunk)
			{
			gwin->set_scrolls(tx, ty);
			gwin->set_all_dirty();
			}
		break;
		}
	case Exult_server::set_edit_mode:
		{
		int md = Read2(ptr);
		if (md >= 0 && md <= 4)
			cheat.set_edit_mode((Cheat::Map_editor_mode) md);
		break;
		}
	case Exult_server::hide_lift:
		{
		int lift = Read2(ptr);
		gwin->skip_lift = lift;
		gwin->set_all_dirty();
		break;
		}
	case Exult_server::reload_shapes:
		gwin->reload_shapes(Read2(ptr));
		break;
	case Exult_server::unused_shapes:
		{			// Send back shapes not used in game.
		unsigned char data[Exult_server::maxlength];
		int sz = 1024/8;	// Gets bits for unused shapes.
		sz = sz > sizeof(data) ? sizeof(data) : sz;
		gwin->get_map()->find_unused_shapes(data, sz);
		Exult_server::Send_data(client_socket, 
				Exult_server::unused_shapes, data, sz);
		break;
		}
#ifdef USECODE_DEBUGGER
	case Exult_server::usecode_debugging:
		Handle_debug_message(&data[0], datalen);
		break;
#endif
		}
	}

/*
 *	Delay for a fraction of a second, or until there's data available.
 *	If a server request comes, it's handled here.
 */

void Server_delay
	(
	 Message_handler handle_message
	)
	{
#ifndef WIN32
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
			{
			handle_message(client_socket);
					// Client gone?
			if (client_socket == -1)
				Set_highest_fd();
			}
		}
#else
	if (listen_socket == -1) return;

	if (Exult_server::is_broken()) {
		std::cout << "Client disconnected." << endl;
		Exult_server::disconnect_from_client();
		Exult_server::setup_connect();
		client_socket = -1;
	}

	SleepEx(20, TRUE);

	if (client_socket == -1) {
		// If 9x, only do this in map edit mode
		if (Exult_server::is_win9x() && !cheat.in_map_editor()) return;

		std::string servename = get_system_path("<STATIC>/");
		if (!Exult_server::try_connect_to_client(servename.c_str())) return;
		else client_socket = 1;
		std::cout << "Connected to client" << endl;
	}

	if (Exult_server::peek_pipe() > 0)
		handle_message(client_socket);
		
	if (Exult_server::is_broken()) {
		if (Exult_server::notify_connection_lost()) std::cout << "Client disconnected." << endl;
		Exult_server::disconnect_from_client();
		Exult_server::setup_connect();
		client_socket = -1;
	}
#endif

	}

void Server_delay()
{
	Server_delay(Handle_client_message);
}

#endif
