/*
 *  Copyright (C) 2001,2005  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"
#include "forked_player.h"

#ifdef USE_FORKED_PLAYER_MIDI

#ifndef ALPHA_LINUX_CXX
#  include <csignal>
#endif
#include <unistd.h>

#include "Configuration.h"
#include "exult.h"
#include "fnames.h"
#include <iostream>

using std::cerr;
using std::endl;

const MidiDriver::MidiDriverDesc forked_player::desc = 
		MidiDriver::MidiDriverDesc ("Forked", createInstance);

// NB: This function doesn't return unless execlp fails!
static  void    playFJmidifile(const char *name)
{
	execlp("playmidi","playmidi","-v","-v","-e",name,0);
}

int forked_player::open()
{
	forked_job = -1;
	return 0;
}

void forked_player::close()
{
	stop_track();
}

void	forked_player::stop_track(void)
{
	if(forked_job!=-1)
		kill(forked_job,SIGKILL);
	forked_job=-1;
}

bool	forked_player::is_playing(void)
{
	if(forked_job==-1)
		return false;
	if(kill(forked_job,0))
		{
		forked_job=-1;
		return false;
		}
	return true;
}


void	forked_player::start_track(const char *filename,bool repeat,int vol)
{
	const char *name = forked_player::get_temp_name();

	repeat_=repeat;
#if DEBUG
	cerr << "Starting midi sequence with forked_player" << endl;
#endif
	if(forked_job!=-1)
	{
#if DEBUG
		cerr << "Stopping any running track" << endl;
#endif
		stop_track();
	}

	forked_job=fork();
	if(!forked_job)
	{
		playFJmidifile(name); // this doesn't return if it started correctly
		cerr << "Starting forked player failed" << endl;
		exit(-1);
	}
}

const char	*forked_player::get_temp_name()
{
	static char *name = 0;

	if (name == 0) {
		name = new char[19];
		strcpy(name, "/tmp/u7midi_XXXXXX");
		::close(mkstemp(name));
		// TODO: delete this file on exit
	}

	return name;
}


#endif // USE_FORKED_PLAYER_MIDI
