//-*-Mode: C++;-*-
#ifndef _Mixer_h_
#define _Mixer_h_

#if __GNUG__ >= 2
#  pragma interface
#endif

#include <list>
#include "SDL_mapping.h"
#include <SDL_audio.h>
#include "Flex.h"

//---- Mixer -----------------------------------------------------------

class Mixer 
{
public:
    Mixer();
	Mixer(unsigned int, unsigned int, unsigned char);
    Mixer(size_t ringsize,size_t bufferlength);
    ~Mixer();

	struct	MixBuffer
		{
		Uint8 *buffer;
		Uint8 num_samples;
		size_t	length;
		MixBuffer(size_t size,Uint8 silence) : buffer(new Uint8[size]),num_samples(0),length(0) { memset(buffer,silence,size); };
		};
	size_t	buffer_length;
	list<MixBuffer>	buffers;
	size_t ring_size;
	void	advance(void);
	Uint8	silence;
	void fill_audio_func(void *, Uint8 *, int);
	void play(Uint8 *, unsigned int);
};


#endif
