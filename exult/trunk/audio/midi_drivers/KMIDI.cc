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

#include "KMIDI.h"

#ifndef ALPHA_LINUX_CXX
#  include <unistd.h>
#  include <csignal>
#endif
#include "fnames.h"

#include "Configuration.h"
extern	Configuration	*config;


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

	config->value("config/audio/midi/kmidi/device",devnum,-2);
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
			config->set("config/audio/midi/kmidi/device",devnum,true);
			throw 0;
			}
		}
	kMidSetDevice(devnum);
	if(changed)
		{
		config->set("config/audio/midi/kmidi/device",devnum,true);
		}
}

KMIDI::~KMIDI()
{}


void	KMIDI::start_track(XMIDIEventList *event_list,bool repeat)
{
	const char *name = MIDITMPFILE;
	event_list->Write(name);

	if(is_playing())
		stop_track();
	repeat_=repeat;
#if DEBUG
	cerr << "Starting midi sequence with KMIDI: " << name << endl;
#endif

	KMidSimpleAPI::kMidLoad(name);

	// Something (probably SDL) traps SIGTERM which makes getting
	// rid of the repeating track a problem. This fixes it:
	signal(SIGTERM,SIG_DFL);

	KMidSimpleAPI::kMidPlay((repeat)?255:0);
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

#endif // XWIN
