/*
Copyright (C) 2005 The Pentagram team
Copyright (C) 2010-2013 The Exult Team

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
#include "pent_include.h"
#include "AudioMixer.h"
#include "Configuration.h"

#include "AudioChannel.h"

#include "MidiDriver.h"

#include <SDL.h>

#include <iostream>

#include "Midi.h"

using namespace Pentagram;

AudioMixer *AudioMixer::the_audio_mixer = 0;

AudioMixer::AudioMixer(int sample_rate_, bool stereo_, int num_channels_) : 
audio_ok(false), 
sample_rate(sample_rate_), stereo(stereo_),
midi(0), midi_volume(255),
num_channels(num_channels_), channels(0), id_counter(0)
{
	the_audio_mixer = this;

	std::cout << "Creating AudioMixer..." << std::endl;

	SDL_AudioSpec desired, obtained;

	desired.format = AUDIO_S16SYS;
	desired.freq = sample_rate_;
	desired.channels = stereo_?2:1;
	desired.callback = sdlAudioCallback;
	desired.userdata = reinterpret_cast<void*>(this);

	// Set update rate to 20 Hz, or there abouts. This should be more then adequate for everyone
	desired.samples=1;
	while(desired.samples<=desired.freq/30) desired.samples<<=1;

	// Open SDL Audio (even though we may not need it)
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	int ret = SDL_OpenAudio(&desired, &obtained);
	audio_ok = (ret == 0);

	if (audio_ok) {
		pout << "Audio opened using format: " << obtained.freq << " Hz "
		     << static_cast<int>(obtained.channels) << " Channels" <<  std::endl;


		Lock();
		{
			sample_rate = obtained.freq;
			stereo = obtained.channels == 2;

			channels = new AudioChannel*[num_channels];
			for (int i=0;i<num_channels;i++)
				channels[i] = new AudioChannel(sample_rate,stereo);

		}
		Unlock();

		// GO GO GO!
		SDL_PauseAudio(0);
	}
}

AudioMixer::~AudioMixer(void)
{
	std::cout << "Destroying AudioMixer..." << std::endl;

	closeMidiOutput();

	SDL_CloseAudio();

	the_audio_mixer = 0;

	if (channels) for (int i=0;i<num_channels;i++) delete channels[i];
	delete [] channels;
}

void AudioMixer::Lock()
{
	SDL_LockAudio();
}

void AudioMixer::Unlock()
{
	SDL_UnlockAudio();
}

void AudioMixer::reset()
{
	if (!audio_ok) return;

	std::cout << "Resetting AudioMixer..." << std::endl;

	midi->stop_music();

	Lock();
	{
		if (channels) for (int i=0;i<num_channels;i++) channels[i]->stop();
	}
	Unlock();
}

sint32 AudioMixer::playSample(AudioSample *sample, int loop, int priority, bool paused, uint32 pitch_shift_, int lvol, int rvol)
{
	if (!audio_ok || !channels) return -1;

	int lowest = -1;
	int lowprior = 65536;

	Lock();
	{

		int i;
		for (i=0;i<num_channels;i++)
		{
			if (!channels[i]->isPlaying()) {
				lowest = i;
				break;
			}
			else if (channels[i]->getPriority() < priority) {
				lowprior = channels[i]->getPriority();
				lowest = i;
			}
		}

		if (i != num_channels || lowprior < priority)
		{
			if (++id_counter < 0) id_counter = 0;
			channels[lowest]->playSample(sample,loop,priority,paused,pitch_shift_,lvol,rvol,id_counter);
		}
		else 
			lowest = -1;

	}
	Unlock();

	return lowest!=-1?id_counter:-1;
}

bool AudioMixer::isPlaying(sint32 instance_id)
{
	if (instance_id < 0 || !channels || !audio_ok) return false;

	bool playing = false;
	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				playing = channels[chan]->isPlaying();
				break;
			}			
		}
	}
	Unlock();

	return playing;
}

bool AudioMixer::isPlaying(AudioSample *sample)
{
	if (!sample || !channels || !audio_ok) return false;

	bool playing = false;
	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getSample() == sample)
			{
				playing = true;
				break;
			}
		}
	}
	Unlock();

	return playing;
}

void AudioMixer::stopSample(sint32 instance_id)
{
	if (instance_id < 0 || !channels || !audio_ok) return;

	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				channels[chan]->stop();
				break;
			}
		}
	}
	Unlock();
}

void AudioMixer::stopSample(AudioSample *sample)
{
	if (!sample || !channels || !audio_ok) return;

	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getSample() == sample)
				channels[chan]->stop();
		}
	}
	Unlock();
}

void AudioMixer::setPaused(sint32 instance_id, bool paused)
{
	if (instance_id < 0 || !channels || !audio_ok) return;

	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				channels[chan]->setPaused(paused);
				break;
			}
		}
	}
	Unlock();
}

bool AudioMixer::isPaused(sint32 instance_id)
{
	if (instance_id < 0 || !channels || !audio_ok) return false;

	bool ret = false;

	Lock();
	{	
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				ret = channels[chan]->isPaused();
				break;
			}
		}

	}
	Unlock();

	return ret;
}

void AudioMixer::setPausedAll(bool paused)
{
	if (!channels || !audio_ok) return;

	Lock();
	{

		for (int chan = 0; chan < num_channels; chan++)
			channels[chan]->setPaused(paused);

	}
	Unlock();
}

void AudioMixer::setVolume(sint32 instance_id, int lvol, int rvol)
{
	if (instance_id < 0 || !channels || !audio_ok) return;

	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				channels[chan]->setVolume(lvol,rvol);
				break;
			}
		}
	}
	Unlock();
}

void AudioMixer::getVolume(sint32 instance_id, int &lvol, int &rvol)
{
	if (instance_id < 0 || !channels || !audio_ok) return;

	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				channels[chan]->getVolume(lvol,rvol);
				break;
			}
		}
	}
	Unlock();
}


bool AudioMixer::set2DPosition(sint32 instance_id, int distance, int angle)
{
	if (instance_id < 0 || !channels || !audio_ok) return false;
	bool playing = false;

	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				channels[chan]->set2DPosition(distance,angle);
				playing = channels[chan]->isPlaying();
				break;
			}
		}
	}
	Unlock();
	return playing;
}

void AudioMixer::get2DPosition(sint32 instance_id, int &distance, int &angle)
{
	if (instance_id < 0 || !channels || !audio_ok) return;

	Lock();
	{
		for (int chan = 0; chan < num_channels; chan++)
		{
			if (channels[chan]->getInstanceId() == instance_id)
			{
				channels[chan]->get2DPosition(distance,angle);
				break;
			}
		}
	}
	Unlock();
}


void AudioMixer::sdlAudioCallback(void *userdata, Uint8 *stream, int len)
{
	AudioMixer *mixer = reinterpret_cast<AudioMixer *>(userdata);

	mixer->MixAudio(reinterpret_cast<sint16*>(stream), len);
}

void AudioMixer::MixAudio(sint16 *stream, uint32 bytes)
{
	if (!audio_ok) return;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	std::memset(stream,0,bytes);
#endif
	if (midi) midi->produceSamples(stream, bytes);
#if !SDL_VERSION_ATLEAST(2, 0, 0)
	else std::memset(stream,0,bytes);
#endif
	if (channels) for (int i=0;i<num_channels;i++)
		if (channels[i]->isPlaying()) channels[i]->resampleAndMix(stream,bytes);
}

void AudioMixer::openMidiOutput()
{
	if (midi) return;
	if (!audio_ok) return;

	MyMidiPlayer * new_driver = new MyMidiPlayer();

	Lock();
	{
		midi = new_driver;
	}
	Unlock();
	midi->load_timbres();
	//midi_driver->setGlobalVolume(midi_volume);
}

void AudioMixer::closeMidiOutput()
{
	if (!midi) return;
	std::cout << "Destroying MidiDriver..." << std::endl;

	midi->stop_music();
	midi->destroyMidiDriver();

	Lock();
	{
		delete midi;
		midi = 0;
	}
	Unlock();
}

