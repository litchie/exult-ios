//-*-Mode: C++;-*-
#ifndef _Midi_h_
#define _Midi_h_

#if __GNUG__ >= 2
#  pragma interface
#endif

#include <vector>
#include "SDL_mapping.h"
#include <SDL_audio.h>
#include "Flex.h"
#include "Mixer.h"


//---- MidiPlayer -----------------------------------------------------------

class	MidiPlayer
{
public:
	MidiPlayer();
	~MidiPlayer();
	MidiPlayer(const char *flexfile);
	void	start_music(int num);
	void	start_track(int num);

private:
	Flex midi_tracks;
	int	current_track;
	pid_t	forked_job;

};

#endif
