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


#include "Midi.h"

#include <unistd.h>
#include <csignal>
#include "../../fnames.h"

#include "Configuration.h"
extern	Configuration	config;



#undef HAVE_TIMIDITY_BIN	// Damn. Can't do this while SDL has the audio device.
// New strategy - Tell timidity to output to stdout. Capture that via a pipe, and
// introduce it back up to the mixing layer.
#if HAVE_TIMIDITY_BIN

static  void    playTBmidifile(const char *name)
{
        execlp("timidity","-Oe","-idvvv",name,0);
}

Timidity_binary::Timidity_binary() :forked_job(-1)
	{
		// Figure out if the binary is where we expect it to be.
		
	}

Timidity_binary::~Timidity_binary()
	{
	// Stop any current player
	stop_track();
	}

void	Timidity_binary::stop_track(void)
	{
	if(forked_job!=-1)
		kill(forked_job,SIGKILL);
	forked_job=-1;
		
		
	}
bool	Timidity_binary::is_playing(void)
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

void	Timidity_binary::start_track(const char *name,int repeats)
{
#if DEBUG
	cerr << "Starting midi sequence with Timidity_binary" << endl;
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
#if DEBUG
	cerr << "Starting to play " << name << endl;
#endif
                playTBmidifile(name);
                raise(SIGKILL);
                }
}

const	char *Timidity_binary::copyright(void)
{
	return "Internal cheapass forked timidity synthesiser";
}

#endif



