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
#include "../fnames.h"

#include "Configuration.h"
extern	Configuration	config;

static  void    playTBmidifile(const char *name)
{
        execlp("timidity","-Oe","-idvvv",name,0);
}


static  void    playFJmidifile(const char *name)
{
        execlp("playmidi","-v","-v","-e",name,0);
}

#undef HAVE_TIMIDITY_BIN	// Damn. Can't do this while SDL has the audio device.
// New strategy - Tell timidity to output to stdout. Capture that via a pipe, and
// introduce it back up to the mixing layer.
#if HAVE_TIMIDITY_BIN
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
#if DEBUG
	cerr << "Stopping any running track" << endl;
#endif
		stop_track();
                }
        forked_job=fork();
        if(!forked_job)
                {
                playFJmidifile(name);
                raise(SIGKILL);
                }
}

const	char *forked_player::copyright(void)
{
	return "Internal cheapass forked midi player";
}


#if HAVE_LIBKMIDI

int	kmidi_device_selection(void)
{
	int	num_devices=kMidDevices();

	if(!num_devices)
		return -1;
	cout << "Device\tType\tIdentity" << endl;
	cout << "-1\tnone\tdisable kmidi" << endl;
	for(int i=0;i<num_devices;i++)
		{
		cout << i <<"\t"<< kMidType(i) << "\t" << kMidName(i) << endl;
		}
	string	s;
	cout << endl << "Select a device for kmidi to use.." << endl;
	cin >> s;
	return atoi(s.c_str());
}

KMIDI::KMIDI()
{
	cerr << "libkmid initialisation..." << endl;
	if(KMidSimpleAPI::kMidInit())
		{
		cerr << "failed. Falling back..." << endl;
		throw 0;
		}

	// This is probably not right for anyone but me

	int	devnum;
	bool	changed=false;

	config.value("config/audio/midi/kmidi/device",devnum,-2);
	if(devnum==-1)
		{
		// kmidi is disabled
		throw 0;
		}
	if(devnum==-2)
		{
		devnum=kmidi_device_selection();
		changed=true;
		if(devnum==-1)
			{
			// User disabled kmidi
			devnum=-2;
			config.set("config/audio/midi/kmidi/device",devnum,true);
			throw 0;
			}
		}
	kMidSetDevice(devnum);
	if(changed)
		{
		config.set("config/audio/midi/kmidi/device",devnum,true);
		}
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


MyMidiPlayer::MyMidiPlayer()	: current_track(-1),midi_device(0)
{
	bool	no_device=true;
	midi_tracks=AccessFlexFile(ADLIBMUS);
	instrument_patches=AccessTableFile(XMIDI_MT);
#if DEBUG
	cerr << "Read in " << midi_tracks.object_list.size() << " tracks" << endl;
#endif
	string	s;
	config.value("config/audio/midi/enabled",s,"---");
	if(s=="---")
		{
		cout << "Config does not specify MIDI. Assuming yes" << endl;
		s="yes";
		}
	if(s=="no")
		{
		cout << "Config says no midi. MIDI disabled";
		no_device=false;
		}
	config.set("config/audio/midi/enabled",s,true);

	if(no_device)
		{
		try {
#if HAVE_TIMIDITY_BIN
		midi_device=new Timidity_binary();
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

