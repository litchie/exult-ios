//-*-Mode: C++;-*-
#ifndef _Audio_h_
#define _Audio_h_

#if __GNUG__ >= 2
#  pragma interface
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
    Audio(int _samplerate,int _channels);
    ~Audio();
	Mixer	*mixer;

	void	play(Uint8 *sound_data,Uint32 len,bool);
	void	playfile(const char *,bool);
	void	mix_audio(void);
	void	mix(Uint8 *sound_data,Uint32 len);
	void	mixfile(const char *fname);
	bool	playing(void);
	void	clear(Uint8 *,int);
	void	start_music(int num);
	void	start_speech(int num,bool wait=false);
	void	start_speech(Flex *f,int num,bool wait=false);

	static	const	unsigned int	ringsize=3000;
//	static	const	int	samplerate=11025;
//	static	const	int	persecond=2;
//	static	const	int	buffering_unit=1024;

private:
	Uint8 * convert_VOC(Uint8 *);
	SDL_AudioSpec wanted;
	MidiPlayer	midi;

	Flex	speech_tracks;
	void build_speech_vector(void);

};

extern	Audio audio;

#endif
