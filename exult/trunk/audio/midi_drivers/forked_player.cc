/*
Copyright (C) 2000  Dancer A.L Vesperman

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


#if __GNUG__ >= 2
#  pragma implementation
#endif


#include "forked_player.h"

#include <unistd.h>
#include <csignal>
#include "../../fnames.h"

#include "Configuration.h"
extern	Configuration	config;

static  void    playFJmidifile(const char *name)
{
	execlp("playmidi","-v","-v","-e",name,0);
}

forked_player::forked_player() : forked_job(-1)
{}

void	forked_player::stop_track(void)
{
	if(forked_job!=-1)
		kill(forked_job,SIGKILL);
	forked_job=-1;
}

forked_player::~forked_player(void)
{
	stop_track();
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


void	forked_player::start_track(const char *name,int numrepeats)
{
	repeats=numrepeats;
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
		for(int i=0;repeats==-2||i<=repeats;i++)
			playFJmidifile(name);
		raise(SIGKILL);
                }
}

const	char *forked_player::copyright(void)
{
	return "Internal cheapass forked midi player";
}


