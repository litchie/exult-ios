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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#ifdef XWIN

#include "Midi.h"

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <csignal>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#endif

#include "fnames.h"
#include "Audio.h"

using std::cerr;
using std::endl;


#include "Configuration.h"
extern	Configuration	*config;


const	Uint32	Timidity_binary_magic=0x345300;
const	Uint32	Timidity_binary_magic_sfx=0x345301;

// #undef HAVE_TIMIDITY_BIN	// Damn. Can't do this while SDL has the audio device.
// New strategy - Tell timidity to output to stdout. Capture that via a pipe, and
// introduce it back up to the mixing layer.

#if HAVE_TIMIDITY_BIN
#include "Timidity_binary.h"

template<class T>
T max(T x,T y)
	{
	return (x>y)?x:y;
	}



static	void	*midi_process(void *p)
{
	Timidity_binary *ptr=static_cast<Timidity_binary *>(p);
	ptr->player();
	return 0;
}

static	void	*sfx_process(void *p)
{
	Timidity_binary *ptr=static_cast<Timidity_binary *>(p);
	ptr->sfxplayer();
	return 0;
}

static	void	copy_to_fh(std::string name,int number)
{
	char	buf[8192];

	int	fh=::open(name.c_str(),O_RDONLY);
	if(fh==-1)
		{
		perror("open");
		return;
		}
	while(1)
		{
		ssize_t r=read(fh, buf, sizeof(buf));
		if(r<=0)
			break;
		write(number,buf,r);
		}
	close(number);
	close(fh);
}

static	void copy_to_name(std::string name1,std::string name2)
{
	int	fh=::open(name2.c_str(),O_WRONLY);
	if(fh==-1)
		{
		perror("open");
		return;
		}
	copy_to_fh(name1,fh);
}

int	timidity_play(std::string filename, bool repeat, std::string &newfilename, pid_t &pid)
{
#if HAVE_MKSTEMP
        char newfn[128];
        std::strcpy(newfn,"/tmp/u7_midi_fileXXXXXX");
        int     dfh=mkstemp(newfn);
        if(dfh==-1)
                {
                perror("mkstemp");
                return -1;
                }
        copy_to_fh(filename,dfh);
	unlink(filename.c_str());
	newfilename=newfn;
#else
	char newfn[L_tmpnam];
	newfilename=tmpnam(newfn);
	copy_to_name(filename,newfilename);
	unlink(filename.c_str());
#endif
	char nbuf[32];
	snprintf(nbuf,32,"%lu", Audio::get_ptr()->actual.freq);
	std::string repstr = "-id";
	if(repeat)
		repstr += "l";
	int pdes[2];
	pipe(pdes);
	switch(pid = fork())
	{
		case -1:
		close(pdes[0]);
		close(pdes[1]);
		break;
		
		case 0:
		dup2(pdes[1], STDOUT_FILENO);
		close(pdes[1]);
		close(pdes[0]);
		close(STDERR_FILENO);
		execlp("timidity","timidity","-Or1slS",repstr.c_str(),"-s",nbuf,"-o-",newfilename.c_str(),0); 
		_exit(127);
		
		default:
			{
			close(pdes[1]);
			return pdes[0];
			}
	}
	return -1;
}

void	Timidity_binary::player(void)
{
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic);
	ProducerConsumerBuf *audiostream=Audio::get_ptr()->Create_Audio_Stream(
					Timidity_binary_magic);
	if (!audiostream)
		{
		cerr << "Timidity_binary(): All audio streams in use" << endl;
		return;
		}
	char buf[4096];
	std::string newfilename;
	pid_t timidity_pid;
	int fd = timidity_play(filename, do_repeat, newfilename, timidity_pid);
	for (;;)
		{
		size_t	x=read(fd, buf, sizeof(buf));
		if(stop_music_flag || x<=0 || !audiostream->is_consuming())
			break;
		audiostream->produce(buf,x);
		}
	close(fd);
	kill(timidity_pid, SIGKILL);
	unlink(newfilename.c_str());
	audiostream->end_production();
	audiostream=0;
}

void	Timidity_binary::sfxplayer(void)
{
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic_sfx);
	ProducerConsumerBuf *audiostream=Audio::get_ptr()->Create_Audio_Stream(
						Timidity_binary_magic_sfx);
	char buf[4096];
	std::string newfilename;
	pid_t timidity_pid;
	int fd = timidity_play(sfxname, false, newfilename, timidity_pid);
	for (;;)
		{
		size_t	x=read(fd, buf, sizeof(buf));
		if(x<=0 || !audiostream->is_consuming())
			break;
		audiostream->produce(buf,x);
		}
	close(fd);
	kill(timidity_pid, SIGKILL);
	unlink(newfilename.c_str());
	audiostream->end_production();
	audiostream=0;
}

Timidity_binary::Timidity_binary() : midi_thread(0), sfx_thread(0), filename(),
		stop_music_flag(false)
	{
		// Figure out if the binary is where we expect it to be.
		
	}

Timidity_binary::~Timidity_binary()
	{
	// Stop any current player
	stop_track();
	stop_sfx();
	}

void	Timidity_binary::stop_track(void)
	{
	stop_music_flag = true;
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic);
	if (midi_thread)
		pthread_join(midi_thread, 0);
	midi_thread = 0;
	stop_music_flag = false;
	}

void	Timidity_binary::stop_sfx(void)
	{
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic_sfx);
	pthread_join(sfx_thread, 0);
	}

bool	Timidity_binary::is_playing(void)
{
	if(Audio::get_ptr()->is_playing(Timidity_binary_magic))
		return true;
	return false;
}

void	Timidity_binary::start_track(XMIDIEventList *event_list,bool repeat)
{
	const char *name = MIDITMPFILE;
	event_list->Write(name);

#if DEBUG
	cerr << "Starting midi sequence with Timidity_binary" << endl;
#endif
                {
#if DEBUG
	cerr << "Stopping any running track" << endl;
#endif
		stop_track();
                }
#if DEBUG
	cerr << "Starting to play " << name << endl;
#endif
	filename=name;
	do_repeat=repeat;
	cerr << "Creating new player thread" << endl;
	if(pthread_create(&midi_thread, 0, midi_process, this)==-1)
		perror("pthread_create");
}

/*
virtual void	start_track( *, bool repeat);
virtual void	start_sfx(XMIDIEventList *);
*/

void	Timidity_binary::start_sfx(XMIDIEventList *event_list)
{
	const char *name = MIDISFXFILE;
	event_list->Write(name);

#if DEBUG
	cerr << "Starting sound effect with Timidity_binary" << endl;
#endif
                {
#if DEBUG
	cerr << "Stopping any running sfx" << endl;
#endif
		stop_sfx();
                }
#if DEBUG
	cerr << "Starting to play " << name << endl;
#endif
	sfxname=name;
	if(pthread_create(&sfx_thread, 0, sfx_process, this)==-1)
		perror("pthread_create");
}

const	char *Timidity_binary::copyright(void)
{
	return "Internal cheapass forked timidity synthesiser";
}

#endif // HAVE_TIMIDITY_BIN

#endif // XWIN
