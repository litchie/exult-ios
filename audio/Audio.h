//-*-Mode: C++;-*-
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

#ifndef _Audio_h_
#define _Audio_h_

#if __GNUG__ >= 2
#  pragma interface
#endif

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <vector>
#include "SDL_mapping.h"
#include <SDL_audio.h>
#include "Flex.h"
#include "Mixer.h"
#include "Midi.h"

//---- Audio -----------------------------------------------------------

class Audio 
{
public:
    Audio();
    void	Init(void);
    void	Init(int _samplerate,int _channels);
    ~Audio();
	Mixer	*mixer;

	void	play(Uint8 *sound_data,Uint32 len,bool);
	void	playfile(const char *,bool);
	void	mix_audio(void);
	void	mix(Uint8 *sound_data,Uint32 len);
	void	mixfile(const char *fname);
	bool	playing(void);
	void	clear(Uint8 *,int);
	void	start_music(int num,int repetition);
	void	start_speech(int num,bool wait=false);
	void	start_speech(Flex *f,int num,bool wait=false);

	static	const	unsigned int	ringsize=3000;
//	static	const	int	samplerate=11025;
//	static	const	int	persecond=2;
//	static	const	int	buffering_unit=1024;

private:
	Uint8 * convert_VOC(Uint8 *,unsigned int &);
	SDL_AudioSpec wanted,actual;
	MyMidiPlayer	*midi;

	Flex	speech_tracks;
	void build_speech_vector(void);

};

extern	Audio audio;

#endif
