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

#ifndef _MIXER_H_
#define _MIXER_H_

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#ifdef HAVE_CONFIG_H
#include "../autoconfig.h"
#endif

#include <list>
#include "SDL_mapping.h"
#include <SDL_audio.h>
#include "pcb.h"
#include "exult_types.h"

//---- Mixer -----------------------------------------------------------

class Mixer 
{
private:
	typedef std::list<ProducerConsumerBuf *>	pcb_list;
	Mixer(const Mixer &m);	// Cannot call me
	pcb_list	audio_streams;
public:
	Mixer();
	Mixer(uint32, uint32, uint8);
	Mixer(size_t ringsize,size_t bufferlength);
	~Mixer();

	size_t	buffer_length;
	uint8	silence;
	SDL_mutex	*stream_mutex;
	void	stream_lock(void) { SDL_mutexP(stream_mutex); };
	void	stream_unlock(void) { SDL_mutexV(stream_mutex); };
	void	cancel_raw(void);
	void fill_audio_func(void *, uint8 *, int);
	void play(uint8 *, uint32);
	ProducerConsumerBuf *Create_Audio_Stream(void);
	void	Destroy_Audio_Stream(uint32 id);
	void	cancel_streams(void);
	bool	is_playing(uint32 id);

	// void	set_auxilliary_audio(int);
	// int	auxilliary_audio;
};


#endif
