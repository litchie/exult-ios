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
#include "../Audio.h"
extern	Audio	*audio;

#include "Configuration.h"
extern	Configuration	*config;



// #undef HAVE_TIMIDITY_BIN	// Damn. Can't do this while SDL has the audio device.
// New strategy - Tell timidity to output to stdout. Capture that via a pipe, and
// introduce it back up to the mixing layer.
#if HAVE_TIMIDITY_BIN
#include "Timidity_binary.h"

static  void    playTBmidifile(const char *name)
{
	const char	*args[]= {
			"timidity",
			"-Oru8S",	// Raw. Unsigned. 8-bit. Stereo
			"-id",
			"-T 175",	// Tempo. Faster than normal to make it
					// sound right
			"-o-",
			name,
			0 };
        // execlp("timidity","-Or","-id","-o-",name,0);
	execvp("timidity",args);
	exit(0);	// Just in case
}

template<class T>
T max(T x,T y)
	{
	return (x>y)?x:y;
	}


static	int	s_outfd,s_infd;

static void	run_server(int in_fd,int out_fd,const char *midi_file_name)
{
	close(0);	// Destroy stdin
	close(1);	// Destroy stdout
	
	// Tie pipes to stdio
	if(dup2(in_fd,0)==-1)
		perror("dup2 stdin");
	if(dup2(out_fd,1)==-1)
		perror("dup2 stdout");

	playTBmidifile(midi_file_name);
	// Shouldn't get here
	perror("execlp");
}

static	pid_t	sub_process(const char *midi_file_name)
{
	int	pipe1[2],pipe2[2];
	pid_t	childpid;

	if(s_infd!=-1)
		{
		close(s_infd);
		s_infd=-1;
		}
	if(s_outfd!=-1)
		{
		close(s_outfd);
		s_outfd=-1;
		}
	
	pipe(pipe1);
	pipe(pipe2);

	if((childpid=fork())==0)
		{
		// Child
		close(pipe1[1]);
		close(pipe2[0]);
		
		run_server(pipe1[0],pipe2[1],midi_file_name);
		exit(0);
		}
	close(pipe1[0]);
	close(pipe2[1]);
	s_outfd=pipe1[1];
	s_infd=pipe2[0];
	return childpid;
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
		{
		close(s_infd);
		close(s_outfd);
		s_infd=s_outfd=-1;
		audio->terminate_external_signal();
		kill(forked_job,SIGKILL);
		}
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
#if DEBUG
	cerr << "Starting to play " << name << endl;
#endif
	forked_job=sub_process(name);
	audio->set_external_signal(s_infd);
}

const	char *Timidity_binary::copyright(void)
{
	return "Internal cheapass forked timidity synthesiser";
}

#endif



