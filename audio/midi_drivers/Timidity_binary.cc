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


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#ifdef XWIN

#include "Midi.h"

#include <unistd.h>
#include <csignal>
#include "fnames.h"
#include "Audio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



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



static	int	sub_process(void *p)
{
	Timidity_binary *ptr=static_cast<Timidity_binary *>(p);
	ptr->player();
	return 0;
}

static	int	sfx_process(void *p)
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

void	Timidity_binary::player(void)
{
#if DEBUG
	cout << "Timidity_binary::player() starting up" << endl;
#endif
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic);
	ProducerConsumerBuf *audiostream=Audio::get_ptr()->Create_Audio_Stream();
#if HAVE_MKSTEMP
        char newfn[128];
        strcpy(newfn,"/tmp/u7_midi_fileXXXXXX");
        int     dfh=mkstemp(newfn);
        if(dfh==-1)
                {
                perror("mkstemp");
                return; // Abort
                }
        copy_to_fh(filename,dfh);
	unlink(filename.c_str());
	string	newfilename=newfn;
#else
	char newfn[L_tmpnam];
	string	newfilename=tmpnam(newfn);
	copy_to_name(filename,newfilename);
	unlink(filename.c_str());
#endif

	audiostream->id=Timidity_binary_magic;
	char nbuf[32];
	sprintf(nbuf,"%lu",Audio::get_ptr()->actual.freq);
	string	s="timidity -Oru8S -id";
	if(do_repeat)
		s+="l";
	s+=" -s ";
	s+=nbuf;
	s+=" -o- ";
	s+=newfilename;
#ifndef DEBUG
	s+=" 2>/dev/null";	// Swallow extraneous output if !debug
#endif
	FILE *data=popen(s.c_str(),"r");
	if(!data)
		return;
	char	buf[1024];
	while(!feof(data))
		{
		if(!audiostream->consuming)
			{
			break;
			}
		size_t	x=fread(buf,1,sizeof(buf),data);
		audiostream->produce(buf,x);
		}
	pclose(data);
	unlink(newfilename.c_str());
	audiostream->end_production();
	audiostream=0;
	cerr << "Timidity cleaned up" << endl;
}

void	Timidity_binary::sfxplayer(void)
{
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic_sfx);
	ProducerConsumerBuf *audiostream=Audio::get_ptr()->Create_Audio_Stream();
#if HAVE_MKSTEMP
	char newfn[128];
	strcpy(newfn,"/tmp/u7_midi_fileXXXXXX");
	int	dfh=mkstemp(newfn);
	if(dfh==-1)
		{
		perror("mkstemp");
		return;	// Abort
		}
	copy_to_fh(sfxname,dfh);
	unlink(sfxname.c_str());
	string newfilename=newfn;
#else
	char newfn[L_tmpnam];
	string	newfilename=tmpnam(newfn);
	copy_to_name(sfxname,newfilename);
	unlink(sfxname.c_str());
#endif

	audiostream->id=Timidity_binary_magic_sfx;
	char nbuf[32];
	sprintf(nbuf,"%lu",Audio::get_ptr()->actual.freq);
	string	s="timidity -Oru8S -id -s ";
	s+=nbuf;
	s+=" -o- ";
	s+=newfilename;
#ifndef DEBUG
	s+=" 2>/dev/null";	// Swallow extraneous output if !debug
#endif
	FILE *data=popen(s.c_str(),"r");
	if(!data)
		return;
	char	buf[1024];
	while(!feof(data))
		{
		if(!audiostream->consuming)
			{
			break;
			}
		size_t	x=fread(buf,1,sizeof(buf),data);
		audiostream->produce(buf,x);
		}
	pclose(data);
	unlink(newfilename.c_str());
	audiostream->end_production();
	audiostream=0;
}

Timidity_binary::Timidity_binary() : my_thread(0),filename()
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
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic);
	}

void	Timidity_binary::stop_sfx(void)
	{
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic_sfx);
	}

bool	Timidity_binary::is_playing(void)
{
	if(Audio::get_ptr()->is_playing(Timidity_binary_magic))
		return true;
	return false;
}

void	Timidity_binary::start_track(const char *name,bool repeat)
{
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
	my_thread=SDL_CreateThread(sub_process,this);
}


void	Timidity_binary::start_sfx(const char *name)
{
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
	sfx_thread=SDL_CreateThread(sfx_process,this);
}

const	char *Timidity_binary::copyright(void)
{
	return "Internal cheapass forked timidity synthesiser";
}

#endif // HAVE_TIMIDITY_BIN

#endif // XWIN
