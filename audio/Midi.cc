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



static  void    playmidifile(const char *name)
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


void	forked_player::start_track(const char *name,int repeats)
{
#if DEBUG
	cerr << "Starting midi sequence with forked_player" << endl;
#endif
        if(forked_job!=-1)
                {
		stop_track();
                }
        forked_job=fork();
        if(!forked_job)
                {
                playmidifile(name);
                raise(SIGKILL);
                }
}

const	char *forked_player::copyright(void)
{
	return "Internal cheapass forked midi player";
}


#if HAVE_LIBKMIDI

KMIDI::KMIDI()
{
	if(KMidSimpleAPI::kMidInit())
		{
		throw 0;
		}

	// This is probably not right for anyone but me
	kMidSetDevice(1);
}

KMIDI::~KMIDI()
{}


void	KMIDI::start_track(const char * name,int repeats)
{
	if(is_playing())
		stop_track();
#if DEBUG
	cerr << "Starting midi sequence with KMIDI: " << name << endl;
#endif

	KMidSimpleAPI::kMidLoad(name);
	KMidSimpleAPI::kMidPlay(repeats);
}

void	KMIDI::stop_track(void)
{
	KMidSimpleAPI::kMidStop();
}

bool	KMIDI::is_playing(void)
{
	return KMidSimpleAPI::kMidIsPlaying();
}

const	char *KMIDI::copyright(void)
{
	return KMidSimpleAPI::kMidCopyright();
}
	
#endif // HAVE_LIBKMIDI

void    MyMidiPlayer::start_track(int num,int repeats)
{
#if DEBUG
        cout << "Audio subsystem request: Music track # " << num << endl;
#endif
        uint32  length;
        char    *music=midi_tracks.read_object(num,length);
        if(!music)
                return;
        FILE    *fp;
        unlink("/tmp/u7midi");
        fp=fopen("/tmp/u7midi","wb");
        if(!fp)
                {
                delete [] music;
                return;
                }
        fwrite(music,length,1,fp);
        fclose(fp);
	if(!midi_device)
		return;
	midi_device->start_track("/tmp/u7midi",repeats);
}

void	MyMidiPlayer::start_music(int num,int repeats)
{
	if(!midi_device)
		return;
	if(current_track==num&&midi_device->is_playing())
		return;	// Already playing it
	current_track=num;
	start_track(num);
}


MyMidiPlayer::MyMidiPlayer()	: current_track(-1)
{
	// chdir("/home/projects/dancer/exult/u7");	// Only if you're me. &&&& Take this out sometime
	bool	no_device=true;
	midi_tracks=AccessFlexFile("static/adlibmus.dat");
#if DEBUG
	cerr << "Read in " << midi_tracks.object_list.size() << " tracks" << endl;
#endif

	if(no_device)
		{
		try {
#if HAVE_LIBKMIDI
		midi_device=new KMIDI();
#else
		throw 0;
#endif
		no_device=false;
		cerr << midi_device->copyright() << endl;
		} catch(...)
			{
			no_device=true;
			}
		}
	if(no_device)
		{
		try {
			midi_device=new forked_player();
			no_device=false;
			cerr << midi_device->copyright() << endl;
			} catch(...)
			{
			no_device=true;
			}
		}
	
	if(no_device)
		{
		midi_device=0;
		cerr << "Unable to create a music device. No music will be played" << endl;
		}
}

MyMidiPlayer::~MyMidiPlayer()
{
	if(midi_device&&midi_device->is_playing())
		midi_device->stop_track();
}

